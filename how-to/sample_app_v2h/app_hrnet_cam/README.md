# Object Detection: HRNet

## Build the application

1. Please refer to [Application Example for V2H](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

   ```bash
   cd $TVM_ROOT/how-to/sample_app_v2h/app_hrnet_cam/src
   mkdir build
   cd build
   
   cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
   sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
   make
   ```

2. The `app_hrnet_cam` application binary is generated.

## AI models

This sample only uses [hrnet](https://github.com/open-mmlab/mmpose/tree/v1.1.0).

```bash
cd $TVM_ROOT/tutorials
git checkout compile_onnx_model_quant.py
sed -i -e '/# Input shape helper/,/return \[batch_dim\] + \[int(d.get("dimValue")) for d in dim_info\[1::\]\]/d' compile_onnx_model_quant.py
sed -i -e 's/get_input_shape(model_file, inp)/opts["input_shape"]/g' compile_onnx_model_quant.py
sed -i -e 's/, 224/, (256,192)/g' compile_onnx_model_quant.py
sed -i -e'/ref_result_output_dir =/,/exist_ok=True)/d' compile_onnx_model_quant.py
sed -i -e'/flatten().tofile(/d' compile_onnx_model_quant.py
sed -i -e '/os.path.join(ref_result_output_dir, "input_"/d' compile_onnx_model_quant.py
sed -i -e 's/ref_result_output_dir/output_dir/g' compile_onnx_model_quant.py
sed -i -e 's/_fp32//g' compile_onnx_model_quant.py 
sed -i -e 's/_fp16//g' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/1080, 1920, 2/g' compile_onnx_model_quant.py
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py
sed -i -e'/config.ops = \[/a\ \t    op.Crop(555, 0, 810, 1080),' compile_onnx_model_quant.py

python3 compile_onnx_model_quant.py \
$TRANSLATOR/../onnx_models/hrnet_w32_coco_256x192_sparse90.onnx \
 -o hrnet_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -i input.1 \
 -s 1,3,256,192 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ \
 -v 100 
```

## Setup the Execution Environment

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_hrnet_cam ; mkdir sample_hrnet_cam
cp $TVM_ROOT/obj/build_runtime/V2H/libtvm_runtime.so sample_hrnet_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_hrnet_cam/src/build/app_hrnet_cam sample_hrnet_cam/
cp -r $TVM_ROOT/tutorials/hrnet_cam sample_hrnet_cam/
tar cvfz sample_hrnet.tar.gz sample_hrnet_cam/
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
scp <yourhost>:sample_hrnet.tar.gz .
tar xvfz sample_hrnet.tar.gz 
cd sample_hrnet_cam/
export LD_LIBRARY_PATH=.
./app_hrnet_cam
```

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_image_hrnet.jpg width=480>

On application window, following information is displayed.

- Camera capture
- Pose Estimation results (stick figure and pose-landmark.)  
- Processing times
  - AI/Camera Frame Rate: The number of AI inferences per second and the number of Camera captures per second. [fps]
  - HRNet
    - HRNet x *: Number of people detected.
    - Total Pre-Proc + Inference Time (DRP-AI): Total processing time taken for AI inference and its pre/post-processes on DRP-AI. [msec]
    - Total Post-Proc Time (CPU): Total processing time taken for post-processes of AI inference on CPU. [msec]

### 4. How Terminate Application

To terminate the application, press `Enter` key on the Linux console terminal of RZ/V2H Evaluation Board Kit.

### 5. Logs

The `<timestamp>_app_hrnet_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : MMPose HRNet | hrnet_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : MIPI Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] HRNet Result-------------------------------------
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 0: (104.47, 102.12): 88.62%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 1: (110.09, 94.62): 85.55%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 2: (98.84, 87.12): 86.96%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 3: (126.97, 94.62): 84.18%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 4: (98.84, 79.62): 84.28%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   . . .
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 14: (53.84, 357.12): 84.08%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 15: (70.72, 447.12): 85.94%
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   ID 16: (25.72, 454.62): 81.45%b
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] HRNet
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Total Pre-Proc + Inference Time (DRP-AI): xx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Total Post-Proc Time (CPU): X.X [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] AI Frame Rate: XX [fps]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```
