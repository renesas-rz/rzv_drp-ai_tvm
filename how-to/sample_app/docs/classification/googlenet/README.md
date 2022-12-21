# Classification

### Model: [GoogleNet](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

## Overview
This page explains about Classification in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/googlenet.jpg width=500>  

## Model Information
- GoogleNet: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/classification/inception_and_googlenet/googlenet) googlenet-9.onnx  
Dataset:  [ILSVRC2014](https://image-net.org/challenges/LSVRC/2014/)  
Input size: 1x3x224x224  
Output size: 1x1000  

### How to compile the model
To run the Classification, `googlenet_onnx` Model Object is required.  
Follow the instuction below to prepare the Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Download the onnx file from [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/classification/inception_and_googlenet/googlenet).  
3. Place the onnx file in `$TVM_HOME/../tutorials`.
4. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to `0x438E0000`.  
5. Run the  with the command below.  
```sh
$ python3 compile_onnx_model.py \
-i data_0 \
-s 1,3,224,224 \
-o googlenet_onnx \
googlenet-9.onnx
```
6. Confirm that `googlenet_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
7. Before running the application, make sure to copy the `googlenet_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_googlenet.cpp](../../../src/recognize/googlenet/tvm_drpai_googlenet.cpp)  

Followings are processing details if user selected "GoogleNet (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime and CPU.  

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB processed by DRP-AI Pre-processing Runtime.|  
|resize |Resize to 224x224 processed by DRP-AI Pre-processing Runtime.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI processed by DRP-AI Pre-processing Runtime.|  
|normalize | Normalize pixel values with mean values of {123.68, 116.779, 103.939}</br>processed by DRP-AI Pre-processing Runtime.|  
|transpose | Transpose HWC to CHW order processed by DRP-AI Pre-processing Runtime. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input</br> processed by DRP-AI Pre-processing Runtime.|  
|rgb2bgr | Convert RGB to BGR processed by CPU.|  

#### Inference
The Object files `googlenet_onnx` is generated from ONNX Model Zoo GoogleNet pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
