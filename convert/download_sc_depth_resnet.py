import os
import sys
sys.path.insert(0,'./repos/SC-SfMLearner-Release')
import json
import traceback
from os.path import join
import argparse

import torch
from models import DispResNet

def save_sc_depth_resnet():
    """ Main function for downloading sc_depth_resnet models in pt format."""
    ##############################
    # Input output paramters.
    ##############################
    parser = argparse.ArgumentParser(description='Download PyTorch model from SC-depth and save it.')
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name")
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    
    args = parser.parse_args()
    model_name = args.model_name
    input_shape = tuple(map(int, args.input_shape.split(',')))

    # Model file name.
    model_file = f'{model_name}.pt'
    
    # Output dir.
    output_dir = f'{model_name}_pytorch'

    # Make output dir
    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)

    device = torch.device("cpu")

    try:
        if '18' in model_name: # resnet18
            weights = './resnet_18_dispnet_model_best.pth.tar'
            model = DispResNet(18, False).to(device)
            weights = torch.load(weights, map_location=device)
            model.load_state_dict(weights['state_dict'])
            model.eval()

        elif '50' in model_name: # resnet50
            weights = './resnet_50_dispnet_model_best.pth.tar'
            model = DispResNet(50, False).to(device)
            weights = torch.load(weights, map_location=device)
            model.load_state_dict(weights['state_dict'])
            model.eval()
        
        # Creat dummy data for trace.
        dummy_input = torch.randn(*input_shape).random_(0, 1)
        strict = True

        with torch.no_grad():
            # Infer using dummy data.
            _ = model(dummy_input)
            # Get trace model.
            scripted_model = torch.jit.trace(model, dummy_input, strict=strict).eval()
        # Save model.
        scripted_model.save(join(output_dir, model_file))

        print(f'{model_name} saved.')

    except Exception as e:
        print('-'*60)
        print(f'Failed to save model: {e}')
        print(traceback.format_exc())
        print('-'*60)
        sys.exit(1)

if __name__ == '__main__':
    # Run main function.
    save_sc_depth_resnet()