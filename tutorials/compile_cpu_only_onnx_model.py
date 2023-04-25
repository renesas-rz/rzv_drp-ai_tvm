#
#  Original code (C) Copyright EdgeCortix, Inc. 2022
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2023
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
import onnx
import numpy as np

import tvm
from tvm.relay import transform
from tvm.relay.param_dict import save_param_dict
from tvm.relay.build_module import bind_params_by_name
from tvm.relay.build_module import build as _build
from tvm.ir.transform import Sequential, PassContext
from tvm.contrib import cc
from optparse import OptionParser

from arg_parser import get_args

import sys
PRODUCT= os.getenv("PRODUCT")
if(PRODUCT == None):
    print("[Error] No environment variable")
    print("        Before running this script,")
    print("        Please set environment variable(PRODUCT)")
    print("        to your product name.")
    print("        e.g. $export PRODUCT=V2L")
    sys.exit(-1)

if __name__ == "__main__":
    # 1. Get argument data
    (opts, model_file) = get_args()
    output_dir=opts["output_dir"]
    os.makedirs(output_dir, exist_ok=True)
    # 2. Set params
    input_name = opts["input_name"]
    input_shape = opts["input_shape"]
    output_dir = opts["output_dir"]
    sdk_root = opts["toolchain_dir"]
    
    # 3. Set cross compiler setting
    host_arch = "arm"
    target = "llvm -device=arm_cpu -mtriple=aarch64-linux-gnu -mattr=+neon"
    fcompile = cc.cross_compiler(
        "{sdk_root}/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++".format(sdk_root = sdk_root),
        [
            "--sysroot={sdk_root}/sysroots/aarch64-poky-linux".format(sdk_root = sdk_root),
            "-march=armv8-a",
            "-mtune=cortex-a53"
        ])
    
    # 2. Load onnx model and set input shape.
    shape_dict = {input_name: input_shape}
    # 2.1 Load onnx model
    onnx_model = onnx.load_model(model_file)
    # 2.2 Set input data information

    # 3.1 Run TVM Frontend
    mod, params = tvm.relay.frontend.from_onnx(onnx_model, shape_dict)
    if params:
        mod["main"] = bind_params_by_name(mod["main"], params)
    
    with PassContext(opt_level=3):
        pass_list = [
            transform.SimplifyInference(),
            transform.FoldConstant(),
            transform.FoldExplicitPadding(),
            transform.BackwardFoldScaleAxis(),
            transform.ForwardFoldScaleAxis(),
            transform.FoldConstant(),
            transform.DynamicToStatic(),
        ]
        if opts["fp16"]:
            pass_list += [
                transform.ToMixedPrecision("float16"),
                transform.FoldConstant(),
            ]
        pass_list += [
            transform.RemoveUnusedFunctions(),
        ]

    simplify = Sequential(pass_list)
    mod = simplify(mod)

    # 3.2 Run TVM backend with CPU only mode
    with PassContext(opt_level=3):
        json, lib, all_params = _build(mod, target=target, target_host=target, params=params)
    
    os.makedirs(output_dir, exist_ok=True)
    lib_path = os.path.join(output_dir, "deploy.so")
    lib.export_library(lib_path, fcompile=fcompile)
    with open(os.path.join(output_dir, "deploy.json"), "w") as f:
        f.write(json)
    with open(os.path.join(output_dir, "deploy.params"), "wb") as f:
        f.write(save_param_dict(all_params))
    print("[TVM compile finished]")
    print("   Please check {0} directory".format(output_dir))

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
    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
    
    # 4.2. Run DRP-AI Pre-processing Runtime
    preruntime.PreRuntime(config, output_dir+"/preprocess", PRODUCT)