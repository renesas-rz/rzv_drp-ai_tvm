# Compile with Sample Scripts
There are three types of sample scripts to compile an AI model.

1. Compile script with onnx model[CPU and DRP-AI accelerator]
2. Compile script with pytorch model[CPU and DRP-AI accelerator]
3. Compile script with onnx model[Only CPU]

All scripts use the DRP-AI Pre-processing Runtime Compile Module to generate Object files for pre-processing, which is `preprocess` directory in the output directory.  
For more details on DRP-AI Pre-processing Runtime, please refer to its [Documentation](../docs/PreRuntime.md).

## 1. Compile onnx models

### 1.1. Example using Resnet from the official ONNX model zoo

```sh
cd $TVM_ROOT/tutorials/
# Download onnx model from official ONNX model zoo
wget https://github.com/onnx/models/raw/main/validated/vision/classification/resnet/model/resnet50-v1-7.onnx

python3 compile_onnx_model_quant.py \
    ./resnet50-v1-7.onnx \
    -o resnet50_v1_onnx \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ \
    -v 100 

```

----
## 2. Compile pytorch models
### 2.1. Example using Resnet from torchvision

```sh
cd $TVM_ROOT/tutorials/
# Download resnet18 model from torchvision & save it as resnet18.pt
pip3 install torchvision==0.9.1
python3 sample_save_torch_model.py
pip3 install torchvision==0.16.2
# Run DRP-AI TVM Compiler script
python3 compile_pytorch_model_quant.py ./resnet18.pt -o resnet18_torch -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 -s 1,3,224,224
```

----
## 3. Compile tensorflow models

### 3.1. Example using Resnet from TensorFlow Hub

```bash
# Download resnet50 model from TensorFlow Hub
mkdir resnet50-v1
wget https://tfhub.dev/google/imagenet/resnet_v1_50/classification/5?tf-hub-format=compressed -O resnet50-v1.tar.gz
tar zxvf resnet50-v1.tar.gz -C resnet50-v1
# Convert model from TensorFlow to TFLite
python3 sample_save_tflite_model.py

# Run DRP-AI TVM Compiler script
python3 compile_tflite_model_quant.py ./resnet50-v1.tflite -o resnet50_tflite -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 -s 1,224,224,3

```

----

## 4. Compile using CPU-only deploy mode

### 4.1. Example using Resnet from the official ONNX model zoo

Same as [Here](./README.md#41-example-using-resnet-from-the-official-onnx-model-zoo).

```bash
python3 compile_cpu_only_onnx_model.py ./resnet50-v1-7.onnx -o resnet50_v1_onnx_cpu -s 1,3,224,224 -i data
 ```

----

[*1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.  
