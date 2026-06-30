# DRP-AI TVM[^1] Application Example (RZ/V2H, RZ/V2N)

This page explains how to use the application provided in this directory, which is the example to run ResNet inference (ResNet18 ONNX or ResNet50 TensorFlow) on the target board.

## Overview

To run inference with the AI model data compiled by DRP-AI TVM[^1], an inference application is necessary.  
At this point, this application must be written in C++ for the DRP-AI TVM[^1] Runtime Library.  
Here, we explain how to compile and deploy the application example for ResNet models, which have already been compiled in the [Compile with Sample Scripts (RZ/V2H)](../tutorials/tutorial_RZV2H.md).

## Set environment variables

Same as [Installation](../setup/README.md#4-set-environment-variables).  

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
| Runtime Library | `drp-ai_tvm/obj/build_runtime/${PRODUCT}/lib/*`  | Binary provided under [obj](../obj/build_runtime) directory.<br>You should use the libraries in the directory with the corresponding product name.              |
| Model Data      | `drp-ai_tvm/tutorials/resnet*`                                                              | Model compiled in the [Compile AI models](../tutorials). DRP-AI Preprocessing Runtime Object files, (`preprocess` directory) are also included.                           |
| Input Data      | `drp-ai_tvm/apps/exe/sample.bmp`                                                            | Windows Bitmap file, which is input data for image classification.                                                                                                        |
| Label List      | `drp-ai_tvm/apps/exe/synset_words_imagenet.txt`<br>`drp-ai_tvm/apps/exe/ImageNetLabels.txt` | `synset_words_imagenet.txt`: Label list for ResNet18 post-processing.<br>`ImageNetLabels.txt`: Label list for ResNet50 post-processing when compiling TensorFlow Hub model. |
| Application     | `drp-ai_tvm/apps/build/tutorial_app`                                                        | Compiled in this [page](#how-to-compile-the-application).                                                                                                                 |

The rootfs should look like below.  

```sh
/
└── home
    └── root
        └── tvm
            ├── lib            
            │   ├── libacl_rt.so
            │   ├── libarm_compute.so
            │   ├── libarm_compute_core.so
            │   ├── libarm_compute_graph.so
            │   ├── libdrp_rt.so
            │   ├── libdrp_tvm_rt.so
            │   ├── libmera2_plan_io.so
            │   ├── libmera2_runtime.so
            │   ├── log_out.bin
            │   ├── softmax_out.bin
            │   └── split_out.bin
            ├── resnet18_onnx
            │   ├── mera.plan
            │   ├── model_subgraphs.json
            │   ├── project.mdp
            │   ├── sub_0000__CPU_DRP_TVM
            │   │   ├── deploy.json
            │   │   ├── deploy.params
            │   │   └── deploy.so
            │   └── preprocess
            │       ├── aimac_desc.bin
            │       ...
            │       └── weight.bin
            ├── sample.bmp
            ├── synset_words_imagenet.txt
            ├── ImageNetLabels.txt
            └── tutorial_app
```

As a working example, a series of commands is described below.

```bash
cd $TVM_ROOT/../
mkdir -p tvm/lib
cp $TVM_ROOT/obj/build_runtime/v2h/lib/* tvm/lib/.
cp $TVM_ROOT/apps/exe/sample.bmp tvm/
cp $TVM_ROOT/apps/exe/ImageNetLabels.txt tvm/
cp $TVM_ROOT/apps/exe/synset_words_imagenet.txt tvm/
cp $TVM_ROOT/apps/build/tutorial_app* tvm/
cp -r $TVM_ROOT/tutorials/resnet18_onnx tvm/
cp -r $TVM_ROOT/tutorials/resnet18_torch tvm/
cp -r $TVM_ROOT/tutorials/resnet50_tflite tvm/
cp -r $TVM_ROOT/tutorials/resnet18_onnx_cpu tvm/
tar cvfz tvm.tar.gz tvm/
```

### 2. Run

After boot-up the board, move to the directory you stored the application and run the `tutorial_app` file.  

```sh
cd ~
tar xvfz tvm.tar.gz
cd ~/tvm
export LD_LIBRARY_PATH=./lib/.
./tutorial_app
#./tutorial_app 5 #The way run DRP-AI with 315Mhz@V2H
```

The application runs the ResNet inference on [sample.bmp](exe/sample.bmp).

### 3. Tips for Faster Running

**Note**: You can specify the number of threads to be used in runtime CPU processing with the `TVM_NUM_THREADS` variable. Use this if you want to control the CPU load. Specify it as follows before running the application (below is an example of using 1 thread):

```bash
export TVM_NUM_THREADS=1
```

This is particularly effective in cases where there are very few operators being inferred on the CPU.

## Important Notes for Using Different Model Types

The application is configured to work with the default ONNX ResNet18 model in a directory named `resnet18_onnx`. If you're using a different model type or directory name, you'll need to make some adjustments.

### Using Different Model Types

The application expects the model to be in a directory named `resnet18_onnx`. To use different model types, create a symbolic link from your model directory to `resnet18_onnx` and run the application:

```sh
# For PyTorch model
ln -sf resnet18_torch resnet18_onnx
./tutorial_app

# For CPU-only ONNX model
ln -sf resnet18_onnx_cpu resnet18_onnx
./tutorial_app
```

### TensorFlow Models Label File

When using TensorFlow models (such as ResNet50 from TensorFlow Hub), you need to replace the label file with the TensorFlow-compatible version in addition to creating the symbolic link:

```sh
# Create symbolic link for TensorFlow model
ln -sf resnet50_tflite resnet18_onnx

# Backup the original label file
cp synset_words_imagenet.txt synset_words_imagenet.txt.bak

# Use the TensorFlow-compatible label file
cp ImageNetLabels.txt synset_words_imagenet.txt

# Run the application
./tutorial_app

# Restore the original label file when done
cp synset_words_imagenet.txt.bak synset_words_imagenet.txt
```

This label file replacement is necessary because TensorFlow models use a different label format than the default one provided for ONNX and PyTorch models. The application expects the label file to be named `synset_words_imagenet.txt`, so we temporarily replace it with the TensorFlow-compatible version.

**Note**: This label file replacement is required for both RZ/V2H/V2N and RZ/V2L/V2M/V2MA platforms when using TensorFlow models.

### Using Non-Classification Models

This application runs ONNX ResNet18 model with pre/post-processing, but it can only be applied to image classification.  
If you would like to run AI inference without pre/post-processing for validation purposes, please use following applications.

- [Validation Application](../how-to/validation) : C++ program to run AI inference only.  
- [Python API Sample Script](../how-to/python) : Python API program to run AI inference only.  ResNet and YOLOX sample pre/post-processing scripts are also provided.  

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