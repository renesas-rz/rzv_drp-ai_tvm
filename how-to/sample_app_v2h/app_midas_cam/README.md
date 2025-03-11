# Depth Estimation: MiDaS

## Build the application

1. Please refer to [Application Example for V2H](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_midas_cam/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `app_midas_cam` application binary is generated.

## AI models

This sample only uses [midas](https://github.com/isl-org/MiDaS).

```bash
cd $TVM_ROOT/tutorials
# Download onnx model (When installing python packages such as timm, it is recommended that you use a virtual environment such as venv.)
pip3 install timm
python3 $TVM_ROOT/how-to/sample_app_v2h/app_midas_cam/midas_download.py
git checkout compile_onnx_model_quant.py
sed -i -e 's/256,/(256, 256),/g' compile_onnx_model_quant.py
sed -i -e 's/, 224/, (256, 256)/g' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/480, 640, 2/g' compile_onnx_model_quant.py
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py

python3 compile_onnx_model_quant.py \
 midas_small_256x256_simple.onnx \
 -o midas_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -i input1 \
 -s 1,3,256,256 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ \
 -v 100
```

## Setup the Execution Environment

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_midas_cam  ; mkdir sample_midas_cam
cp $TVM_ROOT/obj/build_runtime/V2H/libtvm_runtime.so sample_midas_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_midas_cam/src/build/app_midas_cam sample_midas_cam/
cp -r $TVM_ROOT/tutorials/midas_cam sample_midas_cam/
tar cvfz sample_midas.tar.gz sample_midas_cam/
```

## Run the application

### 1. Connecting Camera and Display

- Camera
  - Use a MIPI camera:
    - Please refer to the [e-con Systems product page](https://www.e-consystems.com/renesas/sony-starvis-imx462-ultra-low-light-camera-for-renesas-rz-v2h.asp) for information on obtaining e-CAM22_CURZH
    - Please connect e-con Systems e-CAM22_CURZH to the MIPI connector (CN7) on the EVK board
    <img src=../../img/connect_e-cam22_curzh_to_rzv2h_evk.png width=700>
  - Use a USB camera:
    - Please connect USB camera as shown below on the EVK board
    <img src=./img/hw_conf_v2h.png width=700>
- Display : Please connect to the HDMI port on the EVK board

### 2. **(On RZ/V Board)** Copy and Try it  

For example, as follows.

```sh
scp <yourhost>:sample_midas.tar.gz .
tar xvfz sample_midas.tar.gz 
cd sample_midas_cam/
export LD_LIBRARY_PATH=.
./app_midas_cam
```

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_midas.jpg width=640>

On application window, following information is displayed.

- Left: Camera capture
- Right: Inference result (depth map)
- Processing times
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes.[msec]
  - Inference: Processing time taken for AI inference.[msec]
  - PreProcess: Processing time taken for AI pre-processes.[msec]
  - PostProcess: Processing time taken for AI post-processes.[msec]

### 4. How Terminate Application

To terminate the application, press `Enter` key on the Linux console terminal of RZ/V2H Evaluation Board Kit.

### 5. Logs

The `<timestamp>_app_midas_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : midas | midas_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : USB Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Total AI Time: xxx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference: xxx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PreProcess: x.xx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PostProcess: xxx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```

## Note

### Limit the number of CPU thread for AI model inference

In this midas sample application, the number of CPU threads used for AI model inference is set to 2 to maximize the performance of the demo application including camera input and image output. You can change the maximum number of CPU threads from 1 to 4. A below command is a sample to set the number of thread to 2.

```bash
export TVM_NUM_THREADS=2
```
