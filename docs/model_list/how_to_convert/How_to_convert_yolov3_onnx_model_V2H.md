# How to convert yolov3_onnx model for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                           | Input shape | Task             |
|------------------------------------------------------------------------------------|-------------|------------------|
| [YOLOv3](https://github.com/ultralytics/yolov3/releases/download/v9.6.0/yolov3.pt) | (416, 416)  | Object Detection |
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
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.18.1
pip install -r requirements.txt
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
python export.py --weights yolov3.pt --img ${image_size} --batch 1

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

## 5. Delete the environment for yolov3_onnx model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov3
```

## 6. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_onnx_model_quant.py ../convert/output/yolov3_ultralytics_onnx/yolov3_cut.onnx -o yolov3_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

----
