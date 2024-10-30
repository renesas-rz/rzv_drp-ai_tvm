import os
import json
import traceback
import subprocess
from os.path import join
from collections import namedtuple

import torch
import timm
import argparse

class ModelWrapper(torch.nn.Module):
    def __init__(self, model):
        super().__init__()
        self.model = model

    def forward(self, input_x):
        data = self.model(input_x)
        if isinstance(data, dict):
            data_named_tuple = namedtuple("ModelEndpoints", sorted(data.keys()))
            data = data_named_tuple(**data)
        elif isinstance(data, list):
            data = tuple(data)
        return data
        
def save_pytorch_timm():
    parser = argparse.ArgumentParser(description='Download PyTorch model from TIMM and save it.')
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name")

    args = parser.parse_args()
    model_name = args.model_name
    input_shape = args.input_shape.split(',')
    input_shape = tuple(map(int,input_shape))

    """ Main function for downloading pytorch models. (timm) """
    # Model file name.
    model_file = f'{model_name}.pt'
    # Output dir.
    output_dir = f'{model_name}_timm_pytorch'

    # for output config file start 
    config = {'arch': "pytorch",
              'source': "timm",
              'model_file': model_file,
              'output_dir': output_dir,
              'input_shape': input_shape,
              'input_name': '',
              'memory_address': "0x0"}

    # Make output dir
    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)

    # Save data to config.json file.
    data = json.dumps(config, indent=4, separators=(', ', ': '))
    with open(join(output_dir, 'config.json'), 'w') as f:
        f.write(data)

    try:
        model = timm.create_model(model_name, pretrained=True)
        model.eval()

        model_wrapper = ModelWrapper(model)
        model_wrapper.eval()

        # Creat dummy data for trace.
        dummy_input = torch.randn(*input_shape).random_(0, 1)

        with torch.no_grad():
            _ = model(dummy_input)
            # Get trace model.
            scripted_model = torch.jit.trace(model_wrapper, dummy_input).eval()
        # Save model.
        scripted_model.save(join(output_dir, model_file))

        print(f'{model_name} saved.')
        
    except Exception as e:
        print(f'Failed to save model:{e}')
        return

if __name__ == '__main__':
    save_pytorch_timm()