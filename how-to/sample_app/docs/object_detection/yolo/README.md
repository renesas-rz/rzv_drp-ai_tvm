# Object Detection

### Model: [YOLOv3/YOLOv2/Tiny YOLOv3/Tiny YOLOv2](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

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

### ONNX format model
Here, we use the ONNX format model converted from Darknet weight file by using the scripts provided in this directory.  

#### Requirement
To run the script, PyTorch 1.8.0 must be installed.  

#### File Configuration
| File | Details |  
|:---|:---|  
|darknet_cfg.py |Darknet cfg file parser.|  
|convert_to_pytorch.py |Darknet-PyTorch conversion script.|  
|convert_to_onnx.py | PyTorch-ONNX conversion script.|  
|yolo.py | Conversion configuration file.|  
|yolo.ini | Conversion configuration file parser. |  

#### ONNX conversion
Download the YOLO *.cfg file and *.weights file from [Darknet](https://pjreddie.com/darknet/yolo/) and place them in the same directory of the conversion scripts.  
Run the following commands to convert YOLOv3 model.  
```sh
$ python3 convert_to_pytorch.py yolov3  
# --> yolov3.pth will be generated.
$ python3 convert_to_onnx.py yolov3  
# --> d-yolov3.onnx will be generated.
```
If you would like to convert other models, specify the parameter instead of `yolov3` according to the following table.  

| Parameter | Model |  
|:---|:---|  
|yolov3 |YOLOv3 (Default)|  
|yolov2 |YOLOv2|  
|tinyyolov3| Tiny YOLOv3|  
|tinyyolov2 | Tiny YOLOv2 |  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_yolo.cpp](../../../src/recognize/yolo/tvm_drpai_yolo.cpp)  

Followings are processing details if user selected "YOLOvx (DRP-AI)" or "Tiny YOLOvx (DRP-AI)".  

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB.|  
|resize |Resize to 416x416.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI.|  
|normalize | Normalize pixel values with mean and standard deviation.|  
|transpose | Transpose HWC to CHW order. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input.|  

#### Inference
The Object files `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx` and `tinyyolov2_onnx` are generated from Darknet YOLO pre-trained ONNX model as described in [Model Information](#model-information).  
Please refer to [Compile Tutorial](../../../../../tutorials) for more details on compiling model.

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
