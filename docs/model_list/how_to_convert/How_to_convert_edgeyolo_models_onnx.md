# How to convert edgeyolo_onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                             | Download model name | Input shape | Task             |
|------------------------------------------------------------------------------------------------------|---------------------|-------------|------------------|
| [EdgeYOLO_Tiny](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_tiny_coco.pth) | edgeyolo_tiny       | (640, 640)  | Object Detection |
| [EdgeYOLO_S](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_s_coco.pth)       | edgeyolo_s          | (640, 640)  | Object Detection |
| [EdgeYOLO_M](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_m_coco.pth)       | edgeyolo_m          | (640, 640)  | Object Detection |
| [EdgeYOLO](https://github.com/LSH9832/edgeyolo/releases/download/v0.0.0/edgeyolo_coco.pth)           | edgeyolo            | (640, 640)  | Object Detection |

---

License: [Apache 2.0](https://github.com/LSH9832/edgeyolo/) 

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for edgeyolo_onnx models.

### `Your work PC needs nvidia gpus for tensor RT`

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo
git clone https://github.com/LSH9832/edgeyolo ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
cd ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
git reset --hard "1d15c76db54fc191954bf474480b8c50f05a178a"
. ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo/bin/activate
pip install torch==2.3.1+cpu torchvision==0.18.1+cpu -f https://download.pytorch.org/whl/torch_stable.html
pip install onnx==1.16.0 onnxruntime==1.20.1
pip install numpy==1.23.5 tensorrt protobuf==3.20.3
pip install -r requirements.txt
```

## 3. Convert PyTorch Model (.pth) files to ONNX (.onnx) files.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/lsh9832_edgeyolo`.\
Use the following script to convert the model. \
Set the options refer to the following table.

| option        | value                                           |
|---------------|-------------------------------------------------|
| --weights     | Downloaded PyTorch Model (.pth) file            |
| --input-size  | `Input shape` column in the table above         |
| ${model_base} | `Download model name` column in the table above |
---

```sh
cd ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo

# The following is an example for EdgeYOLO
python export.py --weights edgeyolo_coco.pth --onnx-only --opset 12 --no-fp16 --input-size 320
mv ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo/output/export/edgeyolo_coco/*.onnx "edgeyolo.onnx"

mkdir -p ${TVM_ROOT}/convert/output/edgeyolo_onnx
mv edgeyolo.onnx ${TVM_ROOT}/convert/output/edgeyolo_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── edgeyolo_onnx
           └── edgeyolo.onnx
```
## 4. Cut post-process with onnx file.

edgeyolo models have redundant post-processing part, so cut part from onnx.
Please delete the following six nodes common to all edgeyolo onnxs by looking at the example script below.

| cut point  | node name for edgeyolo-tiny, edgeyolo-s, edgeyolo-m | node name edgeyolo                    |
|------------|-----------------------------------------------------|---------------------------------------|
| 1/8 scale  | /model.113/cls_preds.0/Conv_output_0                | /model.105/cls_preds.0/Conv_output_0  |
| 1/8 scale  | /model.113/rego_preds.0/Conv_output_0               | /model.105/rego_preds.0/Conv_output_0 |
| 1/16 scale | /model.113/cls_preds.1/Conv_output_0                | /model.105/cls_preds.1/Conv_output_0  |
| 1/16 scale | /model.113/rego_preds.1/Conv_output_0               | /model.105/rego_preds.1/Conv_output_0 |
| 1/32 scale | /model.113/cls_preds.2/Conv_output_0                | /model.105/cls_preds.2/Conv_output_0  |
| 1/32 scale | /model.113/rego_preds.2/Conv_output_0               | /model.105/rego_preds.2/Conv_output_0 |

<center><img src=./img/cut_edgeyolo.png></center>

```sh
$ python3
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("<onnx name>", "<cut onnx name>", "<input_node_list>", "<output_node_list>")
>>> exit()

# The following is an example for edgeyolo_tiny.
Python 3.10.12 (main, Aug 15 2025, 14:32:43)
[GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import onnx
>>> onnx.utils.extract_model("edgeyolo_tiny.onnx", "edgeyolo_tiny_cut.onnx", ["input_0"], ["/model.113/cls_preds.0/Conv_output_0", "/model.113/rego_preds.0/Conv_output_0", "/model.113/cls_preds.1/Conv_output_0", "/model.113/rego_preds.1/Conv_output_0", "/model.113/cls_preds.2/Conv_output_0", "/model.113/rego_preds.2/Conv_output_0"])
>>> exit()
```

## 5. Delete the environment for edgeyolo_onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/lsh9832_edgeyolo
rm -R ${TVM_ROOT}/convert/repos/lsh9832_edgeyolo
```

## 6. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.
Please refer below guides.

Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)

----
