# How to convert FCN HRNet W18 onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                            | Download model name                  | Input shape | Task                  |
--------------------------------------|--------------------------------------|-------------|-----------------------|
| FCN HRNet W18                       | fcn_hrnetw18_voc12aug_512x512_40k    | (512, 512)  | Semantic Segmentation |
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

python3 -m venv ${TVM_ROOT}/convert/venvs/fcn-hrnetw18
. ${TVM_ROOT}/convert/venvs/fcn-hrnetw18/bin/activate
git clone -b v2.10.0 https://github.com/PaddlePaddle/PaddleSeg ${TVM_ROOT}/convert/repos/fcn-hrnetw18
cd ${TVM_ROOT}/convert/repos/fcn-hrnetw18
pip install --upgrade pip
pip install .
pip install onnx==1.16.0 pyyaml==6.0.2 paddlepaddle==2.6.2 scikit-image==0.19.3 paddle2onnx==1.3.1 numpy==1.23.5
wget https://github.com/microsoft/onnxruntime/releases/download/v1.18.1/onnxruntime-linux-x64-1.18.1.tgz -O /tmp/onnxruntime.tar.gz && \
    tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/ && \
    mv /tmp/onnxruntime-linux-x64-1.18.1/ /opt/
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Use the following script to convert the model. \
Set the options refer to the following table.

### Export the PaddlePaddle model 
|option            |value                                           |
|------------------|------------------------------------------------|
|--config          | Model configuration file                       |
|--model_path      | Downloaded pretrained model file               |
|--save_dir        | Output directory for the exported model        |
|--input_shape     | Input tensor shape (Batch, Channel, H, W)      |

### Convert Paddle model to ONNX format
|option            |value                                           |
|------------------|------------------------------------------------|
|--model_dir       | Exported model directory (same as --save_dir)  |
|--model_filename  | Model architecture file name                   |
|--params_filename | Model weights file name                        |
|--opset_version   | ONNX opset version                             |
|--save_file       | Output ONNX file path                          |

---


```sh
cd ${TVM_ROOT}/convert/repos/fcn-hrnetw18/tools

# The following is an example for FCN HRNet W18
wget https://bj.bcebos.com/paddleseg/dygraph/pascal_voc12/fcn_hrnetw18_voc12aug_512x512_40k/model.pdparams

cd ${TVM_ROOT}/convert/repos/fcn-hrnetw18/
python3 tools/export.py \
    --config configs/fcn/fcn_hrnetw18_voc12aug_512x512_40k.yml \
    --model_path tools/model.pdparams \
    --save_dir ${TVM_ROOT}/convert/output/fcn_hrnetw18_voc12aug_512x512_40k_onnx  \
    --input_shape 1 3 512 512

paddle2onnx --model_dir ${TVM_ROOT}/convert/output/fcn_hrnetw18_voc12aug_512x512_40k_onnx \
            --model_filename model.pdmodel \
            --params_filename model.pdiparams \
            --opset_version 11 \
            --save_file ${TVM_ROOT}/convert/output/fcn_hrnetw18_voc12aug_512x512_40k_onnx/fcn_hrnetw18_voc12aug_512x512_40k.onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── fcn_hrnetw18_voc12aug_512x512_40k_onnx
           └── fcn_hrnetw18_voc12aug_512x512_40k.onnx
```
## 4. Delete the environment for FCN HRNet W18 models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/fcn-hrnetw18
rm -R ${TVM_ROOT}/convert/repos/fcn-hrnetw18
```

## 6. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For FCN HRNet W18, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/fcn_hrnetw18_voc12aug_512x512_40k_onnx/fcn_hrnetw18_voc12aug_512x512_40k.onnx \
    -o fcn_hrnetw18_voc12aug_512x512_40k_onnx \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    -s 1,3,512,512
```

----
