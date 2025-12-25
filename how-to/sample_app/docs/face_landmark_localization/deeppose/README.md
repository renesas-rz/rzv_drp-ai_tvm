# Face Landmark Localization

### Model: [DeepPose](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [How to compile the model](#how-to-compile-the-model)
- [Processing Details](#processing-details)  

## Overview
This page explains about Facial Landmark Localization in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/web.JPG width=500>  

## Model Information
DeepPose: [MMPose Facial Landmark Localization](https://github.com/open-mmlab/mmpose/blob/v0.28.1/configs/face/2d_kpt_sview_rgb_img/deeppose/wflw/resnet_wflw.md)  
Dataset: [WFLW](https://wywu.github.io/projects/LAB/WFLW.html)  
Input size: 1x3x256x256  
Output size: 1x98x2

## How to compile the model
To run the Face Landmark Localization, `face_deeppose_pt` Model Object is required for DRP-AI mode and `face_deeppose_cpu` is required for CPU mode.  

### Operating Environment
- Python 3.8 or later
- numpy==1.23.5 (required for MMPose)
- mmcv-full==1.6.1  
- MMPose v0.28.1
- Additional dependencies: json_tricks, opencv-python, matplotlib, munkres

### 1. Save the AI model from MMPose
Follow the instructions below to prepare the DeepPose model.  

1. Install the required dependencies:

```bash
pip3 uninstall -y numpy
pip3 install numpy==1.23.5
pip3 uninstall -y xtcocotools
pip3 install xtcocotools
pip3 install json_tricks opencv-python matplotlib munkres
pip3 uninstall -y mmcv-full
pip3 install mmcv-full==1.6.1
```

2. Clone and install MMPose:

```bash
git clone -b v0.28.1 https://github.com/open-mmlab/mmpose.git
cd mmpose
pip3 install -e . --no-deps
```

3. Prepare the save script as below in the MMPose clone directory.
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

4. Download the checkpoint file(`.pth`) from [the mmpose website](https://github.com/open-mmlab/mmpose/blob/v0.28.1/configs/face/2d_kpt_sview_rgb_img/deeppose/wflw/resnet_wflw.md) and place it in the same directory as the save script above.  

```bash
wget https://download.openmmlab.com/mmpose/face/deeppose/deeppose_res50_wflw_256x256-92d0ba7f_20210303.pth
```

5. Run the save script and confirm that `deeppose.pt` is generated.  

```bash
python3 save_model.py
```

6. Copy the generated model to the tutorials directory:

```bash
cp deeppose.pt $TVM_ROOT/tutorials/
```

7. If you need to restore the original numpy version after MMPose processing:

```bash
# Store your current numpy version before changing it
ORIGINAL_NUMPY_VERSION=$(pip3 show numpy | grep -E "^Version:" | cut -d' ' -f2)

# After MMPose processing is complete
pip3 uninstall -y numpy
pip3 install numpy==$ORIGINAL_NUMPY_VERSION
```

### 2. Compile pytorch model for DRP-AI mode
Follow the instructions below to prepare the `face_deeppose_pt` Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the `deeppose.pt` file in `$TVM_ROOT/tutorials`.
3. Reset the compilation script to its original state:

```bash
# If using git repository
git checkout -- $TVM_ROOT/tutorials/compile_pytorch_model.py
```

4. Change the pre-processing details in `compile_pytorch_model.py` as shown below.  

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

You can try using the following sed commands to automatically make these changes:

```bash
# Make sure you can revert changes if needed
cd $TVM_ROOT/tutorials
# Reset the compilation script to its original state
git checkout -- compile_pytorch_model.py

# Try to automatically replace the configuration
sed -i 's/config\.shape_in\s*=\s*\[1,\s*480,\s*640,\s*3\]/config.shape_in     = [1, 480, 640, 2]/' "$TVM_ROOT/tutorials/compile_pytorch_model.py"
sed -i 's/config\.format_in\s*=\s*drpai_param\.FORMAT\.BGR/config.format_in    = drpai_param.FORMAT.YUYV_422/' "$TVM_ROOT/tutorials/compile_pytorch_model.py"
```

5. Run the script with the command below.  
```sh
cd $TVM_ROOT/tutorials
python3 compile_pytorch_model.py \
  -s 1,3,256,256 \
  -o face_deeppose_pt \
  deeppose.pt
```
6. Confirm that `face_deeppose_pt` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.  

7. Before running the application, make sure to copy the `face_deeppose_pt` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

```bash
mkdir -p $TVM_ROOT/tutorials/exe
cp -r face_deeppose_pt $TVM_ROOT/tutorials/exe/
```

### 3. Compile pytorch model for CPU mode
Follow the instructions below to prepare the `face_deeppose_cpu` Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the `deeppose.pt` file in `$TVM_ROOT/tutorials`.
3. Copy and rename the script `compile_cpu_only_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) as shown below.  
```sh
cd $TVM_ROOT/tutorials
# Reset the compilation script to its original state
git checkout -- compile_cpu_only_onnx_model.py
cp compile_cpu_only_onnx_model.py compile_cpu_only_pytorch_model.py
```
4. Change the `compile_cpu_only_pytorch_model.py` script as shown below.  
**Note that this script is only for DeepPose CPU mode and not guaranteed for other models.**  

Before
```py
#L24
import onnx

#L70~77
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
#L24
import torch

#L70~79
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

You can try using the following sed commands to automatically make these changes:

```bash
# Reset to original state if needed
cd $TVM_ROOT/tutorials
git checkout -- compile_cpu_only_pytorch_model.py

# Modify the import statement
sed -i 's/import onnx/import torch/' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"

# Modify specific parts of the model loading section
sed -i 's/# 2\. Load onnx model and set input shape\./# 2. Load model and set input shape./' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
sed -i 's/shape_dict = {input_name: input_shape}/model = torch.jit.load(model_file)\n    model.eval()\n    # 2.2 Set input data information\n    input_name = "input0"\n    shape_list = [(input_name, opts["input_shape"])]/' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
sed -i 's/# 2\.1 Load onnx model/# 2.1 Load model/' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
sed -i '/onnx_model = onnx\.load_model(model_file)/d' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
sed -i 's/mod, params = tvm\.relay\.frontend\.from_onnx(onnx_model, shape_dict)/mod, params = tvm.relay.frontend.from_pytorch(model, shape_list)/' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
```

5. Delete the DRP-AI Pre-processing Runtime statements shown below in `compile_cpu_only_pytorch_model.py`.  
```py
#L119~145
    # 4. Compile pre-processing using DRP-AI Pre-processing Runtime
    # 4.1. Define the pre-processing data
    config = preruntime.Config()
...
    
    # 4.2. Run DRP-AI Pre-processing Runtime
    preruntime.PreRuntime(config, output_dir+"/preprocess", PRODUCT)
```

You can try using the following sed command to automatically remove this section:

```bash
sed -i '/# 4\. Compile pre-processing using DRP-AI Pre-processing Runtime/,/preruntime\.PreRuntime(config, output_dir+"\/preprocess", PRODUCT)/ d' "$TVM_ROOT/tutorials/compile_cpu_only_pytorch_model.py"
```

6. Run the script with the command below.  
```sh
cd $TVM_ROOT/tutorials
python3 compile_cpu_only_pytorch_model.py \
  -s 1,3,256,256 \
  -o face_deeppose_cpu \
  deeppose.pt
```
7. Confirm that `face_deeppose_cpu` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
8. Before running the application, make sure to copy the `face_deeppose_cpu` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

```bash
cp -r face_deeppose_cpu $TVM_ROOT/tutorials/exe/
```

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_deeppose.cpp](../../../src/recognize/deeppose/tvm_drpai_deeppose.cpp)  

Followings are processing details if user selected "DeepPose (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#2-compile-pytorch-model-for-drp-ai-mode) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

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