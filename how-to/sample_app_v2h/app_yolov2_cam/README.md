# Object Detection: YOLOv2

## Build the application

1. Please refer to [Application Example for RZ/V2H and RZ/V2N](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_yolov2_cam/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `app_yolov2_cam` application binary is generated.

## AI models

This sample only uses [yolov2](https://gitlab.com/EAVISE/lightnet/-/tree/v1.1.1?ref_type=tags).

### Edit the script to compile YOLOv2

```bash
cd $TVM_ROOT/tutorials
git checkout compile_onnx_model_quant.py
sed -i -e '/# Input shape helper/,/return \[batch_dim\] + \[int(d.get("dimValue")) for d in dim_info\[1::\]\]/d' compile_onnx_model_quant.py
sed -i -e 's/get_input_shape(model_file, inp)/opts["input_shape"]/g' compile_onnx_model_quant.py
sed -i -e 's/0.485, 0.456, 0.406/0.0, 0.0, 0.0/g' compile_onnx_model_quant.py
sed -i -e 's/0.229, 0.224, 0.225/1.0, 1.0, 1.0/g' compile_onnx_model_quant.py
sed -i -e 's/256/416/g' compile_onnx_model_quant.py
sed -i -e 's/, 224/, 416/g' compile_onnx_model_quant.py
sed -i -e '/ref_result_output_dir =/,/exist_ok=True)/d' compile_onnx_model_quant.py
sed -i -e '/flatten().tofile(/d' compile_onnx_model_quant.py
sed -i -e '/os.path.join(ref_result_output_dir, "input_"/d' compile_onnx_model_quant.py
sed -i -e 's/ref_result_output_dir/output_dir/g' compile_onnx_model_quant.py
sed -i -e 's/_fp32//g' compile_onnx_model_quant.py 
sed -i -e 's/_fp16//g' compile_onnx_model_quant.py
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/1080, 1920, 2/g' compile_onnx_model_quant.py
```

### Additional edit for USB Camera

If you are using a USB camera, execute the following additional commands.

```bash
sed -i -e 's/1080, 1920, 2/480, 640, 2/g' compile_onnx_model_quant.py
```

### Run compile_onnx_model_quant.py

```bash
python3 compile_onnx_model_quant.py \
$TRANSLATOR/../onnx_models/YoloV2_sparse90.onnx \
 -o yolov2_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -i input1 \
 -s 1,3,416,416 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

## Setup the Execution Environment  

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_yolov2_cam ; mkdir sample_yolov2_cam
cp $TVM_ROOT/obj/build_runtime/$PRODUCT/libtvm_runtime.so sample_yolov2_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_yolov2_cam/src/build/app_yolov2_cam sample_yolov2_cam
cp -r $TVM_ROOT/tutorials/yolov2_cam sample_yolov2_cam/
tar cvfz sample_yolov2.tar.gz sample_yolov2_cam/
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
      <img src=./img/hw_conf_v2h.png width=700>
- Display: Please connect to the HDMI port on the EVK board

### 2. **(On RZ/V Board)** Copy and Try it  

For example, as follows.

```sh
scp <yourhost>:sample_yolov2.tar.gz .
tar xvfz sample_yolov2.tar.gz 
cd sample_yolov2_cam/
export LD_LIBRARY_PATH=.
./app_yolov2_cam
```

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_yolov2.png width=480>

On application window, following information is displayed.

- Camera capture
- Object Detection result (Bounding boxes, class name and score.)  
- Processing times
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes. [msec]
  - Inference: Processing time taken for AI inference. [msec]
  - PreProcess: Processing time taken for AI pre-processes. [msec]
  - PostProcess: Processing time taken for AI post-processes. [msec]

### 4. How Terminate Application

To terminate the application, press `Enter` key on the Linux console terminal of RZ/V2H Evaluation Board Kit.

### 5. Logs

The `<timestamp>_app_yolov2_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate.

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : Lightnet YOLOv2 | yolov2_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : XXXX Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (524, 64, 247, 60)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : car (Class 6)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 70.8 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 2
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (172, 272, 249, 299)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : dog (Class 11)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 60.2 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Number : 4
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box        : (X, Y, W, H) = (308, 206, 553, 318)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class      : bicycle (Class 1)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Probability         : 85.5 %
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Bounding Box Count  : 3
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Total AI Time: xx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PreProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PostProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```
