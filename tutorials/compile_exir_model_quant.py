#
#  Original code (C) Copyright EdgeCortix, Inc. 2025
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2025
#
#   *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
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

from drpai_preprocess import * 
import os
import torch
import numpy as np

import sys
import numpy as np
import shutil
import pathlib
import cv2
from PIL import Image
import torchvision.transforms.functional as F

import mera2_ as mera2
from mera2_ import runtime, graph_executor

from google.protobuf.json_format import MessageToDict
from optparse import OptionParser

from arg_parser import get_args

PRODUCT= os.getenv("PRODUCT")
if(PRODUCT != 'V2H' and PRODUCT != 'V2N'):
    print("[Error] Environment variable (PRODUCT) is not V2H or V2N")
    print("        Please set environment variable(PRODUCT)")
    print("        to your product name.")
    print("        e.g. $export PRODUCT=V2H")
    sys.exit(-1)

def create_random_input_from_rt(rt_mod):
    shape_dict = {}
    input_data = []
    input_shape = opts["input_shape"]
    for idx, key in enumerate(rt_mod.input_names.keys()):
        input_name = key
        if idx>0:
            print("multi input is unexpected.")
    shape_dict[input_name] = input_shape
    data = np.random.uniform(0.0, 1.0, input_shape).astype(np.float32)
    input_data.append(data)
    return shape_dict, input_data

def pre_process_imagenet_pytorch(img, mean=[0.485, 0.456, 0.406], stdev=[0.229, 0.224, 0.225], dims=None, need_transpose=False):
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = Image.fromarray(img)
    img = F.resize(img, 256, Image.BILINEAR)
    img = F.center_crop(img, 224)
    img = F.to_tensor(img)
    std = stdev
    img = F.normalize(img, mean, std, inplace=False)
    if not need_transpose:
        img = img.permute(1, 2, 0) # NHWC
    img = np.asarray(img, dtype='float32')
    return img


if __name__ == "__main__":
    """
    This is sample script of DRP-AI TVM[*1] compiler stack.
    Please set following argements to run.
     -o : Output directory name
     -i : Input node name of AI model
     -s : Input shape of AI model
     [Options]
     -d : DRP-AI Trahslator path
     -c : Quantization tool path(Converter)
     -t : toolchain path (Cross compiler/SDK)
    """
    # 1. Get argument data
    (opts, model_file) = get_args()
    output_dir=opts["output_dir"]
    os.makedirs(output_dir, exist_ok=True)

    record_dir = opts["record_dir"]
    if not record_dir:
        record_dir = '.data'
        print("Calibration data record directory not specified: use .data and clear it now.")
        if os.path.exists(record_dir):
            shutil.rmtree(record_dir)
    os.makedirs(record_dir, exist_ok=True)
    print("Calibration data record directory: ", record_dir)

    print("Compiling ", model_file, ":")
    print("  Cross-compilation toolchain: ", opts["toolchain_dir"])
    print("  DRP-AI Quant Translator: ", opts["drp_compiler_dir"])
    print("  DRP-AI Quant Translator Version: ", opts["drp_compiler_version"])
    print("  DRP-AI Quant Converter: ", opts["quantization_tool"])
    print("  DRP-AI Quant Option: ", opts["quantization_option"])
    print("  Calibration data record directory: ", opts["record_dir"])
    print("  Number input random frames: ", opts["num_frame"])

    # 2. Load pytorch model and set input shape.
    
    # 2.1 Set input data information
    input_name = "input0"
    shape_list = [(input_name, opts["input_shape"])]
    input_shape = opts["input_shape"]
    input_data = np.random.uniform(0.0, 1.0, input_shape).astype(np.float32)

    mod, params = mera2.from_exir(model_file, \
                                   is_quant=True)
    # 3.2 Create calibration data(using random values.)
    drp_config = {
        "target": "Fp32DataRecorder",
        "drp_compiler_version": opts["drp_compiler_version"],
        "record_dir": record_dir,
    }

    # 3.2.2 Run the backend compiler for x86 and generate reference output.
    json, params, lib_path = mera2.drp.build(mod, params, "x86", drp_config, \
                                             output_dir=record_dir, \
                                             disable_concat=False, \
                                             cpu_data_type=opts["cpu_data_type"], \
                                            )
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)

    mean    = [0.485, 0.456, 0.406] # Normalization setting sample(Mean)
    stdev   = [0.229, 0.224, 0.225] # Normalization setting sample(Standard deviation)

    input_dir = opts["image_dir"]

    if input_dir is None:
        print("Use randomize input for calibration.")
        num_frame = int(opts["num_frame"])
        print("Number input random frames: ", opts["num_frame"])

        for i in range(num_frame):
            shape_dict, input_data = create_random_input_from_rt(rt_mod)
            for inp_idx in range(len(input_data)):
                rt_mod.set_input(inp_idx, input_data[inp_idx])
            rt_mod.run()
    else:
        input_list = list(pathlib.Path(input_dir).glob('*'))
        for i in range(len(input_list)):
            img_file_name = str(input_list[i])
            image = cv2.imread(img_file_name)
            input_data = pre_process_imagenet_pytorch(image, mean, stdev, need_transpose=True)
            input_data = np.expand_dims(input_data, 0)
            rt_mod.set_input(0, input_data)
            rt_mod.run()
            print("calib data", img_file_name)

    # 3.3.1 Set config for with x86 runtime with int8. 
    drp_config = {
        "target": "InterpreterQuant",
        "drp_compiler_version": opts["drp_compiler_version"],
        "quantization_tool": opts["quantization_tool"],
        "quantization_option": opts["quantization_option"],
        "calibration_data": record_dir
    }
    shape_dict, input_data = create_random_input_from_rt(rt_mod)

    # 3.3.2 Run the backend compiler for x86 and generate reference output of int8.
    ref_result_output_dir = os.path.join(opts["output_dir"],"interpreter_out")
    os.makedirs(ref_result_output_dir, exist_ok=True)
    host_arch = "x86"
    json, params, lib_path = mera2.drp.build(mod, params, host_arch, drp_config, \
                                             output_dir=output_dir, \
                                            disable_concat=False, \
                                            cpu_data_type=opts["cpu_data_type"], \
                                            )
    lib = runtime.load_module(lib_path)
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)
    for inp_idx in range(len(input_data)):
        rt_mod.set_input(inp_idx, input_data[inp_idx])
        # Save input data for interpreter mode
        input_data[inp_idx].flatten().tofile(
            os.path.join(ref_result_output_dir, "input_" + str(inp_idx) + ".bin"))
    rt_mod.run()

    # 3.3.3 Save output data to use it as reference output.
    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i)
        if not isinstance(byoc_output, np.ndarray):
            byoc_output = byoc_output.asnumpy()
        if byoc_output.dtype == "float32":
            byoc_output.flatten().astype(np.float32).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) + "_fp32.bin"))
        elif byoc_output.dtype == "float16":
            byoc_output.flatten().astype(np.float16).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) + "_fp16.bin"))
        elif byoc_output.dtype == "int64":
            byoc_output.flatten().astype(np.int64).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) + "_int64.bin"))
        elif byoc_output.dtype == "int32":
            byoc_output.flatten().astype(np.int32).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) + ".bin"))
        else:
            assert False, "Unsupport this data type" + byoc_output.dtype

    # DrpAi translates onnx quantizer
    # 3.4 Run TVM backend with DRP-AI translator
    print("-------------------------------------------------")
    print("   Run TVM backend compiler with DRP-AI Translator")
    # 3.4.1 Set config for DRP-AI runtime with quantized int8.
    drp_config_runtime = {
        "target": "DrpAiQuant",
        "addr_map_start": 0x00,
        "toolchain_dir": opts["drp_compiler_dir"],
        "drp_compiler_version": opts["drp_compiler_version"],
        "sdk_root": opts["toolchain_dir"],
        "quantization_tool": opts["quantization_tool"],
        "quantization_option": opts["quantization_option"],
        "calibration_data": record_dir
    }
    # 3.4.2 Run backend compiler with Quantizeer.
    json, params, lib_path = mera2.drp.build(mod, params, "arm", \
                                             drp_config_runtime, output_dir, \
                                             disable_concat=False, \
                                             cpu_data_type=opts["cpu_data_type"], \
                                             )

    print("[TVM compile finished]")
    print("   Please check {0} directory".format(opts["output_dir"]))
    print("[Start compiling PreRuntime objects....]")

    # 4. Compile pre-processing using DRP-AI Pre-processing Runtime Only for RZ/V2H
    # 4.1. Define the pre-processing data
    config = preruntime.Config()

    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 3]
    config.format_in    = drpai_param.FORMAT.BGR
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8

    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.RGB
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.

    # 4.1.3. Define operators to be run.
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]

    # 4.2. Run DRP-AI Pre-processing Runtime
    preruntime.PreRuntime(config, opts["output_dir"]+"/preprocess", PRODUCT)
