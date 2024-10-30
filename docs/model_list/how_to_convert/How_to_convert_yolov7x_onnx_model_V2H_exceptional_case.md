# < Exceptional case > How to convert yolov7x_onnx model for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

### `We only confirmed the model was worked successfully in spesific PyTorch version (torch==1.8.0).`

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv7X](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7x.pt)                                                           |yolov7x                          |(320, 320)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov7x_onnx model.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7
git clone https://github.com/WongKinYiu/yolov7 ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
cd ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
git reset --hard "3b41c2cc709628a8c1966931e696b14c11d6db0c"
. ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7/bin/activate
pip install torch==1.8.0 torchvision==0.9.0 onnx==1.9.0 numpy==1.19.5 matplotlib==3.2.2 pandas==1.3.3 protobuf==3.20.*
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/wongkinyiu_yolov7`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|--weights |Downloaded TorchScript (.pt) file      |
|--img-size|`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
python3 export.py --weights yolov7x.pt --img-size 320 --batch 1 --grid --simplify

mkdir -p ${TVM_ROOT}/convert/output/yolov7x_wongkinyiu_onnx
mv yolov7x.onnx ${TVM_ROOT}/convert/output/yolov7x_wongkinyiu_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov7x_wongkinyiu_onnx
           └── yolov7x.onnx
```

## 4. Delete the environment for yolov7x_onnx model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7
rm -R ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv7x, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov7_wongkinyiu_onnx/yolov7x.onnx -o yolov7x_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 
```

---- 
