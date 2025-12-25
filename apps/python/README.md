# How to install python API and run sample

[Note] This release version(V2.7.0) supports for V2H/V2N only. In addition, only the runtime model compiled in mera2 mode are supported.

## Install python API
To run pyhon API, python3-pip and numpy packages are requried. Please refer the [appendix](#appendix).   

### Copy python directory to V2H board
Please copy this "python" directory to V2H(V2N) board including following python script and *.whl files.
```
python/
├── inference.py
├── mera-*-cp38-cp38-linux_aarch64.whl
├── mera2_runtime-*-cp38-cp38-linux_aarch64.whl
└── tvm-*-cp38-cp38-linux_aarch64.whl
```   

### Install python libraries   

Go to "python" directory in V2H(V2N) board, and execute python3-pip to install libraries.
```
root@rzv2h-evk:~# python3 -m pip install *.whl
```

## Run sample python script
Please compile ONNX Resnet model to run the sample python script. For details, refer to the [tutorials](../../tutorials/tutorial_RZV2H.md). After successful compilation, copy the output directory(e.g. resnet18_onnx) to V2H(V2N) board, and run it on the board like below.
```
root@rzv2h-evk:~# python3 inference.py --model_path resnet18_onnx
Load arguments
  Runtime model data path: ./resnet18_onnx/
  Start address: 0x240000000 (decimal: 9663676416)
Load input data
  Generate input data as random data(input_0_random.bin)
  Input shape : (1, 3, 224, 224), Dtype : float32
Run inference
  Loop time : 10
  Average latency: 2.284 ms
Save output data
  Saved output result : ./resnet18_onnx/output_0_fp32.bin
```

In this sample program, the inference results are saved as a binary file(e.g. "output_0_fp32.bin"). You can verify the  inference results. 

The default input size is set to 1,3,224,224. The input shape can be changed using the "--input_shape" argument. It is also possible to specify the input data saved in binary format as an argument("--input_bin_file"). If you want to change the input setting, execute the command as shown below. 
```
root@rzv2h-evk:~# python3 inference.py \
  --model_path ./yolov_sample \
  --input_shape 1,3,640,640 \
  --input_bin_file ./input_640.bin
```
The input binary file specified using --input_bin_file option is expected to contain preprocessed data in this sample program. Please  run the preprocessing in advance or modify this sample code to include the preprocessing step.   

Please customize this sample program as needed to verify the inference results.

## Appendix   

### 1. Verifying whether python3-pip and python3-numpy are installed on AI SDK   

To verify whether python3-pip and python3-numpy are installed on AI SDK, boot the device using the SD image and execute the following command:
```
root@rzv2h-evk:prpm -qa | grep python3-pip
```   
If python3-pip is installed, the version will be displayed as follows:   
```
root@rzv2h-evk:~# rpm -qa | grep python3-pip
python3-pip-24.0-r0.cortexa55
```   

If python3-pip is not installed, nothing will be displayed. The same method can be used to verify python3-numpy.   

### 2. Installing python3-pip and python3-numpy on AI SDK
To install python3-pip and python3-numpy on AI SDK, you need to build AI SDK. Follow the guide on the below page to obtain the AI SDK Source Code and execute the build procedure:    

RZ/V2H: https://renesas-rz.github.io/rzv_ai_sdk/latest/howto_build_aisdk_v2h.html    

RZ/V2N: https://renesas-rz.github.io/rzv_ai_sdk/latest/howto_build_aisdk_v2n.html   

Before executing the bitbake command to build Linux, add the following line to ${YOCTO_WORK}/build/conf/local.conf:   
 - "For AI SDK v6.00 or later (Scarthgap):
```
IMAGE_INSTALL:append = "python3-pip python3-numpy"
```

To install multiple modules, you can specify them as follows:
```
IMAGE_INSTALL:append = "python3-<module_name1> python3-<module_name2> ..."
```

