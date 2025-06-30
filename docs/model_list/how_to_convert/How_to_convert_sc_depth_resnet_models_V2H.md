# How to convert SC-Depth resnet models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| [SC-Depth resnet18_depth_256 dispnet](https://onedrive.live.com/?authkey=%21AP8Z6Tl8RC8waZo&cid=36712431A95E7A25&id=36712431A95E7A25%212457&parId=36712431A95E7A25%212455&o=OneUp)                                                           |sc_depth_resnet18_depth_256_dispnet                        |(1,3,832,256)      | Depth Estimation|
| [SC-Depth resnet50_depth_256 dispnet](https://onedrive.live.com/?authkey=%21AP8Z6Tl8RC8waZo&cid=36712431A95E7A25&id=36712431A95E7A25%212460&parId=36712431A95E7A25%212454&o=OneUp)                                                           |sc_depth_resnet50_depth_256_dispnet                      |(1,3,416,128)     | Depth Estimation|
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/SetupV2H.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for SC-Depth resnet models.

Note : Check the downloaded .tar file from the link in the table above is located in `${TVM_ROOT}/convert`.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/sc_deapth
. ${TVM_ROOT}/convert/venvs/sc_deapth/bin/activate 
pip install torch==2.1.2 torchvision==0.16.2 pytz opencv-python gitpython pandas requests pyyaml tqdm matplotlib seaborn ipython smplx
git clone https://github.com/JiawangBian/SC-SfMLearner-Release.git ${TVM_ROOT}/convert/repos/SC-SfMLearner-Release

# Change name of .tar file
# The following is an example for sc_depth_resnet50_depth_256_dispnet
mv dispnet_model_best.pth.tar resnet_50_dispnet_model_best.pth.tar
```

## 3. Download and save as TorchScript (.pt) files.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                          |
|-------|-----------------------------------------------|
|-n     |`Download model name` column in the table above|
|-s     |`Input shape` column in the table above        |
---

```sh
cd $TVM_ROOT/convert/

# The following is an example for sc_depth_resnet50_depth_256_dispnet
python download_sc_depth_resnet.py -n sc_depth_resnet50_depth_256_dispnet -s 1,3,416,128
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── sc_depth_resnet50_depth_256_dispnet_pytorch
           └── sc_depth_resnet50_depth_256_dispnet.pt
```

## 4. Delete the environment for SC-Depth resnet models.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/sc_deapth
rm -R ${TVM_ROOT}/convert/repos/SC-SfMLearner-Release
rm ${TVM_ROOT}/convert/resnet_50_dispnet_model_best.pth.tar
```

## 5. Next Step

To compile the models, enter the TorchScript (.pt) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command. For sc_depth_resnet50_depth_256_dispnet, as the following.

```sh
python3 compile_pytorch_model_quant.py ../convert/output/sc_depth_resnet50_depth_256_dispnet_pytorch/sc_depth_resnet50_depth_256_dispnet.pt -o sc_depth_resnet50_depth_256_dispnet_torch -t $SDK -d $TRANSLATOR -c $QUANTIZER --images $TRANSLATOR/../GettingStarted/tutorials/calibrate_sample/ -s 1,3,416,128
```

----
