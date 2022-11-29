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
Please refer to [Compile Tutorial](../../../../../tutorials) for more details on compiling model.

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
Please refer to [Compile Tutorial CPU-only deploy mode](../../../../../tutorials/README.md#3-compile-using-cpu-only-deploy-mode) for more details on compiling model.

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
