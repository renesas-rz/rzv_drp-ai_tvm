# Face Detection

### **Note: In this version (v1.1.0), it is known that the UltraFace application causes an error when compiling. If you want to try UltraFace application, please clone v1.0.4 and try it.**

### Model: [UltraFace](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

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
4. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to the following address, depending on the board.  

| Renesas Evaluation Board Kit | Start Address |
|------------------------------|:-------------:|
| RZ/V2L  Evaluation Board Kit | 0x838E0000    |
| RZ/V2M  Evaluation Board Kit | 0xC38E0000    |
| RZ/V2MA Evaluation Board Kit | 0x438E0000    |
  
5. Change the pre-processing details as shown below.  
Before
```py
#L105~128
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
#L105~127
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
    mean         = [127.0, 127.0, 127.0]
    scale_factor = [1.0/128.0, 1.0/128.0, 1.0/128.0]
    cof_add = [-m for m in mean]
    cof_mul = scale_factor
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
6. Run the script with the command below.  
```sh
$ python3 compile_onnx_model.py \
-i input \
-s 1,3,240,320 \
-o ultraface_onnx \
version-RFB-320.onnx
```
7. Confirm that `ultraface_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.  
8. Before running the application, make sure to copy the `ultraface_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_ultraface.cpp](../../../src/recognize/ultraface/tvm_drpai_ultraface.cpp)  

Followings are processing details if user selected "UltraFace (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#how-to-compile-the-model) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
The Object files `ultraface_onnx` is generated from ONNX Model Zoo pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
