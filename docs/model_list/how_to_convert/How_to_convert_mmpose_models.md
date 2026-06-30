# How to convert MMPose models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                       | Download model name  | Input Shape    |
|------------------------------------------------------------------------------------------------|----------------------|----------------|
| DeepPose                                                                                       | deeppose_res50_coco  | (1,3,256,192)  |
| HRNetV2                                                                                        | hrnetv2              | (1,3,256,192)  |
| HRNetV2 DarkPose                                                                               | pose_hrnet_w18_dark  | (1,3,256,192)  |
---

License: Apache 2.0 ([mmcv](https://github.com/open-mmlab/mmcv.git), [mmdetection](https://github.com/open-mmlab/mmdetection.git), [mmpose](https://github.com/open-mmlab/mmpose.git))

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```  

## 2. Create an environment for MMPose models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/mmpose
. ${TVM_ROOT}/convert/venvs/mmpose/bin/activate

pip install torch==2.1.2 torchvision==0.16.2 \
    pytz opencv-python gitpython pandas requests pyyaml \
    tqdm matplotlib seaborn psutil ipython scipy smplx \
    numpy==1.23.5

git clone -b v1.7.2 https://github.com/open-mmlab/mmcv.git ${TVM_ROOT}/convert/repos/mmcv
git clone -b v2.28.2 https://github.com/open-mmlab/mmdetection.git ${TVM_ROOT}/convert/repos/mmdetection
git clone --recursive -b 0.x https://github.com/open-mmlab/mmpose.git ${TVM_ROOT}/convert/repos/mmpose

cd ${TVM_ROOT}/convert/repos/mmcv
MMCV_WITH_OPS=1 pip install -e .
cd ${TVM_ROOT}/convert/repos/mmdetection
MMCV_WITH_OPS=1 pip install -e .
cd ${TVM_ROOT}/convert/repos/mmpose
MMCV_WITH_OPS=1 pip install -e .
```

## 3. Download and save as TorchScript (.pt) files.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-s     |`Input shape` column in the table above         |
|-n     |`Download model name` column in the table above |
---

```sh
cd $TVM_ROOT/convert/ 
# The following is an example for DeepPose
python download_mmpose.py \
    -s 1,3,256,192 \
    -n deeppose_res50_coco
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── deeppose_res50_coco_mmpose_pytorch
           └── deeppose_res50_coco.pt
```

## 4. Delete the environment for MMPose models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/mmpose
rm -R ${TVM_ROOT}/convert/repos/mmcv
rm -R ${TVM_ROOT}/convert/repos/mmdetection
rm -R ${TVM_ROOT}/convert/repos/mmpose
```

## 5. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.
Please refer below guides.

Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)


----