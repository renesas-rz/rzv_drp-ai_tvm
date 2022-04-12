#
# (C) Copyright EdgeCortix, Inc. 2022
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import os
import torch
import numpy as np
import tvm

from tvm import relay, runtime
from tvm.relay.mera import drp
from tvm.contrib import graph_executor
from collections import namedtuple
from optparse import OptionParser
from torchvision import models

network_input_shape = {
    'resnet18': (1, 3, 224, 224),
    'resnet50': (1, 3, 224, 224),
    'resnet34': (1, 3, 224, 224),
    'resnet101': (1, 3, 224, 224),
    'mobilenet_v2': (1, 3, 224, 224),
    'deeplabv3_resnet50': (1, 3, 224, 224),
    'deeplabv3_resnet101': (1, 3, 224, 224),
    'deeplabv3_mobilenet_v3_large': (1, 3, 224, 224),
    'squeezenet1_0': (1, 3, 224, 224),
    'squeezenet1_1': (1, 3, 224, 224),
    'densenet121': (1, 3, 224, 224),
    'googlenet': (1, 3, 224, 224),
    'inception_v3': (1, 3, 299, 299),
    'fcn_resnet101': (1, 3, 224, 224),
    'mnasnet0_5': (1, 3, 224, 224),
}

def get_model(network):
    if network.endswith(('.pt')):
        model = torch.jit.load(network)
    elif network == 'mnasnet0_5':
        model = models.mnasnet0_5(pretrained=True)
    else:
        model = torch.hub.load('pytorch/vision:v0.10.0', network, pretrained=True)

    class ModelWrapper(torch.nn.Module):
        def __init__(self, model):
            super().__init__()
            self.model = model
    
        def forward(self, input_x):
            data = self.model(input_x)
            if isinstance(data, dict):
                data_named_tuple = namedtuple("ModelEndpoints", sorted(data.keys()))
                data = data_named_tuple(**data)
            elif isinstance(data, list):
                data = tuple(data)
            return data

    model.eval()
    model_wrapper = ModelWrapper(model)
    model_wrapper.eval()
    return model_wrapper


def import_model(network, user_input_shape):
    model_wrapper = get_model(network)
    input_shape = None
    for key in network_input_shape.keys():
        if network in key:
            input_shape = network_input_shape[key]
    if user_input_shape is not None:
        input_shape = user_input_shape
    if not input_shape:
        print("Undefined input shape")
        exit(1)
    input_data = torch.randn(input_shape)
    with torch.no_grad():
        traced_model = torch.jit.trace(model_wrapper, input_data).eval()
        traced_model.eval()
    input_name = "input0"
    shape_list = [(input_name, input_data.shape)]
    mod, params = relay.frontend.from_pytorch(traced_model, shape_list)
    return mod, params, input_name, input_data


if __name__ == "__main__":
    torch.manual_seed(1234567)
    usage = "usage: %prog [options] model_name"
    parser = OptionParser(usage)
    parser.set_defaults(drp_compiler_dir='./drp_compiler', toolchain_dir='/opt/poky/2.4.3', user_input_shape=None, disable_concat = False)
    parser.add_option("-d", "--drp_compiler_dir", dest="drp_compiler_dir", help="DRP-AI Translator root directory", metavar="DIR")
    parser.add_option("-t", "--toolchain_dir", dest="toolchain_dir", help="Cross-compilation toolchain root directory", metavar="DIR")
    parser.add_option("-s", "--input_shape", dest="user_input_shape", help="User specified input shape")
    parser.add_option("-m", "--model", dest="display_supported_model", default=False, help="Display supported model for use",action='store_true')
    parser.add_option("-c", "--disable_concat", action="store_true", dest="disable_concat", default=False, help="Disable concat at DRP")

    (options, args) = parser.parse_args()
    opts = vars(options)
    display_supported_model = opts["display_supported_model"]
    if display_supported_model is not False:
        print("Command: python compile_pytorch_model.py -d <path-to-drp-translator-installation-dir> network_name")
        print("The supported `network_name` is as follows:")
        for key in network_input_shape.keys():
            print(key)
        exit(1)
    if len(args) != 1:
        parser.error("Expected a torchvision model name or a traced model file .pt")
    print(opts)
    model_name = str(args[0])
    print("Compiling ", model_name, ":")
    print("  Cross-compilation toolchain: ", opts["toolchain_dir"])
    print("  DRP-AI Translator: ", opts["drp_compiler_dir"])
    user_input_shape = opts["user_input_shape"]
    if user_input_shape is not None:
        user_input_shape = tuple([int(dim) for dim in user_input_shape.split(',')])
        print("  User specified input shape: ", user_input_shape)

    mod, params, input_name, input_data = import_model(model_name, user_input_shape)

    output_dir="_out"
    os.makedirs(output_dir, exist_ok=True)
    input_data.numpy().flatten().astype(np.float32).tofile(os.path.join(output_dir, "input_0.bin"))

    # normal TVM
    target = tvm.target.Target("llvm", host="llvm")
    dev = tvm.cpu(0)
    with tvm.transform.PassContext(opt_level=3):
        lib = relay.build(mod, target=target, params=params)
    
    dtype = "float32"
    m = graph_executor.GraphModule(lib["default"](dev))
    m.set_input(input_name, tvm.nd.array(input_data.numpy().astype(dtype)))
    m.run()
    
    # interpreter mode
    drp_config = {
        "interpreter": True,
        "addr_map_start": 0x5f800000,
        "toolchain_dir": opts["drp_compiler_dir"],
    }
    json, params, lib_path = drp.build(mod, params, "x86", drp_config, output_dir="_out", disable_concat = opts["disable_concat"])
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)
    rt_mod.set_input(input_name, tvm.nd.array(input_data.numpy().astype(dtype)))
    rt_mod.run()
    
    # save to use as reference
    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i).asnumpy()
        if byoc_output.dtype == "float32":
            byoc_output.flatten().astype(np.float32).tofile(
                os.path.join(output_dir, "ref_result_" + str(i) + ".bin"))
        elif byoc_output.dtype == "float16":
            byoc_output.flatten().astype(np.float16).tofile(
                os.path.join(output_dir, "ref_result_" + str(i) + ".bin"))
    
    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i).asnumpy()
        tvm_output = m.get_output(i).asnumpy()
        np.testing.assert_allclose(byoc_output, tvm_output, atol=2e-1, rtol=1e-1)
        print("Non-zero values: ", (np.count_nonzero(byoc_output) / byoc_output.size) * 100.0, "%")
    
    drp_config_runtime = {
        "interpreter": False,
        "addr_map_start": 0x5f800000,
        "toolchain_dir": opts["drp_compiler_dir"],
        "sdk_root": opts["toolchain_dir"]
    }
    json, params, lib_path = drp.build(mod, params, "arm", drp_config_runtime, output_dir="_out", disable_concat = opts["disable_concat"])
