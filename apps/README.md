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
|etc | Other necessary files |
|toolchain | Application compilation toolchain |  
|CMakeLists.txt |CMake configuration |  
|tutorial_app.cpp |C++ application main source code |  
|*.cpp |Other C++ application source code |  
|*.h |C++ header file |  
|README.md |This file. Instructions to use the application.|  

## Necessary Environment
Please refer to [Installation](../README.md#installation) to prepare the following environment.
 - RZ/V2L
      - DRP-AI TVM[^1]
      - DRP-AI Translator V1.81
      - RZ/V Verified Linux Package V3.0.2
      - RZ/V2L DRP-AI Support Package V7.30
  - RZ/V2M
      - DRP-AI TVM[^1]
      - DRP-AI Translator V1.81
      - RZ/V2M Linux Package V1.3.0
      - RZ/V2M DRP-AI Support Package V7.30
 - RZ/V2MA
      - DRP-AI TVM[^1]
      - DRP-AI Translator V1.81
      - RZ/V2MA Linux Package V1.1.0
      - RZ/V2MA DRP-AI Support Package V7.30

This page assumes that the above environment has already prepared and following environment variables has registered.  
```sh
export TVM_ROOT=<.../drp-ai_tvm>                    # Your own path to the cloned repository.
export TVM_HOME=${TVM_ROOT}/tvm
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=</opt/poky/3.1.14>                       # Your own Linux SDK path. Set appropriate path according to your product to use.
export TRANSLATOR=<.../drp-ai_translator_release/>  # Your own DRP-AI Translator path.
export PRODUCT=<V2MA>                               # Product name (V2L, V2M, or V2MA)
```
Please set the values in the table below to the SDK and PRODUCT variables according to Renesas Evaluation Board Kit you use.

| Renesas Evaluation Board Kit | SDK              | PRODUCT  |
|------------------------------|:----------------:|:--------:|
| RZ/V2L  Evaluation Board Kit | /opt/poky/3.1.17 |   V2L    |
| RZ/V2M  Evaluation Board Kit | /opt/poky/3.1.14 |   V2M    |
| RZ/V2MA Evaluation Board Kit | /opt/poky/3.1.14 |   V2MA   |

## How to Compile the Application
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
|Runtime Library | `drp-ai_tvm/obj/build_runtime/${PRODUCT}/libtvm_runtime.so`|Binary provided under [obj](../obj/build_runtime) directory.<br>You should use the `libtvm_runtime.so` in the directory with the corresponding product name. |  
|Model Data | `drp-ai_tvm/tutorials/resnet18_onnx`|Model compiled in the [Compile AI models](../tutorials).|  
|Pre-processing Runtime Object |<ul>V2L<li>`drp-ai_tvm/apps/exe/preprocess_tvm_v2l`</li>V2M<li>`drp-ai_tvm/apps/exe/preprocess_tvm_v2m`</li>V2MA<li>`drp-ai_tvm/apps/exe/preprocess_tvm_v2ma`</li></ul>| Data required for Pre-processing Runtime. <br>For more details, see [DRP-AI Pre-processing Runtime](#drp-ai-pre-processing-runtime). |  
|Input Data |`drp-ai_tvm/apps/exe/sample.yuv`| YUY2 input data for image classification generated from `drp-ai_tvm/apps/etc/sample.bmp`. <br> See [How to prepare YUY2 image data](#how-to-prepare-yuy2-image-data) for more details. |  
|Label List |`drp-ai_tvm/apps/exe/synset_words_imagenet.txt`| Label list for ResNet18 post-processing. |  
|Application |`drp-ai_tvm/apps/build/tutorial_app` | Compiled in this [page](#how-to-compile-the-application). |  

The rootfs should look like below.  
This example is for V2MA.  
**Note that `libtvm_runtime.so` must be placed under `/usr/lib64`.**  
```sh
/
├── usr
│   └── lib64
│       └── libtvm_runtime.so
└── home
    └── root
        └── tvm
            ├── preprocess_tvm_v2ma
            │   ├── drp_param.bin
            │   ...
            │   └── preprocess_tvm_v2ma_weight.dat
            ├── resnet18_onnx
            │   ├── deploy.json
            │   ├── deploy.params
            │   └── deploy.so
            ├── sample.yuv
            ├── synset_words_imagenet.txt
            └── tutorial_app

```

### 2. Run
After boot-up the board, move to the directory you stored the application and run the `tutorial_app` file.  
```sh
export PRODUCT=<V2MA>              # Product name (V2L, V2M, or V2MA)
cd ~/tvm
./tutorial_app
```
The application runs the ResNet inference on [sample.yuv](exe), which is generated from [sample.bmp](etc).   

<!-- <img src="./exe/sample.bmp" width=350>   -->
<!-- GitLabだとBMP画像が表示されない。GitHubならOK -->

Following is the expected output for ResNet18 ONNX model compiled for DRP-AI on RZ/V2MA Evaluation Board Kit.  
```sh
root@rzv2ma:~# ./tutorial_app
[10:48:40] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:66: Loading json data...
[10:48:40] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:72: Loading runtime module...
[10:48:42] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:77: Loading parameters...
[TIME] Pre Processing Time: 4.60 msec.
[10:48:42] /mnt/nvme1n1/TVM/drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:88: Loading input...
Running tvm runtime
[TIME] AI Processing Time: 17.63 msec.
Output data type : FP16.
Result -----------------------
  Top 1 [ 67.0%] : [beagle]
  Top 2 [ 12.4%] : [English foxhound]
  Top 3 [ 11.5%] : [Walker hound, Walker foxhound]
  Top 4 [  5.8%] : [basset, basset hound]
  Top 5 [  0.7%] : [bloodhound, sleuthhound]
```
For reference, processing times are shown below.  
  

| Renesas Evaluation Board Kit | Pre Processing Time | AI Processing Time |
|------------------------------|:-------------------:|:------------------:|
| RZ/V2L  Evaluation Board Kit |       4.80 msec     |     27.95 msec     |
| RZ/V2M  Evaluation Board Kit |       4.56 msec     |     17.83 msec     |
| RZ/V2MA Evaluation Board Kit |       4.60 msec     |     17.63 msec     |

## Application Specification

### Model Information
ResNet18: [ONNX Model Zoo](https://github.com/onnx/models/tree/main/vision/classification/resnet)

### Pre-processing
The application uses DRP-AI Pre-processing Runtime as pre-processing.  
Processing details are shown in the table below.

| Function | Details |  
|:---|:---|  
|conv_yuv2rgb |Convert YUY2 to RGB|  
|resize |Resize to 224x224|  
|cast_to_fp16 | Cast data to FP16 for DRP-AI|  
|normalize | Normalize pixel values with mean and standard deviation|  
|transpose | Transpose HWC to CHW order |  
|cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input|  

Note: Current DRP-AI Pre-processing Runtime only allows YUY2 input data and specific pre-processing sequence. BGR/RGB format will be supported in the future.  

For more details, please refer to [DRP-AI Pre-processing Runtime](#drp-ai-pre-processing-runtime)  


---  
# Appendix
## DRP-AI TVM[^1] Runtime Library API
Regarding the list of DRP-AI TVM[^1] Runtime API used in the application, please see [MERA Wrapper API References](../docs/Runtime_API.md)

## DRP-AI Pre-processing Runtime
DRP-AI Pre-processing Runtime enables high performance AI pre-processing using the hardware accelerator, DRP-AI.  

Note that current DRP-AI Pre-processing Runtime only allows YUY2 input data and specific pre-processing sequence.  
BGR/RGB format will be supported in the future.  
For more details on the restriction, please refer to [Restrictions](#restrictions).<br>
To see how to prepare the input data file, please refer to [How to prepare YUY2 image data](#how-to-prepare-yuy2-image-data).  

### File configuration
#### Source Code
| File | Details |  
|:---|:---|  
|PreRuntime.cpp | Pre-processing Runtime source code |
|PreRuntime.h | Pre-processing Runtime header file. |

#### Pre-processing Runtime Object
Pre-processing Runtime uses DRP-AI to run AI pre-procesing, i.e., resize/normalize.  
AI pre-processing differs depending on the AI model, input data format, etc..  
Pre-processing details can be specified in Pre-processing Runtime Object files.  
File configuration of Pre-processing Runtime Object is as follows.  

| File | Details |  
|:---|:---|  
|`<PREFIX>_addrmap_intm.txt` | Address map file to deploy the Pre-processing Runtime Object on memory. |  
|`<PREFIX>_weight.dat` | Weight file |  
|`<PREFIX>_drpcfg.mem` | Configuration file. |  
|`aimac_desc.bin` | DRP-AI Descriptor file. |  
|`drp_desc.bin` | DRP-AI Descriptor file. |  
|`drp_param.bin` | DRP-AI Parameter file. |  
|`drp_param_info.txt` | DRP-AI Parameter Information. |  

<br>

##### Restrictions
Currently, Pre-processing Runtime Object files are only provided as binary files, which means it allows a fixed sequence.  
Provided fixed sequence is as follows.

| No. | Function | Details |   
|:---|:---|:---|  
| 1 |conv_yuv2rgb |Convert YUY2 to RGB. <br>Default input size is 4096x2160.|  
| 2 |resize |Resize to specified size. <br>Default is 640x640. |  
| 3 |cast_to_fp16 | Cast data to FP16 for DRP-AI.|  
| 4 |normalize | Normalize pixel values with mean and standard deviation.  <br>Default value are mean=[0, 0, 0] and std=[1/255, 1/255, 1/255].|  
| 5 |transpose | Transpose HWC to CHW order. |  
| 6 |cast_fp16_fp32 | Cast FP16 data to FP32 for DRP-AI TVM[^1] input.|

Notes:
- Sequence order cannot be changed.
- All processing operators will be executed.
- Input image size of conv_yuv2rgb must be smaller than or equal to **4096x2160** (WxH).
- Output image size of resize must be smaller than or equal to **640x640** (WxH).  
- Input format must be in YUY2 (YUV422).


### API List
| Function | Description |  
|:---|:---|  
|[Load()](#load) | Loads Pre-processing Runtime Object files. |  
|[Pre()](#pre) | Runs pre-processing on DRP-AI. |  

### API Specification

#### Load
| Items | Details |  
|:---|:---|  
| Overview | Loads Pre-processing Runtime Object files.  |  
| Definition | `uint8_t PreRuntime::Load(const std::string pre_dir)` |  
| Description | Initialize Pre-processing Runtime.  <br> Load Pre-processing Runtime Object files stored in `pre_dir` and deploy them to DRP-AI memory area. |  
| Arguments | `const std::string pre_dir` = Directory name that contains Pre-processing Runtime Object files. |  
| Return | 0  = if function succeeded <br> >0 = otherwise|  

#### Pre
| Items | Details |  
|:---|:---|  
|Overview | Run pre-processing on DRP-AI with specified parameters. |  
| Definition | `uint8_t PreRuntime::Pre(s_preproc_param_t* param, float** out_ptr, uint32_t* out_size)` |  
| Description | Run pre-processing on DRP-AI.  Users can specify the pre-processing parameters in `param`.  The DRP-AI output will be stored in `out_ptr` and its size will be stored in `out_size`. |  
| Arguments | `s_preproc_param_t* param` = Pre-processing parameters to be run.  Refer to [s_preproc_param_t](#s_preproc_param_t) for more details.<br> `float** out_ptr` = Array pointer for DRP-AI output data.  Buffer will last until next call of this function. <br>  `uint32_t* out_size` = DRP-AI output buffer size. |  
| Return | 0  = if function succeeded <br> >0 = otherwise|  

### Structure
#### s_preproc_param_t
DRP-AI pre-processing parameter container.  
Members are shown below.  

| Type | Member variable | Details |Range |Invalid value  |  
|:---|:---|:---|---:|---:|  
|uint16_t | pre_in_shape_w | Input image width of pre-processing. |0~4096 |0xFFFF |  
|uint16_t | pre_in_shape_h | Input image height of pre-processing. |0~2160  |0xFFFF |  
|uint32_t | pre_in_addr | Start address of continuous memory area which stores the input image data. |0~0xFFFFFFFE |0xFFFFFFFF |  
|uint16_t | pre_in_format | Input image format.  Currently YUY2(YUV422) is supported. |0 |0xFFFF |  
|uint8_t | resize_alg | Resize algorithm. |0=Nearest, 1=Bilinear |0xFF |  
|uint16_t | resize_w | Output image width of resize operator. |0~640 |0xFFFF |  
|uint16_t | resize_h | Output image height of resize operator. |0~640 |0xFFFF |  
|float[3] | cof_add | Addition coefficients for normalize operator. |-(FLT_MAX-1)<br>~FLT_MAX |-FLT_MAX |  
|float[3] | cof_mul | Multiplication coefficients for normalize operator. |-(FLT_MAX-1)<br>~FLT_MAX |-FLT_MAX |  

Notes:  
- If users specified the invalid value, the parameter uses the current value.
- pre_in_addr must be specified when `s_preproc_param_t` is defined. For other members, define the value if changes are necessary.

### Integration
1. Place the files listed in [File Configuration](#source-code) in the project directory.  
2. Include the `PreRuntime.h` file in the application source code.  
```cpp
#include "PreRuntime.h"
```
3. Include the Pre-processing source code in the compile command.  
For example, in cmake for [Application Example](.), modify the CMakeLists.txt as follows.
- Before  
```txt
set(SRC tutorial_app.cpp MeraDrpRuntimeWrapper.cpp)
set(EXE_NAME tutorial_app)

add_executable(${EXE_NAME} ${SRC})
target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
```

- After  
```txt
set(SRC tutorial_app.cpp MeraDrpRuntimeWrapper.cpp PreRuntime.cpp)
set(EXE_NAME tutorial_app)

add_executable(${EXE_NAME} ${SRC})
target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
```

## How to prepare YUY2 image data
YUY2 image data can be generated using ffmpeg.  
On Ubuntu20.04, use the following command to install ffmpeg and convert Windows Bitmap image to YUY2 image data.  
```sh
sudo apt install -y ffmpeg  
ffmpeg -i <INPUT>.bmp -s <IMAGE_WIDTH>x<IMAGE_HEIGHT> -pix_fmt yuyv422 <OUTPUT>.yuv
```
For example, the `sample.yuv` used in [Application Example](.) is generated using following command.  
```sh
ffmpeg -i sample.bmp -s 640x480 -pix_fmt yuyv422 sample.yuv
```

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
