# How to convert MiDaS onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                           | Download model name      | Input shape | Task             |
|----------------------------------------------------------------------------------------------------|--------------------------|-------------|------------------|
| [MiDaS v2.1 Small](https://github.com/isl-org/MiDaS/releases/download/v2_1/midas_v21_small_256.pt) | midas_small_256x256      | (256, 256)  | Object Detection |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for MiDaS onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/midas
. ${TVM_ROOT}/convert/venvs/midas/bin/activate
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1
pip install timm==1.0.9 onnxsim==0.4.36
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-s     |`Input shape` column in the table above         |
|-n     |`Download model name` column in the table above |
|-o     |`Download Directory` column in the table above |

---

```sh
cd ${TVM_ROOT}/convert
# The following is an example for MiDaS v2.1 Small
python download_midas_onnx.py \
    -s 1,3,256,256 \
    -n midas_small_256x256 \
    -o ${TVM_ROOT}/convert/output/midas_small_256x256_onnx

cd ${TVM_ROOT}/convert/output/midas_small_256x256_onnx
onnxsim ./midas_small_256x256.onnx ./midas_small_256x256.onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── midas_small_256x256_onnx
           └── midas_small_256x256.onnx
```
## 4. Delete the environment for MiDaS models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/midas
```

## 6. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For MiDaS-Small, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/midas_small_256x256_onnx/midas_small_256x256.onnx \
    -o midas_small_256x256_onnx \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    -s 1,3,256,256
```

----
