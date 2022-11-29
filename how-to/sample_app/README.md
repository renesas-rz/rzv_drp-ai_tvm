# AI Sample Application

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
|docs/ | Documentation for AI details. |  
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
|yolov3_onnx/ | YOLOv3 model for DRP-AI mode. |  
|yolov2_onnx/ | YOLOv2 model for DRP-AI mode. |  
|tinyyolov3_onnx/ | Tiny YOLOv3 model for DRP-AI mode. |  
|tinyyolov2_onnx/ | Tiny YOLOv2 model for DRP-AI mode. |  
|hrnet_onnx/ | HRNet model for DRP-AI mode. |  
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
Please refer to [AI Sample Application](../README.md#ai-sample-application) for model information.

### Mode
User can select AI models and DRP-AI/CPU mode.  
Note that some models only supports DRP-AI mode.  

#### DRP-AI mode
Pre-processing and inference are processed on DRP-AI and CPU.  
Post-processing is processed on CPU.  

#### CPU mode
All processing runs on CPU.  

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
