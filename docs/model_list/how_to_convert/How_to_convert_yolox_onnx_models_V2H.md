# How to convert yolox_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name   |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| [YOLOX_s](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_s.pth)                                              |yolox-s                |(640, 640)     | Object Detection  |
| [YOLOX_m](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_m.pth)                                              |yolox-m                |(640, 640)     | Object Detection  |
| [YOLOX_l](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_l.pth)                                              |yolox-l                |(320, 320)     | Object Detection  |
| [YOLOX_x](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_x.pth)                                              |yolox-x                |(320, 320)     | Object Detection  |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolox_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/yolox
git clone https://github.com/Megvii-BaseDetection/YOLOX ${TVM_ROOT}/convert/repos/yolox
cd ${TVM_ROOT}/convert/repos/yolox
git reset --hard "ac58e0a5e68e57454b7b9ac822aced493b553c53"
. ${TVM_ROOT}/convert/venvs/yolox/bin/activate
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.9.0 numpy==1.19.5 matplotlib==3.2.2 pandas==1.3.3 protobuf==3.20.*
pip install .
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/yolox`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option       |value                                                    |
|-------------|---------------------------------------------------------|
|${onnx_file} |`{ Download model name column in the table above }.onnx` |
|${arg_name}  |`Download model name` column in the table above          |
|${torch_file}|Downloaded PyTorch Model (.pth) file                     |
|${image_size}|`Input shape` column in the table above                  |
---

```sh
cd ${TVM_ROOT}/convert/repos/yolox
python tools/export_onnx.py --output-name ./${onnx_file} -n ${arg_name} -c ./${torch_file} \
    --decode_in_inference test_size ${image_size}

# The following is an example for YOLOX_s
python tools/export_onnx.py --output-name ./yolox-s.onnx -n yolox-s -c ./yolox_s.pth --decode_in_inference test_size 640,640

mkdir -p ${TVM_ROOT}/convert/output/yolox_s_megvii_onnx
mv yolox-s.onnx ${TVM_ROOT}/convert/output/yolox_s_megvii_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolox_s_megvii_onnx
           └── yolox-s.onnx
```

## 4. Delete the environment for yolox_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/yolox
rm -R ${TVM_ROOT}/convert/repos/yolox
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOX_s, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolox_s_megvii_onnx/yolox-s.onnx -o yolox_s_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100
```

----