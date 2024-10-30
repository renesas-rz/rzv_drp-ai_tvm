# How to convert Torchvision models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                       | Download model name  | Input Shape    | Task              |
|------------------------------------------------------------------------------------------------|----------------------|----------------|-------------------|
| ResNet18                                                                                       | resnet18             | (1,3,224,224)  | classification    |
| ResNet34                                                                                       | resnet34             | (1,3,224,224)  | classification    |
| ResNet50                                                                                       | resnet50             | (1,3,224,224)  | classification    |
| ResNet101                                                                                      | resnet101            | (1,3,224,224)  | classification    |
|MobileNetV2                                                                                     | mobilenet_v2         | (1,3,224,224)  | classification    |
| SqueezeNet1_1                                                                                  | squeezenet1_1        | (1,3,224,224)  | classification    |
| DenseNet-121                                                                                   | densenet121          | (1,3,224,224)  | classification    |
| DenseNet-161                                                                                   | densenet161          | (1,3,224,224)  | classification    |
| GoogleNet                                                                                      | googlenet            | (1,3,224,224)  | classification    |
| MnasNet0_5                                                                                     | mnasnet0_5           | (1,3,224,224)  | classification    |
| DeepLabv3-resnet50                                                                             | deeplabv3_resnet50   | (1,3,224,224)  | segmentation      |
| DeepLabv3-resnet101                                                                            | deeplabv3_resnet101  | (1,3,224,224)  | segmentation      |
| FCN_resnet101                                                                                  | fcn_resnet101        | (1,3,224,224)  | segmentation      |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```  

## 2. Create an environment for Torchvision models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/pytorch
. ${TVM_ROOT}/convert/venvs/pytorch/bin/activate

pip install torch==2.1.2 torchvision==0.16.2 \
    pytz opencv-python gitpython pandas requests pyyaml \
    tqdm matplotlib seaborn psutil ipython scipy smplx \
    numpy==1.23.5
```

## 3. Download and save as TorchScript (.pt) files.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-t     |`Task` column in the table above                |
|-s     |`Input shape` column in the table above         |
|-n     |`Download model name` column in the table above |
---

```sh
cd $TVM_ROOT/convert/ 
# The following is an example for DeepLabv3-resnet50
python download_torchvision.py \
    -t segmentation \
    -s 1,3,224,224 \
    -n deeplabv3_resnet50
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── deeplabv3_resnet50_torchvision_pytorch
           └── deeplabv3_resnet50.pt
```

## 4. Delete the environment for Torchvision models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/pytorch
```

## 5. Next Step

To compile the models, enter the TorchScript (.pt) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For DeepLabv3-resnet50, as the following.

```sh
python3 compile_pytorch_model.py ../convert/output/deeplabv3_resnet50_torchvision_pytorch/deeplabv3_resnet50.pt -o deeplabv3_resnet50_torchvision -s 1,3,224,224
```

----