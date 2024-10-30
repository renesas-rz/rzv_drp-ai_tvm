# How to convert yolov3_onnx model for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                          |Input shape    | Task              |
|---------------------------------------------------------------------------------------------------------------------------------------------------|---------------|-------------------|
| [YOLOv3](https://github.com/ultralytics/yolov3/releases/download/v9.6.0/yolov3.pt)                                                                |(416, 416)     | Object Detection  |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov3_onnx model.

```sh
apt update
apt install -y python3-venv

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_onnx
git clone -b v9.6.0 --recursive https://github.com/ultralytics/yolov3.git ${TVM_ROOT}/convert/repos/ultralytics_yolov3
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov3
. ${TVM_ROOT}/convert/venvs/ultralytics_onnx/bin/activate
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.9.0 numpy==1.23.5 matplotlib==3.2.2 pandas==1.3.3 protobuf==3.20.*
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) file to ONNX (.onnx) file.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolov3`.\
Use the following script to get the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|--weights |Downloaded TorchScript (.pt) file      |
|--img     |`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov3
python export.py --weights yolov3.pt --img 416 --batch 1

mkdir -p ${TVM_ROOT}/convert/output/yolov3_ultralytics_onnx
mv yolov3.onnx ${TVM_ROOT}/convert/output/yolov3_ultralytics_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov3_ultralytics_onnx
           └── yolov3.onnx
```

## 4. Delete the environment for yolov3_onnx model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov3
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov3_ultralytics_onnx/yolov3.onnx -o yolov3_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 
```

----