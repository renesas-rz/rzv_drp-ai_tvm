# How to convert facenet_pytorch onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below will be generated according to the following procedure.

| Model name                                                                                                                                    | Description  |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|---------------|-------------------|
| InceptionResnetV1_100                                           |untrained model with 100 classes       |(1,3,160,160)     | Face recognition |
|InceptionResnetV1_1001                                             |untrained 1001-class classifier             |(1,3,160,160)     | Face recognition |
| InceptionResnetV1_casia-webface                                            | pretrained on CASIA-Webface dataset          |(1,3,160,160)     | Face recognition  |
| InceptionResnetV1_vggface2                                              |pretrained on VGGFace2 dataset           |(1,3,160,160)     | Face recognition  |

---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
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

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For InceptionResnetV1_100, as the following.

```sh
python3 compile_onnx_model_quant.py ../convert/InceptionResnetV1_100.onnx  -o InceptionResnetV1_100_onnx -t $SDK -d $TRANSLATOR -c $QUANTIZER  

```

----