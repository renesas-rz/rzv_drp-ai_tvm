# Human Pose Estimation

### Model: [HRNet(High-Resolution Network)](#model-information)
Sample application code and its execution environment are provided in **[here](../../../../sample_app)**.  

### Index
- [Overview](#overview)  
- [Model Information](#model-information)  
- [How to Compile the Model](#how-to-compile-the-model)
  - [Operating Environment](#operating-environment)
  - [Prepare ONNX Model](#prepare-onnx-model)
  - [Compile ONNX Model](#compile-onnx-model)
- [Processing Details](#processing-details)  

## Overview
This page explains about Human Pose Estimation in the [sample application](../../../../sample_app) for DRP-AI TVM[^1].  

<img src=./img/hrnet.jpg width=500>  

## Model Information
- HRNet: [MMpose Topdown Heatmap + Hrnet on Coco (W32)](https://github.com/open-mmlab/mmpose/blob/v0.26.0/configs/body/2d_kpt_sview_rgb_img/topdown_heatmap/coco/hrnet_coco.md)  
Dataset: [COCO](https://cocodataset.org/#home)  
Input size: 1x3x256x192  
Output size: 1x17x64x48  

## How to Compile the Model
To run the Human Pose Estimation, `hrnet_onnx` Model Object is required. This section explains how to prepare and compile the model.

### Operating Environment
- mmcv-full v1.5.1  
- MMPose v0.26.0  
- PyTorch 1.9.0 (recommended)
- onnx
- onnxruntime
- opencv-python
- Additional dependencies:
  - numpy
  - cython
  - scipy
  - matplotlib
  - scikit-image

### Prepare ONNX Model
First, you need to convert the HRNet model provided by MMPose into ONNX format. You can use Docker to create a suitable environment for this conversion.

#### Detailed Docker Conversion Steps

```sh
# Create a directory to store the model data
mkdir -p model_data

# Run a Docker container with Python 3.7 (recommended for MMPose v0.26.0 compatibility)
# If you can't find a specific Docker image, check available images with: docker search pytorch/pytorch
docker run --rm -it \
    -v $(pwd)/model_data:/workspace/model_data \
    --name mmpose_container \
    python:3.7-slim \
    bash

# Inside the container, install the required packages
apt-get update && apt-get install -y build-essential gcc g++ wget git \
    libgl1-mesa-glx libglib2.0-0 libsm6 libxrender1 libxext6

# Set up Python environment
pip install --upgrade pip setuptools wheel

# Install PyTorch 1.9.0
# If this fails, you can try a different PyTorch version, but it may affect compatibility with MMPose
pip install torch==1.9.0+cpu torchvision==0.10.0+cpu -f https://download.pytorch.org/whl/torch_stable.html

# Install dependencies
pip install onnx onnxruntime opencv-python
pip install mmcv-full==1.5.1 -f https://download.openmmlab.com/mmcv/dist/cpu/torch1.9.0/index.html

# Install chumpy dependencies and chumpy from source
# This avoids common installation issues with chumpy
pip install numpy scipy matplotlib scikit-image cython
git clone https://github.com/mattloper/chumpy.git
cd chumpy
pip install -e .
cd ..

# Create workspace directory
mkdir -p /workspace
cd /workspace
# Clone and install MMPose
git clone -b v0.26.0 https://github.com/open-mmlab/mmpose.git
cd mmpose

# Create a custom requirements file without chumpy (already installed)
grep -v chumpy requirements/runtime.txt > custom_requirements.txt
pip install -r custom_requirements.txt
pip install -e .

# Download the pre-trained model
cd /workspace
wget https://download.openmmlab.com/mmpose/top_down/hrnet/hrnet_w32_coco_256x192-c78dce93_20200708.pth

# Convert the model to ONNX format
# You may see deprecation warnings about MMDeploy which can be safely ignored
cd /workspace/mmpose
python tools/deployment/pytorch2onnx.py \
    configs/body/2d_kpt_sview_rgb_img/topdown_heatmap/coco/hrnet_w32_coco_256x192.py \
    /workspace/hrnet_w32_coco_256x192-c78dce93_20200708.pth \
    --output-file /workspace/model_data/hrnet.onnx

# Exit the container
exit
```

After exiting the container, you should have the ONNX model file in the `model_data` directory.

Copy the ONNX model (`hrnet.onnx`) from the `model_data` directory to your DRP-AI TVM tutorials directory (`$TVM_ROOT/tutorials/`).

### Compile ONNX Model
After preparing the ONNX model, you need to compile it using DRP-AI TVM. Before compiling, you need to modify the `compile_onnx_model.py` script to adjust the preprocessing settings.

1. Set the environment variables, i.e. `$TVM_ROOT` etc., according to [Installation](../../../../../setup/).  

2. Reset the compilation script to its original state and then change the pre-processing details as shown below:

```sh
cd $TVM_ROOT/tutorials
git checkout -- compile_onnx_model.py
```

Before
```py
#L105~128
    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 3]
    config.format_in    = drpai_param.FORMAT.BGR
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8
    
    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.RGB
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32 
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.
    
    # 4.1.3. Define operators to be run.
    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```
After
```py
#L105~129
    # 4.1.1. Define input data of preprocessing
    config.shape_in     = [1, 480, 640, 2]
    config.format_in    = drpai_param.FORMAT.YUYV_422
    config.order_in     = drpai_param.ORDER.HWC
    config.type_in      = drpai_param.TYPE.UINT8
    
    # 4.1.2. Define output data of preprocessing (Will be model input)
    model_shape_in = list(opts["input_shape"])
    config.shape_out    = model_shape_in
    config.format_out   = drpai_param.FORMAT.RGB
    config.order_out    = drpai_param.ORDER.CHW
    config.type_out     = drpai_param.TYPE.FP32 
    # Note: type_out depends on DRP-AI TVM[*1]. Usually FP32.
    
    # 4.1.3. Define operators to be run.
    mean    = [0.485, 0.456, 0.406]
    stdev   = [0.229, 0.224, 0.225]
    r = 255
    cof_add = [-m*r for m in mean]
    cof_mul = [1/(s*r) for s in stdev]
    config.ops = [
        op.Crop(184, 0, 270, 480),
        op.Resize(model_shape_in[3], model_shape_in[2], op.Resize.BILINEAR),
        op.Normalize(cof_add, cof_mul)
    ]
```

You can use the following sed commands to automatically make these changes:

```sh
# Change input shape from [1, 480, 640, 3] to [1, 480, 640, 2]
sed -i 's/config\.shape_in\s*=\s*\[1,\s*480,\s*640,\s*3\]/config.shape_in     = [1, 480, 640, 2]/' compile_onnx_model.py

# Change input format from BGR to YUYV_422
sed -i 's/config\.format_in\s*=\s*drpai_param\.FORMAT\.BGR/config.format_in    = drpai_param.FORMAT.YUYV_422/' compile_onnx_model.py

# Add Crop operation to the preprocessing steps
sed -i 's/config\.ops = \[/config.ops = [\n        op.Crop(184, 0, 270, 480),/' compile_onnx_model.py
```

3. Run the compilation script with the command below:  
```sh
# Run DRP-AI TVM[*1] Compiler script
$ python3 compile_onnx_model.py \
./hrnet.onnx \
-o hrnet_onnx \
-s 1,3,256,192 \
-i input.1
```

4. Confirm that three files, `deploy.so`, `deploy.params`, and `deploy.json`, and `preprocess` directory have been created in the `hrnet_onnx` directory.  

5. Copy the `hrnet_onnx` directory to the execution environment directory:
```sh
# Create the exe directory if it doesn't exist
mkdir -p $TVM_ROOT/tutorials/exe

# Copy the model objects to the exe directory
cp -r hrnet_onnx $TVM_ROOT/tutorials/exe/
```

6. For deployment, copy the `hrnet_onnx` directory into the execution environment directory where the compiled sample application `sample_app_drpai_tvm_usbcam_http` is located.  

## Processing Details
### DRP-AI mode
- Source Code: [tvm_drpai_hrnet.cpp](../../../src/recognize/hrnet/tvm_drpai_hrnet.cpp)  

Followings are processing details if user selected "HRNet (DRP-AI)". 

#### Pre-processing
Pre-processing is done by DRP-AI Pre-processing Runtime.  
Please see the [compile script](#compile-onnx-model) to check the pre-processing details.  
For more information on DRP-AI Pre-processing Runtime, see its [Documentation](../../../../../docs/PreRuntime.md).  

The pre-processing steps include:
1. Converting YUYV_422 format input to RGB
2. Cropping the image to the region of interest (184, 0, 270, 480)
3. Resizing the image to the model input size (256x192)
4. Normalizing the pixel values using mean [0.485, 0.456, 0.406] and standard deviation [0.229, 0.224, 0.225]

#### Inference
The Object files `hrnet_onnx` are generated from HRNet pre-trained model provided by MMPose framework as described in [Model Information](#model-information).  

#### Post-processing
Post-processing is processed by CPU.

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.