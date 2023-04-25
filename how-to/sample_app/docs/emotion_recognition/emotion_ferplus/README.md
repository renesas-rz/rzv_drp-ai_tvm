# Emotion Recognition

### **Note: In this version (v1.1.0), it is known that this Emotion Recognition application causes an error when executing. Because it uses UltraFace. If you want to try this application, please clone v1.0.4 and try it.**

### Model: [Emotion FERPlus](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [Processing Details](#processing-details)  

## Overview
This page explains about Emotion Recognition in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/emotionfp.jpg width=500>  

## Model Information
- Emotion FERPlus: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/emotion_ferplus) emotion-ferplus-8.onnx  
Dataset:  See [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/emotion_ferplus#dataset).  
Input size: 1x1x64x64  
Output size: 1x8  

Emotion FERPlus can only classify the face expression of single person.  
To enable multiple face emotion recognition, this application used [UltraFace](../../../docs/face_detection/ultraface/) as pre-processing.  
To see more details on UltraFace, please see [Face Detection](../../../docs/face_detection/ultraface/).


### How to compile the model
To run the Emotion Recognition, `emotion_fp_onnx` Model Object and `ultraface_onnx` Model Object are required.  
Follow the instuction below to prepare the `emotion_fp_onnx` Model Object.  
For `ultraface_onnx` Model Object, please refer to [Face Detection](../../../docs/face_detection/ultraface/).


1. Set the environment variables, i.e. `$TVM_HOME` etc., according to [Installation](../../../../../setup/).  
2. Download the onnx file from [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/body_analysis/emotion_ferplus).  
3. Place the onnx file in `$TVM_HOME/../tutorials`.
4. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to the following address, depending on the board. 

| Renesas Evaluation Board Kit | Start Address |
|------------------------------|:-------------:|
| RZ/V2L  Evaluation Board Kit | 0x842E0000    |
| RZ/V2M  Evaluation Board Kit | 0xC42D0000    |
| RZ/V2MA Evaluation Board Kit | 0x442D0000    |
  
Note that the value **must NOT** be default value, which is used in the other AI models, in order to avoid conflict with UltraFace Model Object.  

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
#L105~124
    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 2]
    config.format_in    = drpai_param.FORMAT.YUYV_422
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8
    
    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.GRAY
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32 
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.
    
    # 4.1.3. Define operators to be run.
    config.ops = [
        #Crop is stated with maximum size to allocate enough memory size.
        op.Crop(0, 0, config.shape_in[2], config.shape_in[1]),
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
    ]
```
6. Run the script with the command below.  
```sh
$ python3 compile_onnx_model.py \
-i Input3 \
-s 1,1,64,64 \
-o emotion_fp_onnx \
emotion-ferplus-8.onnx
```
7. Confirm that `emotion_fp_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files and `preprocess` directory.  
8. Before running the application, make sure to copy the `emotion_fp_onnx` directory and `ultraface_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_emotionfp.cpp](../../../src/recognize/emotionfp/tvm_drpai_emotionfp.cpp)  

Followings are processing details if user selected "Emotion FERPlus (DRP-AI)".  

#### Pre-processing
As a pre-processing, Face Detection model, UltraFace, is used.  
To see details, please refer to [Face Detection Processing Details](../../../docs/face_detection/ultraface/README.md#processing-details).  

Pre-processing, described in the [compile script](#how-to-compile-the-model), is done by DRP-AI Pre-processing Runtime.  
Crop parameters are determined in the application based on the face detection result.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

#### Inference
The Object files `emotion_fp_onnx` is generated from ONNX Model Zoo Emotion FERPlus pre-trained model as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.


#### About processing time
Details of processing time, which is displayed on web browser, are as follow.  

| Processing | Details |  
|:---|:---|  
|Pre-processing | Sum of time taken for following operations. </br>- Face Detection pre-processing, inference and postprocessing</br>- Emotion recognition pre-processing for all detected faces. |  
|Inferene | Time taken to run inference for all detected faces.|  
|Post-processing |Time taken to run post-processing for all detected faces.|  

For example, if there are two bounding box detected in face detection, emotion recognition will be carried out for two times.  
Therefore, inference time will be approximately two times by single inference processing time and same applies for other processing time.  

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
