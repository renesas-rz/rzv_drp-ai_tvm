# How to compare CPU vs DRP-AI TVM[^1] accuracy differences for ONNX model

This page explains how to confirm how far the DRP-AI TVM[^1] inference accuracy differs from the one on PC.  
The target model is ONNX.   

## 1. Overview
Since DRP-AI runs computation in FP16, there may be differences in inference accuracy between CPU and DRP-AI.  
To check how much differences there are, we can compare the DRP-AI TVM[^1] inference result and inference result computed on PC (x86) as shown below.  

<img src=./img/compare_diff.png width=500>   

To prepare this environment, following files are provided in this directory.
- `tutorials/compile_onnx_model_with_reference.py` : Used in [2. Compile](#2-compile)
- `apps/inference_comparison.cpp` : Used in [3. Deploy](#3-deploy)
- `apps/CMakeLists.txt` : Used in [3. Deploy](#3-deploy)

## 2. Compile
### 1. Preparation  
1. Prepare the necessary environment as explained in [Get Started](../../../README.md).  

2. Copy the `compile_onnx_model_with_reference.py` script to `<drp-ai_tvm>/tutorials` directory.  
```sh
cp $TVM_HOME/../how-to/tips/compare_difference/tutorials/compile_onnx_model_with_reference.py $TVM_HOME/../tutorials
```  
3. Change "addr_map_start" in `compile_onnx_model_with_reference.py` script to appropriate address, i.e. C0000000 for RZ/V2M.  
Note that there are **two** "addr_map_start" to change.

### 2. Compile the model
Compile the model using `compile_onnx_model_with_reference.py` script.  
Comand line arguments are explained in [Tutorial](../../../tutorials).  

### 3. Result
After executed compile script, it generates necessary files for Deploy, including DPR-AI TVM x86 result binary file.

Following is the example log for ResNet18 compiled with prefix `resnet18_reference`.  
```sh
...
Function name: tvmgen_default_tvmgen_default_mera_drp_0
Model size: 46808788
GraphSize: 46808788
Run ONNX Runtime
--- input number: 1 ---
Input name: mera_drp_0_i0. Data type: 1. Shape: 1,3,224,224,
--- output number: 1 ---
Output name: add_173. Data type: 1. Shape: 1,1000,
Non-zero values:  100.0 %
-------------------------------------------------
   Run TVM backend compiler with DRP-AI Translator
...
---------------------------------------------

At sub graph name tvmgen_default_tvmgen_default_mera_drp_0, required drp-ai memory: 34(Mbyte)
[TVM compile finished]
   Please check resnet18_reference directory
```

In `resnet18_reference` directory, you will see following files.
```txt
tutorials
 └── resnet18_reference
      ├─ deploy.so
      ├─ deploy.params
      ├─ deploy.json
      ├─ input_0.bin       : Input data used for x86 inference
      ├─ ref_result_0.bin  : Reference data which is x86 inference result
```

#### Note
This script runs both DRP-AI TVM[^1] x86 inference and Apache TVM inference.  
`Non-zero values:  100.0 %` means that both inference results are close enough and the inference result has no zero value.  
To see more details, please refer to the `compile_onnx_model_with_reference.py`.


## 3. Deploy
### 1. Preparation
1. Prepare the RZ/V2x Linux and the target board as explained in [Get Started Installation](../../../README.md#installation) and [Application Tutorial](../../../apps).  

2. Backup the original files in `<drp-ai_tvm>/apps` directory.  
```sh
mv $TVM_HOME/../apps/CMakeLists.txt $TVM_HOME/../apps/CMakeLists.txt.backup
mv $TVM_HOME/../apps/tutorial_app.cpp $TVM_HOME/../apps/tutorial_app.cpp.backup
```  

3. Copy the necessary files to `<drp-ai_tvm>/apps` directory.  
```sh
cp $TVM_HOME/../how-to/tips/compare_difference/apps/* $TVM_HOME/../apps
```  

### 2. Compile the application
Compile the application as explained in [Application Tutorial](../../../apps).  
After the compilation, following file will be generated.  
- `apps/build/inference_comparison`  

### 3. Deploy to the board
Copy the following files to the rootfs of Boot Environment.  

| Name | Path | Details |  
|:---|:---|:---|  
|Runtime Library | `drp-ai_tvm/obj/build_runtime/libtvm_runtime.so`|Binary provided under [obj](../../../obj/build_runtime) directory.|  
|Model Data | `drp-ai_tvm/tutorials/resnet18_reference`|Model compiled with input and reference data.|  
|Application |`drp-ai_tvm/apps/build/inference_comparison` | Compiled in this [page](#2-compile-the-application). |  

The rootfs should look like below.
```sh
/home
└── root
    └── tvm
        ├── libtvm_runtime.so
        ├── resnet18_reference
        │   ├── deploy.json
        │   ├── deploy.params
        │   ├── deploy.so
        │   ├── input_0.bin
        │   └── ref_result_0.bin
        └── inference_comparison
```

### 4. Run the application
To run the application, execute the following command.
```sh
./inference_comparison <model>
```
`<model>` is the directory name of compiled model, i.e., resnet18_comparison.  

Following is the example result when run the ResNet18 model.
```sh
root@rzv2m:~/tvm# ./inference_comparison resnet18_reference/
[09:55:31] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:47: Loading json data...
[09:55:31] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:53: Loading runtime module...
[09:55:33] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:58: Loading parameters...
[09:55:33] /drp-ai_tvm/apps/MeraDrpRuntimeWrapper.cpp:69: Loading input...
[TIME] AI Processing Time: 23.89 msec.
[09:55:33] /drp-ai_tvm/apps/inference_comparison.cpp:159: max abs diff: 0.0123901
[09:55:33] /drp-ai_tvm/apps/inference_comparison.cpp:160: mean abs diff: 0.0024839
[09:55:33] /drp-ai_tvm/apps/inference_comparison.cpp:161: correct ratio: 0.999, with absolute tolerance: 0.01, relative tolerance: 0.001
[09:55:33] /drp-ai_tvm/apps/inference_comparison.cpp:162: Non-zero values: 100(%)
```

Comparison Result will be as follows.  

| Items | Value | Details |  
|:---|:---|:---|  
|max abs diff |0.012| Maximum value of absolute differences between x86 result and DRP-AI TVM[^1] result.|  
|mean abs diff | 0.002|Mean value of absolute differences between x86 result and DRP-AI TVM[^1] result.|  
|correct ratio |0.999 | Ratio of correct values in DRP-AI TVM[^1] result compared with x86 result. |  
|non-zero values |100% | Percentage of non-zero values in DRP-AI TVM[^1] result. |  

Correct ratio is based on the absolute tolerance of 0.01 and relative tolerance of 0.001, where specified in the application source code.  
If the correct ratio shows higher number, there are less differences between DRP-AI TVM[^1] and x86 inference.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework
