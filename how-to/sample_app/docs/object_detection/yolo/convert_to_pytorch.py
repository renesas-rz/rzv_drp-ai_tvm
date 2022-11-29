import numpy as np
import sys
import torch
from torch import nn
from torchvision.transforms import ToTensor

from read_ini import IniFile
from darknet_cfg import DarknetConfig
import yolo

def convert(darknet_cfg_path, darknet_weights_path, save_model_path, model):
    # Load cfg file
    cfg = DarknetConfig(darknet_cfg_path)

    torch_layers = list(model.itr_layers())
    torch_layer_pos = 0

    # Read weight file
    with open(darknet_weights_path, 'rb') as f:
        # header
        major, minor, revision = np.fromfile(f, dtype=np.int32, count=3)

        if major*10 + minor >= 2 and major < 1000 and minor < 1000:
            seen = np.fromfile(f, dtype=np.int64, count=1)[0]
        else:
            seen = np.fromfile(f, dtype=np.int32, count=1)[0]

        print(f"{major}.{minor}.{revision} {seen}")

        # layers
        for cfg_layer in cfg.layers:
            module_pos = 0
            print(cfg_layer.name)
            if type(torch_layers[torch_layer_pos][module_pos]) == nn.Conv2d :
                torch_conv = torch_layers[torch_layer_pos][module_pos]
            if cfg_layer.name == "convolutional":
                n_kernels = int(cfg_layer.params.get('filters'))
                kernel_size = int(cfg_layer.params.get('size'))
                stride = int(cfg_layer.params.get('stride'))
                pad = int(cfg_layer.params.get('pad'))
                if cfg_layer.params.get('batch_normalize') is not None:
                    batch_normalize = int(cfg_layer.params.get('batch_normalize')) == 1
                else:
                    batch_normalize = False

                w = {}
                if batch_normalize:
                    module_pos += 1
                    torch_bn = torch_layers[torch_layer_pos][module_pos]

                    w['bn_b'] = np.fromfile(f, dtype=np.float32, count=torch_bn.bias.numel())
                    w['bn_w'] = np.fromfile(f, dtype=np.float32, count=torch_bn.weight.numel())
                    w['bn_mean'] = np.fromfile(f, dtype=np.float32, count=torch_bn.running_mean.numel())
                    w['bn_var'] = np.fromfile(f, dtype=np.float32, count=torch_bn.running_var.numel())
                    w['bn_b'] = w['bn_b'].reshape(torch_bn.bias.shape)
                    w['bn_w'] = w['bn_w'].reshape(torch_bn.weight.shape)
                    w['bn_mean'] = w['bn_mean'].reshape(torch_bn.running_mean.shape)
                    w['bn_var'] = w['bn_var'].reshape(torch_bn.running_var.shape)

                    torch_bn.bias.data.copy_(torch.from_numpy(w['bn_b']))
                    torch_bn.weight.data.copy_(torch.from_numpy(w['bn_w']))
                    torch_bn.running_mean.data.copy_(torch.from_numpy(w['bn_mean']))
                    torch_bn.running_var.data.copy_(torch.from_numpy(w['bn_var']))
                else:
                    w['conv_b'] = np.fromfile(f, dtype=np.float32, count=torch_conv.bias.numel())
                    w['conv_b'] = w['conv_b'].reshape(torch_conv.bias.shape)
                    torch_conv.bias.data.copy_(torch.from_numpy(w['conv_b']))

                w['conv_w'] = np.fromfile(f, dtype=np.float32, count=torch_conv.weight.numel())
                w['conv_w'] = w['conv_w'].reshape(torch_conv.weight.shape)
                torch_conv.weight.data.copy_(torch.from_numpy(w['conv_w']))
            torch_layer_pos += 1
        rest = len(f.read())
        print(f"REST {rest}")

    # Save pth file
    torch.save(model.state_dict(), save_model_path)

if __name__ == '__main__':
    # Load YOLO neural network parameters
    ini = IniFile("yolo.ini")
    model_dict = ini.model_dict

    # Obtain the model name from command line argument
    if ( len(sys.argv) > 1 and sys.argv[1] in model_dict.keys() ):
        model_name = sys.argv[1]
        print("Model [", model_name, "] is selected.")
    else:
        print("Default model [ yolov3 ] is selected.")
        model_name = "yolov3"
    # Get the model information
    model_params = ini.architecture[model_dict[model_name]].params

    # Load YOLO neural network structure
    model = yolo.Yolo(model_params.get('cfg'))
    # Convert Darknet to PyTorch
    convert(model_params.get('cfg'), model_params.get('weights'), model_params.get('pth'), model)
