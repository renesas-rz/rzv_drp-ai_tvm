# < Exceptional case > How to convert timm models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

### `We only confirmed the model was worked successfully in spesific PyTorch version (torch==1.8.0).`

| AI model                                                                                                                                                                  | Download model name             |Input shape    | Task              |
|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| CSP-ResNet                                                                                                                                                                |cspresnet50                      |(1,3,256,256)  | Classification    |
| CSP-ResNeXt                                                                                                                                                               |cspresnext50                     |(1,3,224,224)  | Classification    |
| DLA(Dense Layer Aggregation)102x                                                                                                                                          |dla102x                          |(1,3,224,224)  | Classification    |
| DLA102x2                                                                                                                                                                  |dla102x2                         |(1,3,224,224)  | Classification    |
| DLA46x_c                                                                                                                                                                  |dla46x_c                         |(1,3,224,224)  | Classification    |
| DLA60x_c                                                                                                                                                                  |dla60x_c                         |(1,3,224,224)  | Classification    |
| DPN(Dual Path Network)107                                                                                                                                                 |dpn107                           |(1,3,224,224)  | Classification    |
| ECA-ResNet101d                                                                                                                                                            |ecaresnet101d                    |(1,3,224,224)  | Classification    |
| ECA-ResNet26t                                                                                                                                                             |ecaresnet26t                     |(1,3,320,320)  | Classification    |
| ECA-ResNet50d                                                                                                                                                             |ecaresnet50d                     |(1,3,224,224)  | Classification    |
| ECA-ResNet50t                                                                                                                                                             |ecaresnet50t                     |(1,3,320,320)  | Classification    |
| ECA-ResNetlight                                                                                                                                                           |ecaresnetlight                   |(1,3,224,224)  | Classification    |
| FBNetV3-B                                                                                                                                                                 |fbnetv3_b                        |(1,3,256,256)  | Classification    |
| FBNetV3-D                                                                                                                                                                 |fbnetv3_d                        |(1,3,256,256)  | Classification    |
| FBNetV3-G                                                                                                                                                                 |fbnetv3_g                        |(1,3,288,288)  | Classification    |
| Global Context Resnet50t (gcresnet50t)                                                                                                                                    |gcresnet50t                      |(1,3,256,256)  | Classification    |
| GhostNet-1.0x                                                                                                                                                             |ghostnet_100                     |(1,3,224,224)  | Classification    |
| (Gluon) ResNeXt101 32x4d                                                                                                                                                  |gluon_resnext101_32x4d           |(1,3,224,224)  | Classification    |
| (Gluon) ResNeXt101 64x4d                                                                                                                                                  |gluon_resnext101_64x4d           |(1,3,224,224)  | Classification    |
| (Gluon) SENet154                                                                                                                                                          |gluon_senet154                   |(1,3,224,224)  | Classification    |
| (Gluon) SE-ResNeXt101 32-4d                                                                                                                                               |gluon_seresnext101_32x4d         |(1,3,224,224)  | Classification    |
| (Gluon) SE-ResNeXt101 64-4d                                                                                                                                               |gluon_seresnext101_64x4d         |(1,3,224,224)  | Classification    |
| (Gluon) SE-ResNeXt50 32-4d                                                                                                                                                |gluon_seresnext50_32x4d          |(1,3,224,224)  | Classification    |
| (Gluon) Xception65                                                                                                                                                        |gluon_xception65                 |(1,3,299,299)  | Classification    |
| HardcoreNAS_A                                                                                                                                                             |hardcorenas_a                    |(1,3,224,224)  | Classification    |
| HardcoreNAS_B                                                                                                                                                             |hardcorenas_b                    |(1,3,224,224)  | Classification    |
| HardcoreNAS_C                                                                                                                                                             |hardcorenas_c                    |(1,3,224,224)  | Classification    |
| HardcoreNAS_D                                                                                                                                                             |hardcorenas_d                    |(1,3,224,224)  | Classification    |
| HardcoreNAS_E                                                                                                                                                             |hardcorenas_e                    |(1,3,224,224)  | Classification    |
| HardcoreNAS_F                                                                                                                                                             |hardcorenas_f                    |(1,3,224,224)  | Classification    |
| Instagram ResNeXt101 32x8 WSL                                                                                                                                             |ig_resnext101_32x8d              |(1,3,224,224)  | Classification    |
| Inception ResNet v2                                                                                                                                                       |inception_resnet_v2              |(1,3,224,224)  | Classification    |
| (Legacy) SENet-154                                                                                                                                                        |legacy_senet154                  |(1,3,224,224)  | Classification    |
| (Legacy) SE-ResNet-152                                                                                                                                                    |legacy_seresnet152               |(1,3,224,224)  | Classification    |
| (Legacy) SE-ResNet-18                                                                                                                                                     |legacy_seresnet18                |(1,3,224,224)  | Classification    |
| (Legacy) SE-ResNet-34                                                                                                                                                     |legacy_seresnet34                |(1,3,224,224)  | Classification    |
| (Legacy) SE-ResNet-50                                                                                                                                                     |legacy_seresnet50                |(1,3,224,224)  | Classification    |
| (Legacy) SE-ResNeXt-26                                                                                                                                                    |legacy_seresnext26_32x4d         |(1,3,224,224)  | Classification    |
| MnasNet-Small depth multiplier 1.0                                                                                                                                        |mnasnet_small                    |(1,3,224,224)  | Classification    |
| MobileNet V3 Large 1.0                                                                                                                                                    |mobilenetv3_large_100            |(1,3,224,224)  | Classification    |
| MobileNet V3 Large 1.0,  21k pretraining                                                                                                                                  |mobilenetv3_large_100_miil_in21k |(1,3,224,224)  | Classification    |
| MobileNet V3 (RW variant)                                                                                                                                                 |mobilenetv3_rw                   |(1,3,224,224)  | Classification    |
| MobileNet V3 Small 0.5                                                                                                                                                    |mobilenetv3_small_050            |(1,3,224,224)  | Classification    |
| MobileNet V3 Small 0.75                                                                                                                                                   |mobilenetv3_small_075            |(1,3,224,224)  | Classification    |
| MobileNet V3 Small 1.0                                                                                                                                                    |mobilenetv3_small_100            |(1,3,224,224)  | Classification    |
| RegNetX 200MF                                                                                                                                                             |regnetx_002                      |(1,3,224,224)  | Classification    |
| RegNetX 400MF                                                                                                                                                             |regnetx_004                      |(1,3,224,224)  | Classification    |
| RegNetX 600MF                                                                                                                                                             |regnetx_006                      |(1,3,224,224)  | Classification    |
| RegNetX 800MF                                                                                                                                                             |regnetx_008                      |(1,3,224,224)  | Classification    |
| RegNetX 1.6GF                                                                                                                                                             |regnetx_016                      |(1,3,224,224)  | Classification    |
| RegNetX 3.2GF                                                                                                                                                             |regnetx_032                      |(1,3,224,224)  | Classification    |
| RegNetX 4.0GF                                                                                                                                                             |regnetx_040                      |(1,3,224,224)  | Classification    |
| RegNetX 6.4GF                                                                                                                                                             |regnetx_064                      |(1,3,224,224)  | Classification    |
| RegNetX 8.0GF                                                                                                                                                             |regnetx_080                      |(1,3,224,224)  | Classification    |
| RegNetX 16GF                                                                                                                                                              |regnetx_160                      |(1,3,224,224)  | Classification    |
| RegNetY 200MF                                                                                                                                                             |regnety_002                      |(1,3,224,224)  | Classification    |
| RegNetY 400MF                                                                                                                                                             |regnety_004                      |(1,3,224,224)  | Classification    |
| RegNetY 600MF                                                                                                                                                             |regnety_006                      |(1,3,224,224)  | Classification    |
| RegNetY 800MF                                                                                                                                                             |regnety_008                      |(1,3,224,224)  | Classification    |
| RegNetY 1.6GF                                                                                                                                                             |regnety_016                      |(1,3,224,224)  | Classification    |
| RegNetY 4.0GF                                                                                                                                                             |regnety_040                      |(1,3,224,224)  | Classification    |
| RegNetY 8.0GF                                                                                                                                                             |regnety_080                      |(1,3,224,224)  | Classification    |
| RegNetY 16GF                                                                                                                                                              |regnety_160                      |(1,3,224,224)  | Classification    |
| RegNetY 32GF                                                                                                                                                              |regnety_320                      |(1,3,224,224)  | Classification    |
| Res2Net-101 26w×4s                                                                                                                                                       |res2net101_26w_4s                |(1,3,224,224)  | Classification    |
| Res2Net-50 14w×8s                                                                                                                                                        |res2net50_14w_8s                 |(1,3,224,224)  | Classification    |
| Res2Net-50 26w×4s                                                                                                                                                        |res2net50_26w_4s                 |(1,3,224,224)  | Classification    |
| Res2Net-50 26w×6s                                                                                                                                                        |res2net50_26w_6s                 |(1,3,224,224)  | Classification    |
| Res2Net-50 48w×2s                                                                                                                                                        |res2net50_48w_2s                 |(1,3,224,224)  | Classification    |
| Res2Next-50                                                                                                                                                               |res2next50                       |(1,3,224,224)  | Classification    |
| ResNeSt-101                                                                                                                                                               |resnest101e                      |(1,3,256,256)  | Classification    |
| ResNeSt-14                                                                                                                                                                |resnest14d                       |(1,3,224,224)  | Classification    |
| ResNeSt-200                                                                                                                                                               |resnest200e                      |(1,3,320,320)  | Classification    |
| ResNeSt-269                                                                                                                                                               |resnest269e                      |(1,3,416,416)  | Classification    |
| ResNeSt-26                                                                                                                                                                |resnest26d                       |(1,3,224,224)  | Classification    |
| ResNeSt-50                                                                                                                                                                |resnest50d                       |(1,3,224,224)  | Classification    |
| ResNeSt-50 1s4×24d                                                                                                                                                       |resnest50d_1s4x24d               |(1,3,224,224)  | Classification    |
| ResNeSt-50 4s2×40d                                                                                                                                                       |resnest50d_4s2x40d               |(1,3,224,224)  | Classification    |
| ResNet-50 blur anti-aliasing                                                                                                                                              |resnetblur50                     |(1,3,224,224)  | Classification    |
| ResNet-RS-101                                                                                                                                                             |resnetrs101                      |(1,3,288,288)  | Classification    |
| ResNet-RS-152                                                                                                                                                             |resnetrs152                      |(1,3,320,320)  | Classification    |
| ResNet-RS-200                                                                                                                                                             |resnetrs200                      |(1,3,320,320)  | Classification    |
| ResNet-RS-50                                                                                                                                                              |resnetrs50                       |(1,3,224,224)  | Classification    |
| ResNeXt-101(32x8d)                                                                                                                                                        |resnext101_32x8d                 |(1,3,224,224)  | Classification    |
| ResNeXt-101(64x4d)                                                                                                                                                        |resnext101_64x4d                 |(1,3,288,288)  | Classification    |
| ResNeXt-50(32x4d)                                                                                                                                                         |resnext50_32x4d                  |(1,3,224,224)  | Classification    |
| ResNeXt-50d(32x4d)                                                                                                                                                        |resnext50d_32x4d                 |(1,3,224,224)  | Classification    |
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
pip install torch==1.8.0 torchvision==0.9.0 pytz opencv-python gitpython pandas requests pyyaml tqdm matplotlib seaborn ipython smplx timm==0.6.12
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

# The following is an example for CSP-ResNet.
python download_pytorch_timm.py \
    -s 1,3,256,256 \
    -n cspresnet50
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── cspresnet50_timm_pytorch
           └── cspresnet50.pt
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
python3 compile_pytorch_model_quant.py ../convert/output/cspresnet50_timm_pytorch/cspresnet50.pt -o cspresnet50_torch -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -v 100 -s 1,3,256,256
```

----
