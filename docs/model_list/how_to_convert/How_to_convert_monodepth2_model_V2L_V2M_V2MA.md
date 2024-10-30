# How to convert Monodepth2 encoder model for V2L/M/MA
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model                                                                                                                                     | Download model name             |Input shape    | Task              |
|----------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------|---------------|-------------------|
| Monodepth2 mono_640x192 encoder                                                           |mono_640x192_encoder                        |(1,3,640,192)     | Depth Estimation|
---

## 1. Set environment variables.

1. Set up the environment, according to [Installation](../../../setup/README.md).  
2. Run the following command, confirm that the TVM_ROOT path is set.

```sh
echo $TVM_ROOT
```

## 2. Create an environment for Monodepth2 encoder model.

```sh
apt update
apt install -y python3-venv 

python3 -m venv ${TVM_ROOT}/convert/venvs/monodepth2
. ${TVM_ROOT}/convert/venvs/monodepth2/bin/activate
pip install torch==1.8.0 torchvision==0.9.0 pytz opencv-python gitpython pandas requests pyyaml tqdm matplotlib seaborn ipython smplx
git clone https://github.com/nianticlabs/monodepth2.git ${TVM_ROOT}/convert/repos/monodepth2
cd ${TVM_ROOT}/convert/repos/monodepth2
git reset --hard "b676244e5a1ca55564eb5d16ab521a48f823af31"
```

## 3. Download and save as TorchScript (.pt) file.

Use the following script to get the model. \
Set the options refer to the following table.

|option |value                                           |
|-------|------------------------------------------------|
|-n     |`Download model name` column in the table above |
|-s     |`Input shape` column in the table above         |
---

```sh
cd ${TVM_ROOT}/convert 
python3 download_monodepth2.py -n mono_640x192_encoder -s 1,3,640,192
```

After the above script is executed, the following file will be generated.

```sh
${TVM_ROOT}/convert
 └── output
      └── mono_640x192_encoder_pytorch
           └── mono_640x192_encoder.pt
```

## 4. Delete the environment for Monodepth2 encoder model.

```sh
deactivate

rm -R ${TVM_ROOT}/convert/venvs/monodepth2
rm -R ${TVM_ROOT}/convert/repos/monodepth2
```

## 5. Next Step

To compile the model, enter the TorchScript (.pt) files into the compilation script in the tutorials.【See [tutorials](../../../tutorials/)】

Run the script in the tutorials with the following command.

```sh
python3 compile_pytorch_model.py ../convert/output/mono_640x192_encoder_pytorch/mono_640x192_encoder.pt -o mono_640x192_encoder_torch -s 1,3,640,192
```

----
