
# About MERA

MERA is the base framework for DRP-AI TVM[^1] developed by EdgeCortix.    

DRP-AI TVM v2.7.0 and later supports "MERA2" mode and it has added several features. For more information, please refer followings explanation.

## Overview
DRP-AI TVM supports compile function with both MERA2 and the conventional MERA1. You can switch modes by adding the -mera1/-mera2 argument to the compilation script in tutorials folder.    

## Additional features/functions of MERA2 
- Enhanced graph optimize function (e.g. YoloV8, etc.,)
- Python api for runtime (V2H/V2N only)
- IR viewer (V2H/V2N only)

## Default setting at tutorial scripts
Sample compile scripts are provided in **tutorials** folder. The default MERA setting of those scripts are below.   

| File |  Target device | AI model format | Default mode |   
|---- |---- |---- |----|   
| compile_onnx_model_quant.py |  V2H/V2N | onnx | mera2 |   
| compile_tflite_model_quant.py |  V2H/V2N | tensorflow(.tflite) | mera2 |   
| compile_pytorch_model_quant.py | V2H/V2N  | pytorch(.pt) |mera1 |   
| compile_exir_model_quant.py |  V2H/V2N | pytorch(.pte) | mera2 |   
| compile_onnx_model.py |  V2L/V2M/V2MA | onnx | mera1 |   
| compile_tflite_model.py |  V2L/V2M/V2MA | tensorflow | mera1 |   
| compile_pytorch_model.py |  V2L/V2M/V2MA | pytorch(.pt) |mera1 |   
| compile_exir_model.py |  V2L/V2M/V2MA | pytorch(.pte) | mera2 |   


[Note] If a compilation error occurs, please try switching the MERA mode.   

## How to swich mera mode
 
Run compiler with mera2 mode.
```sh
python3 compile_onnx_model_quant.py \
    ./resnet50-v1-7.onnx \
    -o resnet50_v1_onnx \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/   \
    --mera2 
```

Run compiler with mera1 mode.
```sh
python3 compile_onnx_model_quant.py \
    ./resnet50-v1-7.onnx \
    -o resnet50_v1_onnx \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/   \
    --mera1 
```

## Runtime model data
The runtime model data(object files) generated in MERA1 mode and MERA2 mode are different. Below is a list of the files and folders generated in each mode.

MERA1 mode output file/folder list
```shell
<output folder>
└──  deploy.*
```

MERA2 mode output file/folder list
```shell
<output folder>
├── mera.plan
├── project.mdp
├── sub_{idx}_CPU_DRP_TVM
│     └── deploy.*
├── sub_{idx}_DRP_REENSAS
│     ├── deploy.*
│     └── model.*
└── model_subgraphs.json
```


## Runtime application code for runtime
The Runtime source code(C++) is common to both MERA1 and MERA2. Regardless of the MERA compilation mode, no changes to the Runtime program are required. For runtime sample code, please refer to the source code in the **apps** folder. These samples are compatible with both MERA1 and MERA2.

## MERA1/2 mode support list

| AI model format | extension | Support status |   
|----| ----| ----|   
| onnx | .onnx | mera1/mera2 |   
| tensorflow | .tflite | mera1/mera2 |   
| pytorch | .pt | mera1 |   
|         | .pte(EXIR) | mera2 |   

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.