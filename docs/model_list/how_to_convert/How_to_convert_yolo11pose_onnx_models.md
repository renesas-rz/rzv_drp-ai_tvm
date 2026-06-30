# How to convert yolo11-pose_onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name   |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| [YOLO11n-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n-pose.pt)                                              |yolo11n-pose             |(640, 640)     | Pose Estimation  |
| [YOLO11s-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s-pose.pt)                                              |yolo11s-pose               |(640, 640)     | Pose Estimation  |
| [YOLO11m-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11m-pose.pt)                                              |yolo11m-pose                |(640, 640)     | Pose Estimation  |
| [YOLO11l-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11l-pose.pt)                                              |yolo11l-pose                |(640, 640)     | Pose Estimation  |

---

License: [AGPL 3.0](https://github.com/ultralytics/ultralytics)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolo11-pose_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ultralytics_yolo11-pose
. ${TVM_ROOT}/convert/venvs/ultralytics_yolo11-pose/bin/activate
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1  onnxslim==0.1.34
pip install ultralytics==8.3.0

```

## 3. Convert PyTorch Model (.pt) files to ONNX (.onnx) files.



```sh
mkdir ${TVM_ROOT}/convert/repos/ultralytics_yolo11-pose
cd ${TVM_ROOT}/convert/repos/ultralytics_yolo11-pose

# The following is an example for yolo11n-pose
yolo mode=export model=yolo11n-pose.pt format=onnx opset=12 imgsz=640

mkdir -p ${TVM_ROOT}/convert/output/yolo11n-pose_ultralytics_onnx
mv yolo11n-pose.onnx ${TVM_ROOT}/convert/output/yolo11n-pose_ultralytics_onnx/yolo11n-pose.onnx

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolo11n-pose_ultralytics_onnx
           └── yolo11n-pose.onnx
```

## 4. Cut post-process with onnx file.
Yolo11 pose models have redundant post-processing part, so cut part from onnx.
Please delete the following nodes common to all yolo11 onnxs.

<center><img src=./img/yolo11_pose_cut_guide.jpg></center>

Below is a sample script to cut yolo11 pose models.
```sh
$ python3
Python 3.10.12 (main, Mar  3 2026, 11:56:32) [GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOv11n-pose.
Python 3.10.12 (main, Mar  3 2026, 11:56:32) [GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolo11n-pose.onnx", "yolo11n-pose_cut.onnx", ["images"], ["/model.23/cv2.0/cv2.0.2/Conv_output_0", "/model.23/cv3.0/cv3.0.2/Conv_output_0", "/model.23/cv2.1/cv2.1.2/Conv_output_0", "/model.23/cv3.1/cv3.1.2/Conv_output_0", "/model.23/cv2.2/cv2.2.2/Conv_output_0", "/model.23/cv3.2/cv3.2.2/Conv_output_0", "/model.23/cv4.0/cv4.0.2/Conv_output_0", "/model.23/cv4.1/cv4.1.2/Conv_output_0", "/model.23/cv4.2/cv4.2.2/Conv_output_0"])
>>> exit()

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolo11n-pose_ultralytics_onnx
           └── yolo11n-pose.onnx
           └── yolo11n-pose_cut.onnx
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   


## 5. Delete the environment for yolox_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/ultralytics_yolo11-pose
rm -R ${TVM_ROOT}/convert/repos/ultralytics_yolo11-pose

```

## 5. Next Step

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