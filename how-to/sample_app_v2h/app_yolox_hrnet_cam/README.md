# Object Detection: YOLOX + HRNet

## Build the application

1. Please refer to [Application Example for RZ/V2H and RZ/V2N](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

   ```bash
   cd $TVM_ROOT/how-to/sample_app_v2h/app_yolox_hrnet_cam/src
   mkdir build
   cd build
   
   cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
   sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
   make -j$(nproc)
   ```

2. The `app_yolox_hrnet_cam` application binary is generated.

## AI models

This sample only uses [yolox](https://github.com/Megvii-BaseDetection/YOLOX) and [hrnet](https://github.com/open-mmlab/mmpose/tree/v1.1.0).

### YOLOX

  - Edit the script to compile YOLOX

    ```bash
    cd $TVM_ROOT/tutorials
    git checkout compile_onnx_model_quant.py
    sed -i -e 's/256/640/g' compile_onnx_model_quant.py
    sed -i -e 's/ 224/ 640/g' compile_onnx_model_quant.py
    sed -i -e 's/to_tensor/pil_to_tensor/g' compile_onnx_model_quant.py
    sed -i -e '/std = stdev/d' compile_onnx_model_quant.py
    sed -i -e '/F.normalize/d' compile_onnx_model_quant.py
    sed -i -e 's/FORMAT.BGR/FORMAT.RGB/g' compile_onnx_model_quant.py
    sed -i -e '/cof_add/d' compile_onnx_model_quant.py
    sed -i -e '/cof_mul/d' compile_onnx_model_quant.py
    sed -i -e 's/480, 640, 3/1920, 1920, 3/g' compile_onnx_model_quant.py
    ```

  - Additional edit for USB Camera

    If you are using a USB camera, execute the following additional commands.

    ```bash
    sed -i -e 's/1920, 1920, 3/640, 640, 3/g' compile_onnx_model_quant.py
    ```

  - Run compile_onnx_model_quant.py

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

### HRNet
  - Edit the script to compile HRNet

    ```bash
    cd $TVM_ROOT/tutorials
    git checkout compile_onnx_model_quant.py
    sed -i -e '/# Input shape helper/,/return \[batch_dim\] + \[int(d.get("dimValue")) for d in dim_info\[1::\]\]/d' compile_onnx_model_quant.py
    sed -i -e 's/get_input_shape(model_file, inp)/opts["input_shape"]/g' compile_onnx_model_quant.py
    sed -i -e 's/, 224/, (256,192)/g' compile_onnx_model_quant.py
    sed -i -e '/ref_result_output_dir =/,/exist_ok=True)/d' compile_onnx_model_quant.py
    sed -i -e '/flatten().tofile(/d' compile_onnx_model_quant.py
    sed -i -e '/os.path.join(ref_result_output_dir, "input_"/d' compile_onnx_model_quant.py
    sed -i -e 's/ref_result_output_dir/output_dir/g' compile_onnx_model_quant.py
    sed -i -e 's/_fp32//g' compile_onnx_model_quant.py 
    sed -i -e 's/_fp16//g' compile_onnx_model_quant.py
    sed -i -e 's/FORMAT.BGR/FORMAT.RGB/g' compile_onnx_model_quant.py
    sed -i -e 's/480, 640, 3/256, 192, 3/g' compile_onnx_model_quant.py
    sed -i -e '/config.ops = \[/a\ \t    op.Crop(0, 0, 192, 256),' compile_onnx_model_quant.py
    ```

  - Run compile_onnx_model_quant.py

    ```bash
    python3 compile_onnx_model_quant.py \
    $TRANSLATOR/../onnx_models/hrnet_w32_coco_256x192_sparse90.onnx \
    -o hrnet_cam \
    -t $SDK \
    -d $TRANSLATOR \
    -c $QUANTIZER \
    -i input.1 \
    -s 1,3,256,192 \
    --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
    ```

## Setup the Execution Environment

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_yolox_hrnet_cam ; mkdir sample_yolox_hrnet_cam
cp $TVM_ROOT/obj/build_runtime/v2h/lib/* sample_yolox_hrnet_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_yolox_hrnet_cam/src/build/app_yolox_hrnet_cam sample_yolox_hrnet_cam/
cp -r $TVM_ROOT/tutorials/yolox_cam sample_yolox_hrnet_cam/
cp -r $TVM_ROOT/tutorials/hrnet_cam sample_yolox_hrnet_cam/
tar cvfz sample_yolox_hrnet.tar.gz sample_yolox_hrnet_cam/
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
  scp <yourhost>:sample_yolox_hrnet.tar.gz .
  tar xvfz sample_yolox_hrnet.tar.gz
  cd sample_yolox_hrnet_cam/
  su
  export LD_LIBRARY_PATH=.
  ./app_yolox_hrnet_cam
  exit  # After terminating the application.
  ```

  > **Note1:** For RZ/V2H and RZ/V2N AI SDK v6.00 and later, you need to switch to the root user with the `su` command when running an application.  
  This is because when you run an application from a weston-terminal, you are switched to the "weston" user, which does not have permission to run the `/dev/xxx` device used in the application.
  
  > **Note2:** The chmod +x <filename> command is necessary if the *.tar.gz file or the application file does not have execution permission.


### 3. Following window shows up on HDMI screen

<img src=./img/application_result_image_yolox_hrnet.jpg width=480>

On application window, following information is displayed.

- Camera capture
- Pose Estimation results (stick figure and pose-landmark.)  
- Processing times
  - HRNet x *: Number of people detected.
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes. \[msec\]
  - Inference: Processing time taken for AI inference. \[msec\]
  - PreProcess: Processing time taken for AI pre-processes. \[msec\]
  - PostProcess: Processing time taken for AI post-processes. \[msec\]

### 4. How Terminate Application

To terminate the application, press `Super(Window) + Tab` keys to display the Linux console terminal of RZ/V2H or RZ/V2N Evaluation Board Kit and press `Enter` key on there.

### 5. Logs

The `<timestamp>_app_yolox_hrnet_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : Megvii-Base Detection YOLOX | yolox_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : MMPose HRNet | hrnet_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : XXXX Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] YOLOX Result-------------------------------------
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 11
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (234, 522, 166, 669)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : person (Class 14)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 80.4 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 15
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (655, 610, 495, 764)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : person (Class 14)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 76.8 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Count  : 15
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Person Count        : 2
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] HRNet Result-------------------------------------
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 0: (137.53, 71.17): 64.6%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 1: (137.53, 60.39): 74.3%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 2: (137.53, 49.64): 76.5%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 3: (114.28, 38.87): 51%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 4: (110.4, 49.64): 90%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 14: (106.53, 437.19): 53.3%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 15: (91.03, 598.68): 51%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 16: (79.4, 598.68): 48%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] HRNet Result-------------------------------------
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 0: (457.16, 118.43): 80.3%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 1: (467.9, 93.93): 74.3%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 2: (446.44, 93.93): 87.8%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 3: (349.88, 69.43): 42%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 4: (414.25, 81.68): 95.3%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 14: (339.15, 547.19): 84%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 15: (178.2, 706.44): 69.8%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 16: (92.38, 510.44): 58.5%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] YOLOX
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  YOLOX Total AI Time: xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  YOLOX Inference: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  YOLOX PreProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  YOLOX PostProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] HRNet
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  HRNet Total AI Time: xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  HRNet Inference: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  HRNet PreProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  HRNet PostProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI Inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```