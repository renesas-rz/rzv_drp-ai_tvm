# How to convert yolov5_onnx models for V2L/M/MA
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv5n](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5n.pt)                                                           |yolov5n                          |(640, 640)     | Object Detection    |
| [YOLOv5s](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5s.pt)                                                           |yolov5s                          |(640, 640)     | Object Detection    |
| [YOLOv5m](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5m.pt)                                                           |yolov5m                          |(640, 640)     | Object Detection    |
| [YOLOv5l](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5l.pt)                                                           |yolov5l                          |(320, 320)     | Object Detection    |
| [YOLOv5x](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5x.pt)                                                           |yolov5x                          |(320, 320)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov5_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_onnx
git clone -b v7.0 --recursive https://github.com/ultralytics/yolov5.git ${TVM_ROOT}/convert/repos/ultralytics_yolov5
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov5
sed -i -e "s@latest@tags/v5.0@g" ./utils/google_utils.py
. ${TVM_ROOT}/convert/venvs/ultralytics_onnx/bin/activate 
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolov5`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|--weights |Downloaded TorchScript (.pt) file      |
|--imgsz   |`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov5
python export.py --weights ${torch_file} --include onnx --opset 12 --imgsz ${image_size}

# The following is an example for YOLOv5l.
python export.py --weights yolov5l.pt --include onnx --opset 12 --imgsz 320

mkdir -p ${TVM_ROOT}/convert/output/yolov5l_ultralytics_onnx
mv yolov5l.onnx ${TVM_ROOT}/convert/output/yolov5l_ultralytics_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov5l_ultralytics_onnx
           └── yolov5l.onnx
```

## 4. Delete the environment for yolov5_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov5
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv5l, as the following.

```sh
python3 compile_onnx_model.py ../convert/output/yolov5l_ultralytics_onnx/yolov5l.onnx -o yolov5l_onnx -s 1,3,320,320 -i data
```

----
