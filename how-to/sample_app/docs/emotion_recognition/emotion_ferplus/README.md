# Emotion Recognition

### Model: [Emotion FERPlus](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [How to Compile the Model](#how-to-compile-the-model)
  - [Prepare the ONNX Model](#prepare-the-onnx-model)
  - [Compile the ONNX Model](#compile-the-onnx-model)
- [Processing Details](#processing-details)  

## Overview
This page explains about Emotion Recognition in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/emotionfp.jpg width=500>  

## Model Information
- Emotion FERPlus: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/validated/vision/body_analysis/emotion_ferplus) emotion-ferplus-8.onnx  
Dataset:  See [ONNX Model Zoo](https://github.com/onnx/models/tree/main/validated/vision/body_analysis/emotion_ferplus#dataset).  
Input size: 1x1x64x64  
Output size: 1x8  

Emotion FERPlus can only classify the face expression of single person.  
To enable multiple face emotion recognition, this application used [UltraFace](../../../docs/face_detection/ultraface/) as pre-processing.  
To see more details on UltraFace, please see [Face Detection](../../../docs/face_detection/ultraface/).

## How to Compile the Model
To run the Emotion Recognition, `emotion_fp_onnx` Model Object and `ultraface_onnx` Model Object are required. This section explains how to prepare and compile the Emotion FERPlus model.

For `ultraface_onnx` Model Object, please refer to [Face Detection](../../../docs/face_detection/ultraface/).

### Prepare the ONNX Model
1. Create a directory to store the model data:
   ```sh
   mkdir -p model_data/emotion_ferplus
   cd model_data/emotion_ferplus
   ```

2. Download the ONNX model from [ONNX Model Zoo](https://github.com/onnx/models/tree/main/validated/vision/body_analysis/emotion_ferplus):
   ```sh
   wget https://github.com/onnx/models/raw/main/validated/vision/body_analysis/emotion_ferplus/model/emotion-ferplus-8.onnx
   ```

3. Copy the downloaded model to your TVM tutorials directory:
   ```sh
   cp emotion-ferplus-8.onnx $TVM_ROOT/tutorials/
   ```

### Compile the ONNX Model
1. Set the environment variables, i.e. `$TVM_ROOT` etc., according to [Installation](../../../../../setup/).

2. Navigate to the tutorials directory and reset the compilation script to its original state:
   ```sh
   cd $TVM_ROOT/tutorials
   git checkout -- compile_onnx_model.py
   ```

3. Change the pre-processing details as shown below:

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

You can use the following sed commands to automatically make these changes:

```sh
# Change input shape and format
sed -i 's/config\.shape_in\s*=\s*\[1,\s*480,\s*640,\s*3\]/config.shape_in     = [1, 480, 640, 2]/' compile_onnx_model.py
sed -i 's/config\.format_in\s*=\s*drpai_param\.FORMAT\.BGR/config.format_in    = drpai_param.FORMAT.YUYV_422/' compile_onnx_model.py

# Change output format from RGB to GRAY
sed -i 's/config\.format_out\s*=\s*drpai_param\.FORMAT\.RGB/config.format_out   = drpai_param.FORMAT.GRAY/' compile_onnx_model.py

# Remove mean, stdev, r, cof_add, cof_mul and normalization related lines
sed -i '/mean\s*=/d' compile_onnx_model.py
sed -i '/stdev\s*=/d' compile_onnx_model.py
sed -i '/r = 255/d' compile_onnx_model.py
sed -i '/cof_add/d' compile_onnx_model.py
sed -i '/cof_mul/d' compile_onnx_model.py
sed -i '/op\.Normalize/d' compile_onnx_model.py

# Add new crop operation after config.ops = [
sed -i '/config\.ops = \[/a \        #Crop is stated with maximum size to allocate enough memory size.\n        op.Crop(0, 0, config.shape_in[2], config.shape_in[1]),' compile_onnx_model.py
```

4. Run the script with the command below:  
```sh
python3 compile_onnx_model.py \
    -i Input3 \
    -s 1,1,64,64 \
    -o emotion_fp_onnx \
    emotion-ferplus-8.onnx
```

5. Confirm that `emotion_fp_onnx` directory is generated and it contains `deploy.so`, `deploy.params`, and `deploy.json` files and `preprocess` directory.  

6. Before running the application, make sure to copy the `emotion_fp_onnx` directory and `ultraface_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located:
   ```sh
   mkdir -p $TVM_ROOT/tutorials/exe
   cp -r emotion_fp_onnx $TVM_ROOT/tutorials/exe/
   ```

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_emotionfp.cpp](../../../src/recognize/emotionfp/tvm_drpai_emotionfp.cpp)  

Followings are processing details if user selected "Emotion FERPlus (DRP-AI)".  

#### Pre-processing
As a pre-processing, Face Detection model, UltraFace, is used.  
To see details, please refer to [Face Detection Processing Details](../../../docs/face_detection/ultraface/README.md#processing-details).  

Pre-processing, described in the [compile script](#compile-the-onnx-model), is done by DRP-AI Pre-processing Runtime.  
Crop parameters are determined in the application based on the face detection result.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

The pre-processing steps include:
1. Converting YUYV_422 format input to GRAY
2. Cropping the image based on face detection results
3. Resizing the image to the model input size (64x64)

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