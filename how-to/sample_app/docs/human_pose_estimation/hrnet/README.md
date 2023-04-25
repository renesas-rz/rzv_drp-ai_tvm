# Human Pose Estimation

### Model: [HRNet(High-Resolution Network)](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

## Overview
This page explains about Human Pose Estimation in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/hrnet.jpg width=500>  

## Model Information
- HRNet: [MMpose Topdown Heatmap + Hrnet on Coco (W32)](https://github.com/open-mmlab/mmpose/blob/v0.26.0/configs/body/2d_kpt_sview_rgb_img/topdown_heatmap/coco/hrnet_coco.md)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x256x192  
Output size: 1x17x64x48  

### How to compile the model
To run the Human Pose Estimation, `hrnet_onnx` Model Object is required.  
- [Operating Environment](#operating-environment)
- [Compile onnx model](#compile-onnx-model)

#### Operating Environment
- mmcv-full v1.5.1  
- MMPose v0.26.0  

#### Compile onnx model
Follow the instuction below to prepare the HRNet Model Object.  

1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Please execute the following command after setting the environment of MMPose to convert HRNet model provided by MMPose into ONNX format. 
```sh
wget https://download.openmmlab.com/mmpose/top_down/hrnet/hrnet_w32_coco_256x192-c78dce93_20200708.pth
python3 tools/deployment/pytorch2onnx.py configs/body/2d_kpt_sview_rgb_img/topdown_heatmap/coco/hrnet_w32_coco_256x192.py hrnet_w32_coco_256x192-c78dce93_20200708.pth
```
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
#L105~129
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
        op.Crop(184, 0, 270, 480),
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
5. Run the script with the command below.  
```sh
# Run DRP-AI TVM[*1] Compiler script
$ python3 compile_onnx_model.py \
./hrnet.onnx \
-o hrnet_onnx \
-s 1,3,256,192 \
-i input.1
```
4. Confirm that three files, `deploy.so`, `deploy.params`, and `deploy.json`, and `preprocess` directory have been created in the `hrnet_onnx` directory.  
5. Copy the `hrnet_onnx` directory into the execution environment directory where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_hrnet.cpp](../../../src/recognize/hrnet/tvm_drpai_hrnet.cpp)  

Followings are processing details if user selected "HRNet (DRP-AI)". 

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#compile-onnx-model) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
The Object files `hrnet_onnx` are generated from HRNet pre-trained model provided by MMPose framework as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
