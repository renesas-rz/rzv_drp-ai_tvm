# Semantic Segmentation

### Model: [DeepLabv3](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [How to Compile the Model](#how-to-compile-the-model)
  - [Operating Environment](#operating-environment)
  - [Save the AI Model from Torchvision](#save-the-ai-model-from-torchvision)
  - [Compile PyTorch Model](#compile-pytorch-model)
- [Processing Details](#processing-details)  

## Overview
This page explains about Semantic Segmentation in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/web.JPG width=500>  

## Model Information
DeepLabv3: [DeepLabV3 model with ResNet-50 backbone.](https://pytorch.org/vision/stable/models/generated/torchvision.models.segmentation.deeplabv3_resnet50.html)  
Dataset: [COCO_WITH_VOC_LABELS_V1](https://pytorch.org/vision/stable/models/generated/torchvision.models.segmentation.deeplabv3_resnet50.html#torchvision.models.segmentation.DeepLabV3_ResNet50_Weights)  
Input size: 1x3x224x224  
Output size: 1x21x224x224, 1x21x224x224

## How to Compile the Model
To run the Semantic Segmentation, `deeplabv3_resnet50_pt` Model Object is required for DRP-AI mode. This section explains how to prepare and compile the model.

### Operating Environment
- torch==1.8.0
- torchvision==0.9.0
- numpy

### Save the AI Model from Torchvision
Follow the instructions below to prepare the DeepLabV3 model.  

1. Create a directory to store the model data:
   ```sh
   mkdir -p model_data/deeplabv3
   cd model_data/deeplabv3
   ```

2. Create a Python script named `save_model.py` with the following content:

   ```sh
   cat > save_model.py << 'EOF'
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
   EOF
   ```

3. Run the script and confirm that `deeplabv3.pt` is generated:
   ```sh
   python3 save_model.py
   ```

4. Copy the generated model to your TVM tutorials directory:
   ```sh
   cp deeplabv3.pt $TVM_ROOT/tutorials/
   ```

### Compile PyTorch Model
After preparing the PyTorch model, you need to compile it using DRP-AI TVM.

1. Set the environment variables, i.e. `$TVM_ROOT` etc., according to [Installation](../../../../../setup/).

2. Navigate to the tutorials directory and reset the compilation script to its original state:
   ```sh
   cd $TVM_ROOT/tutorials
   git checkout -- compile_pytorch_model.py
   ```

3. Change the pre-processing details as shown below:

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

You can use the following sed commands to automatically make these changes:

```sh
# Change input shape and format
sed -i 's/config\.shape_in\s*=\s*\[1,\s*480,\s*640,\s*3\]/config.shape_in     = [1, 480, 640, 2]/' compile_pytorch_model.py
sed -i 's/config\.format_in\s*=\s*drpai_param\.FORMAT\.BGR/config.format_in    = drpai_param.FORMAT.YUYV_422/' compile_pytorch_model.py
```

4. Run the compilation script with the command below:  
```sh
python3 compile_pytorch_model.py \
  -s 1,3,224,224 \
  -o deeplabv3_resnet50_pt \
  deeplabv3.pt
```

5. Confirm that `deeplabv3_resnet50_pt` directory is generated and it contains `deploy.so`, `deploy.params`, and `deploy.json` files and `preprocess` directory.  

6. Before running the application, make sure to copy the `deeplabv3_resnet50_pt` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located:
   ```sh
   mkdir -p $TVM_ROOT/tutorials/exe
   cp -r deeplabv3_resnet50_pt $TVM_ROOT/tutorials/exe/
   ```

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_deeplabv3.cpp](../../../src/recognize/deeplabv3/tvm_drpai_deeplabv3.cpp)  

Followings are processing details if user selected "DeepLabV3 (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#compile-pytorch-model) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

The pre-processing steps include:
1. Converting YUYV_422 format input to RGB
2. Resizing the image to the model input size (224x224)
3. Normalizing the pixel values using mean [0.485, 0.456, 0.406] and standard deviation [0.229, 0.224, 0.225]

#### Inference
The Object files `deeplabv3_resnet50_pt` is generated from Torchvision's DeepLabV3 pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
