# How to convert ViT_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| tiny                                                          |vit_tiny_patch16_224 |(1,3,224,224)  | Image Classification   |
| small                                                           |vit_small_patch16_224 |(1,3,224,224)  | Image Classification   |
| base                                                           |vit_base_patch16_224 |(1,3,224,224)  | Image Classification   |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for ViT_onnx models.

```sh
apt update
apt install -y python3-venv

python3 -m venv ${TVM_ROOT}/convert/venvs/ViT
. ${TVM_ROOT}/convert/venvs/ViT/bin/activate
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.9.0 onnxruntime==1.16.1 numpy==1.23.5 matplotlib==3.2.2 pandas==1.3.3 protobuf==3.20.* timm==1.0.9 onnxsim
```

## 3. Download and save as ONNX (.onnx) files.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-s     |`Input shape` column in the table above         |
|-n     |`Download model name` column in the table above |
---

```sh
cd ${TVM_ROOT}/convert

# The following is an example for ViT-base
python download_vit_onnx.py \
    -s 1,3,224,224 \
    -n vit_base_patch16_224
onnxsim ./vit_base_patch16_224.onnx ./vit_base_patch16_224.onnx 
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── vit_base_patch16_224_onnx
           └── vit_base_patch16_224.onnx
```

## 4. Delete the environment for ViT_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ViT
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_onnx_model_quant.py ../convert/output/vit_base_patch16_224_onnx/vit_base_patch16_224.onnx -o vit_base_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 -f float32 
```

----