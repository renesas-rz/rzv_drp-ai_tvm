[<- What is Pruning and Quantization?](./../README.md)
### Installing DRP-AI Extension Package
Follow the instructions for setting up a host machine using docker. 
If you install the DRP-AI Extension Pack to host machine directly, please refer to the DRP-AI Extension Pack User's manual.

> **Note1:**  
> This guide provides the sparse model that has already been pruned.  
> If you only want to validate RZ/V2H operation, please skip the `pruning then retraining` step.

> **Note2:**  
> If you have set up your development environment using docker such as DRP-AI TVM[^1] installation with Docker, exit the container once and create a docker image for this installation.

#### 1. Preparation
Please prepare the following file from this [Renesas Web Site](https://www.renesas.com/us/en/software-tool/drp-ai-extension-pack-pruning-tool).

|File Name|Description|
|:---:|:---:|
|drpai-extension-pack_ver*.tar.gz|DRP-AI Extension Pack|


#### 2. Build docker image
This chapter explains how to build the docker image for the DRP-AI Extension Pack.

```bash
cd ${TVM_ROOT}/pruning/setup/docker
# Or wget <URL to docker_torch.Dockerfile>

# Copy the the DRP-AI Extension Pack to `docker` directory.
cp <path/to/>drpai-extension-pack_ver*.tar.gz .
ls
# Ensure that the `torch.Dockefile` and `drpai-extension-pack_ver*tar.gz` are included in the current directory.
# torch.Dockerfile
# drpai-extension-pack_ver*tar.gz
# ....

# Build docker image.
# It may take up to an hour to complete.
docker build -t drpai_ext_pt_img -f torch.Dockerfile .

# With output of `drpai_ext_pt_img` word, the setup processing is completed.
docker images | grep drpai_ext_pt_img
# drpai_ext_pt_img                  latest                              XXXXXXXXXXXXX        X seconds ago       12.4GB
```

---
[Pruning and retraining by DRP-AI Extension Package ->](./../how-to/torchvision_resnet50/README.md)

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
