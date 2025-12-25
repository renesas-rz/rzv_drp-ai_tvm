# DRP-AI TVM[^1] Application Example (RZ/V2L, RZ/V2M, RZ/V2MA)

This page explains how to use the application provided in this directory, which is the example to run ResNet inference (ResNet18 ONNX or ResNet50 TensorFlow) on the target board.
If you would like to build an application for **RZ/V2H and RZ/V2N** [see here](./build_appV2H.md).

## Overview

To run the inference with the AI model data compiled by DRP-AI TVM[^1], inference application is necessary.  
This application must use the DRP-AI TVM[^1] Runtime Library API and must be written in C++.  
Here, we explain how to compile and deploy the application example for ResNet models, which are compiled in [Compile AI models](../tutorials).

## File Configuration

| File/Directory | Details |  
|:---|:---|  
|exe | Execution environment required for running the application on the board |
|toolchain | Application compilation toolchain |  
|CMakeLists.txt |CMake configuration |  
|tutorial_app_v2ml.cpp |C++ application main source code |  
|*.cpp |Other C++ application source code |  
|*.h |C++ header file |  
|README.md |This file. Instructions to use the application.|  

## Necessary Environment

Please refer to [Installation](../README.md#installation) to prepare the necessary environment.  

This page assumes that the above environment has already prepared and following environment variables has registered.

```sh
export TVM_ROOT=<.../drp-ai_tvm>                    # Your own path to the cloned repository.
export PYTHONPATH=$TVM_ROOT/tvm/python:${PYTHONPATH}
export SDK=</opt/poky/3.1.21>                       # Your own Linux SDK path. Set appropriate path according to your product to use.
export TRANSLATOR=<.../drp-ai_translator_release/>  # Your own DRP-AI Translator path.
export PRODUCT=<V2MA>                               # Product name (V2L, V2M, or V2MA)
```

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
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake ..
```

### 2. Build

In the `build` directory, run the `make` command.

```sh
make -j$(nproc)
```

After runinng the make command, following file would be generated in the `build` directory.

- tutorial_app_v2ml

## How to Run the Application

This section assumes that the user has prepared the Boot Environment on the target board.

### 1. Copy to the Board

Copy the following files to the rootfs of Boot Environment.  

| Name | Path | Details |  
|:---|:---|:---|  
|Runtime Libraries | `drp-ai_tvm/obj/build_runtime/v2m/lib/*.so`|Required libraries: `libdrp_tvm_rt.so`, `libdrpai_rt.so`, `libmera2_plan_io.so`, `libmera2_runtime.so`<br>Use the libraries in the `v2m` directory regardless of your target product (V2L, V2M, or V2MA). |  
|Model Data | `drp-ai_tvm/tutorials/resnet18_onnx`|Model compiled in the [Compile AI models](../tutorials). DRP-AI Preprocessing Runtime Object files, (`preprocess` directory) are also included.|  
|Input Data |`drp-ai_tvm/apps/exe/sample.bmp`| Windows Bitmap file, which is input data for image classification. |  
|Label List |`drp-ai_tvm/apps/exe/synset_words_imagenet.txt`<br>`drp-ai_tvm/apps/exe/ImageNetLabels.txt`| `synset_words_imagenet.txt`: Label list for ResNet18 post-processing.<br>`ImageNetLabels.txt`: Label list for ResNet50 post-processing when compiling TensorFlow Hub model. |  
|Application |`drp-ai_tvm/apps/build/tutorial_app` | Compiled in this [page](../tutorials/README.md#how-to-compile-the-application) . |  

The rootfs should have the following directory structure for all target boards (RZ/V2L, RZ/V2M, and RZ/V2MA):

```sh
/
└── home
    └── root
        └── tvm
            ├── lib            
            │   ├── libdrp_tvm_rt.so
            │   ├── libmera2_plan_io.so
            │   └── libmera2_runtime.so
            ├── resnet18_onnx
            │   ├── deploy.json
            │   ├── deploy.params
            │   ├── deploy.so
            │   └── preprocess
            │       ├── aimac_desc.bin
            │       ...
            │       └── pp_weight.dat
            ├── ImageNetLabels.txt
            ├── sample.bmp
            ├── synset_words_imagenet.txt
            └── tutorial_app_v2ml

```

Here's an example of commands to prepare the deployment files:

```sh
cd $TVM_ROOT/../
rm -r tvm
mkdir -p tvm/lib
cp $TVM_ROOT/obj/build_runtime/v2m/lib/lib* tvm/lib/.
cp $TVM_ROOT/apps/exe/sample.bmp tvm/
cp $TVM_ROOT/apps/exe/ImageNetLabels.txt tvm/
cp $TVM_ROOT/apps/exe/synset_words_imagenet.txt tvm/
cp $TVM_ROOT/apps/build/tutorial_app* tvm/
cp -r $TVM_ROOT/tutorials/resnet18_onnx tvm/
tar cvfz tvm.tar.gz tvm/
```

### 2. Run

After boot-up the board, move to the directory you stored the application and run the `tutorial_app_v2ml` file.  

```sh
cd ~/tvm
export LD_LIBRARY_PATH=./lib
./tutorial_app_v2ml
```

The application runs the ResNet inference on [sample.bmp](exe/sample.bmp).

Following is the expected output for ResNet18 ONNX model compiled for DRP-AI on RZ/V2MA Evaluation Board Kit.  

```sh
root@rzv2ma:~# ./tutorial_app
[16:54:37] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:66: Loading json data...
[16:54:37] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:72: Loading runtime module...
[16:54:39] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:77: Loading parameters...
[TIME] Pre Processing Time: 3.83 msec.
[16:54:39] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:88: Loading input...
Running tvm runtime
[TIME] AI Processing Time: 17.69 msec.
Output data type : FP16.
Result -----------------------
  Top 1 [ 66.0%] : [beagle]
  Top 2 [ 13.0%] : [English foxhound]
  Top 3 [ 11.4%] : [Walker hound, Walker foxhound]
  Top 4 [  5.9%] : [basset, basset hound]
  Top 5 [  0.7%] : [bloodhound, sleuthhound]
```

For reference, processing times are shown below.  
  
| Renesas Evaluation Board Kit | Pre Processing Time | AI Processing Time |
|------------------------------|:-------------------:|:------------------:|
| RZ/V2L  Evaluation Board Kit |       4.25 msec     |     27.99 msec     |
| RZ/V2M  Evaluation Board Kit |       3.99 msec     |     17.76 msec     |
| RZ/V2MA Evaluation Board Kit |       3.83 msec     |     17.69 msec     |

### 3. Tips for Faster Running

**Note**: You can specify the number of threads to be used in runtime CPU processing with the `TVM_NUM_THREADS` variable. Use this if you want to control the CPU load. Specify it as follows before running the application (below is an example of using 1 thread):

```bash
export TVM_NUM_THREADS=1
```

This is particularly effective in cases where there are very few operators being inferred on the CPU.

## Important Notes for Using Different Model Types

The application is configured to work with the default ONNX ResNet18 model in a directory named `resnet18_onnx`. If you're using a different model type or directory name, you'll need to make some adjustments.

### Using Different Model Directories

The application expects the model to be in a directory named `resnet18_onnx`. If your model is in a different directory, create a symbolic link:

```sh
# For PyTorch model
ln -sf resnet18_torch resnet18_onnx

# For CPU-only ONNX model
ln -sf resnet18_onnx_cpu resnet18_onnx

# For TensorFlow model
ln -sf resnet50_tflite resnet18_onnx
```

### TensorFlow Models Label File

When using TensorFlow models (such as ResNet50 from TensorFlow Hub), you also need to replace the label file:

```sh
# Backup the original label file
cp synset_words_imagenet.txt synset_words_imagenet.txt.bak

# Use the TensorFlow-compatible label file
cp ImageNetLabels.txt synset_words_imagenet.txt

# Run the application
./tutorial_app_v2ml

# Restore the original label file when done
cp synset_words_imagenet.txt.bak synset_words_imagenet.txt
```

This is necessary because TensorFlow models use a different label format than the default one provided for ONNX and PyTorch models. The application expects the label file to be named `synset_words_imagenet.txt`, so we temporarily replace it with the TensorFlow-compatible version.

## Model Compilation

Before running the application, you need to compile the AI models. The model compilation process is explained in detail in the [Compile AI models](../tutorials/README.md) section. Here's a brief overview:

1. **ONNX Models**: Use `compile_onnx_model.py` script to compile ResNet18 ONNX model
2. **PyTorch Models**: Use `compile_pytorch_model.py` script to compile ResNet18 PyTorch model
3. **TensorFlow Models**: Use `compile_tflite_model.py` script to compile ResNet50 TensorFlow model
4. **CPU-only Models**: Use `compile_cpu_only_onnx_model.py` script for CPU-only inference

After compilation, the model files will be generated in the specified output directory (e.g., `resnet18_onnx`, `resnet18_torch`, `resnet50_tflite`, `resnet18_onnx_cpu`). These are the directories that need to be copied to the target board or linked to `resnet18_onnx` as explained above.

## Application Specification

### Model Information

- ResNet18: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/validated/vision/classification/resnet)
- ResNet50: [TensorFlow Hub](https://tfhub.dev/google/imagenet/resnet_v1_50/classification/5)

### Pre-processing

The application uses DRP-AI Pre-processing Runtime as pre-processing.  
For more details on DRP-AI Pre-processing Runtime, please refer to [DRP-AI Pre-processing Runtime Documentation](../docs/PreRuntime.md).  

Processing details are originally defined in the compile script provided in [Compile AI models](../tutorials).  
In this example, `tutorial_app.cpp` changes its parameter to run following preprocessing.  
(**Bold** is changed parameter.)

- Input data  
  - Shape: **640**x**480**x3  
  - Format: **BGR**  
  - Order: HWC  
  - Type: uint8  
- Output data  
  - Shape: 224x224x3  
  - Format: **RGB**  
  - Order: CHW  
  - Type: float  
- Preprocessing operations:  
  - Resize  
  - Normalize  
    - cof_add =[-123.6875, -116.2500, -103.5000]  
    - cof_mul =[0.0171, 0.0175, 0.0174]  

---  

## Appendix

### DRP-AI TVM[^1] Runtime Library API

Regarding the list of DRP-AI TVM[^1] Runtime API used in the application, please see [MERA Wrapper API References](../docs/Runtime_Wrap.md)

### How to install OpenCV to Linux Package

As a preparation, it is required to setup the Build Environment with Linux Package and DRP-AI Support Package.  
Follow the instruction in the DRP-AI Support Package Release Note and before running the `bitbake` command, carry out the following instructions.  

#### 1. Add OpenCV to local.conf

Add the following statement at the end of the `build/conf/local.conf` file.

```sh
IMAGE_INSTALL_append =" opencv "
```

#### 2. Bitbake

Run the `bitbake` command as explained in the DRP-AI Support Package.  

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.