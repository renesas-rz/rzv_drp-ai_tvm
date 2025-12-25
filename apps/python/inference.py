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

import argparse
import mera
import numpy as np
import os
import time

def load_model(args):
    """
    Load runtime model data and retunr its runnner    
    param
      args: Arguments
    """
    target = mera.Target.IP
    # Get the device.
    if args.device == "DRPAI_BUILTIN_MEM":
        device_target = mera.mera_deployment.DeviceTarget.DRPAI_BUILTIN_MEM
    else:
        device_target = mera.mera_deployment.DeviceTarget.DRPAI_CPU_MEM
    # Load MERA model from pre-compiled binary
    dep_obj = mera.load_mera_deployment(args.model_path, target=target)
    # Get the runner(model) object that will do the prediction
    runner = dep_obj.get_runner(device_target=device_target, \
                                start_address=args.start_address, \
                                frequency_index=args.frequency_index)
    return runner


def get_args():
    """
    Get Arguments
      model_path : path for runtime model data
      device : target device
      start_address : start address in physical memory
      frequency_index: frequency of DRP-AI
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--model_path", default="deploy_resnet50/")
    parser.add_argument("--device", default="DRPAI_BUILTIN_MEM", type=str)
    parser.add_argument("--start_address", type=lambda x: int(x, 0), \
                         default=0x240000000, \
                         help="Address in hex or decimal (e.g., 0x240000 or 2359296)")
    parser.add_argument("--frequency_index", type=int, default=1,
                        help="Set Frequency Index (1:1000MHz,  3:630MHz, 4:420MHz, 5:315MHz, \
                                                   6:252MHz, 7:210MHz, 8:180MHz, 9:158MHz, \
                                                   10:140MHz, 11:126MHz, 12:115MHz, 13:105MHz, \
                                                   14: 97MHz, 15: 90MHz, 16: 84MHz)")
    parser.add_argument("--input_shape", default="1,3,224,224", help="User specified input shape. e.g. 1,3,224,224")
    parser.add_argument("--input_bin_file", default="None", help="Input binary file.")
    parser.add_argument("--input_dtype", default="float32", help="Input data type.")
    parser.add_argument("--loop", default=10, help="Number of loop iterations to evaluate inference time")

    args = parser.parse_args()
    #print(f"Device target: {args.device}")
    print(f"  Runtime model data path: {args.model_path}")
    print(f"  Start address: {args.start_address:#x} (decimal: {args.start_address})")
    
    return args

def main(args):

    # Load runner
    runner = load_model(args)

    print(f"Load input data")
    _input_shape = args.input_shape
    input_shape = tuple([int(dim) for dim in _input_shape.split(',')])
    input_dtype = args.input_dtype
    
    input_bin_file = args.input_bin_file
    if(input_bin_file!="None"):
        print(f"  Load input binary file : {input_bin_file}")
        input_path = input_bin_file
    else:
        print(f"  Generate input data as random data(input_0_random.bin)")
        _dummy_din_data = np.random.random(input_shape).astype(input_dtype)   
        _dummy_din_data.tofile("./input_0_random.bin")
        input_path = os.path.join("./input_0_random.bin")
    input_data = np.fromfile(input_path, dtype=input_dtype).reshape(input_shape)    

    print(f"  Input shape : {input_data.shape}, Dtype : {input_data.dtype}")

    runner.set_input([input_data])

    latencies = []
    print("Run inference")
    print(f"  Loop time : {args.loop}")
    for _ in range(args.loop): # default is 10
        start = time.perf_counter()
        runner.run()
        end = time.perf_counter()
        latencies.append((end - start) * 1000)  # ms
    print(f"  Average latency: {sum(latencies)/len(latencies):.3f} ms")

    # Get output data
    pred = runner.get_outputs()

    # Save output result as a binary file
    print("Save output data")
    for i, data_output in enumerate(pred):
        print(f"  [{i}] , {data_output.shape}")
        if data_output.dtype == "float32":
            out_bin_file = os.path.join(args.model_path, "output_" + str(i) + "_fp32.bin")
            data_output.flatten().astype(np.float32).tofile(out_bin_file)
        elif data_output.dtype == "float16":
            out_bin_file = os.path.join(args.model_path, "output_" + str(i) + "_fp16.bin")
            data_output.flatten().astype(np.float16).tofile(out_bin_file)
        elif data_output.dtype == "int64":
            out_bin_file = os.path.join(args.model_path, "output_" + str(i) + "_int64.bin")
            data_output.flatten().astype(np.int64).tofile(out_bin_file)
        elif data_output.dtype == "int32":
            out_bin_file = os.path.join(args.model_path, "output_" + str(i) + "_int32.bin")
            data_output.flatten().astype(np.int32).tofile(out_bin_file)
        else:
            assert False, "Unsupport this data type" + data_output.dtype
        print(f"  Saved output result : {out_bin_file}")


if __name__ == "__main__":
    print("Load arguments")
    args = get_args()
    main(args)
