# Semantic Segmentation

### Model: [DeepLabv3](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

## Overview
This page explains about Semantic Segmentation in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/web.JPG width=500>  

## Model Information
DeepLabv3: [DeepLabV3 model with ResNet-50 backbone.](https://pytorch.org/vision/stable/models/generated/torchvision.models.segmentation.deeplabv3_resnet50.html)  
Dataset: [COCO_WITH_VOC_LABELS_V1](https://pytorch.org/vision/stable/models/generated/torchvision.models.segmentation.deeplabv3_resnet50.html#torchvision.models.segmentation.DeepLabV3_ResNet50_Weights)  
Input size: 1x3x224x224  
Output size: 1x21x224x224, 1x21x224x224

### How to compile the model
To run the Semantic Segmentation, `deeplabv3_resnet50_pt` Model Object is required for DRP-AI mode.
- [Semantic Segmentation](#semantic-segmentation)
    - [Model: DeepLabv3](#model-deeplabv3)
    - [Index](#index)
  - [Overview](#overview)
  - [Model Information](#model-information)
    - [How to compile the model](#how-to-compile-the-model)
      - [Operating Environment](#operating-environment)
      - [1. Save the AI model from Torchvision](#1-save-the-ai-model-from-torchvision)
      - [2. Compile pytorch model for DRP-AI mode](#2-compile-pytorch-model-for-drp-ai-mode)
  - [Processing Details](#processing-details)
    - [DRP-AI mode](#drp-ai-mode)
      - [Pre-processing](#pre-processing)
      - [Inference](#inference)
      - [Post-processing](#post-processing)

#### Operating Environment
- torch==1.8.0
- torchvision==0.9.0
- numpy

#### 1. Save the AI model from Torchvision
Follow the instuction below to prepare the DeepLabV3 model.  

1. Prepare the save script as below.
```py
import numpy as np

# PyTorch imports
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

model_name = "deeplabv3_resnet50"
model = getattr(torchvision.models.segmentation, model_name)(pretrained=True)

# Custom model wrapper to convert dict output.
model = DictOutputModelWrapper(model)

model = model.eval()

# We grab the TorchScripted model via tracing
input_shape = [1, 3, 224, 224]
input_data = torch.randn(input_shape)
scripted_model = torch.jit.trace(model, input_data).eval()

scripted_model.save('deeplabv3.pt') # Save
print("Torch model saved to ./deeplabv3.pt")

```
2. Run the save script and confirm that `deeplabv3.pt` is generated.

#### 2. Compile pytorch model for DRP-AI mode
Follow the instuction below to prepare the `deeplabv3_resnet50_pt` Model Object.

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the `deeplabv3.pt` file in `$TVM_HOME/../tutorials`.
3. Change the pre-processing details as shown below.  

Before
```py
#L107~130
    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 3]
    config.format_in    = drpai_param.FORMAT.BGR
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8
    
    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.RGB
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32 
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.
    
    # 4.1.3. Define operators to be run.
    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
After
```py
#L107~130
    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 2]
    config.format_in    = drpai_param.FORMAT.YUYV_422
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8
    
    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.RGB
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32 
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.
    
    # 4.1.3. Define operators to be run.
    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
4. Run the script with the command below.  
```sh
$ python3 compile_pytorch_model.py \
-s 1,3,224,224 \
-o deeplabv3_resnet50_pt \ 
deeplabv3.pt
```
5. Confirm that `deeplabv3_resnet50_pt` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.
   
6. Before running the application, make sure to copy the `deeplabv3_resnet50_pt` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_deeplabv3.cpp](../../../src/recognize/deeplabv3/tvm_drpai_deeplabv3.cpp)  

Followings are processing details if user selected "DeepLabV3 (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#2-compile-pytorch-model-for-drp-ai-mode) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
The Object files `deeplabv3_resnet50_pt` is generated from Torchvision's DeepLabV3 pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
