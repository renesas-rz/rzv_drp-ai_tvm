# How to convert edgeyolo_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [EdgeYOLO_M](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_m_coco.pth)                                                           |edgeyolo_m                          |(320, 320)     | Classification    |
| [EdgeYOLO](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_coco.pth)                                                           |edgeyolo                         |(320, 320)     | Classification    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for edgeyolo_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo
git clone https://github.com/LSH9832/edgeyolo ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
cd ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
git reset --hard "1d15c76db54fc191954bf474480b8c50f05a178a"
. ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo/bin/activate
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.9.0 numpy==1.23.5 tensorrt onnxruntime protobuf==3.20.*
pip install -r requirements.txt
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/lsh9832_edgeyolo`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option        |value                                          |
|--------------|-----------------------------------------------|
|--weights     |Downloaded PyTorch Model (.pth) file           |
|--input-size  |`Input shape` column in the table above        |
|${model_base} |`Download model name` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
python export.py --weights ${torch_file} --onnx-only --opset 12 --no-fp16 --input-size ${image_size}
mv ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo/output/export/${model_base}_coco/*.onnx "${model_base}.onnx"

# The following is an example for EdgeYOLO
python export.py --weights edgeyolo_coco.pth --onnx-only --opset 12 --no-fp16 --input-size 320
mv ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo/output/export/edgeyolo_coco/*.onnx "edgeyolo.onnx"

mkdir -p ${TVM_ROOT}/convert/output/edgeyolo_onnx
mv edgeyolo.onnx ${TVM_ROOT}/convert/output/edgeyolo_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── edgeyolo_onnx
           └── edgeyolo.onnx
```

## 4. Delete the environment for edgeyolo_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo
rm -R ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For EdgeYOLO, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/edgeyolo_onnx/edgeyolo.onnx -o edgeyolo_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 
```

----
