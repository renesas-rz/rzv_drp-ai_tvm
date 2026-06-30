# How to convert yolov7_onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [YOLOv7](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7.pt)                                                              |yolov7                           |(640, 640)     | Object Detection    |
| [YOLOv7-x](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7x.pt)                                                            |yolov7x                          |(640, 640)     | Object Detection    |
| [YOLOv7-w6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-w6.pt)                                                         |yolov7-w6                        |(640, 640)     | Object Detection    |
| [YOLOv7-e6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-e6.pt)                                                         |yolov7-e6                        |(640, 640)     | Object Detection    |
| [YOLOv7-d6](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-d6.pt)                                                         |yolov7-d6                        |(640, 640)     | Object Detection    |
| [YOLOv7-e6e](https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-e6e.pt)                                                       |yolov7-e6e                       |(640, 640)     | Object Detection    |
---

License: [GPL 3.0](https://github.com/WongKinYiu/yolov7/)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
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
pip install onnx==1.16.0 onnxruntime==1.20.1
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
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv7.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov7.onnx", "yolov7_cut.onnx", ["images"],
["/model.105/m.0/Conv_output_0", "/model.105/m.1/Conv_output_0", "/model.105/m.2/Conv_output_0"])
>>> exit()
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 5. Delete the environment for yolov7_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov7
rm -R ${TVM_ROOT}/convert/repos/wongkinyiu_yolov7
```

## 6. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.
Please refer below guides.

Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)

## Appendix   

### :information_source: Notes on Large Models   
#### RZ/V2H & RZ/V2N  
RZ/V2H EVK with the default settings, a 512MB area is allocated to the DRP-AI area in the Linux memory map settings.
If the object after AI model conversion does not fit within this size, it cannot be processed. An error will occur when running on the board.   
If you expand the DRP-AI area in the Linux memory map settings, you will be able to handle a larger AI model.
For example on memory expansion methods, please refer to this [document](https://github.com/renesas-rz/rzv_drp-ai_tvm/blob/main/docs/model_list/how_to_expand_drpai_memory.md).

#### RZ/V2M & RZ/V2L
If the model size is too large, it may exceed the DRP-AI memory area and cannot be executed on the board. Using a smaller model is recommended.

---- 
