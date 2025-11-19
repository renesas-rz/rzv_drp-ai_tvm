# How to convert yolov8_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv8n](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.pt)                                                           |yolov8n                          |(640, 640)     | Object Detection    |
| [YOLOv8s](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8s.pt)                                                           |yolov8s                          |(640, 640)     | Object Detection    |
| [YOLOv8m](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8m.pt)                                                           |yolov8m                          |(640, 640)     | Object Detection    |
| [YOLOv8l](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8l.pt)                                                           |yolov8l                          |(640, 640)     | Object Detection    |
| [YOLOv8x](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8x.pt)                                                           |yolov8x                          |(640, 640)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov8_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_onnx
. ${TVM_ROOT}/convert/venvs/ultralytics_onnx/bin/activate
git clone --recursive https://github.com/ultralytics/ultralytics ${TVM_ROOT}/convert/repos/ultralytics_yolov8
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov8
git reset --hard "b1119d512e738e90f2327b316216b069ed576a56"
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1
pip install ultralytics==8.0.104
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolov8`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|model     |Downloaded TorchScript (.pt) file      |
|imgsz     |`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov8
yolo mode=export model=${torch_file} format=onnx opset=12 imgsz=${image_size}

# The following is an example for YOLOv8n.
yolo mode=export model=yolov8n.pt format=onnx opset=12 imgsz=640

mkdir -p ${TVM_ROOT}/convert/output/yolov8n_ultralytics_onnx
mv yolov8n.onnx ${TVM_ROOT}/convert/output/yolov8n_ultralytics_onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov8n_ultralytics_onnx
           └── yolov8n.onnx
```
## 4. Cut post-process with onnx file.

Yolov8 models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolov8 onnxs by looking at the example script below.

| cut point | node name |
| --- | --- |
| 1/8 scale | /model.22/cv3.0/cv3.0.2/Conv_output_0 |
| 1/8 scale | /model.22/cv2.0/cv2.0.2/Conv_output_0 |
| 1/16 scale | /model.22/cv3.1/cv3.1.2/Conv_output_0 |
| 1/16 scale | /model.22/cv2.1/cv2.1.2/Conv_output_0 |
| 1/32 scale | /model.22/cv3.2/cv3.2.2/Conv_output_0 |
| 1/32 scale | /model.22/cv2.2/cv2.2.2/Conv_output_0 |

<center><img src=./img/cut_yolov8.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv5l.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov8n.onnx", "yolov8n_cut.onnx", ["images"], ["/model.22/cv3.0/cv3.0.2/Conv_output_0", "/model.22/cv2.0/cv2.0.2/Conv_output_0", "/model.22/cv3.1/cv3.1.2/Conv_output_0", "/model.22/cv2.1/cv2.1.2/Conv_output_0", "/model.22/cv3.2/cv3.2.2/Conv_output_0", "/model.22/cv2.2/cv2.2.2/Conv_output_0"])
>>> exit()
```

## 5. Delete the environment for yolov8_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov8
```

## 6. Next Step
To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv8n, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov8n_ultralytics_onnx/yolov8n_cut.onnx -o yolov8n_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

----