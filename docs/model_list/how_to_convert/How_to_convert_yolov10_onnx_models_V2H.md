# How to convert yolov10_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below are available via direct links.

| AI model                                                                            | Download model name | Input shape | Task             |
| ---                                                                                 | ---                 | ---         | ---              |
| [YOLOv10n](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10n.onnx) | yolov10n            | (640, 640)  | Object Detection |
| [YOLOv10s](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10s.onnx) | yolov10s            | (640, 640)  | Object Detection |
| [YOLOv10m](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10m.onnx) | yolov10m            | (640, 640)  | Object Detection |
| [YOLOv10b](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10b.onnx) | yolov10b            | (640, 640)  | Object Detection |
| [YOLOv10l](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10l.onnx) | yolov10l            | (640, 640)  | Object Detection |
| [YOLOv10x](https://github.com/THU-MIG/yolov10/releases/download/v1.1/yolov10x.onnx) | yolov10x            | (640, 640)  | Object Detection |


---

## 1. Set environment variables.

Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  

## 2. Cut post-process with onnx file.

Yolov10 models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolov10 onnxs by looking at the example script below.

| cut point  | node name                                             |
| ---        | ---                                                   |
| 1/8 scale  | /model.23/one2one_cv3.0/one2one_cv3.0.2/Conv_output_0 |
| 1/8 scale  | /model.23/one2one_cv2.0/one2one_cv2.0.2/Conv_output_0 |
| 1/16 scale | /model.23/one2one_cv3.1/one2one_cv3.1.2/Conv_output_0 |
| 1/16 scale | /model.23/one2one_cv2.1/one2one_cv2.1.2/Conv_output_0 |
| 1/32 scale | /model.23/one2one_cv3.2/one2one_cv3.2.2/Conv_output_0 |
| 1/32 scale | /model.23/one2one_cv2.2/one2one_cv2.2.2/Conv_output_0 |

<center><img src=./img/cut_yolov8.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv10n.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov10n.onnx", "yolov10n_cut.onnx", ["images"], ["/model.23/one2one_cv3.0/one2one_cv3.0.2/Conv_output_0", "/model.23/one2one_cv2.0/one2one_cv2.0.2/Conv_output_0", "/model.23/one2one_cv3.1/one2one_cv3.1.2/Conv_output_0", "/model.23/one2one_cv2.1/one2one_cv2.1.2/Conv_output_0", "/model.23/one2one_cv3.2/one2one_cv3.2.2/Conv_output_0", "/model.23/one2one_cv2.2/one2one_cv2.2.2/Conv_output_0"])
>>> exit()
```

## 3. Next Step
To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv10n, as the following.

```sh
python3 compile_onnx_model_quant.py yolov10n_cut.onnx -o yolov10n_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

----
