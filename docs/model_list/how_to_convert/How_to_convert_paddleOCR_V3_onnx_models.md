# How to convert PPOCR models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model | Download model name|Input shape    | Task              |   
|----------|--------------------|---------------|-------------------|   
| Paddle OCR V3 det | en_PP-OCRv3_det                                                           |(1, 3, 640, 640)     | Character Detection    |
| Paddle OCR V3 rec | en_PP-OCRv3_rec                                                           |(1, 3, 48, 480)     | Character Recognition    |
---

License: [Apache 2.0](https://github.com/PaddlePaddle/PaddleOCR/blob/main/LICENSE)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for PPOCR onnx models.
See this [Paddle2ONNX model transformation](https://github.com/PaddlePaddle/PaddleOCR/blob/main/deploy/paddle2onnx/readme.md) guide for the detail. 
```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/ppocr
. ${TVM_ROOT}/convert/venvs/ppocr/bin/activate
git clone -b main https://github.com/PaddlePaddle/PaddleOCR.git ${TVM_ROOT}/convert/repos/PaddleOCR
cd ${TVM_ROOT}/convert/repos/PaddleOCR
python3 pip install -e .
python3 -m pip install paddle2onnx==2.1.0 packaging paddlepaddle==3.2.1 onnxruntime==1.23.2
```

## 3. Convert PaddlePaddle files to ONNX (.onnx) files.


```sh
cd ${TVM_ROOT}/convert/repos/PaddleOCR

#download paddleocr files
wget -nc -P ./inference https://paddleocr.bj.bcebos.com/PP-OCRv3/english/en_PP-OCRv3_det_infer.tar
cd ./inference && tar xf en_PP-OCRv3_det_infer.tar 
cd ..
wget -nc  -P ./inference https://paddleocr.bj.bcebos.com/PP-OCRv3/english/en_PP-OCRv3_rec_infer.tar
cd ./inference && tar xf en_PP-OCRv3_rec_infer.tar
cd ..

#Convert to onnx
paddle2onnx --model_dir ./inference/en_PP-OCRv3_det_infer \
--model_filename inference.pdmodel \
--params_filename inference.pdiparams \
--save_file ./inference/det_onnx/model_det.onnx \
--opset_version 11 \
--enable_onnx_checker True
paddle2onnx --model_dir ./inference/en_PP-OCRv3_rec_infer \
--model_filename inference.pdmodel \
--params_filename inference.pdiparams \
--save_file ./inference/rec_onnx/model_rec.onnx \
--opset_version 11 \
--enable_onnx_checker True

#Simplify onnx and make the input static
mkdir -p ${TVM_ROOT}/convert/output/PaddleOCR
onnxsim inference/det_onnx/model_det.onnx ${TVM_ROOT}/convert/output/PaddleOCR/ocr_v3_det_simpl_640x640.onnx --overwrite-input-shape 1,3,640,640
onnxsim inference/rec_onnx/model_rec.onnx ${TVM_ROOT}/convert/output/PaddleOCR/ocr_v3_rec_simpl_48x480.onnx --overwrite-input-shape 1,3,48,480
```



After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── output
      └── PaddleOCR
           └── ocr_v3_det_simpl_640x640.onnx
           └── ocr_v3_rec_simpl_48x480.onnx
           
```

## 4. Delete the environment for yolov8_onnx models.

```sh
deactivate
cd ${TVM_ROOT}
rm -R ${TVM_ROOT}/convert/venvs/ppocr
rm -R ${TVM_ROOT}/convert/repos/PaddleOCR
```

## 5. Next Step

To compile the model, enter the ONNX (.onnx) files into the compilation script in the tutorials.
Please refer below guides.

Basic guide
 - [Compilation tutorial](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/main/tutorials)

Other guides
 - [How to compile model: Section 2](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_sample_model.html)
 - [How to compile your own model: Section 1, 2, 3](https://renesas-rz.github.io/rzv_drp-ai_tvm/compile_your_own_model.html)

----
