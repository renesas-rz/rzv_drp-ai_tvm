# DRP-AI TVM[^1] Application Example

This page explains how to use the application provided in this directory, which is the example to run ResNet18 inference on the target board.

## Overview
To run the inference with the AI model data compiled by DRP-AI TVM[^1], inference application is necessary.  
This application must use the DRP-AI TVM[^1] Runtime Library API and must be written in C++.  
Here, we explain how to compile and deploy the application example for ResNet 18, which is already compiled in [Compile AI models](../tutorials).

## File Configuration
| File/Directory | Details |  
|:---|:---|  
|exe | Execution environment required for running the application on the board |
|toolchain | Application compilation toolchain |  
|CMakeLists.txt |CMake configuration |  
|tutorial_app.cpp |C++ application main source code |  
|*.cpp |Other C++ application source code |  
|*.h |C++ header file |  
|README.md |This file. Instructions to use the application.|  

## Necessary Environment
Please refer to [Installation](../#installation) to prepare the following environment.
- DRP-AI TVM[^1]
- RZ/V2M Linux Package with OpenCV (To install OpenCV, see [How to install OpenCV](#how-to-install-opencv-to-linux-package))
- RZ/V2M DRP-AI Support Package

This page assumes that the above environment has already prepared and following environment variables has registered.  
```sh
export TVM_HOME=<.../drp-ai_tvm>/tvm                # Your own path to the cloned repository.
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=</opt/poky/2.4.3>                        # Your own RZ/V2M Linux SDK path.
export TRANSLATOR=<.../drp-ai_translator/>          # Your own DRP-AI Translator path.
```

## How to Compile the Application
### 1. Prepare the environment
Move to the application directory and create `build` directory.
```sh
cd $TVM_HOME/../apps
mkdir build
cd build
```
Run `cmake` command.
```sh
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake ..
```

### 2. Compile
In the `build` directory, run the `make` command.
```sh
make -j$(nproc)
```
After runinng the make command, following file would be generated in the `build` directory.
- tutorial_app

## How to Run the Application
This section assumes that the user has prepared the Boot Environment on the target board.

### 1. Copy to the Board
Copy the following files to the rootfs of Boot Environment.  

| Name | Path | Details |  
|:---|:---|:---|  
|Runtime Library | `drp-ai_tvm/obj/build_runtime/libtvm_runtime.so`|Binary provided under [obj](../obj/build_runtime) directory.|  
|Model Data | `drp-ai_tvm/tutorials/resnet18_onnx`|Model compiled in the [Compile AI models](../tutorials).|  
|Input Image |`drp-ai_tvm/apps/exe/sample.bmp`| Input data for image classification. |  
|Label List |`drp-ai_tvm/apps/exe/synset_words_imagenet.txt`| Label list for ResNet18 post-processing. |  
|Application |`drp-ai_tvm/apps/build/tutorial_app` | Compiled in this [page](#how-to-compile-the-application). |  

The rootfs should look like below.
```sh
/home
└── root
    └── tvm
        ├── libtvm_runtime.so
        ├── resnet18_onnx
        │   ├── deploy.json
        │   ├── deploy.params
        │   └── deploy.so
        ├── sample.bmp
        ├── synset_words_imagenet.txt
        └── tutorial_app
```

### 2. Run
After boot-up the board, move to the directory you stored the above files and run the `tutorial_app` file.  
```sh
cd ~/tvm
./tutorial_app
```
The application runs the ResNet inference on [sample.bmp](exe).  

<!-- <img src="./exe/sample.bmp" width=350>   -->
<!-- GitLabだとBMP画像が表示されない。GitHubならOK -->

Following is the expected output for ResNet18 ONNX model compiled for DRP-AI.
```sh
[08:01:20] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:47: Loading json data...
[08:01:20] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:53: Loading runtime module...
[08:01:24] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:58: Loading parameters...
[08:01:24] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:69: Loading input...
Running tvm runtime
[TIME] AI Processing Time: 22.01 msec.
Result -----------------------
  Top 1 [ 66.0%] : [beagle]
  Top 2 [ 13.0%] : [English foxhound]
  Top 3 [ 11.4%] : [Walker hound, Walker foxhound]
  Top 4 [  5.8%] : [basset, basset hound]
  Top 5 [  0.7%] : [bloodhound, sleuthhound]
```

## Application Specification

### Model Information
ResNet18: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/classification/resnet)

### Pre/Post-processing
The application uses following C++ OpenCV functions as pre/post-processing.  

| Function | Details |  
|:---|:---|  
|imread |Read the input image |  
|cvtColor |BGR to RGB conversion|  
|resize | Resize to 224x224 with Linear algorithm|  
|normalize | Normalize pixel values to [0,1]|  

---  
# Appendix
## DRP-AI TVM[^1] Runtime Library API
Regarding the list of DRP-AI TVM[^1] Runtime API used in the application, please see [MERA Wrapper API References](../obj)

## How to install OpenCV to Linux Package
As a preparation, it is required to setup the Build Environment with Linux Package and DRP-AI Support Package.  
Follow the instruction in the DRP-AI Support Package Release Note and before running the `bitbake` command, carry out the following instructions.  

### 1. Add OpenCV to local.conf
Add the following statement at the end of the `build/conf/local.conf` file.
```sh
IMAGE_INSTALL_append =" opencv "
```
### 2. Update libxshmfence to 1.3
Disable `libxshmfence_1.2.bb`.
```sh
cd <Build Environment>/poky/meta/recipes-graphics/xorg-lib
mv libxshmfence_1.2.bb libxshmfence_1.2.bb.disabled
```
Create `libxshmfence_1.3.bb` in `<Build Environment>/poky/meta/recipes-graphics/xorg-lib` directory
```sh
touch libxshmfence_1.3.bb
```
Write the following statements to `libxshmfence_1.3.bb`
```sh:libshmfence_1.3.bb
SUMMARY = "Shared memory 'SyncFence' synchronization primitive"

DESCRIPTION = "This library offers a CPU-based synchronization primitive compatible \
with the X SyncFence objects that can be shared between processes \
using file descriptor passing."

require xorg-lib-common.inc

LICENSE = "MIT-style"
LIC_FILES_CHKSUM = "file://COPYING;md5=47e508ca280fde97906eacb77892c3ac"

DEPENDS += "virtual/libx11"

EXTRA_OECONF += "--with-shared-memory-dir=/dev/shm"

BBCLASSEXTEND = "native nativesdk"

SRC_URI[md5sum] = "42dda8016943dc12aff2c03a036e0937"
SRC_URI[sha256sum] = "b884300d26a14961a076fbebc762a39831cb75f92bed5ccf9836345b459220c7"
```
### 3. Bitbake
Run the `bitbake` command as explained in the DRP-AI Support Package.  

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.