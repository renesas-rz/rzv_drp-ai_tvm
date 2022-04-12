# Compile AI models   

This tutorial show how to compile an onnx AI model.


### Download pretraind onnx model

Before running compile script, prepare pretrained onnx model. This tutorial uses resnet18 of onnx model zoo.

```sh
cd ${TVM_ROOT}/tutorials/
wget https://github.com/onnx/models/raw/main/vision/classification/resnet/model/resnet18-v1-7.onnx
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
    "addr_map_start": 0xC0000000,
    "toolchain_dir": <TRANSRATOR PATH>,
    "sdk_root": <SDK PATH>
}
```
Pleases set **0xC0000000** to "addr_map_start" for Renesas RZ/V2M Evaluation Board Kit. "toolchain_dir" and "adk_root" is directories where DRP-AI Translator and SDK are installed, respectively.

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


## 1. Compile onnx models

### 1.1. Example using Resnet from the official ONNX model zoo
```sh
cd ./tutorials/
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/vision/classification/resnet/model/resnet18-v1-7.onnx
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

----
## 3. Compile using CPU-only deploy mode
### 3.1. Example using Resnet from the official ONNX model zoo
```sh
# At <drp-ai_tvm>/tutorials
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/vision/classification/resnet/model/resnet18-v1-7.onnx
python3 compile_cpu_only_onnx_model.py \
        ./resnet18-v1-7.onnx \
        -o resnet18_onnx_cpu \
        -s 1,3,224,224 \
        -i data
```

----
## Frontend APIs
You can refer to the official TVM site for how to use the frontend of compile script. Please check below site.   
[Compile Deep Learning Models](https://tvm.apache.org/docs/how_to/compile_models/index.html)   


## Backend APIs
### Function   

|API |Note|
|---|---|
| mera.drp.build() | Compile relay to runtime object files.|   


### Sample code:   

```py
from tvm.relay.mera import drp
drp.build(mod, \
               params, \
               "arm", \
               drp_config_runtime, \
               output_dir=output_dir, \
               disable_concat = opts["disable_concat"]
               )
```

| Argument index | Note |
|----|----|
| 1 | mod : IR format AI model generated by TVM frontend.|
| 2 | params : Parameter date  generated by TVM frontend.
| 3 | "arm" : Target architecture. * Currently only "arm" supported.|
| 4 | drp_config_runtime| |
| 5 | output_dir| Output directory to save compile results. |
| 6 | disable_concat|Assign all "concat" operation to the CPU. Default:False|

### DRP runtime config
+ Sample code:
```py
    drp_config_runtime = {
        "interpreter": False,
        "addr_map_start": 0xC0000000,
        "toolchain_dir": opts["drp_compiler_dir"],
        "sdk_root": opts["toolchain_dir"]
    }
```   

| Argument | Note |
|----|----|
| "interpreter" | Interpreter mode setting. True : Run inference on PC. False : Make runtime object files to run target RZ/V board.|
| "addr_map_start"| Start address assigned for DRP-AI |
|"toolchain_dir"| DRP-AI Translator installed directory |
|"sdk_root" | SDK(Cross compiler) installed directory. |

Set **0xC0000000** to "addr_map_start" for Renesas RZ/V2M Evaluation Board Kit


----
## Appendix 1 : Argments of sample scripts

|option|Note|
|----|----|
|-o/--output_dir|Output directory to save compile results.|
|-i/--input_name|AI model input node name. (Not required for pytorch models)|
|-s/--input_shape|AI model input node shape|
|[option]-d/--drp_compiler_dir|DRP-AI Translator installed directory. This argument can be omitted by setting environment variables as shown follwoing: "export TRANSLATOR=<.../drp-ai_translator> "|
|[option]-t/--toolchain_dir|SDK(Cross compiler) installed directory. This argument can be omitted by setting environment variables as shown follwoing: "export SDK=</opt/poky/2.4.3>|
|[option]-c/--disable_concat|Assign all "concat" operation to the CPU. Complex models can also be deployed, but inference speed is slower.|

* Disable concat option   
Using option `-c` or `--disable_concat` to disable concat, so concat operator will be moved to CPU part. Use this option when a runtime error occurs. By lowering the optimization level during compile, it reduces the risk of error occurrence in inference processing.
Example :
<img src=./img/concat_option.jpg width=400>  


[*1]: DRP-AI TVM is powered by EdgeCortix MERA Compiler