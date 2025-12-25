# Semantic Image Segmentation: DeepLabv3

## Build the application

1. Please refer to [Application Example for RZ/V2H and RZ/V2N](./../../../apps/build_appV2H.md#how-to-build-the-application).  An example of command execution is shown below.

    ```bash
    cd $TVM_ROOT/how-to/sample_app_v2h/app_deeplabv3_cam/src
    mkdir build
    cd build

    cmake -DCMAKE_TOOLCHAIN_FILE=$TVM_ROOT/apps/toolchain/runtime.cmake -DAPP_NAME=app_deeplabv3_cam ..
    sed -i -e 's/INPUT_CAM_TYPE 0/INPUT_CAM_TYPE 1/g' ../define.h # Not executed when using a USB camera.
    make
    ```

2. The `app_deeplabv3_cam` application binary is generated.

## AI models

This sample only uses [deeplabv3](https://pytorch.org/hub/pytorch_vision_deeplabv3_resnet101/).

### Edit the script to compile DeepLabv3

```bash
cd $TVM_ROOT/tutorials
git checkout compile_onnx_model_quant.py
sed -i -e '/# Input shape helper/,/return \[batch_dim\] + \[int(d.get("dimValue")) for d in dim_info\[1::\]\]/d' compile_onnx_model_quant.py
sed -i -e 's/get_input_shape(model_file, inp)/opts["input_shape"]/g' compile_onnx_model_quant.py
sed -i -e 's/256,/(513, 513),/g' compile_onnx_model_quant.py
sed -i -e 's/, 224/, 513/g' compile_onnx_model_quant.py
sed -i -e'/ref_result_output_dir =/,/exist_ok=True)/d' compile_onnx_model_quant.py
sed -i -e'/flatten().tofile(/d' compile_onnx_model_quant.py
sed -i -e '/os.path.join(ref_result_output_dir, "input_"/d' compile_onnx_model_quant.py
sed -i -e 's/ref_result_output_dir/output_dir/g' compile_onnx_model_quant.py
sed -i -e 's/_fp32//g' compile_onnx_model_quant.py
sed -i -e 's/_fp16//g' compile_onnx_model_quant.py
sed -i -e 's/_int64//g' compile_onnx_model_quant.py
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
$TRANSLATOR/../onnx_models/DeepLabV3_sparse90.onnx \
 -o deeplabv3_cam \
 -t $SDK \
 -d $TRANSLATOR \
 -c $QUANTIZER \
 -i input1 \
 -s 1,3,513,513 \
 --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/
```

## Setup the Execution Environment

### 1. Copy and archive files

```bash
cd $TVM_ROOT/../
rm -r sample_deeplabv3_cam  ; mkdir sample_deeplabv3_cam
cp $TVM_ROOT/obj/build_runtime/v2h/lib/* sample_deeplabv3_cam/
cp $TVM_ROOT/how-to/sample_app_v2h/app_deeplabv3_cam/src/build/app_deeplabv3_cam sample_deeplabv3_cam/
cp -r $TVM_ROOT/tutorials/deeplabv3_cam sample_deeplabv3_cam/
tar cvfz sample_deeplabv3.tar.gz sample_deeplabv3_cam/
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
scp <yourhost>:sample_deeplabv3.tar.gz .
tar xvfz sample_deeplabv3.tar.gz 
cd sample_deeplabv3_cam/
su
export LD_LIBRARY_PATH=.
./app_deeplabv3_cam
exit # After terminating the application.
```

  > **Note1:** For RZ/V2H and RZ/V2N AI SDK v6.00 and later, you need to switch to the root user with the `su` command when running an application.  
  This is because when you run an application from a weston-terminal, you are switched to the "weston" user, which does not have permission to run the `/dev/xxx` device used in the application.
  
  > **Note2:** The chmod +x <filename> command is necessary if the *.tar.gz file or the application file does not have execution permission.

### 3. Following window shows up on HDMI screen

<img src=./img/application_result_on_hdmi_deeplabv3.png width=480>

On application window, following information is displayed.

- Camera capture
- Segmentation result (Objects segmentation and detected class names.)  
- Processing times
  - Total AI Time: Processing time taken for AI inference and its pre/post-processes. \[msec\]
  - Inference: Processing time taken for AI inference. \[msec\]
  - PreProcess: Processing time taken for AI pre-processes. \[msec\]
  - PostProcess: Processing time taken for AI post-processes. \[msec\]

### 4. How Terminate Application

To terminate the application, press `Super(Window) + Tab` keys to display the Linux console terminal of RZ/V2H or RZ/V2N Evaluation Board Kit and press `Enter` key on there.

### 5. Logs

The `<timestamp>_app_deeplabv3_cam.log` file is to be generated under the `logs` folder and is to be recorded the text logs of AI inference results and AI processing time and rate. 

```txt
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   RZ/V2H DRP-AI Sample Application
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Model : PyTorch DeepLabv3 | deeplabv3_cam
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]   Input : XXXX Camera
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] ************************************************
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 1
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 15 : person (RGB:0x7cfc00)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info]  Detected Class 17 : sheep (RGB:0xf000ff)
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Total AI Time: xxx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference: xxx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PreProcess: x.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] PostProcess: xxx.x [ms]
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] [START] Start DRP-AI inference...
[XXXX-XX-XX XX:XX:XX.XXX] [logger] [info] Inference ----------- No. 2
```
