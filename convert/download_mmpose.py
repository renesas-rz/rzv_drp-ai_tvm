import os
import json
import traceback
import subprocess
from os.path import join
import argparse

import torch
import torchvision

from mmdet.apis import init_detector
from mmpose.apis import init_pose_model

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

def _convert_batchnorm(module):
    """Convert the syncBNs into normal BN3ds."""
    module_output = module
    if isinstance(module, torch.nn.SyncBatchNorm):
        module_output = torch.nn.BatchNorm3d(module.num_features, module.eps,
                                             module.momentum, module.affine,
                                             module.track_running_stats)
        if module.affine:
            module_output.weight.data = module.weight.data.clone().detach()
            module_output.bias.data = module.bias.data.clone().detach()
            # keep requires_grad unchanged
            module_output.weight.requires_grad = module.weight.requires_grad
            module_output.bias.requires_grad = module.bias.requires_grad
        module_output.running_mean = module.running_mean
        module_output.running_var = module.running_var
        module_output.num_batches_tracked = module.num_batches_tracked
    for name, child in module.named_children():
        module_output.add_module(name, _convert_batchnorm(child))
    del module
    return module_output

def mm_download(model_name, source):
    """ Download mmpose or mmpose detection models """
    try:
        mm_file = './mm_data.json'
        with open(mm_file) as f:
            mm_dict = json.load(f)
    except Exception as e:
        print(f'Failed to open mm_data.json file. \n {e}')
        return

    config_file = join('./repos', mm_dict[model_name]['config'])
    checkpoint_file = mm_dict[model_name]['checkpoint']

    if source == 'mmpose':

        model = init_pose_model(config_file, checkpoint_file, device='cpu')
    elif source == 'mmdetection':
        model = init_detector(config_file, checkpoint_file, device='cpu')

    model = _convert_batchnorm(model)

    if hasattr(model, 'forward_dummy'):
        model.forward = model.forward_dummy
    else:
        raise NotImplementedError(
            'Please implement the forward method for exporting.')
    return model

def save_pytorch():
    """ Main function for downloading pytorch models.  """
    parser = argparse.ArgumentParser(description='Download PyTorch model and save it.')
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name")

    args = parser.parse_args()
    model_name = args.model_name
    source = "mmpose"
    input_shape = tuple(map(int, args.input_shape.split(',')))

    # Model file name.
    model_file = f'{model_name}.pt' 
    # Output dir.
    output_dir = f'{model_name}_{source}_pytorch'

    # Make output dir
    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)

    try:
        model = mm_download(model_name, source)

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