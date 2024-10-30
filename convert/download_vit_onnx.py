import os
import onnxruntime
from os.path import join

import torch
import timm
import numpy as np
import argparse

def main():
    parser = argparse.ArgumentParser(description='Download ViT models and save it.')
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name.")
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    
    args = parser.parse_args()
    model_name = args.model_name
    input_shape = args.input_shape.split(',')
    input_shape = tuple(map(int, input_shape))

    model_file = f'{model_name}.onnx'
    output_dir = f'{model_name}_onnx'

    model = timm.create_model(model_name, pretrained=True)
    model.eval()

    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)  
    
    dummy_input = torch.randn(input_shape)

    with torch.no_grad():
        torch_out = model(dummy_input)
        onnx_path = join(output_dir, model_file) 
        torch.onnx.export(model, dummy_input, onnx_path, export_params=True, opset_version=13, input_names=['input'], output_names=['output'])
        
    print(f"ONNX model saved at {onnx_path}")

if __name__ == "__main__":
    main()
