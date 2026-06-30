# How to convert yolov3_onnx model
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                           | Input shape | Task             |
|------------------------------------------------------------------------------------|-------------|------------------|
| [YOLOv3](https://github.com/ultralytics/yolov3/releases/download/v9.6.0/yolov3.pt) | (416, 416)  | Object Detection |
---

License: [AGPL-3.0 license](https://github.com/ultralytics/yolov3)
## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
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
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.18.1
pip install ultralytics==8.2.103
```

## 3. Convert TorchScript (.pt) file to ONNX (.onnx) file.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolov3`.\
Use the following script to get the model. \
Set the options refer to the following table.

| option    | value                                   |
|-----------|-----------------------------------------|
| --weights | Downloaded TorchScript (.pt) file       |
| --img     | `Input shape` column in the table above |
---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov3

# The following is an example for YOLOv3 that input shape is 416.
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
## 4. Cut post-process with onnx file.

Yolov3 models have redundant post-processing part, so cut part from onnx.
Please delete the following three nodes to yolov3 onnxs by looking at the example script below.

+ "/model.28/m.0/Conv_output_0"
+ "/model.28/m.1/Conv_output_0"
+ "/model.28/m.2/Conv_output_0"

<center><img src=./img/cut_yolov3.png></center>

```sh
$ python3
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv3.
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov3.onnx", "yolov3_cut.onnx", ["images"],["/model.28/m.0/Conv_output_0", "/model.28/m.1/Conv_output_0", "/model.28/m.2/Conv_output_0"])
>>> exit()
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 5. Delete the environment for yolov3_onnx model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov3
```

## 6. Next Step


To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.
Please refer below guides.

Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)


----
