import os
import torch
import timm
import argparse
from os.path import join

def main():
    parser = argparse.ArgumentParser(description="Export Swin Transformer to ONNX format")
    parser.add_argument('-n', '--model_name', type=str, required=True,
                        help="Model name: swin_tiny_patch4_window7_224, swin_small_patch4_window7_224, swin_base_patch4_window7_224")
    parser.add_argument('-s', '--input_shape', type=str, default="1,3,224,224",
                        help="Input shape in the format 'batch_size,channels,height,width', e.g., '1,3,224,224'")

    args = parser.parse_args()
    input_shape = tuple(map(int, args.input_shape.split(',')))
    model_name = args.model_name
    model = timm.create_model(model_name, pretrained=True)
    model.eval()

    model_file = f'{model_name}.onnx'
    output_dir = f'{model_name}_onnx'
    onnx_path = join(output_dir, model_file)


    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)
    
    dummy_input = torch.randn(input_shape)

    with torch.no_grad():
        torch_out = model(dummy_input)
        onnx_path = join(output_dir, model_file)
        torch.onnx.export(
            model, 
            dummy_input, 
            onnx_path, 
            export_params=True, 
            input_names=['input'],
            output_names=['output'], 
            opset_version=13,
            do_constant_folding=True
        )
    print(f"ONNX model saved at {onnx_path}")

if __name__ == "__main__":
    main()
