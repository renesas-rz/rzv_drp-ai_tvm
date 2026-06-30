#
#  Original code (C) Copyright EdgeCortix, Inc. 2026
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2026
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
from onnx_graph_tune.onnx_graph_converter import run_onnx_converter

def main():
    """
    A model optimized for DRP-AI will be created. 
    Be aware that extremely small errors can occur 
    because the order of floating-point operations may differ 
    between the ONNX models before and after conversion.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("onnx", help="onnx")
    parser.add_argument("--out_onnx",  default="./variant_model.onnx", help="output onnx model")
    args = parser.parse_args()

    run_onnx_converter(args.onnx, args.out_onnx)


if __name__ == "__main__":
    """
    How to use:
    $ python3 drp-ai_variant_onnx_converter.py {input_onnx} --out_onnx {output_onnx}
    """
    main()