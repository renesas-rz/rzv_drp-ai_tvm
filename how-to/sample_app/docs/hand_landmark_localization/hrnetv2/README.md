# Hand Landmark Localization

### Model: [HRNet(High-Resolution Network) v2](#model-information)
Sample application code are provided in **[here](../../../../sample_app)**.  

## Overview
This page explains about Hand Landmark Localization in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/hrnetv2.jpg width=500>  

## Model Information
- HRNetv2: [MMPose Topdown Heatmap + Hrnetv2 + Coco + Wholebody on Coco_wholebody_hand](https://mmpose.readthedocs.io/en/latest/papers/backbones.html#topdown-heatmap-hrnetv2-coco-wholebody-on-coco-wholebody-hand)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x256x256  
Output size: 1x21x64x64    

### How to compile the model
#### Operating Environment
- mmcv-full v1.6.1  
- MMPose v0.28.1  

#### Save the AI model from MMPose
1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Clone mmpose repository.  

```sh
git clone -b v0.28.1 https://github.com/open-mmlab/mmpose.git
```

3. Download the checkpoint file from [the mmpose website](https://mmpose.readthedocs.io/en/latest/papers/backbones.html#topdown-heatmap-hrnetv2-coco-wholebody-on-coco-wholebody-hand) and place it in the MMPose clone directory.

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
5. Copy the hrnetv2.pt to the `drp-ai_tvm/tutorials` directory and run the sample script [compile_pytorch_model.py](../../../../../tutorials/compile_pytorch_model.py) as shown below.  
Make sure to change the `addr_map_start` setting in `compile_pytorch_model.py` to `0x438E0000`.  
The output directory name `hrnetv2_pt` is used in the sample application and should not be changed.  
Confirm that three files, deploy.so, deploy.params, and deploy.json, have been created in the `hrnetv2_pt` directory.  

```sh
# Run DRP-AI TVM[*1] Compiler script
python3 compile_pytorch_model.py \
    ./hrnetv2.pt \
    -o hrnetv2_pt \
    -s 1,3,256,256
```
 
6. Copy the `hrnetv2_pt` directory into the execution environment directory where the compiled sample application sample_app_drpai_tvm_usbcam_http is located.  

## Processing Details

### DRP-AI mode
- Source Code: [tvm_drpai_hrnet.cpp](../../../src/recognize/hrnet/tvm_drpai_hrnet.cpp). 

The source code is common to that of the HRNet, except for the macro definitions. See [the header file](../../../src/recognize/hrnet/tvm_drpai_hrnet.h).
Followings are processing details if user selected "HRNetv2 (DRP-AI)". 

#### Pre-processing
First, crop process is executed by CPU. Then pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|crop |Crop image processed by CPU.| 
|conv_yuv2rgb |Convert YUY2 to RGB processed by DRP-AI Pre-processing Runtime.|  
|resize |Resize to 256x256 processed by DRP-AI Pre-processing Runtime.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI processed by DRP-AI Pre-processing Runtime.|  
|normalize | Normalize pixel values with mean and standard deviation processed by DRP-AI Pre-processing Runtime.|  
|transpose | Transpose HWC to CHW order processed by DRP-AI Pre-processing Runtime. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input processed by DRP-AI Pre-processing Runtime.|  

#### Inference
Inference performs the processing of `hrnetv2_pt` generated in [Model Information](#model-information).  
  
#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
