# Face Detection

### Model: [UltraFace](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

## Overview
This page explains about Face Detection in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/ultraface.jpg width=500>  

## Model Information
UltraFace: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/ultraface)  
Dataset: See [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/ultraface#dataset)  
Input size: 1x3x240x320  
Output size: 1x4420x2, 1x4420x4


### How to compile the model
To run the Face Detection, `ultraface_onnx` Model Object is required.  
Follow the instuction below to prepare the `ultraface_onnx` Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Download the onnx file from [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/ultraface).  
3. Place the onnx file in `$TVM_HOME/../tutorials`.
4. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to `0x438E0000`.  
5. Run the  with the command below.  
```sh
$ python3 compile_onnx_model.py \
-i input \
-s 1,3,240,320 \
-o ultraface_onnx \
version-RFB-320.onnx
```
6. Confirm that `ultraface_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
7. Before running the application, make sure to copy the `ultraface_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_ultraface.cpp](../../../src/recognize/ultraface/tvm_drpai_ultraface.cpp)  

Followings are processing details if user selected "UltraFace (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB.|  
|resize |Resize to 320x240.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI.|  
|normalize | Normalize pixel values with mean value 127 and scale factor 1.0/128.|  
|transpose | Transpose HWC to CHW order. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input.|  

#### Inference
The Object files `ultraface_onnx` is generated from ONNX Model Zoo pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
