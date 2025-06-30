# How to convert yolov11_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                              | Download model name | Input shape | Task             |
| ---                                                                                   | ---                 | ---         | ---              |
| [yolov11n](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n.pt) | yolov11n            | (640, 640)  | Object Detection |
| [yolov11s](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s.pt) | yolov11s            | (640, 640)  | Object Detection |
| [yolov11m](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11m.pt) | yolov11m            | (640, 640)  | Object Detection |
| [yolov11l](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11l.pt) | yolov11l            | (640, 640)  | Object Detection |
| [yolov11x](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11x.pt) | yolov11x            | (640, 640)  | Object Detection |

---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov11_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_yolov11
. ${TVM_ROOT}/convert/venvs/ultralytics_yolov11/bin/activate
git clone -b v8.3.0 --recursive https://github.com/ultralytics/ultralytics ${TVM_ROOT}/convert/repos/ultralytics_yolov11
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov11
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.18.1  onnxslim==0.1.34
pip install ultralytics==8.3.0
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in ${TVM_ROOT}/convert/repos/ultralytics_yolov11 \
Use the following script to convert the model. \
Set the options refer to the following table.

| option   | value                                 |
| ---      | ---                                   |
| model    | Downloaded TorchScript (.pt) file     |
| img_size | Input shape column in the table above |

---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolov11

# execute below python sctiprs
from ultralytics import YOLO
     model = YOLO(${model})↵
     _ = model.export(format="onnx", opset=13, imgsz=${img_size})

# The following is an example for YOLOv11n
# execute below python sctiprs
from ultralytics import YOLO
     model = YOLO(yolo11n.pt)↵
     _ = model.export(format="onnx", opset=13, imgsz=640)

mkdir -p ${TVM_ROOT}/convert/output/yolov11n_ultralytics_onnx
mv yolo11n.onnx ${TVM_ROOT}/convert/output/yolov11n_ultralytics_onnx/yolov11.onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov11n_ultralytics_onnx
           └── yolov11n.onnx
```
## 4. Cut post-process with onnx file.

Yolov11 models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolov11 onnxs by looking at the example script below.

| cut point  | node name                            |
| ---        | ---                                  |
| 1/8 scale  | /model.23/cv3.0/cv3.0.2/Conv_output_0 |
| 1/8 scale  | /model.23/cv2.0/cv2.0.2/Conv_output_0 |
| 1/16 scale | /model.23/cv3.1/cv3.1.2/Conv_output_0 |
| 1/16 scale | /model.23/cv2.1/cv2.1.2/Conv_output_0 |
| 1/32 scale | /model.23/cv3.2/cv3.2.2/Conv_output_0 |
| 1/32 scale | /model.23/cv2.2/cv2.2.2/Conv_output_0 |

<center><img src=./img/cut_yolov8.png></center>

```sh
$ python3
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv11n.
Python 3.8.10 (default, Feb  4 2025, 15:02:54)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov11n.onnx", "yolov11n_cut.onnx", ["images"], ["/model.23/cv3.0/cv3.0.2/Conv_output_0", "/model.23/cv2.0/cv2.0.2/Conv_output_0", "/model.23/cv3.1/cv3.1.2/Conv_output_0", "/model.23/cv2.1/cv2.1.2/Conv_output_0", "/model.23/cv3.2/cv3.2.2/Conv_output_0", "/model.23/cv2.2/cv2.2.2/Conv_output_0"])
>>> exit()
```

## 5. Delete the environment for yolov11_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_yolov11
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolov11
```

## 6. Next Step
To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv11n, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/output/convert/output/yolov11n_ultralytics_onnx/yolov11n_cut.onnx -o yolov11n_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

----
