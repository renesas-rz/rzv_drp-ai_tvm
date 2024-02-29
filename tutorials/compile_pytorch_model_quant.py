#
#  Original code (C) Copyright EdgeCortix, Inc. 2022
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2023
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

import tvm
import numpy as np
import shutil
import pathlib
import cv2
from PIL import Image
import torchvision.transforms.functional as F

from tvm import relay, runtime
from tvm.relay import mera
from tvm.relay.mera import drp
from tvm.contrib import graph_executor
from collections import namedtuple
from optparse import OptionParser
from torchvision import models

from arg_parser import get_args

import sys

PRODUCT= os.getenv("PRODUCT")
if(PRODUCT != 'V2H'):
    print("[Error] Environment variable (PRODUCT) is not V2H")
    print("        Please set environment variable(PRODUCT)")
    print("        to your product name.")
    print("        e.g. $export PRODUCT=V2H")
    sys.exit(-1)

def create_random_input_from_rt(rt_mod):
    shape_dict = {}
    input_data = []
    input_shape = opts["input_shape"]
    for key, value in rt_mod.get_input_info()[0].items():
        if input_shape == tuple(value):
            if 'input_name' in locals():
                print("multi input is unexpected.")
            input_name = key
            if rt_mod.get_input_info()[1][key] != "float32":
                print("unexpected sta type")
    shape_dict[input_name] = input_shape
    data = np.random.uniform(0.0, 1.0, input_shape).astype(np.float32)
    input_data.append(data)
    return shape_dict, input_data


def create_random_input(inputs, model_vars, model_file):
    shape_dict = {}
    input_data = []
    inp_idx = 0
    for inp in inputs:
        input_name = inp.name
        if input_name not in model_vars:
            continue
        input_shape = opts["input_shape"]
        shape_dict[input_name] = input_shape
        data = np.random.uniform(0.0, 1.0, input_shape).astype(np.float32)
        input_data.append(data)
        data.flatten().astype(np.float32).tofile(os.path.join(output_dir, "input_" + str(inp_idx) + ".bin"))
        inp_idx += 1
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
     -s : Input shape of AI model
     [Options]
     -i : Input node name of AI model
     -d : DRP-AI Trahslator path
     -t : toolchain path (Cross compiler/SDK)
     -c : Concat disable mode. Safety compile mode.
    """
    np.random.seed(1234567)
    
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
    
    print("Calibration data record directory: ", record_dir)
    
    # 2. Load pytorch model and set input shape.
    # 2.1 Load pytorch(.pt) model
    model = torch.jit.load(model_file)
    model.eval()
    
    # 2.2 Set input data information
    input_name = "input0"
    shape_list = [(input_name, opts["input_shape"])]

    # 3. Run DRP-AI TVM[*1] compiler 
    # 3.1 Run TVM Frontend
    print("-------------------------------------------------")
    print("   Run TVM frotend compiler ")
    mod, params = relay.frontend.from_pytorch(model, shape_list)

    # 3.2 Create calibration data(using random values.)
    drp_config = {
        "target": "Fp32DataRecorder",
        "drp_compiler_version": opts["drp_compiler_version"],
        "record_dir": record_dir,
    }

    # 3.2.2 Run the backend compiler for x86 and generate reference output.
    json, params, lib_path = drp.build(mod, params, "x86", drp_config, output_dir="_out", disable_concat= opts["disable_concat"], cpu_data_type=opts["cpu_data_type"])
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)

    input_dir = opts["image_dir"]

    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]

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
            single_input_data = pre_process_imagenet_pytorch(image, mean, stdev, need_transpose=True)
            single_input_data = np.expand_dims(single_input_data, 0)
            input_data = [single_input_data]
            for inp_idx in range(len(input_data)):
                rt_mod.set_input(inp_idx, input_data[inp_idx])
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
    output_dir="_out_quant"
    os.makedirs(output_dir, exist_ok=True)
    # 3.3.2 Run the backend compiler for x86 and generate reference output of int8.
    ref_result_output_dir = os.path.join(opts["output_dir"],"interpreter_out")
    os.makedirs(ref_result_output_dir, exist_ok=True)
    input_data[0].flatten().astype(np.float32).tofile(
            os.path.join(ref_result_output_dir, "input_" + str(0) + ".bin"))
    json, params, lib_path = mera.drp.build(mod, params, "x86", drp_config, output_dir, disable_concat= opts["disable_concat"], cpu_data_type=opts["cpu_data_type"])
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)
    rt_mod.set_input(0, input_data[0])
#    for inp_idx in range(len(input_data)):
#        rt_mod.set_input(inp_idx, input_data[inp_idx])
    rt_mod.run()

    # 3.3.3 Save to use as reference output.
    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i).asnumpy()
        if byoc_output.dtype == "float32":
            byoc_output.flatten().astype(np.float32).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) + "_fp32.bin"))
        elif byoc_output.dtype == "float16":
            byoc_output.flatten().astype(np.float16).tofile(
                os.path.join(ref_result_output_dir, "ref_result_" + str(i) +"_fp16.bin"))

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
    json, params, lib_path = mera.drp.build(mod, params, "arm", drp_config_runtime, output_dir=opts["output_dir"], disable_concat = opts["disable_concat"], cpu_data_type=opts["cpu_data_type"])

    print("[TVM compile finished]")
    print("   Please check {0} directory".format(opts["output_dir"]))
    print("[Start compiling PreRuntime objects....]")

    # 4. Compile pre-processing using DRP-AI Pre-processing Runtime
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
