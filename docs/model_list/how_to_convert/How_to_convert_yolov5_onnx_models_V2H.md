# How to convert yolov5_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
|~~[YOLOv5n](https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5n.pt)~~                                                           |~~yolov5n~~                          |~~(320, 320)~~     | ~~Object Detection~~    |
| [YOLOv5s](https://github.com/ultralytics/yolov5/releases/download/v5.0/yolov5s.pt)                                                           |yolov5s                          |(640, 640)     | Object Detection    |
| [YOLOv5m](https://github.com/ultralytics/yolov5/releases/download/v5.0/yolov5m.pt)                                                           |yolov5m                          |(640, 640)     | Object Detection    |
| [YOLOv5l](https://github.com/ultralytics/yolov5/releases/download/v5.0/yolov5l.pt)                                                           |yolov5l                          |(640, 640)     | Object Detection    |
| [YOLOv5x](https://github.com/ultralytics/yolov5/releases/download/v5.0/yolov5x.pt)                                                           |yolov5x                          |(640, 640)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov5_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_onnx
git clone -b v5.0 --recursive https://github.com/ultralytics/yolov5.git ${TVM_ROOT}/convert/repos/ultralytics_yolov5
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov5
sed -i -e "s@latest@tags/v5.0@g" ./utils/google_utils.py
. ${TVM_ROOT}/convert/venvs/ultralytics_onnx/bin/activate 
pip install torch==2.1.2 torchvision==0.16.2 onnx==1.9.0 numpy==1.23.5 matplotlib==3.2.2 pandas==1.3.3 protobuf==3.20.*
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolov5`.\
Use the following script to convert the model. \
Set the options refer to the following table.

|option    |value                                  |
|----------|---------------------------------------|
|--weights | Downloaded TorchScript (.pt) file      |
|--img-size   |`Input shape` column in the table above|
---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov5
python export.py --weights ${torch_file} --img-size ${image_size}

# The following is an example for YOLOv5l.
python export.py --weights yolov5l.pt --img-size 640 640

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

## 4. Cut post-process with onnx file.

Yolov5 models have redundant post-processing part, so cut part from onnx.
Please delete the following three nodes common to all yolov5 onnxs by looking at the example script below.

+ "/model.24/m.0/Conv_output_0"
+ "/model.24/m.1/Conv_output_0"
+ "/model.24/m.2/Conv_output_0"

<center><img src=./img/cut_yolov5.png></center>

```sh
$ python3
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv5l.
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov5l.onnx", "yolov5l_cut.onnx", ["images"],["/model.24/m.0/Conv_output_0", "/model.24/m.1/Conv_output_0", "/model.24/m.2/Conv_output_0"])
>>> exit()
```

## 5. Delete the environment for yolov5_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov5
```

## 6. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv5l, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov5l_ultralytics_onnx/yolov5l_cut.onnx -o yolov5l_onnx \
-t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100
```

----
