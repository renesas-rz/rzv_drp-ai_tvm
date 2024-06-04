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
import onnx
import sys

from tvm import relay
from tvm.relay.mera import drp

from drpai_preprocess import preruntime, drpai_param, op
from arg_parser import get_args

PRODUCT= os.getenv("PRODUCT")
if(PRODUCT == None):
    print("[Error] No environment variable")
    print("        Before running this script,")
    print("        Please set environment variable(PRODUCT)")
    print("        to your product name.")
    print("        e.g. $export PRODUCT=V2L")
    sys.exit(-1)

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
    # 1. Get argument data
    (opts, model_file) = get_args()
    output_dir=opts["output_dir"]
    os.makedirs(output_dir, exist_ok=True)

    # 2. Load onnx model and set input shape.
    # 2.1 Load onnx model
    onnx_model = onnx.load_model(model_file)
    
    # 2.2 Set input data information
    shape_dict = {opts["input_name"]:opts["input_shape"]}

    try:
        # 3. Run DRP-AI TVM[*1] compiler
        # 3.1 Run TVM Frontend
        print("-------------------------------------------------")
        print("   Run TVM frontend compiler ")
        mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)
    except Exception as e:
        print(f"An error occured: {e}")
        if not opts["input_name"]: print("Try again with ' -i : Input node name of AI model'")

        if not opts["input_shape"]: print("Try again with ' -s : Input shape of AI model'")
        elif opts["input_shape"] == [1, 3, 224, 224]: print("Did you pass an input shape with -s?")
        sys.exit(1)

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
    drp.build(mod, \
                params, \
                "arm", \
                drp_config_runtime, \
                   output_dir=output_dir, \
                   disable_concat = opts["disable_concat"]
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
