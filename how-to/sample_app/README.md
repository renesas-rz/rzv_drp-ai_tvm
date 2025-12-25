# AI Sample Application for RZ/V2M, V2L, V2MA

## Important Notice
> **CAUTION**: Compatibility issues have been observed with DRP-AI TVM[^1] v2.7.0 and the latest Linux packages. For stable operation of this sample application, it is strongly recommended to use DRP-AI TVM[^1] v2.6.0 or earlier with Linux package v3.0.6.

## Index  
- [AI Sample Application for RZ/V2M, V2L, V2MA](#ai-sample-application-for-rzv2m-v2l-v2ma)
  - [Important Notice](#important-notice)
  - [Index](#index)
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
    - [3. Setup PC Network Environment](#3-setup-pc-network-environment)
  - [Run the application](#run-the-application)
  - [Quit the application](#quit-the-application)
  - [Notes](#notes)
    - [1. IP Address](#1-ip-address)
    - [2. Enqueue \_capture buffer Error](#2-enqueue-_capture-buffer-error)
    - [3. Special Notes for RZ/V2M, V2L, V2MA](#3-special-notes-for-rzv2m-v2l-v2ma)
      - [Version Compatibility](#version-compatibility)
      - [Model-Specific Considerations](#model-specific-considerations)
  - [Application Specification](#application-specification)
    - [Model Information](#model-information)
    - [Mode](#mode)
      - [DRP-AI mode](#drp-ai-mode)
      - [CPU mode](#cpu-mode)

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
|pascal-voc-labels.txt | Label list for Semantic Segmentation. |

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

  ```sh
  cd $TVM_ROOT/how-to/sample_app/src
  mkdir build
  cd build
  cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
  make -j$(nproc)
  ```

3. Check that `sample_app_drpai_tvm_usbcam_http` application binary is generated.

## Setup the Execution Environment

### 1. Setup the board

1. Copy the `sample_app_drpai_tvm_usbcam_http` application binary to `exe` directory.  

2. **Important for RZ/V2M, V2L, V2MA:** Copy the following runtime libraries from `obj/build_runtime/v2m/lib/` directory to the same directory as the application:
   - `libdrp_tvm_rt.so`
   - `libmera2_runtime.so`
   - `libmera2_plan_io.so`

   These libraries are required for the application to run properly on these boards. When running the application, you'll need to set the library path to include the directory where these libraries are located:

   ```sh
   export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
   ```

3. Compile the AI models to generate following directories according to the instruction provided in each "How to create Model Object" column.  

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
|deeplabv3_resnet50_pt/ | DeepLabV3 model for DRP-AI mode. |[Semantic Segmentation](docs/semantic_segmentation/deeplabv3) |

4. Copy the generated model directories to `exe` directory.  
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
├── pascal-voc-labels.txt
├── libdrp_tvm_rt.so
├── libmera2_runtime.so
├── libmera2_plan_io.so
└── sample_app_drpai_tvm_usbcam_http
```

5. Copy the application directory to the target board.  
**Note for RZ/V2M, V2L, V2MA:** The runtime libraries should be placed in the same directory as the application. You can place the application directory anywhere on the target board's filesystem.

The directory structure on the target board should look like this (where `<application_directory>` is the directory where you placed the application):

```sh
<application_directory>/
├── sample_app_drpai_tvm_usbcam_http
├── libdrp_tvm_rt.so
├── libmera2_runtime.so
├── libmera2_plan_io.so
├── coco-labels-2014_2017.txt
├── synset_words_imagenet.txt
├── pascal-voc-labels.txt
└── ... model directories ...
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
As default, the PC IP address data should be set to **"192.168.1.10"**.  
**Note:** Both PC local IP address and RZ/V IP address need to be changed if any of those IP address is set to other address than default setting.  
To see how to change the RZ/V IP address, please refer to [Run the application](#run-the-application).

## Run the application
This section assumes that the user has prepared the Boot Environment on the target board.  

1. **(On RZ/V Board)** Move to the directory where the application is stored.  
```sh
cd <application_directory>
```

2. **(On RZ/V Board)** Set up the environment:
```sh
# Set library path to include current directory
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH

# Check current IP address
CURRENT_IP=$(ifconfig eth0 | grep "inet " | awk '{print $2}')
echo "Current IP address: $CURRENT_IP"

# Set board IP address only if not already set
if [ -z "$CURRENT_IP" ]; then
    echo "No IP address configured. Setting IP address to 192.168.1.11"
ifconfig eth0 192.168.1.11
else
    echo "Using existing IP address: $CURRENT_IP"
    # If you need to change the IP address, you can use:
    # ifconfig eth0 down
    # ifconfig eth0 192.168.1.11
    # ifconfig eth0 up
fi
```

3. **(On RZ/V Board)** Before running the application, make sure the PC can be accessed from the board:
```sh
# Get current IP address
BOARD_IP=$(ifconfig eth0 | grep "inet " | awk '{print $2}')

# Ping the PC (assuming PC IP is 192.168.1.10)
ping -c 3 192.168.1.10
```

- If the network connection and IP address setting are correct, you will see successful ping responses.
- If the network connection or IP address setting is wrong, the ping will fail.  
Please check the IP address setting and the network connection of PC and RZ/V Board.  

4. **(On RZ/V Board)** Update the WebSocket client configuration to match your board's IP address:
```sh
# Get current IP address
BOARD_IP=$(ifconfig eth0 | grep "inet " | awk '{print $2}')

# If you have the Websocket_Client directory on the board, update the IP address
if [ -f "Websocket_Client/js/websocket_demo.js" ]; then
    # Create a backup
    cp Websocket_Client/js/websocket_demo.js Websocket_Client/js/websocket_demo.js.bak
    
    # Replace the IP address
    sed -i "s/192\.168\.1\.11/$BOARD_IP/g" Websocket_Client/js/websocket_demo.js
    echo "Updated WebSocket client configuration with IP: $BOARD_IP"
fi

# Note: If the Websocket_Client is on your PC, you'll need to manually update
# the IP address in Websocket_Client/js/websocket_demo.js
echo "Make sure to update the IP address in Websocket_Client/js/websocket_demo.js on your PC to: $BOARD_IP"
```

5. **(On RZ/V Board)** Run the application:
```sh
./sample_app_drpai_tvm_usbcam_http
```

6. **(On RZ/V Board)** When the application starts successfully, you should see:
```
################################################
#    RZ/V DRP-AI TVM Sample Application        #
################################################
Start Server ws://192.168.1.11:3000/ws/
```

7. **(On PC)** Open the directory where the `Websocket_Client` folder is stored.   
Open the `index.html` using the web browser that support websocket.  

8. **(On RZ/V Board)** The following message will be displayed on the console after the `index.html` is opened.
```
Client connected.
```

9. **(On PC)** As shown in the figure below, select the AI model listed.  
Once user selected the AI model, the application in both web browser
side (PC side) and RZ/V side will start to work.  

<img src=./img/run_1.png width=600>  

</br>

10. **(On PC)** During the AI inference, the web page will show the following features.  
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

For more flexible deployment across different networks, you can modify the server address in `src/main.cpp` to use `0.0.0.0` instead of a specific IP address:
```cpp
ws.start_server("0.0.0.0", "3000", "ws");
```

This will make the server listen on all available network interfaces. After changing the server address, you must also update the client configuration in `etc/Websocket_Client/js/websocket_demo.js` to use the actual IP address of the board.

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

### 3. Special Notes for RZ/V2M, V2L, V2MA

#### Version Compatibility
Compatibility issues have been observed with DRP-AI TVM[^1] v2.7.0 and the latest Linux packages. For stable operation of this sample application, it is strongly recommended to use:
- DRP-AI TVM[^1] v2.6.0 or earlier
- Linux package v3.0.6

Using newer versions may result in unexpected behavior or application failures.

#### Model-Specific Considerations
When working with the provided models on RZ/V2M, V2L, or V2MA boards, be aware of the following model-specific considerations:

- **YOLOv3**: This model requires significant memory resources and may encounter issues with large input sizes. Consider using YOLOv2 or Tiny YOLOv3 for more reliable operation.

- **UltraFace**: May experience memory allocation issues in some configurations. If you encounter problems, try reducing the input image resolution.

- **HRNet and HRNetv2**: These models have high memory requirements. Consider using smaller input resolutions when possible.

- **DeepLabV3**: This semantic segmentation model requires substantial memory and may encounter issues with large input sizes.

- **DeepPose, YOLOv2, Tiny YOLOv2, Tiny YOLOv3, Emotion FERPlus, and GoogleNet**: These models generally work reliably on all boards with the default configurations.

If you encounter memory issues with any model, first try reducing the input image size or using a more memory-efficient model variant.

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