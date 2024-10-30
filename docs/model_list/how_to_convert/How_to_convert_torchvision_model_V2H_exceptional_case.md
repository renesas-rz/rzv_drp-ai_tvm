# < Exceptional case > How to convert Torchvision models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

### `We only confirmed the model was worked successfully in spesific PyTorch version (torch==1.8.0).`

| AI model                                                                                       | Download model name  | Input Shape    | Task              |
|------------------------------------------------------------------------------------------------|----------------------|----------------|-------------------|
| ResNeXt-50-32x4d                                                                               | resnext50_32x4d      | (1,3,224,224)  | classification    |
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

pip install torch==1.8.0 torchvision==0.9.0 \
    pytz opencv-python gitpython pandas requests pyyaml \
    tqdm matplotlib seaborn psutil ipython scipy smplx \
    numpy==1.23.5 timm==0.6.12
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

python download_torchvision.py \
    -t classification \
    -s 1,3,224,224 \
    -n resnext50_32x4d
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── resnext50_32x4d_torchvision_pytorch
           └── resnext50_32x4d.pt
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
python3 compile_pytorch_model.py ../convert/output/resnext50_32x4d_torchvision_pytorch/resnext50_32x4d.pt -o resnext50_32x4d_torchvision -s 1,3,224,224
```

----