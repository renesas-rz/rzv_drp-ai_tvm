# DRP-AI TVM[^1] Application Example (RZ/V2L, RZ/V2M, RZ/V2MA)

This page explains how to use the application provided in this directory, which is the example to run ResNet18 inference on the target board.
If you would like to build an application for **RZ/V2H** [see here](./build_appV2H.md).

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
|tutorial_app_v2ml.cpp |C++ application main source code |  
|*.cpp |Other C++ application source code |  
|*.h |C++ header file |  
|README.md |This file. Instructions to use the application.|  

## Necessary Environment
Please refer to [Installation](../README.md#installation) to prepare the necessary environment.  

This page assumes that the above environment has already prepared and following environment variables has registered.  
```sh
export TVM_ROOT=<.../drp-ai_tvm>                    # Your own path to the cloned repository.
export TVM_HOME=${TVM_ROOT}/tvm
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
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
- tutorial_app

## How to Run the Application
This section assumes that the user has prepared the Boot Environment on the target board.

### 1. Copy to the Board
Copy the following files to the rootfs of Boot Environment.  

| Name | Path | Details |  
|:---|:---|:---|  
|Runtime Library | `drp-ai_tvm/obj/build_runtime/${PRODUCT}/libtvm_runtime.so`|Binary provided under [obj](../obj/build_runtime) directory.<br>You should use the `libtvm_runtime.so` in the directory with the corresponding product name. |  
|Model Data | `drp-ai_tvm/tutorials/resnet18_onnx`|Model compiled in the [Compile AI models](../tutorials). DRP-AI Preprocessing Runtime Object files, (`preprocess` directory) are also included.|  
|Input Data |`drp-ai_tvm/apps/exe/sample.bmp`| Windows Bitmap file, which is input data for image classification. |  
|Label List |`drp-ai_tvm/apps/exe/synset_words_imagenet.txt`<br>`drp-ai_tvm/apps/exe/ImageNetLabels.txt`| `synset_words_imagenet.txt`:Label list for ResNet18 post-processing.<br>`ImageNetLabels.txt`:Label list for ResNet50 post-processing when compiling Tensorflow Hub model. |  
|Application |`drp-ai_tvm/apps/build/tutorial_app` | Compiled in this [page](#how-to-compile-the-application). |  

The rootfs should look like below.  
This example is for V2MA.  
**Note that `libtvm_runtime.so` must be placed under `/usr/lib64`.**  
**Note that if you compiled the model in Tensorflow Hub, rename the label list `ImageNetLabels.txt` to `synset_words_imagenet.txt` and use it.**  
```sh
/
├── usr
│   └── lib64
│       └── libtvm_runtime.so
└── home
    └── root
        └── tvm
            ├── resnet18_onnx
            │   ├── deploy.json
            │   ├── deploy.params
            │   ├── deploy.so
            │   └── preprocess
            │       ├── aimac_desc.bin
            │       ...
            │       └── pp_weight.dat
            ├── sample.bmp
            ├── synset_words_imagenet.txt
            └── tutorial_app

```

### 2. Run
After boot-up the board, move to the directory you stored the application and run the `tutorial_app` file.  
```sh
cd ~/tvm
./tutorial_app
```
The application runs the ResNet inference on [sample.bmp](exe/sample.bmp).   

<!-- <img src="./exe/sample.bmp" width=350>   -->
<!-- GitLabだとBMP画像が表示されない。GitHubならOK -->

Following is the expected output for ResNet18 ONNX model compiled for DRP-AI on RZ/V2MA Evaluation Board Kit.  
```sh
root@rzv2ma:~# ./tutorial_app
[16:54:37] /mnt/nvme1n1/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:66: Loading json data...
[16:54:37] /mnt/nvme1n1/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:72: Loading runtime module...
[16:54:39] /mnt/nvme1n1/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:77: Loading parameters...
[TIME] Pre Processing Time: 3.83 msec.
[16:54:39] /mnt/nvme1n1/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:88: Loading input...
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

## Application Specification

### Model Information

ResNet18: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/classification/resnet)

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
