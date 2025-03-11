# DRP-AI TVM[^1] Application Example (RZ/V2H, RZ/V2N)

This page explains how to use the application provided in this directory, which is the example to run ResNet18 inference on the target board.

## Overview

To run inference with the AI model data compiled by DRP-AI TVM[^1], an inference application is necessary.  
At this point, this application must be written in C++ for the DRP-AI TVM[^1] Runtime Library.  
Here, we explain how to compile and deploy the application example for ResNet-18, which has already been compiled in the [Compile with Sample Scripts (RZ/V2H)](../tutorials/tutorial_RZV2H.md).

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
#./tutorial_app 5 #The way run DRP-AI with 315Mhz@V2H


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

## Build benchmark runtime
"benchmark.cpp" is a sample program that measures only the inference time. This benchmark program can measure the inference time of various models, such as Classification, Detection and so on without runtime program modification.    

### 1. Build 

The build procedure is as follows.
```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake \
  -DV2H=ON -DBENCH=ON ..
make -j$(nproc)
```
A runtime program: "benchmark" is generated. Please copy this runtime program to the V2H board to measure the inference time.

### 2. Run on V2H board
The execution command on the V2H board is as follows. Specify the model directory and the number of loops.   

> ./benchmark [modle directory] [loop time]

An example of a folder structure is shown below.   
```sh
/
└── tvm
    ├── libtvm_runtime.so
    ├── model_dir_onnx
    │   ├── deploy.json
    │   ├── deploy.params
    │   ├── deploy.so
    └── benchmark
```
As shown below, the inference time is displayed on the console.

```sh
./benchmark ./model_dir_onnx 100
...
Running tvm runtime
 Loop time : 100
[TIME] AI Processing Time: 2.80 msec.
Get profile data
...
Profile data is saved as ./profile_table.txt & profile.csv
```

## Build inference runtime

In this section, we will introduce a runtime program that performs only inference processing. 
This sample program read binary data as input and saves the inference results as binary data. 
Combining this program with pre- and post-processing written in Python code makes debugging easier. 
If you have python code that run inference evaluation on the host PC, please use that code. 
The sample usage is described below.   

1. Build runtime 
2. Perform pre-processing and prepare input data in binary format by Python code
3. Execute inference on the RZ/V board
4. Verify the results by Python code

When developing an actual application, it is recommended to implement the pre- and post-processing into a C runtime program.

### 1. Build
The build procedure is as follows. Please add **-DINFERENCE=ON** option.
```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/runtime.cmake -DV2H=ON -DINFERENCE=ON ..
make -j$(nproc)
```
The runtime program (**run_inference**) is generated. 

### 2. Prepare input bin file by Python
By saving the pre-processing results as binary file, it is possible to use them as input data for inference on the board.    

Below is an example of pre-processing in Python. Reading bmp image file, running pre-processing, then saving the results as binary data.   

```py
from PIL import Image
import numpy as np

im = Image.open("./exe/sample.bmp")
im = im.resize((224,224),Image.BILINEAR)  # resize to 224x224
im_np = np.asarray(im)               # convert to numpy array
im_np = im_np.transpose(2,0,1)       # Transpose to (3,224,224)
im_np = im_np.astype(np.float32)     # cast to FP32
im_np = im_np/255                    # data range 0.0 - 1.0
mean = [0.485, 0.456, 0.406]         # define normalize parameters
std = [0.229, 0.224, 0.225]          # define normalize parameters
for i in range(3):                   # normalize
    im_np[i] = (im_np[i] - mean[i])/std[i]
im_np.tofile("input_0.bin")          # save as binary file
# The input_0.bin size : 3*224*224*4 = 602,112[Byte]
```
The results of the pre-processing performed in Python need to be saved as a binary file, which can be used as input data for the inference application(run_inference). 
The above code is just a sample, so please modify the code according with the model you want to evaluate.

### 3. Run inference on V2H/V2N board

The execution command on the V2H/N board is as follows. Specify the model directory. Please copy "input_0.bin" file generated by pre-processing python into target model directory.  

> ./run_inference [modle directory]

An example of a folder structure is shown below.   
```sh
/
└── tvm
    ├── resnet50_onnx
    │   ├── input_0.bin ... input data generated Python code
    │   ├── deploy.json
    │   ├── deploy.params
    │   └── deploy.so
    └── run_inference 
```
As shown below, the inference result is saved as a binary file(data_out_0_fp16.bin).

```sh
./run_inference ./resnet50_onnx
...
/
└── tvm
    ├── resnet50_onnx
    │   ├── input_0.bin ... input data generated Python code
    │   ├── deploy.json
    │   ├── deploy.params
    │   ├── deploy.so
    │   ├── data_out_0_fp16.bin ... output dat. dtype is fp16. 
    └── run_inference 
```
If the output data type is fp32, a file named **dat_out_0_fp32.bin** will be saved. 
Additionally, if the model has multiple outputs, multiple files will be saved
 (e.g., data_out_0_fp16.bin, data_out_1_fp16.bin, ...).   

### 4. Check output bin file by Python
Next, perform post-processing by Python code.    
If post-processing code is available on HOST PC, you can read the inference results from the V2H board as numpy data arrays and combine them to verify the inference results.    

Below is an example of how to check the results for a classification task. 
The labels saved in the {DRP-AI TVM}/apps/exe folder.   

```py
import numpy as np

def show_top5(pred_out):
    # run softmax
    sum = np.sum(np.exp(pred_out))
    pred_softmax = np.exp(pred_out)/sum
    # show label
    top5 = pred_softmax.argsort()[-5:]
    # open label data
    with open("./exe/ImageNetLabels.txt","r") as f:
        labels = f.readlines()
    # Get inference result
    for i in range(5):
        index = top5[4-i]
        name = labels[index+1].replace("\n","")
        score = pred_softmax[index]
        print(f" TOP {i+1} : class {name:<20}, Score : {score:.3f}, index : {index}")

print("[V2H result]")
pred = np.fromfile("./data_out_0_fp16_0.bin",np.float16).astype(np.float32)
#pred = np.fromfile("./data_out_0_fp32_0.bin",np.float32) # FP32 Case
show_top5(pred)
```

You will get results like the following:

```sh
[V2H result]
 TOP 1 : class beagle              , Score : 0.625, index : 162
 TOP 2 : class English foxhound    , Score : 0.182, index : 167
 TOP 3 : class Walker hound        , Score : 0.108, index : 166
 TOP 4 : class basset              , Score : 0.038, index : 161
 TOP 5 : class bloodhound          , Score : 0.011, index : 163
```
This process is just an example for a classification task.
This approach can also be applied to other tasks such as object detection or segmentation. 

If the expected inference results are not obtained, please recheck the normalization coefficients in compilation script, the calibration data settings, the data types and data order in pre- and post-processing.


[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
