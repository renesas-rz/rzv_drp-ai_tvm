#
#  (C) Copyright Renesas Electronics Corporation 2026
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

import numpy as np
import os
import argparse

def get_args():
    """
    Get Arguments
      output_path : path for output data of AI inference
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--output_path", default="resnet/resnet_onnx/output_0_fp16.bin",  help="Output binary data.")

    args = parser.parse_args()
    
    return args

def show_top5(pred_out):
    # run softmax
    sum = np.sum(np.exp(pred_out))
    pred_softmax = np.exp(pred_out)/sum
    
    # show label
    top5 = pred_softmax.argsort()[-5:]
    # open label data
    with open("resnet/ImageNetLabels.txt","r") as f:
        labels = f.readlines()
    # Get inference result
    for i in range(5):
        index = top5[4-i]
        name = labels[index+1].replace("\n","")
        score = pred_softmax[index]
        print(f" TOP {i+1} : class {name:<20}, Score : {score:.3f}, index : {index}")
    

if __name__ == "__main__":
    args = get_args()

    print("Sample script for ResNet")
    if not os.path.isfile(args.output_path):
        print(f"Error: OUTPUT_PATH does not exist or is not a directory: { args.output_path}")
        exit()

    print("Load input data")
    print(f"  AI inference output binary data: ", args.output_path)
    print(f"\nResult")

    if ("fp16" in args.output_path):
        pred = np.fromfile(args.output_path,np.float16).astype(np.float32)
    else:
        pred = np.fromfile(args.output_path,np.float32) # FP32 Case

    show_top5(pred)
