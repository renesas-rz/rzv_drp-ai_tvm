# Sample Application for USB Camera
*Updated on 2022/10/18.*

## Overview
This page explains about the sample application for DRP-AI TVM[^1] that uses USB Camera as an input and transfer the result via HTTP to display on HTML.  
Sample application code and its execution environment are provided in this directory.  
This application is for RZ/V2MA Evaluation Board Kit.  

<img src=./img/web.JPG width=500>  

## Hardware Configuration
Hardware configuration of this application is shown in figure below.
This application has the same hardware configuration as USB Camera HTTP version application in RZ/V2MA DRP-AI Sample Application in RZ/V2MA DRP-AI Support Package.  
For more details on hardware configuration, please refer to RZ/V2MA DRP-AI Sample Application Note provided in RZ/V2MA DRP-AI Support Package.  

<img src=./img/hw_conf.png width=700>  



## File Configuration
In this directory, followings are provided.  

| File/Directory | Details |  
|:---|:---|  
|exe/ | Necessary files for target board execution environment. |
|etc/ | Other necessary files, i.e. for client PC. |
|src/ | Application source code. |  
|img/ | Image files for this document. |  
|README.md | This document. |  


### Source Code
In `src` directory, followings are provided.  

| File/Directory | Details |  
|:---|:---|  
|recognize/ | Directory that contains application source code for AI image recognition processing. |
|toolchain/ | Application compilation toolchain. |
|*/ | Other directories that stores the application source code. |  
|CMakeLists.txt |CMake configuration. |  
|main.cpp |Main application source code. |  
|*.cpp |Other C++ source code. |  
|*.h |Other header file.|  

### Execution Environment
In `exe` directory, following files are provided as execution environment to be placed on target board.  

| File/Directory | Details |  
|:---|:---|  
|face_deeppose_pt/ | DeepPose model for DRP-AI mode. |  
|face_deeppose_cpu/ | DeepPose model for CPU mode. |  
|preprocess_tvm_v2ma/ | Pre-processing Runtime Object files. |  
|sample_app_drpai_tvm_usbcam_http | Application itself. |  


In `etc` directory, following files are provided as execution environment to be placed on client PC that displays HTTP result.  

| File/Directory | Details |  
|:---|:---|  
|Websocket_Client/css/ | CSS files for demo page on client PC. |
|Websocket_Client/js/ | Javascript files for demo page on client PC. |
|Websocket_Client/libs/ | Libraries for demo page on client PC. |  
|Websocket_Client/index.html | Demo page HTML file on client PC. |  

## Compile the application
1. Prepare the SDK that includes following packages.
    1. libwebsockets and jansson  
    Please refer to "Compile DRP-AI Sample Application" in RZ/V2MA DRP-AI Sample Application Note provided in RZ/V2MA DRP-AI Support Package.  
    2. opencv  
    Pease refer to [How to install OpenCV to Linux Package](../../apps/README.md#how-to-install-opencv-to-linux-package).  

2. Use the SDK generated in 1 to compile the sample application.  
Please refer to [Application Example](../../apps/README.md#how-to-compile-the-application).  
Please make sure to change the SDK path to the one generated in 1.  

## Run the application
To run the application, please refer to the instruction for USB Camera HTTP version application in RZ/V2MA DRP-AI Sample Application Note provided in RZ/V2MA DRP-AI Support Package.  

## Application Specification
### Model Information
DeepPose: [MMPose Facial Landmark Localization](https://mmpose.readthedocs.io/en/latest/topics/face.html#deeppose-resnet-on-wflw)  
Dataset: [WFLW](https://wywu.github.io/projects/LAB/WFLW.html)  
Input size: 1x3x256x256  
Output size: 1x98x2

### DRP-AI mode
Followings are processing details if user selected "DeepPose (DRP-AI)".  
#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime, which allows following pre-processing on DRP-AI.  

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB.|  
|resize |Resize to 256x256.|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI.|  
|normalize | Normalize pixel values with mean and standard deviation.|  
|transpose | Transpose HWC to CHW order. |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input.|  

#### Inference
The Object files `face_deeppose_pt` is generated from MMPose DeepPose pre-trained model as described in [Model Information](#model-information).  
Please refer to [Compile Tutorial](../../tutorials) for more details on compiling model.

### CPU mode
Followings are processing details if user selected "DeepPose (CPU)".  
#### Pre-processing
CPU pre-processing details are as follows.  
Note that some of them are processed by C++ OpenCV.

| Function | Details |  
|:---|:---|  
|cvtColor | C++ OpenCV. Convert YUY2 to RGB.|  
|resize |C++ OpenCV. Resize to 256x256.|  
|normalize | C++ OpenCV. Normalize pixel values to [0,1]|  
|standardize | Normalize pixel values with mean and standard deviation.|  
|transpose |Transpose HWC to CHW order.|  

#### Inference
The Object files `face_deeppose_cpu` provided in this directory is generated from MMPose DeepPose pre-trained model as described in [Model Information](#model-information) using CPU-only deploy mode.  
Please refer to [Compile Tutorial CPU-only deploy mode](../../tutorials/README.md#3-compile-using-cpu-only-deploy-mode) for more details on compiling model.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
