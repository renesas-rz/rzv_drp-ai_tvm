import os
import sys
sys.path.insert(0,'./repos/monodepth2')
import json
import traceback
from os.path import join, isfile
from glob import glob
import shutil
import argparse

import torch
import networks
from utils import download_model_if_doesnt_exist

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
    
def save_mono_640x192():
    """ Main function for downloading mono_640x192 models in pt format."""
    ##############################
    # Input output paramters.
    ##############################
    parser = argparse.ArgumentParser(description='Download Monodepth model and save it.')
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name")
    parser.add_argument("-s", "--input_shape",dest="input_shape", help="User specified input shape")
    args = parser.parse_args()
    model_name = args.model_name
    input_shape = tuple(map(int, args.input_shape.split(',')))

    # mono_640x192
    model_n = '_'.join(model_name.split('_')[:-1])

    # Model file name.
    model_file = f'{model_name}.pt'
    
    # Output dir.
    output_dir = f'{model_name}_pytorch'

    # Make output dir
    output_dir = join('output', output_dir)
    os.makedirs(output_dir, exist_ok=True)

    device = torch.device("cpu")
    
    try:
        download_model_if_doesnt_exist(model_n)
        model_path = join("models", model_n)

        # Loding pretrained model.
        encoder_path = join(model_path, "encoder.pth")
        encoder = networks.ResnetEncoder(18, False)
        loaded_dict_enc = torch.load(encoder_path, map_location=device)
        filtered_dict_enc = {k: v for k, v in loaded_dict_enc.items() if k in encoder.state_dict()}
        encoder.load_state_dict(filtered_dict_enc)

        encoder.to(device)
        encoder.eval()

        print("   Loading pretrained decoder")
        depth_decoder_path = join(model_path, "depth.pth")
        depth_decoder = networks.DepthDecoder(num_ch_enc=encoder.num_ch_enc, scales=range(4))
        loaded_dict_dec = torch.load(depth_decoder_path, map_location=device)
        filtered_dict_dec = {k: v for k, v in loaded_dict_dec.items() if k in depth_decoder.state_dict()}
        depth_decoder.load_state_dict(filtered_dict_dec)

        depth_decoder.to(device)
        depth_decoder.eval()

        # Creat dummy data for trace.
        dummy_input = torch.randn(*input_shape).random_(0, 1)

        with torch.no_grad():
            # Infer using dummy data.
            features = encoder(dummy_input)
            outputs = depth_decoder(features)
            strict = True
            
            if 'encoder' in model_name:
                # Get trace model.
                scripted_model = torch.jit.trace(encoder, dummy_input, strict=strict).eval()
            else:
                # Get trace model.
                depth_decoder = DictOutputModelWrapper(depth_decoder)

                scripted_model = torch.jit.script(depth_decoder).eval()
        
        # Delete temp model files.
        for path in glob(model_path + '*'):
            if isfile(path):
                os.remove(path)
            else:
                shutil.rmtree(path)

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
    # Get arguments.
    save_mono_640x192()