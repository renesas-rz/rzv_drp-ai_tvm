# How to convert FaceNet model for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [FaceNet](https://drive.google.com/open?id=1R77HmFADxe87GmoLwzfgMu_HY0IhcyBz )                                                            |facenet                        |(160, 160)      | Face Detection   |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for FaceNet model.

Note : Check the downloaded .zip file from the link in the table above is located in `${TVM_ROOT}/convert`.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/tensorflow
. ${TVM_ROOT}/convert/venvs/tensorflow/bin/activate 
pip install tensorflow==2.5 tensorflow-hub protobuf==3.13 tflite==2.1.0 scikit-learn==1.2.2 scipy==1.10.1 tf_slim==1.1.0

git clone https://github.com/davidsandberg/facenet ${TVM_ROOT}/convert/repos/facenet_tf
cd ${TVM_ROOT}/convert/repos/facenet_tf
git reset --hard "096ed770f163957c1e56efa7feeb194773920f6e"
tf_upgrade_v2 --intree ./ --outtree ./
sed s/"import tensorflow.contrib.slim"/"import tf_slim"/ ./src/models/inception_resnet_v1.py > ./src/models/inception_resnet_v1.py.mod
mv ./src/models/inception_resnet_v1.py.mod ./src/models/inception_resnet_v1.py

cd ${TVM_ROOT}/convert
unzip 20180408-102900.zip
mv 20180408-102900 FaceNet_Tensorflow
```

## 3. Download and save as TensorFlow(.tflite) file.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-n     |`Download model name` column in the table above |
---

```sh
cd ${TVM_ROOT}/convert 
python facenet_to_tflite.py -n facenet
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── facenet_tflite
           └── facenet.tflite
```

## 4. Delete the environment for FaceNet model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/tensorflow
rm -R ${TVM_ROOT}/convert/repos/facenet_tf
rm ${TVM_ROOT}/convert/FaceNet_Tensorflow 
rm ${TVM_ROOT}/convert/20180408-102900.zip 
rm -R ${TVM_ROOT}/convert/tmp
```

## 5. Next Step

To compile the model, enter the TensorFlow (.tflite) file into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_tflite_model_quant.py ../convert/output/facenet_tflite/facenet.tflite -o facenet_tflite -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 -s 1,160,160,3
```

----