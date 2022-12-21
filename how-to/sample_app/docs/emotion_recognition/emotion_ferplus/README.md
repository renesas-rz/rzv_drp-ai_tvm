# Emotion Recognition

### Model: [Emotion FERPlus](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

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
4. Change the `addr_map_start` setting in `compile_onnx_model.py` provided in [Compile Tutorial](../../../../../tutorials) to `0x442d0000`.  
Note that the value **must NOT** be default value `0x438E0000` in order to avoid conflict with UltraFace Model Object.
5. Run the  with the command below.  
```sh
$ python3 compile_onnx_model.py \
-i Input3 \
-s 1,1,64,64 \
-o emotion_fp_onnx \
emotion-ferplus-8.onnx
```
6. Confirm that `emotion_fp_onnx` directory is generated and it contains `deploy.json`, `deploy.so` and `deploy.params` files.  
7. Before running the application, make sure to copy the `emotion_fp_onnx` directory and `ultraface_onnx` directory into the execution environment directory `exe` where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  


## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_emotionfp.cpp](../../../src/recognize/emotionfp/tvm_drpai_emotionfp.cpp)  

Followings are processing details if user selected "Emotion FERPlus (DRP-AI)".  

#### Pre-processing
As a pre-processing, Face Detection model, UltraFace, is used.  
To see details, please refer to [Face Detection Processing Details](../../../docs/face_detection/ultraface/README.md#processing-details).  

For each face detected, following pre-processing is done by CPU..  
Note that some of them are processed by C++ OpenCV.

| Function | Details |  
|:---|:---|  
|Crop | Crop YUYV image. Processed by CPU. |  
|cvtColor | C++ OpenCV. Convert YUY2 to Grayscale.|  
|resize |C++ OpenCV. Resize to 64x64.|  
|transpose |Transpose HWC to CHW order. Processed by CPU.|  

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
