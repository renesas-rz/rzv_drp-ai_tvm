# Installation
## Index
- [Installing DRP-AI TVM](#installing-drp-ai-tvm1-rzv2h)[^1] [(RZ/V2H)](#installing-drp-ai-tvm1-rzv2h) 
- [Installing DRP-AI TVM](#installing-drp-ai-tvm1-with-docker-rzv2h)[^1] [with Docker (RZ/V2H)](#installing-drp-ai-tvm1-with-docker-rzv2h) 

## Requirements
Requirements are listed below. Unfortunately, a link is not yet available; please contact your FAE representative to request these archived media.

- OS : Ubuntu 20.04  
- Python : 3.8  
- Package : git
- Evaluation Board: RZ/V2H EVK
- Related Software Version:
  - [DRP-AI Translator i8 v1.01][def2]
    - DRP-AI_Translator_i8-v1.01-Linux-x86_64-Install or later.
  - [RZ/V2H AI SDK v3.00][def]
    - RTK0EF0180F03000SJ.zip or later.
  <!--- - [DRP-AI Support Package for RZ/V2H Vxxx](https://www.renesas.com/us/en/products/microcontrollers-microprocessrs/rz-arm-based-high-end-32-64-bit-mpus/rzv2H-drp-ai-support-package) -->

To install DRP-AI TVM[^1] without Dockerfile, see [Installing DRP-AI TVM](#installing-drp-ai-tvm1-rzv2h)[^1].  
To install DRP-AI TVM[^1] with Dockerfile, see [Installing DRP-AI TVM](#installing-drp-ai-tvm1-with-docker-rzv2h)[^1] [with Docker for RZ/V2H](#installing-drp-ai-tvm1-with-docker-rzv2h).

## Installing DRP-AI TVM[^1] (RZ/V2H)  
### 1. Preparation
Before installing DRP-AI TVM[^1], please follow the instruction below to install the software listed in [Requirements](#requirements).  
<!--
#The information is completely protected, so I don't know how you get the files.
apt update
apt install -y wget

cd /tmp
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/DRP-AI_Translator_i8-v1.00-Linux-x86_64-Install
chmod a+x DRP-AI_Translator_i8-v1.00-Linux-x86_64-Install
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/Translator_v1.01_pre_005.tar.gz
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/drpAI_Quantizer_ver1.0.1_pre1.tar.gz

wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/RTK0EF0180F03000SJ.zip

-->

#### Install DRP-AI Translator
Download the DRP-AI Translator from the Software section in [DRP-AI](invalid-https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software) and install it by following the *User's Manual*.  
The following example shows a case where downloaded software is stored under /tmp/.
```bash
cd /opt
apt update && DEBIAN_FRONTEND=noninteractive apt install -y git wget unzip curl libboost-all-dev libeigen3-dev build-essential python3-pip libgl1-mesa-dev
chmod +x /tmp/DRP-AI_Translator_i8-v1.01-Linux-x86_64-Install
/tmp/DRP-AI_Translator_i8-v1.01-Linux-x86_64-Install
export PYTHONPATH=<Install dir>/DRP-AI_Translator_i8/drpAI_Quantizer:${PYTHONPATH}

```
<!--
```bash
apt update
apt install -y python3-pip unzip
cd /opt
yes | /tmp/DRP-AI_Translator_i8-v1.00-Linux-x86_64-Install
rm -r DRP-AI_Translator_i8
tar xvfz /tmp/drpAI_Quantizer_ver1.0.1_pre1.tar.gz 
tar xvfz /tmp/Translator_v1.01_pre_005.tar.gz 
mkdir DRP-AI_Translator_i8
mv drpAI_Quantizer_ver1.0.1_pre1 DRP-AI_Translator_i8/drpAI_Quantizer
mv translator DRP-AI_Translator_i8/
```
-->

#### Build and install RZ/V Software
1. Download the *RZ/V2H AI SDK* from [Renesas Web Page](https://renesas-rz.github.io/rzv_ai_sdk/latest/).  

2. Install SDK.  

```bash
cd /tmp
unzip RTK0EF0180F03000SJ.zip */poky*sh
chmod a+x ./ai_sdk_setup/poky-glibc-x86_64-core-image-weston-aarch64-rzv2h-evk-ver1-toolchain-3.1.26.sh
./ai_sdk_setup/poky-glibc-x86_64-core-image-weston-aarch64-rzv2h-evk-ver1-toolchain-3.1.26.sh -y
```

### 2. Install the minimal pre-requisites
Please follow the steps below by root or sudo user.

```sh
# Install packagess
apt update
DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
add-apt-repository ppa:ubuntu-toolchain-r/test
apt update
DEBIAN_FRONTEND=noninteractive apt install -y build-essential cmake \
libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev \
libxml2-dev llvm-8-dev g++-9 gcc-9
apt install -y libboost-all-dev libeigen3-dev
apt install -y libgl1-mesa-dev

pip3 install --upgrade pip
pip3 install scipy==1.5.4 psutil
pip3 install cython==0.29.32
pip3 install decorator attrs
pip3 install torchvision==0.16.2 tqdm
pip3 install tensorflow tflite

# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.1/onnxruntime-linux-x64-1.16.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
mv /tmp/onnxruntime-linux-x64-1.16.1/ /opt/
```

### 3. Clone the respository
```sh
apt install -y git
cd ~/YOURWORKDIR
git clone --recursive -b v2.1.0 https://github.com/renesas-rz/rzv_drp-ai_tvm.git drp-ai_tvm

#cd drp-ai_tvm
cd drp-ai_tvm
```
<!--
apt install -y git
cd ~/
git clone --recursive -b v2.2.0_ReleaseCandidate http://10.166.252.135/sh-okumura/drp-tvm_dev.git 
cd drp-tvm_dev
-->

### 4. Set environment variables
Run the following commands to set environment variables.
Note that environment variables must be set every time when opening the terminal.
```sh
export TVM_ROOT=$PWD                                              # or path to your own cloned repository.
export TVM_HOME=${TVM_ROOT}/tvm
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=/opt/poky/3.1.26                                       # or path to your own Linux SDK.
export TRANSLATOR=/opt/DRP-AI_Translator_i8/translator/     # or path to your own DRP-AI Translator.
export QUANTIZER=/opt/DRP-AI_Translator_i8/drpAI_Quantizer/ # or path to your own DRP-AI Quantizer.
export PRODUCT=V2H                                                # Product name (The case of V2L, V2M, and V2MA is not described in this document.)
```

### 5. Setup DRP-AI TVM[^1] environment
```sh
cd $TVM_ROOT
bash setup/make_drp_env.sh
```

## Installing DRP-AI TVM[^1] with Docker (RZ/V2H) 
### 1. Preparation
Before installing DRP-AI TVM[^1] with Docker, please prepare the following files in the working directory using the software listed in [Requirements](#requirements).

- DRP-AI_Translator_i8-v*-Linux-x86_64-Install
  - DRP-AI Translator installer
    - Download from the Software section in [DRP-AI](https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software)
- poky*.sh
  - SDK installer
    - Intermediate generated file when building image/SDK.
    - Prepare only one poky*.sh for the product to be tried in the working directory
      - Pre-builds will be able to download from public site.

### 2. Download Dockerfile
```sh
wget https://raw.githubusercontent.com/renesas-rz/rzv_drp-ai_tvm/v2.1.0/DockerfileV2H
```
<!--
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/DRP-AI_Translator_i8-v1.00-Linux-x86_64-Install
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/Translator_v1.01_pre_005.tar.gz
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/drpAI_Quantizer_ver1.0.1_pre1.tar.gz
wget http://tdrp1.dgn.renesas.com:8081/sakamoto/rzv2h-linuxpkg/raw/master/RTK0EF0180F03000SJ.zip
wget http://10.166.252.135/sh-okumura/drp-tvm_dev/raw/v220_apply_4c_up05/DockerfileV2H
-->

### 3. Build docker image
```sh
unzip RTK0EF0180F03000SJ.zip */poky*sh
mv ai_sdk_setup/* .
docker build -t drp-ai_tvm_v2h_image --build-arg SDK="/opt/poky/3.1.26" --build-arg PRODUCT="V2H" -f DockerfileV2H .
```

### 4. Run docker image
```sh
mkdir data
docker run -it --name drp-ai_tvm_v2h_container -v $(pwd)/data:/drp-ai_tvm/data drp-ai_tvm_v2h_image
```

The local `$(pwd)/data` is mounted to `/drp-ai_tvm/data` on the Docker container by the above command option.  
For example, you can use this directory to copy files created on the Docker container to your local environment.


[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.


[def]: https://renesas-rz.github.io/rzv_ai_sdk/latest/
[def2]: https://www.renesas.com/software-tool/drp-ai-translator-i8

