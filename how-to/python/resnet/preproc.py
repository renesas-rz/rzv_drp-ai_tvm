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

from PIL import Image
import numpy as np
import argparse

def get_args():
    """
    Get Arguments
      input_img : path for input image file
      bin_name : path for output binary file to be saved
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--input_img", default="resnet/sample.bmp")
    parser.add_argument("--bin_name", default="resnet/input_0.bin")
    args = parser.parse_args()
    return args
    
        
if __name__ == "__main__":
    args = get_args()

    print("Sample script for ResNet")
    print("Load input data")
    print("  Input image: ", args.input_img)
    im = Image.open(args.input_img)
    im = im.resize((224,224),Image.BILINEAR)  # resize to 224x224
    im_np = np.asarray(im)               # convert to numpy array
    im_np = im_np.transpose(2,0,1)       # Transpose to (3,224,224)
    im_np = im_np.astype(np.float32)     # cast to FP32
    im_np = im_np/255                    # data range 0.0 - 1.0
    mean = [0.485, 0.456, 0.406]         # define normalize parameters
    std = [0.229, 0.224, 0.225]          # define normalize parameters
    for i in range(3):                   # normalize
        im_np[i] = (im_np[i] - mean[i])/std[i]
    im_np.tofile(args.bin_name)          # save as binary file
    print("Save output data")
    print("  Saved pre-processed data: ", args.bin_name)