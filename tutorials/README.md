# Compile AI models (RZ/V2L, RZ/V2M, RZ/V2MA)  

This tutorial show how to compile an onnx AI model.
And how to compile for **RZ/V2H**, [see here](tutorial_RZV2H.md).

### Download pretraind onnx model

Before running compile script, prepare pretrained onnx model. This tutorial uses resnet18 of onnx model zoo.

```sh
cd ${TVM_ROOT}/tutorials/
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx
```

### Import libraries

The compilation is done using python. The processing procedure in python is described below.   
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

### Load pretraind model

In this tutorial, downloaded onnx model is used to show sample compilation flow.

```py
onnx_model = onnx.load_model("./resnet18-v1-7.onnx")
```

If you want to know other sample case, please reference below tutorials.   
[Compile Deep Learning Models](https://tvm.apache.org/docs/how_to/compile_models/index.html)

### Compile model with relay

```py
input_node_name = "data"
input_shape     =s[1,3,224,224]
shape_dict = {input_node_name:input_shape}
mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)
```

### Run backend to build deploy files

Before run build function, set drp configuration for runtime.

```py
drp_config_runtime = {
    "interpreter": False,
    "addr_map_start": 0x0,
    "toolchain_dir": <TRANSRATOR PATH>,
    "sdk_root": <SDK PATH>
}
```

The definition of "addr_map_start" is currently not working, so 0x0 is set.

"toolchain_dir" and "sdk_root" is directories where DRP-AI Translator and SDK are installed, respectively.

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

After build operation, the following files will be generated.

```txt
tutorials
 └── resnet18_onnx
      ├─ deploy.so
      ├─ deploy.params
      ├─ deploy.json
```

These files are needed to run the run-time program on a evaluation board. See ["How to Run the Application"](../apps/README.md)  for how to deploy.

----

# Sample Scripts

There are three types of sample scripts to compile an AI model.

1. Compile script with onnx model [CPU and DRP-AI accelerator]
2. Compile script with pytorch model [CPU and DRP-AI accelerator]
3. Compile script with onnx model [Only CPU]

All scripts use the DRP-AI Pre-processing Runtime Compile Module to generate Object files for pre-processing, which is `preprocess` directory in the output directory.  
For more details on DRP-AI Pre-processing Runtime, please refer to its [Documentation](../docs/PreRuntime.md).

## 1. Compile onnx models

### 1.1. Example using Resnet from the official ONNX model zoo

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

----

## 2. Compile pytorch models

### 2.1. Example using Resnet from torchvision

```sh
# At <drp-ai_tvm>/tutorials
# Download resnet18 model from torchvision & save it as resnet18.pt
python3 sample_save_torch_model.py
# Run DRP-AI TVM[*1] Compiler script
python3 compile_pytorch_model.py \
    ./resnet18.pt \
    -o resnet18_torch \
    -s 1,3,224,224
```

**Note**: Only TorchScripted model is supported. See [here for reference](https://tvm.apache.org/docs/how_to/compile_models/from_pytorch.html).

----

## 3. Compile tensorflow models

### 3.1. Example using Resnet from TensorFlow Hub

```sh
# At <drp-ai_tvm>/tutorials
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

----

## 4. Compile using CPU-only deploy mode

### 4.1. Example using Resnet from the official ONNX model zoo

```sh
# At <drp-ai_tvm>/tutorials
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx
python3 compile_cpu_only_onnx_model.py \
        ./resnet18-v1-7.onnx \
        -o resnet18_onnx_cpu \
        -s 1,3,224,224 \
        -i data
```

Please see [Compile API](../docs/Compile_API.md) for details

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
