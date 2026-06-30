# How to convert Swin Transformer models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                | Download model name         |Input shape    | Task                    |   
|-------------------------|----------------------------|---------------|-------------------------|   
| Swin Transformer(tiny)  |swin_tiny_patch4_window7_224 |(1,3,224,224)  | Image Classification    |   


---

License: [Apache 2.0](https://huggingface.co/microsoft/swin-tiny-patch4-window7-224)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for Swin Transformer models.

```sh
apt update
apt install -y python3-venv

python3 -m venv ${TVM_ROOT}/convert/venvs/Swin
. ${TVM_ROOT}/convert/venvs/Swin/bin/activate
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.16.0 onnxruntime onnxsim numpy==1.26.4 matplotlib==3.10.3 pandas==2.2.3 protobuf==3.20.* timm==1.0.9
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

# The following is an example for Swin-tiny
python download_swin_onnx.py \
    -s 1,3,224,224 \
    -n swin_tiny_patch4_window7_224

# Optimize onnx files using onnxsim
cd output/swin_tiny_patch4_window7_224_onnx
onnxsim swin_tiny_patch4_window7_224.onnx swin_tiny_patch4_window7_224.onnx

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── swin_tiny_patch4_window7_224_onnx
           └── swin_tiny_patch4_window7_224.onnx
```

## 4. Delete the environment for Swin Transformer models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/Swin
```

## 5. Next Step

**:bulb: Note** For the models provided in this guide, inference time can be optimized by enabling the "-f float" option.
```sh
# EXAMPLE
$ python3 compile_onnx_model_quant.py \
    ./target_model.onnx \
    -o test_out \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    -f float32
```
:information_source: To maintain accuracy, please refer to the guide below and configure the calibration images and preprocessing appropriately.


Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)

## Appendix   

### :information_source: Notes on Large Models   
#### RZ/V2H & RZ/V2N  
RZ/V2H EVK with the default settings, a 512MB area is allocated to the DRP-AI area in the Linux memory map settings.
If the object after AI model conversion does not fit within this size, it cannot be processed. An error will occur when running on the board.   
If you expand the DRP-AI area in the Linux memory map settings, you will be able to handle a larger AI model.
For example on memory expansion methods, please refer to this [document](https://github.com/renesas-rz/rzv_drp-ai_tvm/blob/main/docs/model_list/how_to_expand_drpai_memory.md).

#### RZ/V2M & RZ/V2L
If the model size is too large, it may exceed the DRP-AI memory area and cannot be executed on the board. Using a smaller model is recommended.

----