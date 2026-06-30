# How to convert yolo26_onnx (one to many) models 

<!-- Below is a list of AI models supported by this manual. -->

The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                   | Download model name | Input shape | Task             |
| ------------------------------------------------------------------------------------------ | ------------------- | ----------- | ---------------- |
| [YOLO26n](https://huggingface.co/Ultralytics/YOLO26/resolve/main/yolo26n.pt?download=true) | yolo26n             | (640, 640)  | Object Detection |
| [YOLO26s](https://huggingface.co/Ultralytics/YOLO26/resolve/main/yolo26s.pt?download=true) | yolo26s             | (640, 640)  | Object Detection |
| [YOLO26m](https://huggingface.co/Ultralytics/YOLO26/resolve/main/yolo26m.pt?download=true) | yolo26m             | (640, 640)  | Object Detection |
| [YOLO26l](https://huggingface.co/Ultralytics/YOLO26/resolve/main/yolo26l.pt?download=true) | yolo26l             | (640, 640)  | Object Detection |
| [YOLO26x](https://huggingface.co/Ultralytics/YOLO26/resolve/main/yolo26x.pt?download=true) | yolo26x             | (640, 640)  | Object Detection |

License: [AGPL 3.0](https://huggingface.co/Ultralytics/YOLO26)

---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolo26_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_onnx
. ${TVM_ROOT}/convert/venvs/ultralytics_onnx/bin/activate
git clone --recursive https://github.com/ultralytics/ultralytics ${TVM_ROOT}/convert/repos/ultralytics_yolo26
cd ${TVM_ROOT}/convert/repos/ultralytics_yolo26
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1
pip install ultralytics==8.4.19
```

## 3. Convert TorchScript (.pt) files to ONNX (.onnx) files.

Note : Check the downloaded TorchScript (.pt) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/ultralytics_yolo26`.\
Use the following script to convert the model. \
Set the options refer to the following table.

| option | value                                   |
| ------ | --------------------------------------- |
| model  | Downloaded TorchScript (.pt) file       |
| imgsz  | `Input shape` column in the table above |

---

```sh
cd ${TVM_ROOT}/convert/repos/ultralytics_yolo26
yolo mode=export model=${torch_file} format=onnx opset=12 imgsz=${image_size} end2end=False

# The following is an example for YOLO26n.
yolo mode=export model=yolo26n.pt format=onnx opset=12 imgsz=640 end2end=False

mkdir -p ${TVM_ROOT}/convert/output/yolo26n_ultralytics_onnx
mv yolo26n.onnx ${TVM_ROOT}/convert/output/yolo26n_ultralytics_onnx
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolo26n_ultralytics_onnx
           └── yolo26n.onnx
```

## 4. Cut post-process with onnx file.

Yolo26 models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolo26 onnxs by looking at the example script below.

| cut point  | node name                                             |
| ---------- | ----------------------------------------------------- |
| 1/8 scale  | /model.23/cv3.0/cv3.0.2/Conv_output_0 |
| 1/8 scale  | /model.23/cv2.0/cv2.0.2/Conv_output_0 |
| 1/16 scale | /model.23/cv3.1/cv3.1.2/Conv_output_0 |
| 1/16 scale | /model.23/cv2.1/cv2.1.2/Conv_output_0 |
| 1/32 scale | /model.23/cv3.2/cv3.2.2/Conv_output_0 |
| 1/32 scale | /model.23/cv2.2/cv2.2.2/Conv_output_0 |

<center><img src=./img/cut_yolo26.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLO26n.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolo26n.onnx", "yolo26n_cut.onnx", ["images"], ["/model.23/cv3.0/cv3.0.2/Conv_output_0", "/model.23/cv2.0/cv2.0.2/Conv_output_0", "/model.23/cv3.1/cv3.1.2/Conv_output_0", "/model.23/cv2.1/cv2.1.2/Conv_output_0", "/model.23/cv3.2/cv3.2.2/Conv_output_0", "/model.23/cv2.2/cv2.2.2/Conv_output_0"])
>>> exit()
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 5. Delete the environment for yolo26_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_onnx
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolo26
```

## 6. Next Step   

**:bulb: Note** For the models provided in this guide, inference time on RZ/V2H & RZ/V2N can be optimized by enabling the "--yolo_attention" option.

```sh
# EXAMPLE
$ python3 compile_onnx_model_quant.py \
    ./target_model.onnx \
    -o test_out \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    --yolo_attention
```
:information_source: To maintain accuracy, please refer to the guide below and configure the calibration images and preprocessing appropriately.

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
If the model size is too large, it may exceed the DRP-AI memory region and cannot be executed on the board. Using a smaller model is recommended.

----