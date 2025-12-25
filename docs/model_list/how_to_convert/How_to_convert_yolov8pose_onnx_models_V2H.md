# How to convert yolov8-pose_onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name   |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| [YOLOv8n-pose](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n-pose.pt)                                              |yolov8n-pose             |(640, 640)     | Pose Estimation  |
| [YOLOv8s-pose](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8s-pose.pt)                                              |yolov8s-pose               |(640, 640)     | Pose Estimation  |
| [YOLOv8m-pose](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8m-pose.pt)                                              |yolov8m-pose                |(640, 640)     | Pose Estimation  |
| [YOLOv8l-pose](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8l-pose.pt)                                              |yolov8l-pose                |(640, 640)     | Pose Estimation  |
| [YOLOv8x-pose](https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8x-pose.pt)                                              |yolov8x-pose                |(640, 640)     | Pose Estimation  |


---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolov8-pose_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/yolov8-pose
. ${TVM_ROOT}/convert/venvs/yolov8-pose/bin/activate
pip install ultralytics==8.3.146
```

## 3. Convert PyTorch Model (.pt) files to ONNX (.onnx) files.



```sh
cd ${TVM_ROOT}/convert/

# The following is an example for YOLOv8n-pose
wget https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n-pose.pt

#onnx file will be generated in the same folder
python3 convert_yolo_pt_to_onnx.py yolov8n-pose.pt 

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── convert_yolo_pt_to_onnx.py
 └── yolov8n-pose.onnx
 └── yolov8n-pose.pt
```

## 4. Cut post-process with onnx file.
Yolov8 pose models have redundant post-processing part, so cut part from onnx.
Please delete the following nodes common to all yolov8 pose onnxs.

<center><img src=./img/yolov8_pose_cut_guide.jpg></center>

Below is a sample script to cut yolov8 pose models.
```py
import onnx
import sys

onnx_model = sys.argv[1] #target onnx model file

model = onnx.load(onnx_model)
graph = model.graph

# Search post processing parts
Concat_list = ["/model.22/Concat_3","/model.22/Concat_2","/model.22/Concat_1"]
Concat_reshape_list = ["/model.22/Concat"]

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
print(f"Save yolov8-pose model >>> {output_onnx}")

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── convert_yolo_pt_to_onnx.py
 └── yolov8n-pose.onnx
 └── yolov8n-pose_wo_post.onnx
 └── yolov8n-pose.pt
```

## 5. Delete the environment for yolox_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/yolov8-pose
```

## 5. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For YOLOv8n, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/yolov8n-pose_wo_post.onnx  -o yolov8-pose_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER  

```

----