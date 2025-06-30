# How to convert timm models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                                                  | Download model name             |Input shape    | Task              |
|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| ConvNeXt atto                                                                                                                                                             |convnext_atto.d2_in1k            |(1,3,224,224)  | Classification    |
| ConvNeXt femto                                                                                                                                                            |convnext_femto.d1_in1k           |(1,3,224,224)  | Classification    |
| ConvNeXt femto ols                                                                                                                                                        |convnext_femto_ols.d1_in1k       |(1,3,224,224)  | Classification    |
| CSP-Darknet                                                                                                                                                               |cspdarknet53                     |(1,3,256,256)  | Classification    |
| Darknet-53                                                                                                                                                                |darknet53                        |(1,3,228,228)  | Classification    |
| Darknet-aa53                                                                                                                                                              |darknetaa53                      |(1,3,228,228)  | Classification    |
| DenseNet121                                                                                                                                                               |densenet121                      |(1,3,224,224)  | Classification    |
| DenseNet161                                                                                                                                                               |densenet161                      |(1,3,224,224)  | Classification    |
| DenseNet169                                                                                                                                                               |densenet169                      |(1,3,224,224)  | Classification    |
| DenseNet201                                                                                                                                                               |densenet201                      |(1,3,224,224)  | Classification    |
| DenseNet Blur 121d                                                                                                                                                        |densenetblur121d                 |(1,3,224,224)  | Classification    |
| DPN68                                                                                                                                                                     |dpn68                            |(1,3,224,224)  | Classification    |
| DPN68b                                                                                                                                                                    |dpn68b                           |(1,3,224,224)  | Classification    |
| EfficientNet Edge Large                                                                                                                                                   |efficientnet_el                  |(1,3,300,300)  | Classification    |
| pruned EfficientNet Edge Large                                                                                                                                            |efficientnet_el_pruned           |(1,3,240,240)  | Classification    |
| EfficientNet Edge Medium                                                                                                                                                  |efficientnet_em                  |(1,3,300,300)  | Classification    |
| EfficientNet Edge Small                                                                                                                                                   |efficientnet_es                  |(1,3,224,224)  | Classification    |
| pruned EfficientNet Edge Small                                                                                                                                            |efficientnet_es_pruned           |(1,3,224,224)  | Classification    |
| EfficientNet Lite0                                                                                                                                                        |efficientnet_lite0               |(1,3,224,224)  | Classification    |
| Ensemble Adversarial Inception ResNet v2                                                                                                                                  |ens_adv_inception_resnet_v2      |(1,3,299,299)  | Classification    |
| ESE-VoVNet 19-dw                                                                                                                                                          |ese_vovnet19b_dw                 |(1,3,224,224)  | Classification    |
| ESE-VoVNet 39b                                                                                                                                                            |ese_vovnet39b                    |(1,3,224,224)  | Classification    |
| FBNet-C                                                                                                                                                                   |fbnetc_100                       |(1,3,224,224)  | Classification    |
| GPU-Efficient ResNet Large (gernet_l)                                                                                                                                     |gernet_l                         |(1,3,256,256)  | Classification    |
| GPU-Efficient ResNet Middle (gernet_m)                                                                                                                                    |gernet_m                         |(1,3,224,224)  | Classification    |
| GPU-Efficient ResNet Small (gernet_s)                                                                                                                                     |gernet_s                         |(1,3,224,224)  | Classification    |
| (Gluon) ResNet101 v1b                                                                                                                                                     |gluon_resnet101_v1b              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet101 v1c                                                                                                                                                     |gluon_resnet101_v1c              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet101 v1d                                                                                                                                                     |gluon_resnet101_v1d              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet101 v1s                                                                                                                                                     |gluon_resnet101_v1s              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet152 v1b                                                                                                                                                     |gluon_resnet152_v1b              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet152 v1c                                                                                                                                                     |gluon_resnet152_v1c              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet152 v1d                                                                                                                                                     |gluon_resnet152_v1d              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet152 v1s                                                                                                                                                     |gluon_resnet152_v1s              |(1,3,224,224)  | Classification    |
| (Gluon) ResNet18 v1b                                                                                                                                                      |gluon_resnet18_v1b               |(1,3,224,224)  | Classification    |
| (Gluon) ResNet34 v1b                                                                                                                                                      |gluon_resnet34_v1b               |(1,3,224,224)  | Classification    |
| (Gluon) ResNet50 v1b                                                                                                                                                      |gluon_resnet50_v1b               |(1,3,224,224)  | Classification    |
| (Gluon) ResNet50 v1c                                                                                                                                                      |gluon_resnet50_v1c               |(1,3,224,224)  | Classification    |
| (Gluon) ResNet50 v1d                                                                                                                                                      |gluon_resnet50_v1d               |(1,3,224,224)  | Classification    |
| (Gluon) ResNet50 v1s                                                                                                                                                      |gluon_resnet50_v1s               |(1,3,224,224)  | Classification    |
| HRNet w18                                                                                                                                                                 |hrnet_w18                        |(1,3,224,224)  | Classification    |
| HRNet w18small                                                                                                                                                            |hrnet_w18_small                  |(1,3,224,224)  | Classification    |
| HRNet w18small V2                                                                                                                                                         |hrnet_w18_small_v2               |(1,3,224,224)  | Classification    |
| HRNet w30                                                                                                                                                                 |hrnet_w30                        |(1,3,224,224)  | Classification    |
| HRNet w32                                                                                                                                                                 |hrnet_w32                        |(1,3,224,224)  | Classification    |
| HRNet w40                                                                                                                                                                 |hrnet_w40                        |(1,3,224,224)  | Classification    |
| HRNet w44                                                                                                                                                                 |hrnet_w44                        |(1,3,224,224)  | Classification    |
| HRNet w48                                                                                                                                                                 |hrnet_w48                        |(1,3,224,224)  | Classification    |
| PP-LCNet-0.5x                                                                                                                                                             |lcnet_050                        |(1,3,224,224)  | Classification    |
| PP-LCNet-0.75x                                                                                                                                                            |lcnet_075                        |(1,3,224,224)  | Classification    |
| PP-LCNet-1x                                                                                                                                                               |lcnet_100                        |(1,3,224,224)  | Classification    |
| MnasNet-B1 depth multiplier 1.0                                                                                                                                           |mnasnet_100                      |(1,3,224,224)  | Classification    |
| MobileNet V2 with channel multiplier of 0.5                                                                                                                               |mobilenetv2_050                  |(1,3,224,224)  | Classification    |
| MobileNet V2 with channel multiplier of 1.0                                                                                                                               |mobilenetv2_100                  |(1,3,224,224)  | Classification    |
| MobileNet V2 with channel multiplier of 1.1                                                                                                                               |mobilenetv2_110d                 |(1,3,224,224)  | Classification    |
| MobileNet V2 with channel multiplier of 1.2                                                                                                                               |mobilenetv2_120d                 |(1,3,224,224)  | Classification    |
| MobileNet V2 with channel multiplier of 1.4                                                                                                                               |mobilenetv2_140                  |(1,3,224,224)  | Classification    |
| RepVGG-A2                                                                                                                                                                 |repvgg_a2                        |(1,3,224,224)  | Classification    |
| RepVGG-B0                                                                                                                                                                 |repvgg_b0                        |(1,3,224,224)  | Classification    |
| RepVGG-B1                                                                                                                                                                 |repvgg_b1                        |(1,3,224,224)  | Classification    |
| RepVGG-B1g4                                                                                                                                                               |repvgg_b1g4                      |(1,3,224,224)  | Classification    |
| RepVGG-B2                                                                                                                                                                 |repvgg_b2                        |(1,3,224,224)  | Classification    |
| RepVGG-B2g4                                                                                                                                                               |repvgg_b2g4                      |(1,3,224,224)  | Classification    |
| RepVGG-B3                                                                                                                                                                 |repvgg_b3                        |(1,3,224,224)  | Classification    |
| RepVGG-B3g4                                                                                                                                                               |repvgg_b3g4                      |(1,3,224,224)  | Classification    |
| ResNet-101                                                                                                                                                                |resnet101                        |(1,3,224,224)  | Classification    |
| ResNet-101-D                                                                                                                                                              |resnet101d                       |(1,3,320,320)  | Classification    |
| ResNet-10-T                                                                                                                                                               |resnet10t                        |(1,3,224,224)  | Classification    |
| ResNet-14-T                                                                                                                                                               |resnet14t                        |(1,3,224,224)  | Classification    |
| ResNet-152                                                                                                                                                                |resnet152                        |(1,3,224,224)  | Classification    |
| ResNet-152-D                                                                                                                                                              |resnet152d                       |(1,3,320,320)  | Classification    |
| ResNet-18                                                                                                                                                                 |resnet18                         |(1,3,224,224)  | Classification    |
| ResNet-18-D                                                                                                                                                               |resnet18d                        |(1,3,224,224)  | Classification    |
| ResNet-200-D                                                                                                                                                              |resnet200d                       |(1,3,320,320)  | Classification    |
| ResNet-26                                                                                                                                                                 |resnet26                         |(1,3,224,224)  | Classification    |
| ResNet-26-D                                                                                                                                                               |resnet26d                        |(1,3,224,224)  | Classification    |
| ResNet-26-T                                                                                                                                                               |resnet26t                        |(1,3,256,256)  | Classification    |
| ResNet-34                                                                                                                                                                 |resnet34                         |(1,3,224,224)  | Classification    |
| ResNet-34-D                                                                                                                                                               |resnet34d                        |(1,3,224,224)  | Classification    |
| ResNet-50                                                                                                                                                                 |resnet50                         |(1,3,224,224)  | Classification    |
| ResNet-50-D                                                                                                                                                               |resnet50d                        |(1,3,224,224)  | Classification    |
| ResNet-50 avgpool anti-aliasing                                                                                                                                           |resnetaa50                       |(1,3,288,288)  | Classification    |
| ResNet101-v2                                                                                                                                                              |resnetv2_101                     |(1,3,224,224)  | Classification    |
| ResNet50-v2                                                                                                                                                               |resnetv2_50                      |(1,3,224,224)  | Classification    |
| SelecSLS42_B                                                                                                                                                              |selecsls42b                      |(1,3,224,224)  | Classification    |
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for timm models.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/timm 
. ${TVM_ROOT}/convert/venvs/timm/bin/activate 
pip install torchvision==0.16.2 \
    pytz opencv-python gitpython pandas requests pyyaml \
    tqdm matplotlib seaborn psutil ipython scipy smplx \
    numpy==1.23.5 timm==1.0.9
```

## 3. Download and save as TorchScript (.pt) files.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                         |
|-------|----------------------------------------------|
|-s     |Input shape column in the table above         |
|-n     |Download model name column in the table above |
---

```sh
cd ${TVM_ROOT}/convert 

# The following is an example for DenseNet169.
python download_pytorch_timm.py \
    -s 1,3,224,224 \
    -n densenet169
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── densenet169_timm_pytorch
           └── densenet169.pt
```

## 4. Delete the environment for timm models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/timm
```

## 5. Next Step

To compile the models, enter the TorchScript (.pt) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For DenseNet169, as the following.

```sh
python3 compile_pytorch_model_quant.py ../convert/output/densenet169_timm_pytorch/densenet169.pt -o densenet169_torch -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -s 1,3,224,224
```

----
