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

### How to compile the model
To run the Human Pose Estimation, `hrnet_onnx` Model Object is required.  

#### Operating Environment
- mmcv-full v1.5.1  
- MMPose v0.26.0  

#### Compile onnx model
Follow the instuction below to prepare the HRNet Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Follow the [MMPose tutorial](https://mmpose.readthedocs.io/en/latest/tutorials/5_export_model.html#prerequisite) to convert HRNet model provided by MMPose into ONNX format. 
3. Next, run the sample script [compile_onnx_model.py](../../../../../tutorials/compile_onnx_model.py) as shown below.  
Make sure to change the `addr_map_start` setting in `compile_onnx_model.py` to `0x438E0000`.  
The output directory name `hrnet_onnx` is used in the sample application and should not be changed.  

```sh
# Run DRP-AI TVM[*1] Compiler script
$ python3 compile_onnx_model.py \
./hrnet.onnx \
-o hrnet_onnx \
-s 1,3,256,192 \
-i input.1
```
4. Confirm that three files, deploy.so, deploy.params, and deploy.json, have been created in the `hrnet_onnx` directory.  
5. Copy the `hrnet_onnx` directory into the execution environment directory where the compiled sample application sample_app_drpai_tvm_usbcam_http is located.  

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

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
