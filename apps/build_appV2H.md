# DRP-AI TVM[^1] Application Example (RZ/V2H)

This page explains how to use the application provided in this directory, which is the example to run ResNet18 inference on the target board.

## Overview

To run the inference with the AI model data compiled by DRP-AI TVM[^1], inference application is necessary.  
This application must use the DRP-AI TVM[^1] Runtime Library API and must be written in C++.  
Here, we explain how to compile and deploy the application example for ResNet 18, which is already compiled in [Compile AI models](../tutorials).

## File Configuration

| File/Directory   | Details                                                                 |
|:---------------- |:----------------------------------------------------------------------- |
| exe              | Execution environment required for running the application on the board |
| toolchain        | Application compilation toolchain                                       |
| CMakeLists.txt   | CMake configuration                                                     |
| tutorial_app.cpp | C++ application main source code                                        |
| *.cpp            | Other C++ application source code                                       |
| *.h              | C++ header file                                                         |
| README.md        | This file. Instructions to use the application.                         |

## Set environment variables

Same as [Installation](../setup/SetupV2H.md#4-set-environment-variables).  

## How to Build the Application

### 1. Prepare the environment

Move to the application directory and create `build` directory.

```sh
cd $TVM_ROOT/apps
mkdir build
cd build
```

Run `cmake` command.

```sh
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake -DV2H=ON ..

```

### 2. Build

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

| Name            | Path                                                                                        | Details                                                                                                                                                                   |
|:--------------- |:------------------------------------------------------------------------------------------- |:------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Runtime Library | `drp-ai_tvm/obj/build_runtime/${PRODUCT}/libtvm_runtime.so`                                 | Binary provided under [obj](../obj/build_runtime) directory.<br>You should use the `libtvm_runtime.so` in the directory with the corresponding product name.              |
| Model Data      | `drp-ai_tvm/tutorials/resnet*`                                                              | Model compiled in the [Compile AI models](../tutorials). DRP-AI Preprocessing Runtime Object files, (`preprocess` directory) are also included.                           |
| Input Data      | `drp-ai_tvm/apps/exe/sample.bmp`                                                            | Windows Bitmap file, which is input data for image classification.                                                                                                        |
| Label List      | `drp-ai_tvm/apps/exe/synset_words_imagenet.txt`<br>`drp-ai_tvm/apps/exe/ImageNetLabels.txt` | `synset_words_imagenet.txt`:Label list for ResNet18 post-processing.<br>`ImageNetLabels.txt`:Label list for ResNet50 post-processing when compiling Tensorflow Hub model. |
| Application     | `drp-ai_tvm/apps/build/tutorial_app`                                                        | Compiled in this [page](#how-to-compile-the-application).                                                                                                                 |

The rootfs should look like below.  
**Note that if you compiled the model in Tensorflow Hub, rename the label list `ImageNetLabels.txt` to `synset_words_imagenet.txt` and use it.**  

```sh
/
└── home
    └── root
        └── tvm
            ├── libtvm_runtime.so
            ├── resnet50_v1_onnx
            │   ├── deploy.json
            │   ├── deploy.params
            │   ├── deploy.so
            │   └── preprocess
            │       ├── aimac_desc.bin
            │       ...
            │       └── weight.bin
            ├── sample.bmp
            ├── synset_words_imagenet.txt
            └── tutorial_app
```

As a working example, a series of commands is described below.

```bash
cd $TVM_ROOT/../
mkdir tvm
cp $TVM_ROOT/obj/build_runtime/$PRODUCT/libtvm_runtime.so tvm/
cp $TVM_ROOT/apps/exe/sample.bmp tvm/
cp $TVM_ROOT/apps/exe/ImageNetLabels.txt tvm/
cp $TVM_ROOT/apps/exe/synset_words_imagenet.txt tvm/
cp $TVM_ROOT/apps/build/tutorial_app* tvm/
cp -r $TVM_ROOT/tutorials/resnet50_v1_onnx tvm/
cp -r $TVM_ROOT/tutorials/resnet18_torch/  tvm/
cp -r $TVM_ROOT/tutorials/resnet50_tflite/  tvm/
cp -r $TVM_ROOT/tutorials/resnet50_v1_onnx_cpu/  tvm/

tar cvfz tvm.tar.gz tvm/
```

### 2. Run

After boot-up the board, move to the directory you stored the application and run the `tutorial_app` file.  

```sh
cd ~
tar xvfz tvm.tar.gz
cd ~/tvm
export LD_LIBRARY_PATH=.
cp -r resnet50_v1_onnx resnet18_onnx
./tutorial_app
#./tutorial_app 5 #run DRP-AI with 315Mhz


rm -r resnet18_onnx
cp -r resnet18_torch resnet18_onnx
./tutorial_app

rm -r resnet18_onnx
cp -r resnet50_v1_onnx_cpu resnet18_onnx
./tutorial_app

rm -r resnet18_onnx
cp synset_words_imagenet.txt synset_words_imagenet.txt.bak
cp ImageNetLabels.txt synset_words_imagenet.txt
cp -r resnet50_tflite resnet18_onnx
./tutorial_app
cp synset_words_imagenet.txt.bak synset_words_imagenet.txt
```

The application runs the ResNet inference on [sample.bmp](exe/sample.bmp).

### 3. Tips for Faster Running

**Note**: You can specify the number of threads to be used in runtime CPU processing with the `TVM_NUM_THREADS` variable. Use this if you want to control the CPU load. Specify it as follows before running the application (below is an example of using 1 thread):

```bash
export TVM_NUM_THREADS=1
```

This is particularly effective in cases where there are very few operators being inferred on the CPU.

<!-- <img src="./exe/sample.bmp" width=350>   -->
<!-- GitLabだとBMP画像が表示されない。GitHubならOK -->

## Application Specification

Same as for V2M. See [here](README.md#application-specification).

---  

# Appendix

## DRP-AI TVM[^1] Runtime Library API

Regarding the list of DRP-AI TVM[^1] Runtime API used in the application, please see [MERA Wrapper API References](../docs/Runtime_Wrap.md)

## How to install OpenCV to Linux Package

As a preparation, it is required to setup the Build Environment with Linux Package and DRP-AI Support Package.  
Follow the instruction in the DRP-AI Support Package Release Note and before running the `bitbake` command, carry out the following instructions.  

### 1. Add OpenCV to local.conf

Add the following statement at the end of the `build/conf/local.conf` file.

```sh
IMAGE_INSTALL_append =" opencv "
```

### 2. Bitbake

Run the `bitbake` command as explained in the DRP-AI Support Package.  

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
