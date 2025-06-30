# How to convert hrnet_onnx model for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------|-------------------|
| HRNet                                                        |hrnet                          | Body Keypiont 2D     |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
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
git clone -b v2.1.0 https://github.com/open-mmlab/mmcv.git ${TVM_ROOT}/convert/repos/mmcv
git clone -b v3.3.0 https://github.com/open-mmlab/mmdetection.git ${TVM_ROOT}/convert/repos/mmdetection
git clone --recursive -b v1.3.1 https://github.com/open-mmlab/mmpose.git ${TVM_ROOT}/convert/repos/mmpose
git clone -b v1.3.1 https://github.com/open-mmlab/mmdeploy.git ${TVM_ROOT}/convert/repos/mmdeploy
pip install mmpose torch==2.1.2 torchvision==0.16.2 

cd ${TVM_ROOT}/convert/repos/mmcv
pip install -r requirements.txt
MMCV_WITH_OPS=1 pip install -e .

cd ${TVM_ROOT}/convert/repos/mmdeploy
pip install mmdeploy==1.3.1 mmdeploy-runtime==1.3.1 mmdeploy-runtime-gpu==1.3.1

cd ${TVM_ROOT}/convert/repos/mmdetection
MMCV_WITH_OPS=1 pip install -e .

cd ${TVM_ROOT}/convert/repos/mmpose
MMCV_WITH_OPS=1 pip install -e .
```

## 3. Download and save as PyTorch Model (.pth) file.

Use the following script to convert the model.

```sh
cd ${TVM_ROOT}/convert/repos/mmdeploy
sed -i 's/onnx_config = dict(input_shape=None)/onnx_config = dict(input_shape=\[192,256\],opset_version=11,save_file="hrnet.onnx")/' configs/mmpose/pose-detection_onnxruntime_static.py

python3 tools/deploy.py \
    configs/mmpose/pose-detection_onnxruntime_static.py \
    ../mmpose/configs/wholebody_2d_keypoint/topdown_heatmap/coco-wholebody/td-hm_hrnet-w32_8xb64-210e_coco-wholebody-256x192.py \
    https://download.openmmlab.com/mmpose/top_down/hrnet/hrnet_w32_coco_wholebody_256x192-853765cd_20200918.pth \
    demo/resources/human-pose.jpg \
    --work-dir output/hrnet_onnx \
    --device cpu \
    --show \
    --dump-info

mkdir -p ${TVM_ROOT}/convert/output/hrnet_mmpose_onnx
mv output/hrnet_onnx/hrnet.onnx ${TVM_ROOT}/convert/output/hrnet_mmpose_onnx/
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
rm -R ${TVM_ROOT}/convert/repos/mmdeploy
```

## 5. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_onnx_model_quant.py ../convert/output/hrnet_mmpose_onnx/hrnet.onnx -o hrnet_mmpose_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ 
```

----

