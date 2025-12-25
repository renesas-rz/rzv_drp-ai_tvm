# Object Detection: YOLOX

## Build the application

1. Please refer to [Application Example for RZ/V2H and RZ/V2N](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_yolox_cam/src
    cp $TVM_ROOT/how-to/sample_app_v2h/app_deeplabv3_cam/src/CMakeLists.txt CMakeLists.txt
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake -DAPP_NAME=app_yolox_cam ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `app_yolox_cam` application binary is generated.

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
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py
sed -i -e '/cof_add/d' compile_onnx_model_quant.py
sed -i -e '/cof_mul/d' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/1920, 1920, 2/g' compile_onnx_model_quant.py
```

### Additional edit for USB Camera

If you are using a USB camera, execute the following additional commands.

```bash
sed -i -e 's/1920, 1920, 2/640, 640, 2/g' compile_onnx_model_quant.py
```

### Run compile_onnx_model_quant.py

```bash
python3 compile_onnx_model_quant.py \
$TRANSLATOR/../onnx_models/YoloX-S_VOC_sparse70.onnx \
 -o yolox_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -s 1,3,640,640 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

## Setup the Execution Environment  

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_yolox_cam ; mkdir sample_yolox_cam
cp $TVM_ROOT/obj/build_runtime/v2h/lib/* sample_yolox_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_yolox_cam/src/build/app_yolox_cam sample_yolox_cam/
cp -r $TVM_ROOT/tutorials/yolox_cam sample_yolox_cam/
tar cvfz sample_yolox.tar.gz sample_yolox_cam/
```

## Run the application

### 1. Connecting Camera and Display

- Camera:
  - Use a MIPI camera:
    - Please refer to the [e-con Systems product page](https://www.e-consystems.com/renesas/sony-starvis-imx462-ultra-low-light-camera-for-renesas-rz-v2h.asp) for information on obtaining e-CAM22_CURZH
    - Please connect e-con Systems e-CAM22_CURZH to the MIPI connector (CN7) on the EVK board
    <img src=../../img/connect_e-cam22_curzh_to_rzv2h_evk.png width=700>

  - Use a USB camera:
    - Please connect USB camera as shown below on the EVK board
      <table>
        <tr>
          <th>RZ/V2H EVK</th>
          <th>RZ/V2N EVK</th>
        </tr>
        <tr>
          <td><img src=../../img/hw_conf_v2h.png width=600></td>
          <td><img src=../../img/hw_conf_v2n.png width=600></td>
        </tr>
      </table>

- Display: Please connect to the HDMI port on the EVK board

### 2. **(On RZ/V Board)** Copy and Try it  

For example, as follows.

```sh
scp <yourhost>:sample_yolox.tar.gz .
tar xvfz sample_yolox.tar.gz
cd sample_yolox_cam/
su
export LD_LIBRARY_PATH=.
./app_yolox_cam
exit # After terminating the application.
```

  > **Note1:** For RZ/V2H and RZ/V2N AI SDK v6.00 and later, you need to switch to the root user with the `su` command when running an application.  
  This is because when you run an application from a weston-terminal, you are switched to the "weston" user, which does not have permission to run the `/dev/xxx` device used in the application.
  
  > **Note2:** The chmod +x <filename> command is necessary if the *.tar.gz file or the application file does not have execution permission.

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_yolox.png width=480>

On application window, following information is displayed.

- Camera capture
- Object Detection result (Bounding boxes, class name and score.)  
- Processing times
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes. \[msec\]
  - Inference: Processing time taken for AI inference. \[msec\]
  - PreProcess: Processing time taken for AI pre-processes. \[msec\]
  - PostProcess: Processing time taken for AI post-processes. \[msec\]

### 4. How Terminate Application

To terminate the application, press `Super(Window) + Tab` keys to display the Linux console terminal of RZ/V2H or RZ/V2N Evaluation Board Kit and press `Enter` key on there.

### 5. Logs

The `<timestamp>_app_yolox_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : Megvii-Base Detection YOLOX | yolox_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : XXXX Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (525, 51, 203, 59)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : car (Class 6)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 57.7 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 3
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (330, 185, 497, 268)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : bicycle (Class 1)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 89.6 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 13
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (149, 283, 255, 384)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : dog (Class 11)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 51.2 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Count  : 3
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Total AI Time  : xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PreProcess     : xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference      : xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PostProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI Inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```
