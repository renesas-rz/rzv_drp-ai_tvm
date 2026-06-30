# How to convert facenet_pytorch onnx models
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below will be generated according to the following procedure.

| Model name                                                                                                                                    | Description  |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| InceptionResnetV1_100                                           |untrained model with 100 classes       |(1,3,160,160)     | Face recognition |
|InceptionResnetV1_1001                                             |untrained 1001-class classifier             |(1,3,160,160)     | Face recognition |
| InceptionResnetV1_casia-webface                                            | pretrained on CASIA-Webface dataset          |(1,3,160,160)     | Face recognition  |
| InceptionResnetV1_vggface2                                              |pretrained on VGGFace2 dataset           |(1,3,160,160)     | Face recognition  |

---

License: [MIT License](https://github.com/timesler/facenet-pytorch/tree/master)

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for facenet_pytorch onnx models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/facenet_pytorch
. ${TVM_ROOT}/convert/venvs/facenet_pytorch/bin/activate

pip install facenet-pytorch onnx==1.16.0
```

## 3. Generate the onnx files.



```sh
cd ${TVM_ROOT}/convert/

#onnx files will be generated in the same folder
python3 download_facenet_pytorch.py  

```

After the above command is executed, the file structure will be as follows.

```sh
${TVM_ROOT}/convert
 └── download_facenet_pytorch.py
 └── InceptionResnetV1_100.onnx
 └── InceptionResnetV1_1001.onnx
 └── InceptionResnetV1_casia-webface.onnx
 └── InceptionResnetV1_vggface2.onnx
```

## 4. Delete the environment for facenet_pytorch onnx models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/facenet_pytorch
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