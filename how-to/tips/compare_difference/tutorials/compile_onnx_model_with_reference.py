#
#  Original code (C) Copyright EdgeCortix, Inc. 2022
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2022
#
#   *1 DRP-AI TVM is powered by EdgeCortix MERA Compiler Framework
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
import onnx
import tvm
import sys
import numpy as np

from tvm import relay, runtime
from tvm.relay.mera import drp
from tvm.contrib import graph_executor
from google.protobuf.json_format import MessageToDict
from optparse import OptionParser

from arg_parser import get_args

if __name__ == "__main__":
    """
    This is sample script of DRP-AI TVM[*1] compiler stack.
    Please set following argements to run.
     -o : Output directory name
     -i : Input node name of AI model
     -s : Input shape of AI model
     [Options]
     -d : DRP-AI Trahslator path
     -t : toolchain path (Cross compiler/SDK)
     -c : Concat disable mode. Safety compile mode.
    """
    np.random.seed(1234567)
    # 1. Get argument data
    (opts, model_file) = get_args()
    output_dir=opts["output_dir"]
    os.makedirs(output_dir, exist_ok=True)

    # 2. Load onnx model, set input shape and make dummy input data.
    # 2.1 Load onnx model
    onnx_model = onnx.load_model(model_file)

    model_initializers = set()
    model_inputs = set()
    for init in onnx_model.graph.initializer:
        model_initializers.add(init.name)
    for inp in onnx_model.graph.input:
        model_inputs.add(inp.name)
    model_vars = []
    for inp in model_inputs:
        if inp not in model_initializers:
            model_vars.append(inp)

    # 2.1 Make input data for validation
    shape_dict = {}
    input_data = []
    inp_idx = 0
    for inp in onnx_model.graph.input:
        input_name = inp.name
        if input_name not in model_vars:
            continue

        input_shape = opts["input_shape"]
        shape_dict = {opts["input_name"]:opts["input_shape"]}
        data = np.random.uniform(-1.0, 1.0, input_shape).astype(np.float32)

        input_data.append(data)
        data.flatten().astype(np.float32).tofile(os.path.join(output_dir, "input_" + str(inp_idx) + ".bin"))
        inp_idx += 1

    # 3. Run DRP-AI TVM[*1] compiler
    # 3.1 Run TVM Frontend
    print("-------------------------------------------------")
    print("   Run TVM frotend compiler ")
    mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)

    # 4. Compile(TVM Backend)
    # 4.1 Run tvm compiler to check relay format. (Option)
    target = tvm.target.Target("llvm", host="llvm")
    dev = tvm.cpu(0)
    with tvm.transform.PassContext(opt_level=3):
        lib = relay.build(mod, target=target, params=params)

    m = graph_executor.GraphModule(lib["default"](dev))
    for inp_idx in range(len(input_data)):
        m.set_input(inp_idx, input_data[inp_idx])
    m.run()

    # 4.2 Run TVM backend WITHOUT DRP-AI translator
    # ... generates expected data by running onnxruntime inference.
    print("-------------------------------------------------")
    print("   Run TVM backend compiler without DRP-AI Translator")
    drp_config = {
        "interpreter": True,
        "addr_map_start": 0xc0000000,
        "toolchain_dir": opts["drp_compiler_dir"],
    }

    # Target architecture is x86.
    json, params, lib_path = drp.build(mod, \
                   params, \
                   "x86", \
                   drp_config, \
                   output_dir=output_dir, \
                   disable_concat = opts["disable_concat"]
                   )
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)

    # Set input data
    for inp_idx in range(len(input_data)):
        rt_mod.set_input(inp_idx, input_data[inp_idx])

    # Run inference on PC to generete expected data.
    rt_mod.run()

    # Save output data to use as reference
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
        np.testing.assert_allclose(byoc_output, tvm_output, atol=1e-1, rtol=1e-1)
        print("Non-zero values: ", (np.count_nonzero(byoc_output) / byoc_output.size) * 100.0, "%")

    # 4.3 Run TVM backend with DRP-AI translator
    print("-------------------------------------------------")
    print("   Run TVM backend compiler with DRP-AI Translator")
    # 4.3.1 Set config for DRP-AI runtime
    drp_config_runtime = {
        "interpreter": False,
        "addr_map_start": 0xc0000000,
        "toolchain_dir": opts["drp_compiler_dir"],
        "sdk_root": opts["toolchain_dir"]
    }

    drp.build(mod, \
                   params, \
                   "arm", \
                   drp_config_runtime, \
                   output_dir=output_dir, \
                   disable_concat = opts["disable_concat"]
                   )
    print("[TVM compile finished]")
    print("   Please check {0} directory".format(output_dir))
