import argparse
import os
from os.path import join

import torch
import onnx
import timm


def main():
    parser = argparse.ArgumentParser(description="Download Swin models and save it.")
    parser.add_argument(
        "-n", "--model_name", dest="model_name", help="User specified model name."
    )
    parser.add_argument(
        "-s", "--input_shape", dest="input_shape", help="User specified input shape"
    )
    parser.add_argument(
        "-o", "--output_dir", dest="output_dir", help="User specified output dir"
    )

    args = parser.parse_args()
    model_name = args.model_name
    input_shape = tuple(map(int, args.input_shape.split(",")))
    output_dir = args.output_dir
    os.makedirs(output_dir, exist_ok=True)

    # donwload model from torch hub
    model_type = "MiDaS_small"  # MiDaS v2.1 - Small
    midas = torch.hub.load("intel-isl/MiDaS", model_type)
    device = torch.device("cpu")
    midas.to(device)
    midas.eval()

    # Define dummy input shape
    tsr_din = torch.ones(*input_shape)

    # Export to onnx model
    onnx_path = join(output_dir, model_name + ".onnx")
    with torch.no_grad():
        torch.onnx.export(
            midas,
            tsr_din,
            onnx_path,
            input_names=["input"],
        )

    # Use above model as DRP-AI TVM input data
    print(f"ONNX model saved at {model_name}")


if __name__ == "__main__":
    main()
