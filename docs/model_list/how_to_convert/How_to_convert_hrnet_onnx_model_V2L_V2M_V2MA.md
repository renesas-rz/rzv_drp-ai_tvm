# How to convert hrnet_onnx model for V2L/M/MA
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------|-------------------|
| [HRNet](https://download.openmmlab.com/mmpose/top_down/hrnet/hrnet_w32_coco_256x192-c78dce93_20200708.pth)                                                           |hrnet                          | Body Keypiont 2D     |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for hrnet_onnx model.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/mmlab 
. ${TVM_ROOT}/convert/venvs/mmlab/bin/activate
git clone -b v1.7.0 https://github.com/open-mmlab/mmcv.git ${TVM_ROOT}/convert/repos/mmcv
git clone -b v2.28.2 https://github.com/open-mmlab/mmdetection.git ${TVM_ROOT}/convert/repos/mmdetection
git clone --recursive -b v0.29.0 https://github.com/open-mmlab/mmpose.git ${TVM_ROOT}/convert/repos/mmpose
pip install torch==1.8.0 torchvision==0.9.0 timm==0.6.12 onnx onnxruntime

cd ${TVM_ROOT}/convert/repos/mmcv
MMCV_WITH_OPS=1 pip install -e .

cd ${TVM_ROOT}/convert/repos/mmdetection
MMCV_WITH_OPS=1 pip install -e .

cd ${TVM_ROOT}/convert/repos/mmpose
MMCV_WITH_OPS=1 pip install -e .
```

## 3. Convert PyTorch Model (.pth) file to ONNX (.onnx) file.

Note : Check the downloaded PyTorch Model (.pth) file from the link in the table above is located in `${TVM_ROOT}/convert/repos/mmpose`.\
Use the following script to convert the model.

```sh
cd ${TVM_ROOT}/convert/repos/mmpose
python3 tools/deployment/pytorch2onnx.py \
    configs/body/2d_kpt_sview_rgb_img/topdown_heatmap/coco/hrnet_w32_coco_256x192.py \
    ./hrnet_w32_coco_256x192-c78dce93_20200708.pth \
    --opset-version 11 \
    --shape 1 3 256 192 \
    --output-file hrnet.onnx

mkdir -p ${TVM_ROOT}/convert/output/hrnet_mmpose_onnx
mv hrnet.onnx ${TVM_ROOT}/convert/output/hrnet_mmpose_onnx/
```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── hrnet_mmpose_onnx
           └── hrnet.onnx
```

## 4. Delete the environment for hrnet_onnx model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/mmlab
rm -R ${TVM_ROOT}/convert/repos/mmcv
rm -R ${TVM_ROOT}/convert/repos/mmdetection
rm -R ${TVM_ROOT}/convert/repos/mmpose
```

## 5. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_onnx_model.py ../convert/output/hrnet_mmpose_onnx/hrnet.onnx -o hrnet_mmpose_onnx -s 1,3,256,192 -i data
```

----

