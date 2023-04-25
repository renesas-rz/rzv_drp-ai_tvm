# AI Sample Application

## Index  
- [Overview](#overview)  
- [Necessary Equipment](#necessary-equipment)
- [Hardware Configuration](#hardware-configuration)
- [File Configuration](#file-configuration)  
    - [Source Code](#source-code)  
    - [Execution Environment](#execution-environment)  
- [Build the application](#build-the-application)  
- [Setup the Execution Environment](#setup-the-execution-environment)  
    - [1. Setup the board](#1-setup-the-board)  
    - [2. Setup the PC filesystem](#2-setup-the-pc-filesystem)  
    - [3. Setup Setup PC Network Environment](#3-setup-pc-network-environment)  
- [Run the application](#run-the-application)  
- [Quit the application](#quit-the-application)  
- [Notes](#notes)
    - [1. IP Address](#1-ip-address)
    - [2. Enqueue _capture buffer Error](#2-enqueue-_capture-buffer-error)
- [Application Specification](#application-specification)  
    - [Model Information](#model-information)  
    - [Mode](#mode)  

## Overview
This page explains about the sample application for DRP-AI TVM[^1] that uses USB Camera as an input and transfer the result via HTTP to display on HTML.  
Sample application code and its execution environment are provided in this directory.  
This application is for **RZ/V2L Evaluation Board Kit**, **RZ/V2M Evaluation Board Kit** and **RZ/V2MA Evaluation Board Kit**.  

<img src=./img/web.JPG width=500>  

## Necessary Equipment  

| Equipment | Details |  
|:---|:---|  
|RZ/V Evaluation Board | Evaluation Board to run DRP-AI. |
|Power Adapter and cable| For RZ/V Evaluation Board. |
|USB Camera| Used to capture the input image. </br>May require type-A to type-C adapter depending on the Evaluation Board.|
|Ethernet cable| Used for ethernet communication with the Evaluation Board. |  
|Serial to micro USB cable| Used for serial communication with the Evaluation Board. |  
|PC | Windows PC to communicate with Evaluation Board via console and to display the inference result on web blowser |  

## Hardware Configuration
Hardware configuration of this application is shown in figure below.  

<img src=./img/hw_conf.png width=700>  

The data flow is shown in figure below.  
<img src=./img/data_flow.png width=700>  

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
**Note that Model Object files (DRP-AI TVM[^1] compile result) are not provided.**

| File/Directory | Details |  
|:---|:---|  
|sample_app_drpai_tvm_usbcam_http | Application itself. |  
|coco-labels-2014_2017.txt | Label list for Object Detection. |  
|synset_words_imagenet.txt | Label list for Classification. |  

In `etc` directory, following files are provided as execution environment to be placed on client PC that displays HTTP result.  

| File/Directory | Details |  
|:---|:---|  
|Websocket_Client/css/ | CSS files for demo page on client PC. |
|Websocket_Client/js/ | Javascript files for demo page on client PC. |
|Websocket_Client/libs/ | Libraries for demo page on client PC. |  
|Websocket_Client/index.html | Demo page HTML file on client PC. |  

## Build the application
1. Prepare the cross compiler that includes following package.
    - opencv  
    Pease refer to [How to install OpenCV to Linux Package](../../apps/README.md#how-to-install-opencv-to-linux-package).  
2. Use the cross compiler generated in 1 to build the sample application.  
Please refer to [Application Example](../../apps/README.md#how-to-build-the-application).  
Please make sure to change the cross compiler path (`$SDK`) to the one generated in 1.  
3. Check that `sample_app_drpai_tvm_usbcam_http` application binary is generated.

## Setup the Execution Environment  
### 1. Setup the board  
1. Copy the `sample_app_drpai_tvm_usbcam_http` application binary to `exe` directory.  

2. Compile the AI models to generate following directories according to the instruction provided in each "How to create Model Object" column.  

| File/Directory | Details | How to create Model Object |  
|:---|:---|:---|  
|face_deeppose_pt/ | DeepPose model for DRP-AI mode. |[Face Landmark Localization](docs/face_landmark_localization/deeppose) |  
|face_deeppose_cpu/ | DeepPose model for CPU mode. |[Face Landmark Localization](docs/face_landmark_localization/deeppose) |  
|yolov3_onnx/ | YOLOv3 model for DRP-AI mode. |[Object Detection](docs/object_detection/yolo) |  
|yolov2_onnx/ | YOLOv2 model for DRP-AI mode. |[Object Detection](docs/object_detection/yolo) |  
|tinyyolov3_onnx/ | Tiny YOLOv3 model for DRP-AI mode. |[Object Detection](docs/object_detection/yolo) |  
|tinyyolov2_onnx/ | Tiny YOLOv2 model for DRP-AI mode. |[Object Detection](docs/object_detection/yolo) |  
|ultraface_onnx/ | UltraFace model for DRP-AI mode.|[Face Detection](docs/face_detection/ultraface) |  
|hrnet_onnx/ | HRNet model for DRP-AI mode.|[Human Pose Estimation](docs/human_pose_estimation/hrnet) |  
|hrnetv2_pt/ | HRNetv2 model for DRP-AI mode. |[Hand Landmark Localization](docs/hand_landmark_localization/hrnetv2) |  
|emotion_fp_onnx/ | Emotion FERPlus model for DRP-AI mode. |[Emotion Recognition](docs/emotion_recognition/emotion_ferplus) |  
|googlenet_onnx/ | GoogleNet model for DRP-AI mode. |[Classification](docs/classification/googlenet) |  

3. Copy the generated model directories to `exe` directory.  
`exe` directory should look like below.  

```sh
exe/
├── face_deeppose_pt/
│   ├── deploy.json
│   ├── deploy.params
│   └── deploy.so
├── face_deeppose_cpu/
│   ... Other Model Object directories ...
│  
├── coco-labels-2014_2017.txt
├── synset_words_imagenet.txt
└── sample_app_drpai_tvm_usbcam_http
```

4. Copy the `exe` directory to the rootfs of Boot Environment.  
The rootfs should look like below.  
**Note that `libtvm_runtime.so` must also be copied to under `/usr/lib64`.**  
To see how where to get `libtvm_runtime.so`, please refer to [Application Example: How to Run the Application](../../apps/README.md#1-copy-to-the-board)
```sh
/
├── usr
│   └── lib64
│       └── libtvm_runtime.so
└── home
    └── root
        └── exe
```

### 2. Setup the PC filesystem  
1. Copy the PC execution environment directory, `etc/Websocket_Client` to any folder in the PC.  
Directory structure of the folder in PC will be as follows.  
```sh
<any_directory>
└── Websocket_Client
    ├── css
    ├── js
    ├── libs
    └── index.html
```

### 3. Setup PC Network Environment  
1. Connect the RZ/V series Evaluation Board and the PC via Ethernet cable as shown in the [Hardware Configuration](#hardware-configuration).  

2. Install the web browser that support websocket protocol.  
Microsoft Edge version (version 101.0.1210.47) / Google Chrome (version 102.0.5005.115) are verified to work with this application.  

3. Set the PC local IP address to be static.  
As default, the PC IP address data should be set to **“192.168.1.10”**.  
**Note:** Both PC local IP address and RZ/V IP address need to be changed if any of those IP address is set to other address than default setting.  
To see how to change the RZ/V IP address, please refer to [Run the application](#run-the-application).

## Run the application
This section assumes that the user has prepared the Boot Environment on the target board.  

1. **(On RZ/V Board)** Move to the directory where the application is stored.  
Note: `<PATH_to_FILE>` is a path to the directory that the application is stored.
For the example, in [1. Setup the board](#1-setup-the-board), the path would be `/home/root/exe`.
```sh
cd <PATH_to_FILE>
```
2. **(On RZ/V Board)** Run the following command to set RZ/V IP address.  
```sh
ifconfig eth0 192.168.1.11
```

3. **(On RZ/V Board)** Run the following command to confirm that PC can be accessed by RZ/V Board.  
```sh
ping 192.168.1.10
```
- If the network connection and IP address setting are correct, the following message will be displayed.
(The time info may be different depending on the environment)
    ```
    64 bytes from 192.168.1.10: seq=0 ttl=128 time=4.392 ms
    64 bytes from 192.168.1.10: seq=1 ttl=128 time=2.332 ms
    ```
- If the network connection or IP address setting is wrong, the message above will not be displayed.  
Please check the IP address setting and the network connection of PC and RZ/V Board.  
- After confirmation, please input `Ctrl+C` to exit.  

4. **(On RZ/V Board)** Run the following command to execute the application.  
```sh
./sample_app_drpai_tvm_usbcam_http
################################################
#    RZ/V DRP-AI TVM Sample Application        #
################################################
Start Server ws://192.168.1.11:3000/ws/
```

5. **(On PC)** Open the directory where the `Websocket_Client` folder is stored.   
Open the `index.html` using the web browser that support websocket.  

6. **(On RZ/V Board)** The following message will be displayed on the console after the `index.html` is opened.
```
Client connected.
```

7. **(On PC)** As shown in the figure below, select the AI model listed.  
Once user selected the AI model, the application in both web browser
side (PC side) and RZ/V side will start to work.  

<img src=./img/run_1.png width=600>  

</br>

8. **(On PC)** During the AI inference, the web page will show the following features.  
    1. USB Camera Capture Image  
    2. Inference Result  
    3. CPU Usagae  
    4. AI Model Selection  
    5. Inference Info  

<img src=./img/run_2.png width=600>  


## Quit the application
To terminate the application, close the webpage of `index.html`.  
The application in web browser side (PC side) and RZ/V Board side will both stop.  

## Notes
### 1. IP Address
The IP address can be changed to other address by modifying the following part in the source code.  
Please make sure the changed IP address can be accessed by PC.
- `src/main.cpp` L81:  
```
ws.start_server("192.168.1.11", "3000", "ws");
```
- `etc/Websocket_Client/js/websocket_demo.js` L22:
```
let socket = new WebSocket('ws://192.168.1.11:3000/ws/');
```

### 2. Enqueue _capture buffer Error
When the web browser is closed to terminate the application following error may appear on the console.  
```sh
[ERROR] Failed to enqueue _capture buffer.
Send application message to web client.[Failed to enqueue _capture buffer.
Restart the application.]

...

********************** END *********************
***** Stop Recognize. *****
<<<<<<<<<<<<<<<<<<<<< Message Thread Terminated >>>>>>>>>>>>>>>>>>
All Finish
```
Since the application has already been terminated, the error does not affect the application.  
Application can be restarted without any special procedures, i.e. reboot the board.  

## Application Specification
### Model Information
Please refer to each AI application page, which can be accessed from [AI Sample Application](../README.md#ai-sample-application).

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
