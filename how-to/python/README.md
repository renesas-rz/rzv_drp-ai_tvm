# How to install and run Python API


## Index
- [Overview](#overview)
- [File Configuration](#file-configuration)
- [How to Use](#how-to-use)
  - [Step 1. Preparation](#step-1-preparation)
  - [Step 2. Install Python API](#step-2-install-python-api)
  - [Step 3. Run AI processing](#step-3-run-ai-processing)
- [Script Specification](#script-specification)
- [Licenses](#licenses)
- [Appendix](#appendix)


## Overview
RUHMI[^1] provides C++ and Python API for running AI models on the target board.  
This page provides sample scripts to run ResNet and YOLOX models using the Python API.  
- ResNet18 ([ONNX Model Zoo](https://github.com/onnx/models))
- YOLOX-S ([YOLOX repository](https://github.com/Megvii-BaseDetection/YOLOX/tree/main/demo/ONNXRuntime))

> To see C++ sample, please refer to [Application Example](../../apps/build_appV2H.md) or [AI Sample Application](../README.md#ai-sample-application-for-rzv2h-and-rzv2n).  

The following diagram shows the implementation flow for running the sample scripts.  
Please note that pre-processing, inference, and post-processing are provided as separate Python scripts.  
Users can reuse the inference script to validate other AI models as well.

<img src=./img/PythonAPI_impl_flow.jpg width=400>

The following table shows the target environment.  
| Device | AI SDK | [Compile mode](../../docs/About_mera.md) | Status |
| ---- | ---- | ---- | ---- |
| RZ/V2H |[RZ/V2H AI SDK v6.00 or later](https://www.renesas.com/software-tool/rzv2h-ai-software-development-kit) | mera2 | :white_check_mark: SUPPORTED |
| RZ/V2N |[RZ/V2N AI SDK v6.30 or later](https://www.renesas.com/software-tool/rzv2n-ai-software-development-kit) | mera2| :white_check_mark: SUPPORTED |

> **[Notes]**   
> 1. The Python API feature is supported by RUHMI[^1] R2025-12 (DRP-AI TVM v2.7.0) and later.  
> 2. If you are using the Dunfell AI SDK (earlier than v5.x), please use  installers (*.whl) from a [previous RUHMI version](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/v2.7.0/apps/python). 


## File Configuration
| File/Directory | Details | 
| ---- | ---- | 
| img/ | Directory to store images used in this README.md. |
| resnet/ | Files required to run ResNet model. |
| yolox/ | Files required to run YOLOX model |
| inference.py | Sample script to run AI inference via Python API. |
| *.whl | Wheel files to install Python API. |
| sample_run_resnet.sh | Sample script to run ResNet model with Python API. |
| sample_run_yolox.sh | Sample script to run YOLOX model with Python API. |
| README.md |This document. |

## How to Use
### Step 1. Preparation
#### Prerequisites
- RUHMI[^1] environment is installed according to [Installation](../../setup/README.md).

#### 1. Compile AI models
- ResNet
  1. Follow the instruction in [Compile with Sample Script](../../tutorials/tutorial_RZV2H.md) to compile **ONNX model** and obtain the `resnet18_onnx` directory.  

- YOLOX
  1. Download YOLOX-S model (`yolox_s.onnx`) from [YOLOX repository](https://github.com/Megvii-BaseDetection/YOLOX/tree/main/demo/ONNXRuntime).  
      ```sh
      cd $TVM_ROOT/tutorials
      wget https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_s.onnx
      ```

  2. Create `update_script.sh` and copy and paste the following.
      ```sh
      #!/bin/sh
      sed -i -e 's/256/640/g' $1 #Change input size 
      sed -i -e 's/ 224/ 640/g' $1 #Change input size 
      sed -i -e 's/to_tensor/pil_to_tensor/g' $1
      sed -i -e '/std = stdev/d' $1 #Delete normalize parameter
      sed -i -e '/F.normalize/d' $1 #Delete normalize 
      ```

  3. Create `compile_onnx_model_quant_yolox.py` and run `update_script.sh`.
      ```sh
      git checkout compile_onnx_model_quant.py
      cp compile_onnx_model_quant.py compile_onnx_model_quant_yolox.py
      sh update_script.sh compile_onnx_model_quant_yolox.py
      ```

  4. Compile the model by running compile script and obtain the `yolox_onnx` directory.  
      ```sh
      python3 compile_onnx_model_quant_yolox.py \
      yolox_s.onnx \
      -o yolox_onnx \
      -t $SDK \
      -d $TRANSLATOR \
      -c $QUANTIZER \
      -s 1,3,640,640 \
      --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
      ```

#### 2. Copy files to microSD card

1. **[For ResNet]** Copy necessary files to `python/resnet` directory.
    ```sh
    cd $TVM_ROOT/how-to/python/resnet
    cp $TVM_ROOT/apps/exe/sample.bmp .
    cp $TVM_ROOT/apps/exe/ImageNetLabels.txt .
    cp $TVM_ROOT/tutorials/resnet18_onnx . -a
    ```
    - After running the command above, `python/resnet` directory should contain following files/directories.  

      | File/Directory | Details | 
      | ---- | ---- | 
      | ImageNetLabels.txt  |  Label list for ResNet18 post-processing. |
      | postproc.py| Post-processing script.|
      | preproc.py| Pre-processing script.|
      | resnet18_onnx | Compiled AI model objects. |
      | sample.bmp | Sample input image.|


2. **[For YOLOX]** Download/copy necessary files to `python/yolox` directory.
    ```sh
    cd $TVM_ROOT/how-to/python/yolox
    wget https://raw.githubusercontent.com/Megvii-BaseDetection/YOLOX/refs/tags/0.3.0/yolox/data/datasets/coco_classes.py
    wget https://raw.githubusercontent.com/Megvii-BaseDetection/YOLOX/refs/tags/0.3.0/yolox/utils/demo_utils.py
    wget https://raw.githubusercontent.com/Megvii-BaseDetection/YOLOX/refs/tags/0.3.0/yolox/utils/visualize.py
    wget https://raw.githubusercontent.com/Megvii-BaseDetection/YOLOX/refs/tags/0.3.0/yolox/data/datasets/voc_classes.py
    cp $TVM_ROOT/tutorials/yolox_onnx . -a
    ```  
    - After running the command above, `python/yolox` directory should contain following files/directories.  

      | File/Directory | Details | 
      | ---- | ---- | 
      | coco_classes.py| Label list for COCO dataset. |
      | demo_utils.py| Used for YOLOX post-processing.|
      | dog.jpg| Sample input image.|
      | postproc.py| Post-processing script.|
      | preproc.py| Pre-processing script.|
      | visualize.py| Used for YOLOX post-processing.|
      | voc_classes.py| Label list for VOC dataset.<br>Only used when using models trained with VOC dataset. |
      | yolox_onnx| Compiled AI model objects.|

3. Copy entire `python` directory to the microSD card for target board.  
  Expected directory structure on the board is shown below.
    ```sh
    python/
    ├── inference.py
    ├── mera-*-cp312-cp312-linux_aarch64.whl
    ├── mera2_runtime-*-cp312-cp312-linux_aarch64.whl
    ├── resnet/
    ├── sample_run_resnet.sh
    ├── sample_run_yolox.sh
    ├── tvm-*-cp312-cp312-linux_aarch64.whl
    └── yolox/
    ```   
### Step 2. Install Python API

#### Prerequisites
- To install Python API, the target board must be connected to the internet.
- To run Python API, python3-pip and numpy packages are required (already installed in AI SDK v6.00 and later.)  
  Please refer to [Appendix](#a1-verifying-whether-python3-pip-and-python3-numpy-are-installed-on-ai-sdk) to verify whether they are installed.  

#### Instruction
> Note: This procedure is required only for the initial setup.  

1. Check following *.whl files are present on the target board.
    ```sh
    python/
    ...
    ├── mera-*-cp312-cp312-linux_aarch64.whl
    ├── mera2_runtime-*-cp312-cp312-linux_aarch64.whl
    └── tvm-*-cp312-cp312-linux_aarch64.whl
    ```   

2. Execute python3-pip to install libraries.
    ```sh
    cd <path_to_python_directory>/python
    python3 -m pip install *.whl
    ```

3. Following log will be shown if libraries are installed successfully.
    ```sh
    Successfully installed PyYAML-6.0.3 cloudpickle-3.1.2 contourpy-1.3.3 cycler-0.12.1 decorator-5.3.1 fonttools-4.63.0 kiwisolver-1.5.0 matplotlib-3.10.9 mera-2.6.0 mera2-runtime-2.6.0 portalocker-3.2.0 psutil-7.2.2 pyparsing-3.3.2 scipy-1.17.1 seaborn-0.13.2 synr-0.6.0 tabulate-0.10.0 tornado-6.5.6 tqdm-4.67.3 tvm-0.7.0.dev1619+g57142b014 tzdata-2026.2
    ```
    > Note 1: If the installation encounter errors, please check the internet connection is valid.  
    > Note 2: Once libraries are installed, you can disconnect the internet.  
  
  
### Step 3. Run AI processing
The sample script, `sample_run_*.sh`, is a shell script that executes the procedures described in the table below.  
Each procedure is implemented as a separate Python script file.  
| Processing | Input data| Output data | Filename | Note|
| ---- | ---- |  ---- | ---- | ---- | 
| Pre-processing  |Input image |Input binary file| `preproc.py` | Each script is designed **specifically for its corresponding AI model**. |
| Inference  |Input binary file |Output binary file| `inference.py` | The script is designed for general use across various models. <br>For more details, see [Script Specification](#script-specification) |
| Post-processing |  Output binary file | Result | `postproc.py` | Each script is designed **specifically for its corresponding AI model**. |


#### Prerequisites
- Users must complete the instructions in [Step 1. Preparation](#step-1-preparation) and [Step 2. Install Python API](#step-2-install-python-api). 
<!-- Please compiled ONNX Resnet model to run the sample python script. For details, refer to the [tutorials](../../tutorials/tutorial_RZV2H.md). After successful compilation, copy the output directory (e.g. resnet18_onnx) to V2H(V2N) board, and run it on the board like below. -->
#### Run ResNet sample script  
1. Run the following command to run the script.  
    ```sh
    sh sample_run_resnet.sh
    ```
    - Sample script takes input image [`sample.bmp`](../../apps/exe/sample.bmp) and shows Top-5 classification result.  
    - The following log is the expected output of shell script.  

      ```sh
      root@rzv2h-evk:/home/weston/python# sh sample_run_resnet.sh
      Python API ResNet sample script
      [ Pre-processing ]
      Sample script for ResNet
      Load input data
        Input image:  resnet/sample.bmp
      Save output data
        Saved pre-processed data:  resnet/input_0.bin

      [ Inference ]
      Load arguments
        Runtime model data path: resnet/resnet18_onnx
      Load input data
        Load input binary file : resnet/input_0.bin
        Input shape : (1, 3, 224, 224), Dtype : float32
      Run inference
        Loop time : 10
        Average latency: 2.264 ms
      Save output data
        [0] , (1, 1000)
        Saved output result : resnet/resnet18_onnx/output_0_fp16.bin

      [ Post-processing ]
      Sample script for ResNet
      Load input data
        AI inference output binary data:  resnet/resnet18_onnx/output_0_fp16.bin

      Result
      TOP 1 : class beagle              , Score : 0.468, index : 162
      TOP 2 : class English foxhound    , Score : 0.284, index : 167
      TOP 3 : class Walker hound        , Score : 0.146, index : 166
      TOP 4 : class basset              , Score : 0.039, index : 161
      TOP 5 : class bloodhound          , Score : 0.017, index : 163
      ```

#### Run YOLOX sample script
1. Run the following command to run the script.  
    ```sh
    sh sample_run_yolox.sh
    ```
    - YOLOX sample script takes [`dog.jpg`](yolox/dog.jpg) input image, run object detection and saves the output image `result.jpg` with bounding boxes.  
    - The following log is the expected output of shell script.  

      ```sh
      root@rzv2h-evk:/home/weston/python# sh sample_run_yolox.sh
      Python API YOLOX sample script
      [ Pre-processing ]
      Sample script for YOLOX
      Load input data
        Input image:  yolox/dog.jpg
      Save output data
        Saved pre-processed data:  yolox/input_0.bin

      [ Inference ]
      Load arguments
        Runtime model data path: yolox/yolox_onnx
      Load input data
        Load input binary file : yolox/input_0.bin
        Input shape : (1, 3, 640, 640), Dtype : float32
      Run inference
        Loop time : 10
        Average latency: 20.546 ms
      Save output data
        [0] , (1, 8400, 85)
        Saved output result : yolox/yolox_onnx/output_0_fp16.bin

      [ Post-processing ]
      Sample script for YOLOX
      Load input data
        Load input image file : yolox/dog.jpg
        Dataset               : COCO with 80 classes
        YOLOX output shape    : (1, 8400, 85)
        AI inference output binary data: yolox/yolox_onnx/output_0_fp16.bin
      Save output data
        Saved result image: result.jpg
      ```
    - Following is the expected output image (`result.jpg`).  

      <img src=./img/result.jpg width=250>


##### Note for YOLOX model differences
YOLOX model used in this sample is obtained from official YOLOX repository without cutting decoding.  
If you would like to use YOLOX model with cutting decoding (explained in [How to convert yolox_onnx models](../../docs/model_list/how_to_convert/How_to_convert_yolox_onnx_models.md)), please use following command.  
```
sh sample_run_yolox.sh with_cut
```
> Note: Please be aware that `sample_run_yolox.sh` use VOC dataset as default for `with_cut` option.  
Users can switch COCO and VOC dataset by modifying the shellscript.  

## Script Specification
The inference script `inference.py` can also be used other AI models for validation purposes.  
> Note: Since `preproc.py` and `postproc.py` are different for each AI model, please refer to each script for processing details.  

It has following functionalities.
- Run AI model inference without pre/post-processing.  
- Obtain the average AI inference processing time without system dependencies.  
- Save AI inference output as a binary file.  

### Usage  
`inference.py` allows following command line input options.

```sh
python3 inference.py \
  --model_path <model_directory> \
  --start_address <start_addr> \
  --frequency_index <freq> \
  --input_shape <input_shape> \
  --input_bin_file <input_file> \
  --input_dtype <input_type> \
  --loop <num_loop>
```
|Option| Operand| Description | Required | Default |
|---|:--|:--|---|---|
| `--model_path`  | `<model_directory>`| Directory path to the AI model objects compiled by RUHMI[^]. | Mandatory | N/A |  
|`--start_address`| `<start_addr>`|Hexadecimal address of the DRP-AI memory area that AI model objects use.|Optional|Start address of DRP-AI memory area. (Depends on each product.)|
|`--frequency_index`|`<freq>`| DRP-AI frequency index.|Optional|1|
| `--input_shape`  | `<input_shape>`| AI model input shape which also be used when compiling the model. | Optional | 1,3,224,224 |  
| `--input_bin_file`  | `<input_binary_file>`| Filepath to input data after pre-processing in binary file format.<br>If not specified, random input data will be used.  | Optional | None | 
|`--input_dtype`|`<input_type`|Input data type. | Optional|float32|
|`--loop`|`<num_loop>`|Integer number of loop iterations to evaluate inference time | Optional|10|
  
### Output 
`inference.py` saves the AI inference result as binary files in <model_directory>(e.g., `<model_directory>/output_0_fp16.bin`).  
You can verify the inference result by applying the post-processing.


## Licenses
Scripts provided on this page are subject to the repository license.  
Files downloaded from external sources are subject to their respective licenses.


## Appendix   

### A1. Verifying whether python3-pip and python3-numpy are installed on AI SDK   

1. To verify whether python3-pip and python3-numpy are installed on AI SDK, boot the device using the SD image and execute the following command:
    ```sh
    rpm -qa | grep python3-pip
    ```   
2. If python3-pip is installed, the version will be displayed as follows:   
    ```sh
    root@rzv2h-evk:~# rpm -qa | grep python3-pip
    python3-pip-24.0-r0.cortexa55
    ```   

3. If python3-pip is not installed, nothing will be displayed. The same method can be used to verify python3-numpy.   


[^1]: RUHMI AI compiler for RZ/V is powered by EdgeCortix MERA&trade;.  
