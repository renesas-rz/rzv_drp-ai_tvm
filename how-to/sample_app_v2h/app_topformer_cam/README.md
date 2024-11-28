# Semantic Image Segmentation: TopFormer

## Build the application

1. Please refer to [Application Example for V2H](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_topformer_cam/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `app_topformer_cam` application binary is generated.

## AI models

This sample only uses [topformer](https://github.com/hustvl/TopFormer?tab=readme-ov-file).
<!---
[The ready-to-use onnx file is here.](./deeplabv3_513x513.onnx)
githubにはリポジトリ総量制限(1G/5G)や単体ファイルサイズ制限(100M)があり、余裕がないのでユーザ自身で生成できるonnxは置かないのが望ましい
--->

```bash
cd $TVM_ROOT/tutorials
# Download onnx model from github
wget https://github.com/renesas-rz/rzv_drp-ai_tvm/releases/download/v2.4.1/topformer_tiny_512x512.onnx
git checkout compile_onnx_model_quant.py
sed -i -e 's/256,/(512, 512),/g' compile_onnx_model_quant.py
sed -i -e 's/, 224/, (512, 512)/g' compile_onnx_model_quant.py
sed -i -e 's/480, 640, 3/480, 640, 2/g' compile_onnx_model_quant.py
sed -i -e 's/FORMAT.BGR/FORMAT.YUYV_422/g' compile_onnx_model_quant.py

python3 compile_onnx_model_quant.py \
 topformer_tiny_512x512.onnx \
 -o topformer_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -i input1 \
 -s 1,3,512,512 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ \
 -v 100
```

## Setup the Execution Environment

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_topformer_cam  ; mkdir sample_topformer_cam
cp $TVM_ROOT/obj/build_runtime/V2H/libtvm_runtime.so sample_topformer_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_topformer_cam/src/build/app_topformer_cam sample_topformer_cam/
cp -r $TVM_ROOT/tutorials/topformer_cam sample_topformer_cam/
tar cvfz sample_topformer.tar.gz sample_topformer_cam/
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
scp <yourhost>:sample_topformer.tar.gz .
tar xvfz sample_topformer.tar.gz 
cd sample_topformer_cam/
export LD_LIBRARY_PATH=.
./app_topformer_cam
```

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_topformer.jpg width=640>

On application window, following information is displayed.

- Camera capture
- Segmentation result (Objects segmentation and detected class names.)  
- Processing times
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes.[msec]
  - Inference: Processing time taken for AI inference.[msec]
  - PreProcess: Processing time taken for AI pre-processes.[msec]
  - PostProcess: Processing time taken for AI post-processes.[msec]

### 4. How Terminate Application

To terminate the application, press `Enter` key on the Linux console terminal of RZ/V2H Evaluation Board Kit.

### 5. Logs

The `<timestamp>_app_topformer_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate. 

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : TopFormer | topformer_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : MIPI Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 3 : floor (RGB:0x050432)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 5 : ceiling (RGB:0x788c50)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 12 : person (RGB:0x96783d)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 14 : door (RGB:0x08ff33)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Total AI Time: xxx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference: xxx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PreProcess: x.xx [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PostProcess: xxx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] AI Frame Rate x [fps]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```

## Note   

### Limit the number of CPU thread for AI model inference
In this topformer sample application, the number of CPU threads used for AI model inference is set to 2 to maximize the performance of the demo application including camera input and image output. You can change the maximum number of CPU threads from 1 to 4. A below command is a sample to set the number of thread to 2.

```
export TVM_NUM_THREADS=2
```

### Pseudo error message in compliation   
```
...
Error: drpai_quantize.py failed
However, the output file exists! The quantization process continued.
...
```
Above error message is shown during compile process at several time.
However, above error does not affect the result of compilation for topformer model. Please ignore it.
