# Object Detection

### Model: [YOLOv3/YOLOv2/Tiny YOLOv3/Tiny YOLOv2](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [How to compile the model](#how-to-compile-the-model)
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

## How to compile the model
To run the Object Detection, `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx` and `tinyyolov2_onnx` Model Object are required.  
Follow the instructions below to prepare the Model Object.  
- [1. Save ONNX format model](#1-save-onnx-format-model)
- [2. Compile onnx models](#2-compile-onnx-models)

### 1. Save ONNX format model
Here, we use the ONNX format model converted from Darknet weight file by using the scripts provided in this directory.  

#### 1.1 Requirement
To run the script, PyTorch 1.8.0 must be installed. You can install it using:

```sh
pip3 install torch==1.8.0 torchvision==0.9.0
```

#### 1.2 File Configuration
| File | Details |  
|:---|:---|  
|darknet_cfg.py |Darknet cfg file parser.|  
|convert_to_pytorch.py |Darknet-PyTorch conversion script.|  
|convert_to_onnx.py | PyTorch-ONNX conversion script.|  
|yolo.py | Conversion configuration file.|  
|yolo.ini | Conversion configuration file parser. |  

#### 1.3 ONNX conversion
1. Create a working directory and copy all the conversion scripts to it:

```sh
mkdir -p yolo_work
cp how-to/sample_app/docs/object_detection/yolo/*.py yolo_work/
cp how-to/sample_app/docs/object_detection/yolo/yolo.ini yolo_work/
cd yolo_work
```

2. Download the YOLO `*.cfg` file and `*.weights` file from [Darknet](https://pjreddie.com/darknet/yolo/) and place them in the same directory as the conversion scripts:

```sh
# For YOLOv3
wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3.cfg
wget https://pjreddie.com/media/files/yolov3.weights

# For YOLOv2
wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov2-voc.cfg
wget https://pjreddie.com/media/files/yolov2-voc.weights

# For Tiny YOLOv3
wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov3-tiny.cfg
wget https://pjreddie.com/media/files/yolov3-tiny.weights

# For Tiny YOLOv2
wget https://raw.githubusercontent.com/pjreddie/darknet/master/cfg/yolov2-tiny-voc.cfg
wget https://pjreddie.com/media/files/yolov2-tiny-voc.weights
```

3. Update the `yolo.ini` file with the correct paths to your cfg and weights files. If the files are in the current directory, you can use relative paths:

```ini
[yolov3]
cfg     =yolov3.cfg
weights =yolov3.weights
pth     =yolov3.pth
input   =["input1"]
output  =["output1", "output2", "output3"]
onnx    =d-yolov3.onnx

[yolov2]
cfg     =yolov2-voc.cfg
weights =yolov2-voc.weights
pth     =yolov2-voc.pth
input   =["input1"]
output  =["output1"]
onnx    =d-yolov2.onnx

[tinyyolov3]
cfg     =yolov3-tiny.cfg
weights =yolov3-tiny.weights
pth     =yolov3-tiny.pth
input   =["input1"]
output  =["output1", "output2"]
onnx    =d-tinyyolov3.onnx

[tinyyolov2]
cfg     =yolov2-tiny-voc.cfg
weights =yolov2-tiny-voc.weights
pth     =yolov2-tiny-voc.pth
input   =["input1"]
output  =["output1"]
onnx    =d-tinyyolov2.onnx
```

4. Run the following commands to convert the models:

```sh
# For YOLOv3
python3 convert_to_pytorch.py yolov3  
# --> yolov3.pth will be generated.
python3 convert_to_onnx.py yolov3  
# --> d-yolov3.onnx will be generated.

# For YOLOv2
python3 convert_to_pytorch.py yolov2  
# --> yolov2-voc.pth will be generated.
python3 convert_to_onnx.py yolov2  
# --> d-yolov2.onnx will be generated.

# For Tiny YOLOv3
python3 convert_to_pytorch.py tinyyolov3  
# --> yolov3-tiny.pth will be generated.
python3 convert_to_onnx.py tinyyolov3  
# --> d-tinyyolov3.onnx will be generated.

# For Tiny YOLOv2
python3 convert_to_pytorch.py tinyyolov2  
# --> yolov2-tiny-voc.pth will be generated.
python3 convert_to_onnx.py tinyyolov2  
# --> d-tinyyolov2.onnx will be generated.
```

5. Copy the generated ONNX files to the tutorials directory:

```sh
cp d-*.onnx $TVM_ROOT/tutorials/
```

### 2. Compile onnx models
1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Place the onnx file in `$TVM_ROOT/tutorials`.  

3. Reset the compilation script to its original state and then change the pre-processing details as shown below:

```sh
cd $TVM_ROOT/tutorials
git checkout -- compile_onnx_model.py
```

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

You can use the following sed commands to automatically make these changes:

```sh
# Change input format from BGR to YUYV_422
sed -i 's/config\.shape_in\s*=\s*\[1,\s*480,\s*640,\s*3\]/config.shape_in     = [1, 480, 640, 2]/' compile_onnx_model.py
sed -i 's/config\.format_in\s*=\s*drpai_param\.FORMAT\.BGR/config.format_in    = drpai_param.FORMAT.YUYV_422/' compile_onnx_model.py

# Change mean and stdev values
sed -i 's/mean\s*=\s*\[0\.485,\s*0\.456,\s*0\.406\]/mean    = [0.0, 0.0, 0.0]/' compile_onnx_model.py
sed -i 's/stdev\s*=\s*\[0\.229,\s*0\.224,\s*0\.225\]/stdev   = [1.0, 1.0, 1.0]/' compile_onnx_model.py
```

4. Run the script with the command below to compile YOLOv3 model:  
```sh
cd $TVM_ROOT/tutorials
python3 compile_onnx_model.py \
  -i input1 \
  -s 1,3,416,416 \
  -o yolov3_onnx \
  d-yolov3.onnx
```
5. Confirm that `yolov3_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.  
6. Repeat the step for other models to generate following Model Object:  

| Model | Model Object Name | Command |
|:---|:---|:---|
|d-yolov2.onnx |yolov2_onnx| `python3 compile_onnx_model.py -i input1 -s 1,3,416,416 -o yolov2_onnx d-yolov2.onnx` |
|d-tinyyolov3.onnx |tinyyolov3_onnx| `python3 compile_onnx_model.py -i input1 -s 1,3,416,416 -o tinyyolov3_onnx d-tinyyolov3.onnx` |
|d-tinyyolov2.onnx| tinyyolov2_onnx| `python3 compile_onnx_model.py -i input1 -s 1,3,416,416 -o tinyyolov2_onnx d-tinyyolov2.onnx` |

7. Before running the application, make sure to copy the `yolov3_onnx`, `yolov2_onnx`, `tinyyolov3_onnx`, and `tinyyolov2_onnx` directories into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located:

```sh
mkdir -p $TVM_ROOT/tutorials/exe
cp -r yolov3_onnx yolov2_onnx tinyyolov3_onnx tinyyolov2_onnx $TVM_ROOT/tutorials/exe/
```

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