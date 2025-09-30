# How to convert yolov6_onnx models for V2L/M/MA
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv6N](https://github.com/meituan/YOLOv6/releases/download/0.4.0/yolov6n.pt)                                                           |yolov6n                          |(640, 640)     | Object Detection    |
| [YOLOv6S](https://github.com/meituan/YOLOv6/releases/download/0.4.0/yolov6s.pt)                                                           |yolov6s                          |(640, 640)     | Object Detection    |
| [YOLOv6M](https://github.com/meituan/YOLOv6/releases/download/0.4.0/yolov6m.pt)                                                           |yolov6m                          |(640, 640)     | Object Detection    |
| [YOLOv6L](https://github.com/meituan/YOLOv6/releases/download/0.4.0/yolov6l.pt)                                                           |yolov6l                          |(640, 640)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov6_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/meituan_yolov6
git clone -b 0.4.0 https://github.com/meituan/YOLOv6 ${TVM_ROOT}/convert/repos/meituan_yolov6 
cd ${TVM_ROOT}/convert/repos/meituan_yolov6
. ${TVM_ROOT}/convert/venvs/meituan_yolov6/bin/activate
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1 requests
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/meituan_yolov6`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|--weights |Downloaded TorchScript (.pt) file      |
|--img     |`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/meituan_yolov6
python ./deploy/ONNX/export_onnx.py --weights ${torch_file} --simplify --img ${image_size}

# The following is an example for YOLOv6N.
python ./deploy/ONNX/export_onnx.py --weights yolov6n.pt --simplify --img 640

mkdir -p ${TVM_ROOT}/convert/output/yolov6n_meituan_onnx
mv yolov6n.onnx ${TVM_ROOT}/convert/output/yolov6n_meituan_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov6n_meituan_onnx
           └── yolov6n.onnx
```

## 4. Delete the environment for yolov6_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/meituan_yolov6
rm -R ${TVM_ROOT}/convert/repos/meituan_yolov6
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv6N, as the following.

```sh
python3 compile_onnx_model.py ../convert/output/yolov6n_meituan_onnx/yolov6n.onnx -o yolov6n_onnx -s 1,3,640,640 -i data
```

----