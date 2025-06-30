# How to convert yolo11-pose_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name   |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| [YOLO11n-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n-pose.pt)                                              |yolo11n-pose             |(640, 640)     | Pose Estimation  |
| [YOLO11s-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s-pose.pt)                                              |yolo11s-pose               |(640, 640)     | Pose Estimation  |
| [YOLO11m-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11m-pose.pt)                                              |yolo11m-pose                |(640, 640)     | Pose Estimation  |
| [YOLO11l-pose](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11l-pose.pt)                                              |yolo11l-pose                |(640, 640)     | Pose Estimation  |

---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolo11-pose_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/yolo11-pose
. ${TVM_ROOT}/convert/venvs/yolo11-pose/bin/activate
pip install ultralytics==8.3.146
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.



```sh
cd ${TVM_ROOT}/convert/

# The following is an example for YOLO11n-pose
wget https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n-pose.pt

#onnx file will be generated in the same folder
python3 convert_yolo11pose_to_onnx.py yolo11n-pose.pt 

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── convert_yolo11pose_to_onnx.py
 └── yolo11n-pose.onnx
 └── yolo11n-pose.pt
```

## 4. Cut post-process with onnx file.
Yolo11 pose models have redundant post-processing part, so cut part from onnx.
Please delete the following nodes common to all yolo11 onnxs.

<center><img src=./img/yolo11_pose_cut_guide.jpg></center>

Below is a sample script to cut yolo11 pose models.
```py
import onnx
import sys
onnx_model = sys.argv[1] # Target onnx model file
model = onnx.load(onnx_model)
graph = model.graph
# Search post processing satart points
#  If the node name is different from your model, please modiy below list
Concat_list = ["/model.23/Concat_3","/model.23/Concat_2","/model.23/Concat_1"]
Concat_reshape_list = ["/model.23/Concat"]
cut_node_list = list()
reshape_out_list = list()
for node in graph.node:
    name = node.name
    if(name in Concat_list):
        cut_node_list.extend(node.input)
    if(name in Concat_reshape_list):
        reshape_out_list.extend(node.input)
for node in graph.node:
    o_node = node.output[0]
    if(o_node in reshape_out_list):
        cut_node_list.append(node.input[0])

# Cut post processing parts
output_onnx = onnx_model.replace(".onnx","_wo_post.onnx")
input_names = ['images']
output_names = cut_node_list
onnx.utils.extract_model(onnx_model, output_onnx, input_names, output_names)
print(f"Save yolo11 model >>> {output_onnx}")
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── convert_yolo11pose_to_onnx.py
 └── yolo11n-pose.onnx
 └── yolo11n-pose_wo_post.onnx
 └── yolo11n-pose.pt
```

## 5. Delete the environment for yolox_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/yolo11-pose
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOX_s, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/yolo11n-pose_wo_post.onnx  -o yolov11-pose_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER  

```

----