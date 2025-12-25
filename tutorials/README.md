# Compile AI models (RZ/V2L, RZ/V2M, RZ/V2MA)

This tutorial shows how to compile AI models.
For instructions on how to compile for **RZ/V2H and RZ/V2N**, [see here](tutorial_RZV2H.md).

## Compile model with Sample Scripts

There are four types of sample scripts to compile an AI model:

1. Compile script with ONNX model \[CPU and DRP-AI accelerator\]
2. Compile script with PyTorch model \[CPU and DRP-AI accelerator\]
3. Compile script with TensorFlow models \[CPU and DRP-AI accelerator\]
4. Compile script with ONNX model \[Only CPU\]

All scripts use the DRP-AI Pre-processing Runtime Compile Module to generate Object files for pre-processing, which is `preprocess` directory in the output directory.  
For more details on DRP-AI Pre-processing Runtime, please refer to its [Documentation](../docs/PreRuntime.md).

### 1. Compile ONNX models

#### 1.1. Example using ResNet18 from the official ONNX model zoo

```sh
cd $TVM_ROOT/tutorials/
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx
python3 compile_onnx_model.py \
    ./resnet18-v1-7.onnx \
    -o resnet18_onnx \
    -s 1,3,224,224 \
    -i data
```

**Post-compilation Note**: The compiled model will be in the `resnet18_onnx` directory.

----

### 2. Compile PyTorch models

#### 2.1. Example using ResNet18 from torchvision

```sh
# At $TVM_ROOT/tutorials
# Download resnet18 model from torchvision & save it as resnet18.pt
python3 sample_save_torch_model.py
# Run DRP-AI TVM[*1] Compiler script
python3 compile_pytorch_model.py \
    ./resnet18.pt \
    -o resnet18_torch \
    -s 1,3,224,224
```

**Note**: Only TorchScripted model is supported. See [here for reference](https://tvm.apache.org/docs/how_to/compile_models/from_pytorch.html).

**Post-compilation Note**: The compiled model will be in the `resnet18_torch` directory.

----

### 3. Compile TensorFlow models

#### 3.1. Example using ResNet50 from TensorFlow Hub

```sh
# At $TVM_ROOT/tutorials
# Download resnet50 model from TensorFlow Hub
mkdir resnet50-v1
wget https://tfhub.dev/google/imagenet/resnet_v1_50/classification/5?tf-hub-format=compressed -O resnet50-v1.tar.gz
tar zxvf resnet50-v1.tar.gz -C resnet50-v1
# Convert model from TensorFlow to TFLite
python3 sample_save_tflite_model.py
# Run DRP-AI TVM[*1] Compiler script
python3 compile_tflite_model.py \
    ./resnet50-v1.tflite \
    -o resnet50_tflite -s 1,224,224,3
```

**Post-compilation Note**: The compiled model will be in the `resnet50_tflite` directory. When running the application with this model, you'll need to replace the label file with the TensorFlow-compatible version. See the [Application README](../apps/README.md#important-notes-for-using-different-model-types) for details.

----

### 4. Compile using CPU-only deploy mode

#### 4.1. Example using ResNet18 from the official ONNX model zoo

```sh
# At $TVM_ROOT/tutorials
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx
python3 compile_cpu_only_onnx_model.py \
        ./resnet18-v1-7.onnx \
        -o resnet18_onnx_cpu \
        -s 1,3,224,224 \
        -i data
```

**Post-compilation Note**: The compiled model will be in the `resnet18_onnx_cpu` directory.

----

## Description of compile scripts

### Download pretrained ONNX model

Before running compile script, prepare pretrained ONNX model. This tutorial uses ResNet18 from the ONNX model zoo.

```sh
cd ${TVM_ROOT}/tutorials/
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx
```

### Import libraries

The compilation is done using Python. The processing procedure in Python is described below.
First, import libraries.

```py
import os
import onnx
import tvm
import sys
import numpy as np

from tvm import relay, runtime
from tvm.relay import mera
from tvm.contrib import graph_executor
from google.protobuf.json_format import MessageToDict
from optparse import OptionParser
```

### Load pretrained model

In this tutorial, downloaded ONNX model is used to show sample compilation flow.

```py
onnx_model = onnx.load_model("./resnet18-v1-7.onnx")
```

If you want to know other sample case, please reference below tutorials.   
[Compile Deep Learning Models](https://tvm.apache.org/docs/how_to/compile_models/index.html)

### Compile model with relay

```py
input_node_name = "data"
input_shape     = [1,3,224,224]
shape_dict = {input_node_name:input_shape}
mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)
```

### Run backend to build deploy files

Before run build function, set DRP configuration for runtime.

```py
drp_config_runtime = {
    "interpreter": False,
    "addr_map_start": 0x0,
    "toolchain_dir": <TRANSLATOR PATH>,
    "sdk_root": <SDK PATH>
}
```

The definition of "addr_map_start" is currently not working, so 0x0 is set.

"toolchain_dir" and "sdk_root" are directories where DRP-AI Translator and SDK are installed, respectively.

To make deploy files for runtime application, run build function.

```py
# Define output directory name
output_dir = "./resnet18_onnx"
# Run build operation of TVM backend
drp.build(mod, \
          params, \
         "arm", \
          drp_config_runtime, \
          output_dir=output_dir, \
          disable_concat = opts["disable_concat"]
)
```

After build operation, the following files will be generated:

```txt
tutorials
 └── resnet18_onnx
      ├─ deploy.so
      ├─ deploy.params
      ├─ deploy.json
      └─ preprocess
          ├─ aimac_desc.bin
          ...
          └─ pp_weight.dat
```

These files are needed to run the run-time program on an evaluation board. See ["How to Run the Application"](../apps/README.md) for how to deploy.

Please see [Compile API](../docs/Compile_API.md) for details.

## Post-Compilation Steps

After compiling your models, you'll need to follow the instructions in the [Application README](../apps/README.md) to run them on the target board. The application is designed to work with a model directory named `resnet18_onnx`, so for other model types, you'll need to create symbolic links or adjust the directory structure as explained in the [Application README](../apps/README.md#important-notes-for-using-different-model-types).

When using TensorFlow models, note that you'll need to replace the label file with the TensorFlow-compatible version, as TensorFlow models use a different label format than the default one provided for ONNX and PyTorch models.

## Note

Start Address [above](#run-backend-to-build-deploy-files) is defined for following preprocessing.

- Input data
  - Shape: 4096x2060x2  
  - Format: YUV  
  - Order: HWC  
  - Type: uint8  
- Output data  
  - Shape: 640x640x3  
  - Format: RGB  
  - Order: CHW  
  - Type: float  
- Preprocessing operations:  
  - Resize  
  - Normalize  

When using preprocessing which has more computation than above conditions, the Start Address **must be larger** than the above value.  

To set the new address, satisfy following conditions.

- The address must be within the DRP-AI memory area, which is reserved memory area in RZ/V Linux Package.
- The address must be aligned with 64-byte boundary.
- The address must be larger than `(Start address of DRP-AI memory area) + (Size of memory area required by DRP-AI Pre-processing Runtime Object files)`

[*1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.  