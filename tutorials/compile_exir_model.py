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
if(PRODUCT == None):
    print("[Error] No environment variable")
    print("        Before running this script,")
    print("        Please set environment variable(PRODUCT)")
    print("        to your product name.")
    print("        e.g. $export PRODUCT=V2L")
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
     -t : toolchain path (Cross compiler/SDK)
    """
    # 1. Get argument data
    (opts, model_file) = get_args()
    output_dir=opts["output_dir"]
    os.makedirs(output_dir, exist_ok=True)

    # 2. Set input shape.
    input_shape = opts["input_shape"]
    input_data = np.random.uniform(-1.0, 1.0, input_shape).astype(np.float32)
    input_data.flatten().astype(np.float32).tofile(os.path.join(output_dir, "input_0" + ".bin"))
    
    # 2.2 Set input data information
    input_name = "input0"
    shape_list = [(input_name, opts["input_shape"])]

    # 3. Run DRP-AI TVM[*1] compiler 
    # 3.1 Run TVM Frontend
    print("-------------------------------------------------")
    print("   Run TVM frotend compiler ")
    mod, params = mera2.from_exir(model_file, is_quant=False)

    # 3.2 Run TVM backend with DRP-AI translator
    print("-------------------------------------------------")
    print("   Run TVM backend compiler with DRP-AI Translator")
    # 3.2.1 Set config for DRP-AI runtime
    drp_config_runtime = {
        "target": "DrpAi",
        "addr_map_start": 0x0,
        "toolchain_dir": opts["drp_compiler_dir"],
        "sdk_root": opts["toolchain_dir"]
    }
    # 3.2.2 Run backend compiler
    json, params, lib_path = mera2.drp.build(mod, \
                                             params, \
                                            "arm", \
                                            drp_config_runtime, \
                                            output_dir=output_dir, \
                                            disable_concat = opts["disable_concat"], \
                                            cpu_data_type=opts["cpu_data_type"]\
                                            )
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
