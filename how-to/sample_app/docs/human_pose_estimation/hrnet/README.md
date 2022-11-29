# Human Pose Estimation

### Model: [HRNet(High-Resolution Network)](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

## Overview
This page explains about Human Pose Estimation in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/hrnet.jpg width=500>  

## Model Information
- HRNet: [MMpose Topdown Heatmap + Hrnet on Coco (W32)](https://mmpose.readthedocs.io/en/latest/papers/algorithms.html#topdown-heatmap-hrnet-on-coco)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x256x192  
Output size: 1x17x64x48  
  
### ONNX format model
 Here, we follow this [tutorial](https://mmpose.readthedocs.io/en/latest/tutorials/5_export_model.html#prerequisite) to convert HRNet model provided by MMPose into ONNX format. The MMpose version we checked is v0.26.0.    

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_hrnet.cpp](../../../src/recognize/hrnet/tvm_drpai_hrnet.cpp)  

Followings are processing details if user selected "HRNet (DRP-AI)". 

#### Pre-processing
First, crop process is executed by CPU. Then pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|crop |Crop image processed by CPU.| 
|conv_yuv2rgb |Convert YUY2 to RGB processed by DRP-AI Pre-processing Runtime.|  
|resize |Resize to 256x192 processed by DRP-AI Pre-processing Runtime.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI processed by DRP-AI Pre-processing Runtime.|  
|normalize | Normalize pixel values with mean and standard deviation processed by DRP-AI Pre-processing Runtime.|  
|transpose | Transpose HWC to CHW order processed by DRP-AI Pre-processing Runtime. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input processed by DRP-AI Pre-processing Runtime.|  

#### Inference
The Object files `hrnet_onnx` are generated from HRNet pre-trained model provided by MMPose framework as described in [Model Information](#model-information).  
Please refer to [Compile Tutorial](../../../../../tutorials) for more details on compiling model.

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
