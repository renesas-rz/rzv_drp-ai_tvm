# Face Landmark Localization

### Model: [DeepPose](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

## Overview
This page explains about Facial Landmark Localization in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/web.JPG width=500>  

## Model Information
DeepPose: [MMPose Facial Landmark Localization](https://mmpose.readthedocs.io/en/latest/topics/face.html#deeppose-resnet-on-wflw)  
Dataset: [WFLW](https://wywu.github.io/projects/LAB/WFLW.html)  
Input size: 1x3x256x256  
Output size: 1x98x2


### How to compile the model
To run the Face Landmark Localization, `face_deeppose_pt` Model Object is required for DRP-AI mode and `face_deeppose_cpu` is required for CPU mode.  
#### Operating Environment
- mmcv-full v1.6.1  
- MMPose v0.28.1  

#### 1. Save the AI model from MMPose
Follow the instuction below to prepare the DeepPose model.  

1. Prepare the save script as below in the MMPose clone directory.
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

config_file = 'configs/face/2d_kpt_sview_rgb_img/deeppose/wflw/res50_wflw_256x256.py'
checkpoint_file = 'deeppose_res50_wflw_256x256-92d0ba7f_20210303.pth'
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

scripted_model.save('deeppose.pt')# Save
print("Torch model saved to ./deeppose.pt")
```
2. Download the checkpoint file(`.pth`) from [the mmpose website](https://mmpose.readthedocs.io/en/latest/topics/face.html#deeppose-resnet-on-wflw) and place them in the same directory as the save script above.  
3. Run the save script and confirm that `deeppose.pt` is generated.  

#### 2. Compile pytorch model for DRP-AI mode
Follow the instuction below to prepare the `face_deeppose_pt` Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the `deeppose.pt` file in `$TVM_HOME/../tutorials`.
3. Change the `addr_map_start` setting in `compile_pytorch_model.py` provided in [Compile Tutorial](../../../../../tutorials) to `0x438E0000`.  
4. Run the script with the command below.  
```sh
$ python3 compile_pytorch_model.py \
-s 1,3,256,256 \
-o face_deeppose_pt \
deeppose.pt
```
5. Confirm that `face_deeppose_pt` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
6. Before running the application, make sure to copy the `face_deeppose_pt` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


#### 3. Compile pytorch model for CPU mode
Follow the instuction below to prepare the `face_deeppose_cpu` Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the `deeppose.pt` file in `$TVM_HOME/../tutorials`.
3. Copy and rename the script `compile_cpu_only_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) as shown below.  
```sh
$ cp compile_cpu_only_onnx_model.py compile_cpu_only_pytorch_model.py
```
4. Change the `compile_cpu_only_pytorch_model.py` script as shown below.  
**Note that this script is only for DeepPose CPU mode and not guaranteed for other models.**  

Before
```py
#L23
import onnx

#L59~66
    # 2. Load onnx model and set input shape.
    shape_dict = {input_name: input_shape}
    # 2.1 Load onnx model
    onnx_model = onnx.load_model(model_file)
    # 2.2 Set input data information

    # 3.1 Run TVM Frontend
    mod, params = tvm.relay.frontend.from_onnx(onnx_model, shape_dict)
```
After
```py
#L23
import torch

#L59~68
    # 2. Load model and set input shape.
    # 2.1 Load model
    model = torch.jit.load(model_file)
    model.eval()
    # 2.2 Set input data information
    input_name = "input0"
    shape_list = [(input_name, opts["input_shape"])]

    # 3.1 Run TVM Frontend
    mod, params = tvm.relay.frontend.from_pytorch(model, shape_list)
```
5. Run the script with the command below.  
```sh
$ python3 compile_cpu_only_pytorch_model.py \
-s 1,3,256,256 \
-o face_deeppose_cpu \
deeppose.pt
```
6. Confirm that `face_deeppose_cpu` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
7. Before running the application, make sure to copy the `face_deeppose_cpu` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_deeppose.cpp](../../../src/recognize/deeppose/tvm_drpai_deeppose.cpp)  

Followings are processing details if user selected "DeepPose (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB.|  
|resize |Resize to 256x256.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI.|  
|normalize | Normalize pixel values with mean and standard deviation.|  
|transpose | Transpose HWC to CHW order. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input.|  

#### Inference
The Object files `face_deeppose_pt` is generated from MMPose DeepPose pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

### CPU mode
- Source Code: [tvm_cpu_deeppose.cpp](../../../src/recognize/deeppose/tvm_cpu_deeppose.cpp)  

Followings are processing details if user selected "DeepPose (CPU)".  
#### Pre-processing
CPU pre-processing details are as follows.  
Note that some of them are processed by C++ OpenCV.

| Function | Details |  
|:---|:---|  
|cvtColor | C++ OpenCV. Convert YUY2 to RGB.|  
|resize |C++ OpenCV. Resize to 256x256.|  
|normalize | C++ OpenCV. Normalize pixel values to [0,1]|  
|standardize | Normalize pixel values with mean and standard deviation.|  
|transpose |Transpose HWC to CHW order.|  

#### Inference
The Object files `face_deeppose_cpu` provided in this directory is generated from MMPose DeepPose pre-trained model as described in [Model Information](#model-information) using CPU-only deploy mode.  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
