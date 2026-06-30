# How to convert yolox_onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                        | Download model name | Input shape | Task             |
|-------------------------------------------------------------------------------------------------|---------------------|-------------|------------------|
| [YOLOX_s](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_s.pth) | yolox-s             | (640, 640)  | Object Detection |
| [YOLOX_m](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_m.pth) | yolox-m             | (640, 640)  | Object Detection |
| [YOLOX_l](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_l.pth) | yolox-l             | (640, 640)  | Object Detection |
| [YOLOX_x](https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_x.pth) | yolox-x             | (640, 640)  | Object Detection |
---

License: [Apache 2.0](https://github.com/Megvii-BaseDetection/YOLOX)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for yolox_onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/yolox
git clone https://github.com/Megvii-BaseDetection/YOLOX ${TVM_ROOT}/convert/repos/yolox
cd ${TVM_ROOT}/convert/repos/yolox
git reset --hard "ac58e0a5e68e57454b7b9ac822aced493b553c53"
. ${TVM_ROOT}/convert/venvs/yolox/bin/activate
pip install --upgrade pip 
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1 wheel
pip install --no-build-isolation .
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/yolox`.\
Use the following script to convert the model. \
Set the options refer to the following table.

| option        | value                                                    |
|---------------|----------------------------------------------------------|
| ${onnx_file}  | `{ Download model name column in the table above }.onnx` |
| ${arg_name}   | `Download model name` column in the table above          |
| ${torch_file} | Downloaded PyTorch Model (.pth) file                     |
| ${image_size} | `Input shape` column in the table above                  |

---

```sh
cd ${TVM_ROOT}/convert/repos/yolox

# The following is an example for YOLOX_s
python tools/export_onnx.py --output-name ./yolox-s.onnx -n yolox-s -c ./yolox_s.pth --decode_in_inference test_size 640,640

mkdir -p ${TVM_ROOT}/convert/output/yolox_s_megvii_onnx
mv yolox-s.onnx ${TVM_ROOT}/convert/output/yolox_s_megvii_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── yolox_s_megvii_onnx
           └── yolox-s.onnx
```
## 4. Cut post-process with onnx file.

Yolox models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all yolox onnxs by looking at the example script below.

| cut point  | node name               |
| ---        | ---                     |
| 1/8 scale  | /head/Concat_output_0   |
| 1/16 scale | /head/Concat_1_output_0 |
| 1/32 scale | /head/Concat_2_output_0 |

<center><img src=./img/cut_yolox.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for YOLOx-s.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("yolox-s.onnx", "yolox-s_cut.onnx", ["images"], ["/head/Concat_output_0", "/head/Concat_1_output_0", "/head/Concat_2_output_0",])
>>> exit()
```

:information_source:　Node names in saved ONNX models may change depending on the OSS library versions.
If the cut operation fails, please check both the saved ONNX model and the node names for the cut.   

## 5. Delete the environment for yolox_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/yolox
rm -R ${TVM_ROOT}/convert/repos/yolox
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
