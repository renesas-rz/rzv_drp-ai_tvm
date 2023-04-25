# Object Detection

### Model: [YOLOv3/YOLOv2/Tiny YOLOv3/Tiny YOLOv2](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

## Overview
This page explains about Object Detection in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/yolov3.jpg width=500>  

## Model Information
- YOLOv3/Tiny YOLOv3: [Darknet](https://pjreddie.com/darknet/yolo/)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x416x416  
Output1 size: 1x13x13x255  
Output2 size: 1x26x26x255  
(YOLOv3 only) Output3 size: 1x52x52x255   

- YOLOv2/Tiny YOLOv2: [Darknet](https://pjreddie.com/darknet/yolov2/)  
Dataset: [Pascal VOC](http://host.robots.ox.ac.uk/pascal/VOC/index.html)  
Input size: 1x3x416x416  
Output size: 1x13x13x125  


### How to compile the model
To run the Object Detection, `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx` and `tinyyolov2_onnx` Model Object are required.  
Follow the instuction below to prepare the Model Object.  
- [1. Save ONNX format model](#1-save-onnx-format-model)
- [2. Compile onnx models](#2-compile-onnx-models)

#### 1. Save ONNX format model
Here, we use the ONNX format model converted from Darknet weight file by using the scripts provided in this directory.  

##### 1.1 Requirement
To run the script, PyTorch 1.8.0 must be installed.  

##### 1.2 File Configuration
| File | Details |  
|:---|:---|  
|darknet_cfg.py |Darknet cfg file parser.|  
|convert_to_pytorch.py |Darknet-PyTorch conversion script.|  
|convert_to_onnx.py | PyTorch-ONNX conversion script.|  
|yolo.py | Conversion configuration file.|  
|yolo.ini | Conversion configuration file parser. |  

##### 1.3 ONNX conversion
1. Download the YOLO `*.cfg` file and `*.weights` file from [Darknet](https://pjreddie.com/darknet/yolo/) and place them in the same directory of the conversion scripts.  
2. Run the following commands to convert YOLOv3 model.  
```sh
$ python3 convert_to_pytorch.py yolov3  
# --> yolov3.pth will be generated.
$ python3 convert_to_onnx.py yolov3  
# --> d-yolov3.onnx will be generated.
```
3. If you would like to convert other models, specify the parameter instead of `yolov3` according to the following table.  

| Parameter | Model |  
|:---|:---|  
|yolov3 |YOLOv3 (Default)|  
|yolov2 |YOLOv2|  
|tinyyolov3| Tiny YOLOv3|  
|tinyyolov2 | Tiny YOLOv2 |  

#### 2. Compile onnx models
1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the onnx file in `$TVM_HOME/../tutorials`.  
3. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to the following address, depending on the board. 

| Renesas Evaluation Board Kit | Start Address |
|------------------------------|:-------------:|
| RZ/V2L  Evaluation Board Kit | 0x838E0000    |
| RZ/V2M  Evaluation Board Kit | 0xC38E0000    |
| RZ/V2MA Evaluation Board Kit | 0x438E0000    |
  
4. Change the pre-processing details as shown below.  

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
#L105~128
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
    mean    = [0.0, 0.0, 0.0]
    stdev   = [1.0, 1.0, 1.0]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
4. Run the script with the command below to compile YOLOv3 model.  
```sh
$ python3 compile_onnx_model.py \
-i input1 \
-s 1,3,416,416 \
-o yolov3_onnx \
d-yolov3.onnx
```
5. Confirm that `yolov3_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.  
6. Repeat the step for other models to generate following Model Object.  

| Model | Model Object Name |  
|:---|:---|  
|d-yolov2.onnx |yolov2_onnx|  
|d-tinyyolov3.onnx |tinyyolov3_onnx|  
|d-tinyyolov2.onnx| tinyyolov2_onnx|  

7. Before running the application, make sure to copy the `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx`, and `tinyyolov2_onnx` directories into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_yolo.cpp](../../../src/recognize/yolo/tvm_drpai_yolo.cpp)  

Followings are processing details if user selected "YOLOvx (DRP-AI)" or "Tiny YOLOvx (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#2-compile-onnx-models) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
The Object files `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx` and `tinyyolov2_onnx` are generated from Darknet YOLO pre-trained ONNX model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
