# How to convert yolov7_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv7](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7.pt)                                                              |yolov7                           |(640, 640)     | Object Detection    |
| [YOLOv7X](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7x.pt)                                                            |yolov7x                          |(640, 640)     | Object Detection    |
| [YOLOv7W6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-w6.pt)                                                         |yolov7-w6                        |(640, 640)     | Object Detection    |
| [YOLOv7E6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-e6.pt)                                                         |yolov7-e6                        |(640, 640)     | Object Detection    |
| [YOLOv7D6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-d6.pt)                                                         |yolov7-d6                        |(640, 640)     | Object Detection    |
| [YOLOv7E6E](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-e6e.pt)                                                       |yolov7-e6e                       |(640, 640)     | Object Detection    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov7_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7
git clone https://github.com/WongKinYiu/yolov7 ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
cd ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
git reset --hard "3b41c2cc709628a8c1966931e696b14c11d6db0c"
. ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7/bin/activate
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.18.1
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
python3 export.py --weights ${torch_file} --img-size ${image_size} --batch 1 --grid --simplify

# The following is an example for YOLOv7
python3 export.py --weights yolov7.pt --img-size 640 --batch 1 --grid --simplify

mkdir -p ${TVM_ROOT}/convert/output/yolov7_wongkinyiu_onnx
mv yolov7.onnx ${TVM_ROOT}/convert/output/yolov7_wongkinyiu_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov7_wongkinyiu_onnx
           └── yolov7.onnx
```

## 4. Cut post-process with onnx file.

Yolov7 models have redundant post-processing part, so cut part from onnx.
Please delete the following three or four nodes all yolov7 onnxs by looking at the example script below.

+ case 1 cut 3 nodes with yolov7 or yolov7x node names.

| cut point | yolov7 | yolov7x |
| --- | --- | --- |
| 1/8 scale | /model.105/m.0/Conv_output_0 | /model.121/m.0/Conv_output_0 |
| 1/16 scale | /model.105/m.1/Conv_output_0 | /model.121/m.1/Conv_output_0 |
| 1/32 scale | /model.105/m.2/Conv_output_0 | /model.121/m.2/Conv_output_0 |

<center><img src=./img/cut_yolov7.png></center>

+ case 2 cut 4 nodes with yolov7-w6, yolov7-e6, yolov7-d6, yolov7-e6e node names.

| cut point | yolov7-w6 | yolov7-e6 | yolov7-d6 | yolov7-e6e |
| --- | --- | --- | --- | --- |
| 1/8 scale | /model.118/m.0/Conv_output_0 | /model.140/m.0/Conv_output_0 | /model.162/m.0/Conv_output_0 | /model.261/m.0/Conv_output_0 |
| 1/16 scale | /model.118/m.1/Conv_output_0 | /model.140/m.1/Conv_output_0 | /model.162/m.1/Conv_output_0 | /model.261/m.1/Conv_output_0 |
| 1/32 scale | /model.118/m.2/Conv_output_0 | /model.140/m.2/Conv_output_0 | /model.162/m.2/Conv_output_0 | /model.261/m.2/Conv_output_0 |
| 1/64 scale | /model.118/m.3/Conv_output_0 | /model.140/m.3/Conv_output_0 | /model.162/m.3/Conv_output_0 | /model.261/m.3/Conv_output_0 |

<center><img src=./img/cut_yolov7-w6.png></center>

```sh
$ python3
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv7.
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov7.onnx", "yolov7_cut.onnx", ["images"],
["/model.105/m.0/Conv_output_0", "/model.105/m.1/Conv_output_0", "/model.105/m.2/Conv_output_0"])
>>> exit()
```

## 5. Delete the environment for yolov7_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7
rm -R ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
```

## 6. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv7, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov7_wongkinyiu_onnx/yolov7_cut.onnx -o yolov7_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

---- 
