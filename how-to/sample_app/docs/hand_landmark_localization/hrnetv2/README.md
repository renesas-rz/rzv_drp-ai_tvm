# Hand Landmark Localization

### Model: [HRNet(High-Resolution Network) v2](#model-information)
Sample application code are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

## Overview
This page explains about Hand Landmark Localization in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/hrnetv2.jpg width=500>  

## Model Information
- HRNetv2: [MMPose Topdown Heatmap + Hrnetv2 + Coco + Wholebody on Coco_wholebody_hand](https://github.com/open-mmlab/mmpose/blob/v0.28.1/configs/hand/2d_kpt_sview_rgb_img/topdown_heatmap/coco_wholebody_hand/hrnetv2_coco_wholebody_hand.md)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x256x256  
Output size: 1x21x64x64    

### How to compile the model
To run the Hand Landmark Localization, `hrnetv2_pt` Model Object is required.  
- [Operating Environment](#operating-environment)
- [Save the AI model from MMPose](#save-the-ai-model-from-mmpose)
- [Compile pytorch model](#compile-pytorch-model)

#### Operating Environment
- mmcv-full v1.6.1  
- MMPose v0.28.1  

#### Save the AI model from MMPose
1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Clone mmpose repository.  

```sh
git clone -b v0.28.1 https://github.com/open-mmlab/mmpose.git
```

3. Download the checkpoint file from [the mmpose website](https://github.com/open-mmlab/mmpose/blob/v0.28.1/configs/hand/2d_kpt_sview_rgb_img/topdown_heatmap/coco_wholebody_hand/hrnetv2_coco_wholebody_hand.md) and place it in the MMPose clone directory.

4. Save the script below in this directory and run it.  
```py
import numpy as np

# PyTorch imports
import torch
import torchvision

from mmpose.apis import init_pose_model

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

config_file = 'configs/hand/2d_kpt_sview_rgb_img/topdown_heatmap/coco_wholebody_hand/hrnetv2_w18_coco_wholebody_hand_256x256.py'
checkpoint_file = 'hrnetv2_w18_coco_wholebody_hand_256x256-1c028db7_20210908.pth'
model = init_pose_model(config_file, checkpoint_file, device='cpu') 
model = _convert_batchnorm(model)
model = model.eval()

# implement the forward method
if hasattr(model, 'forward_dummy'):
    model.forward = model.forward_dummy

# We grab the TorchScripted model via tracing
input_shape = [1, 3, 256, 256]
input_data = torch.randn(input_shape)
scripted_model = torch.jit.trace(model, input_data).eval()

scripted_model.save('hrnetv2.pt') # Save
print("Torch model saved to ./hrnetv2.pt")
```
5. Check that `hrnetv2.pt` is generated.

#### Compile pytorch model
1. Copy the `hrnetv2.pt` to the `drp-ai_tvm/tutorials` directory.

2. Change the `addr_map_start` setting in `compile_pytorch_model.py` provided in [Compile Tutorial](../../../../../tutorials) to the following address, depending on the board. 

| Renesas Evaluation Board Kit | Start Address |
|------------------------------|:-------------:|
| RZ/V2L  Evaluation Board Kit | 0x838E0000    |
| RZ/V2M  Evaluation Board Kit | 0xC38E0000    |
| RZ/V2MA Evaluation Board Kit | 0x438E0000    |
 

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
#L107~131
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
        op.Crop(80, 0, 480, 480),
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```

4. Run the script with the command below.  
```sh
# Run DRP-AI TVM[*1] Compiler script
python3 compile_pytorch_model.py \
    ./hrnetv2.pt \
    -o hrnetv2_pt \
    -s 1,3,256,256
```
5. Confirm that three files, `deploy.so`, `deploy.params`, and `deploy.json`, and `preprocess` directory have been created in the `hrnetv2_pt` directory.  
6. Copy the `hrnetv2_pt` directory into the execution environment directory where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

## Processing Details

### DRP-AI mode
- Source Code: [tvm_drpai_hrnet.cpp](../../../src/recognize/hrnet/tvm_drpai_hrnet.cpp). 

The source code is common to that of the HRNet, except for the macro definitions. See [the header file](../../../src/recognize/hrnet/tvm_drpai_hrnet.h).  
Followings are processing details if user selected "HRNetv2 (DRP-AI)". 

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#compile-pytorch-model) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
Inference performs the processing of `hrnetv2_pt` generated in [Model Information](#model-information).  
  
#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
