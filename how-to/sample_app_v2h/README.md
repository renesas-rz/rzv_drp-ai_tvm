# AI Sample Application for RZ/V2H

## Build the application

1. Please refer to [Application Example for V2H](../../apps/README.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
    # sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../camera/define.h # If you use MIPI camera.
    make
    ```

2. The `sample_app_drpai_tvm_usbcam_yolox` application binary is generated.

## AI models

This sample only uses [yolox](https://github.com/Megvii-BaseDetection/YOLOX).
[The ready-to-use onnx file is here.](./yolox-S_VOC.onnx)

```bash
cd $TVM_ROOT/tutorials
sed -i -e 's/256/640/g' compile_onnx_model_quant.py
sed -i -e 's/ 224/ 640/g' compile_onnx_model_quant.py
sed -i -e 's/to_tensor/pil_to_tensor/g' compile_onnx_model_quant.py
sed -i -e '/std = stdev/d' compile_onnx_model_quant.py
sed -i -e '/F.normalize/d' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/1920, 1920, 2/g' compile_onnx_model_quant.py
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py
sed -i -e '/cof_add/d' compile_onnx_model_quant.py
sed -i -e '/cof_mul/d' compile_onnx_model_quant.py

python3 compile_onnx_model_quant.py \
$TVM_ROOT/how-to/sample_app_v2h/yolox-S_VOC.onnx \
-o yolox_cam \
-t $SDK \
-d $TRANSLATOR \
-c $QUANTIZER \
-s 1,3,640,640 \
-v 100 \
--images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ 
```

## Setup the Execution Environment  

### 1. Setup the board  

```bash
cd $TVM_ROOT/../
rm -r sample_app ; mkdir sample_app
cp $TVM_ROOT/obj/build_runtime/V2H/libtvm_runtime.so sample_app/
cp $TVM_ROOT/how-to/sample_app_v2h/src/build/sample_app_drpai_tvm_usbcam_yolox sample_app/
cp -r $TVM_ROOT/tutorials/yolox_cam sample_app/
tar cvfz sample.tar.gz sample_app/

```

## Run the application

### 1. Connecting Camera and Display

- Camera : Use a MIPI camera
	- Please refer to the [e-con Systems product page](https://www.e-consystems.com/renesas/sony-starvis-imx462-ultra-low-light-camera-for-renesas-rz-v2h.asp) for information on obtaining e-CAM22_CURZH
	- Please connect e-con Systems e-CAM22_CURZH to the MIPI connector (CN7) on the EVK board
		<img src=./img/connect_e-cam22_curzh_to_rzv2h_evk.png width=700>
- Display : Please connect to the HDMI port on the EVK board

### 2. **(On RZ/V Board)** Copy and Try it  

For example, as follows.
```sh
scp <yourhost>:sample.tar.gz .
tar xvfz sample.tar.gz 
cd sample_app/
export LD_LIBRARY_PATH=.
./sample_app_drpai_tvm_usbcam_yolox
# ./sample_app_drpai_tvm_usbcam_yolox 2 6 # run DRP-AI at 315Mhz
```
