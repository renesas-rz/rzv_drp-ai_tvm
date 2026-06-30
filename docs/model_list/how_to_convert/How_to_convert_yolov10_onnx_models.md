# How to convert yolov10_onnx models
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

License: [AGPL 3.0](https://github.com/THU-MIG/yolov10/)

## 1. Set environment variables.

Set up the environment, according to [Installation](../../../README.md).  

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

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 3. Next Step

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
