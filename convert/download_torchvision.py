import os
import json
import traceback
import subprocess
from os.path import join
import argparse

import torch
import torchvision

class DictOutputModelWrapper(torch.nn.Module):
    """ Wrapper class to convert output from dict to tuple  """
    def __init__(self, model):
        super().__init__()
        self.model = model

    def forward(self, inp):
        out = self.model(inp)
        if isinstance(out, dict):
            return self.dict_to_tuple(out)
        elif isinstance(out[0], dict):
            return self.dict_to_tuple(out[0])
        else:
            return out

    def dict_to_tuple(self, out_dict):
        keys = sorted(list(out_dict.keys()))
        out = tuple([out_dict[key] for key in keys])
        return out

def save_pytorch():
    """ Main function for downloading pytorch models.  """
    parser = argparse.ArgumentParser(description='Download PyTorch model and save it.')
    parser.add_argument("-t", "--model_type", dest="model_type", help="User specified model type")
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name")

    args = parser.parse_args()
    model_name = args.model_name
    model_type = args.model_type
    input_shape = tuple(map(int, args.input_shape.split(',')))

    # Model file name.
    model_file = f'{model_name}.pt' 
    # Output dir.
    output_dir = f'{model_name}_torchvision_pytorch'

    # Make output dir
    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)

    try:
        if model_type == 'classification':
            # Get model from torchvision zoo.
            model = getattr(torchvision.models,
                            model_name)(pretrained=True)

        elif model_type == 'segmentation':
            # Get model from torchvision zoo.
            model = getattr(torchvision.models.segmentation,
                            model_name)(pretrained=True)

        # Custom model wrapper to convert dict output.
        model = DictOutputModelWrapper(model)

        strict = True

        model.eval()
        # Creat dummy data for trace.
        dummy_input = torch.randn(*input_shape).random_(0, 1)
        with torch.no_grad():
            # Infer using dummy data.
            _ = model(dummy_input)
            # Get trace model.
            scripted_model = torch.jit.trace(model, dummy_input, strict=strict).eval()
        # Save model.
        scripted_model.save(join(output_dir, model_file))
        print(f'{model_name} saved.')

    except Exception as e:
        print(f'Failed to save model:{e}')
        return
        
if __name__ == '__main__':
    save_pytorch()