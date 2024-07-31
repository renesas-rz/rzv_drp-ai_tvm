# Object Detection: YOLOX

## Build the application

1. Please refer to [Application Example for V2H](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_yolox_cam/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `sample_app_drpai_tvm_yolox_cam` application binary is generated.

## AI models

This sample only uses [yolox](https://github.com/Megvii-BaseDetection/YOLOX).
[The ready-to-use onnx file is here.](./yolox-S_VOC.onnx)

```bash
cd $TVM_ROOT/tutorials
git checkout compile_onnx_model_quant.py
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
$TRANSLATOR/../onnx_models/YoloX-S_VOC_sparse70.onnx \
 -o yolox_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -s 1,3,640,640 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ \
 -v 100
```

## Setup the Execution Environment  

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_yolox_cam ; mkdir sample_yolox_cam
cp $TVM_ROOT/obj/build_runtime/V2H/libtvm_runtime.so sample_yolox_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_yolox_cam/src/build/sample_app_drpai_tvm_yolox_cam sample_yolox_cam/
cp -r $TVM_ROOT/tutorials/yolox_cam sample_yolox_cam/
tar cvfz sample_yolox.tar.gz sample_yolox_cam/
```

## Run the application

### 1. Connecting Camera and Display

- Camera : Use a MIPI camera
  - Please refer to the [e-con Systems product page](https://www.e-consystems.com/renesas/sony-starvis-imx462-ultra-low-light-camera-for-renesas-rz-v2h.asp) for information on obtaining e-CAM22_CURZH
  - Please connect e-con Systems e-CAM22_CURZH to the MIPI connector (CN7) on the EVK board
    <img src=../../img/connect_e-cam22_curzh_to_rzv2h_evk.png width=700>
- Display : Please connect to the HDMI port on the EVK board

### 2. **(On RZ/V Board)** Copy and Try it  

For example, as follows.

```sh
scp <yourhost>:sample_yolox.tar.gz .
tar xvfz sample_yolox.tar.gz
cd sample_yolox_cam/
export LD_LIBRARY_PATH=.
./sample_app_drpai_tvm_yolox_cam
# ./sample_app_drpai_tvm_yolox_cam 2 5 # run DRP-AI at 315Mhz
```

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_yolox.png width=480>

On application window, following information is displayed.

- Camera capture
- Object Detection result (Bounding boxes, class name and score.)  
- Processing times
  - Pre-Proc + Inference (DRP-AI): Processing time taken for AI inference and its pre/post-processes on DRP-AI. [msec]
  - Post-Proc (CPU): Processing time taken for post-processes of AI inference on CPU. [msec]
  - AI/Camera Frame Rate: The number of AI inferences per second and the number of Camera captures per second. [fps]

### 4. How Terminate Application

To terminate the application, press `Enter` key on the Linux console terminal of RZ/V2H Evaluation Board Kit.

### 5. Logs

The `<timestamp>_app_yolox_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : Megvii-Base Detection YOLOX | yolox_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : MIPI Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 4
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (457, 245, 208, 427)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : person (Class 14)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 89.1 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 7
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (457, 354, 297, 250)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : bicycle (Class 1)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 90.9 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 10
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (181, 214, 388, 441)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : bus (Class 5)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 50.5 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  bounding box Count  : 3
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Pre-Proc + Inference (DRP-AI): XX.X [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Post-Proc (CPU): X.X [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] AI Frame Rate XX [fps]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```
