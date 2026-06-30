# How to convert yolov9_onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                      | Download model name | Input shape | Task             |
| ---                                                                                           | ---                 | ---         | ---              |
| [YOLOv9-t](https://github.com/WongKinYiu/yolov9/releases/download/v0.1/yolov9-t-converted.pt) | yolov9-t            | (640, 640)  | Object Detection |
| [YOLOv9-s](https://github.com/WongKinYiu/yolov9/releases/download/v0.1/yolov9-s-converted.pt) | yolov9-s            | (640, 640)  | Object Detection |
| [YOLOv9-m](https://github.com/WongKinYiu/yolov9/releases/download/v0.1/yolov9-m-converted.pt) | yolov9-m            | (640, 640)  | Object Detection |

---

License: [GPL 3.0](https://github.com/WongKinYiu/yolov9)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov9_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov9
. ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov9/bin/activate
git clone https://github.com/WongKinYiu/yolov9 ${TVM_ROOT}/convert/repos/wongkinyiu_yolov9
cd ${TVM_ROOT}/convert/repos/wongkinyiu_yolov9
git reset --hard "5b1ea9a8b3f0ffe4fe0e203ec6232d788bb3fcff"
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1 onnx-simplifier==0.4.36
pip install -r requirements.txt
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in ${TVM_ROOT}/convert/repos/wongkinyiu_yolov9. \
Use the following script to convert the model. \
Set the options refer to the following table.

| option | value                                 |
| ---    | ---                                   |
| model  | Downloaded TorchScript (.pt) file     |
| imgsz  | Input shape column in the table above |
---

```sh
cd ${TVM_ROOT}/convert/repos/wongkinyiu_yolov9

# The following is an example for YOLOv9-t.
wget https://github.com/WongKinYiu/yolov9/releases/download/v0.1/yolov9-t-converted.pt
python3 export.py --weights ./yolov9-t-converted.pt --imgsz 640 --batch-size 1 --include onnx --simplify

mkdir -p ${TVM_ROOT}/convert/output/yolov9-t_wongkinyiu_onnx
mv yolov9-t-converted.onnx ${TVM_ROOT}/convert/output/yolov9-t_wongkinyiu_onnx/yolov9-t.onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolov9_t_wongkinyiu_onnx
           └── yolov9-t.onnx
```
## 4. Cut post-process with onnx file.

Yolov9 models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolov9 onnxs by looking at the example script below.

| cut point  | node name for yolov9-t, s, m, c       | node name for yolov9-e                |
| ---        | ---                                   | ---                                   |
| 1/8 scale  | /model.22/cv3.0/cv3.0.2/Conv_output_0 | /model.42/cv3.0/cv3.0.2/Conv_output_0 |
| 1/8 scale  | /model.22/cv2.0/cv2.0.2/Conv_output_0 | /model.42/cv2.0/cv2.0.2/Conv_output_0 |
| 1/16 scale | /model.22/cv3.1/cv3.1.2/Conv_output_0 | /model.42/cv3.1/cv3.1.2/Conv_output_0 |
| 1/16 scale | /model.22/cv2.1/cv2.1.2/Conv_output_0 | /model.42/cv2.1/cv2.1.2/Conv_output_0 |
| 1/32 scale | /model.22/cv3.2/cv3.2.2/Conv_output_0 | /model.42/cv3.2/cv3.2.2/Conv_output_0 |
| 1/32 scale | /model.22/cv2.2/cv2.2.2/Conv_output_0 | /model.42/cv2.2/cv2.2.2/Conv_output_0 |

<center><img src=./img/cut_yolov8.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv9-t.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolov9-t.onnx", "yolov9-t_cut.onnx", ["images"], ["/model.22/cv3.0/cv3.0.2/Conv_output_0", "/model.22/cv2.0/cv2.0.2/Conv_output_0", "/model.22/cv3.1/cv3.1.2/Conv_output_0", "/model.22/cv2.1/cv2.1.2/Conv_output_0", "/model.22/cv3.2/cv3.2.2/Conv_output_0", "/model.22/cv2.2/cv2.2.2/Conv_output_0"])
>>> exit()
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 5. Delete the environment for yolov9_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/wongkinyiu_yolov9
rm -R ${TVM_ROOT}/convert/repos/wongkinyiu_yolov9
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
