# ONNX graph tune guide   
:information_source: **[NOTE]** This feature is currently in beta version.

## What is ONNX graph tune
This feature tunes the ONNX graph to increase execution efficiency on DRP-AI. The tuned ONNX model produces inference results equivalent to the original ONNX model. (Small numerical differences may occur because of floating-point computation order.) The effect is not guaranteed for all ONNX models.   
This transformation is recommended when the ONNX graph satisfies the conditions outlined below.

| Target <br>Operator | Conditions | ONNX graph after tuning |
|:--- | :--- | :--- |
| ConvTranspose | kernel_shape=[k, k] <br>strides=[s,s] <br> dilation=1, <br>group=1. <br>Support for 2 conditions below <br>**CASE1**: k==s & pads=[0,0,0,0]  <br>&emsp;&emsp;&emsp;&emsp;& output_padding=[0,0,0,0]<br>**CASE2**: k!=s & pads=[k//2,k//2,k//2,k//2] <br>&emsp;&emsp;&emsp;&emsp;& output_padding=[s-1,s-1,s-1,s-1]| Conv2D & DepthToSpace |
| Conv(3D) | auto_pad=default(NOTSET) <br> dilation=1, <br>group=1.<br>Support for 3 conditions below <br>**Case1**: strides=[1, 1, 1] or [1, 2, 2], <br>&emsp;&emsp;&emsp;&emsp;&kernel_shape = [any, any, any], <br>&emsp;&emsp;&emsp;&emsp;&pads = [any, any, any, any, any, any] <br>**Case2**: strides=[2, 2, 2] or [2, 1, 1], <br>&emsp;&emsp;&emsp;&emsp;&kernel_shape = [1, any, any], <br>&emsp;&emsp;&emsp;&emsp;&pads = [0, any, any, 0, any, any] <br>**Case3**: strides=[2, 2, 2] or [2, 1, 1], <br>&emsp;&emsp;&emsp;&emsp;&kernel_shape = [3, any, any], <br>&emsp;&emsp;&emsp;&emsp;&pads = [1, any, any, 1, any, any] | Pad & Transpose & Reshape & Conv(2D) & Reshape <br><br>:warning: Note: Parameter size in ONNX file will increase after tuning. |   

:information_source: [**NOTE**] 
- If performance improvement on DRP-AI is not expected, the ONNX graph may not be modified even if the above conditions are satisfied.   
- Dynamic Shape is not supported.

## How to use ONNX tune
First, install the onnx-tune by following the steps below.   
```sh
$ pip3 install ./onnx_graph_tune-*.whl
```

Then, create and run a Python script like the example below to convert the prepared ONNX file.

```sh
$ python3 example.py input_model.onnx --out_onnx output_model.onnx
```
After successful tuning, compile the generated ONNX model using RUHMI.
Please refer to the tutorial for detailed instructions.   
- [Compilation tutorials](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

----