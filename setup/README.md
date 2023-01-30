# Installation
## Index
- [Installing DRP-AI TVM[^1]](#installing-drp-ai-tvm1)  
- [Installing DRP-AI TVM[^1] with Docker](#installing-drp-ai-tvm1-with-docker)  

## Requirements
Requirements are listed below.  
- OS : Ubuntu 20.04  
- Python : 3.8  
- Package : git
- Evaluation Board: RZ/V2L EVK, RZ/V2M EVK, RZ/V2MA EVK
- Related Software Version:
    - RZ/V2L
        - [DRP-AI Translator V1.81](#drp-ai-translator)
        - [RZ/V Verified Linux Package V3.0.2](#rzv-software)
        - [RZ/V2L DRP-AI Support Package V7.30](#rzv-software)
    - RZ/V2M
        - [DRP-AI Translator V1.81](#drp-ai-translator)
        - [RZ/V2M Linux Package V1.3.0](#rzv-software)
        - [RZ/V2M DRP-AI Support Package V7.30](#rzv-software)
    - RZ/V2MA
        - [DRP-AI Translator V1.81](#drp-ai-translator)
        - [RZ/V2MA Linux Package V1.1.0](#rzv-software)
        - [RZ/V2MA DRP-AI Support Package V7.30](#rzv-software)


To install DRP-AI TVM[^1] without Docker, see [Installing DRP-AI TVM[^1]](#installing-drp-ai-tvm1).  

To install DRP-AI TVM[^1] with Docker, see [Installing DRP-AI TVM[^1] with Docker](#installing-drp-ai-tvm1-with-docker)  

## Installing DRP-AI TVM[^1]  
### 1. Preparation
Before installing DRP-AI TVM[^1], please follow the instruction below to install the software listed in [Requirements](#requirements).  

#### Install DRP-AI Translator
Download the DRP-AI Translator from the Software section in [DRP-AI](https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software) and install it by following the *User's Manual*.  

#### Build and install RZ/V Software
1. Download the *DRP-AI Support Package* from [Renesas Web Page](https://www.renesas.com/products/microcontrollers-microprocessors/rz-mpus/rzv-embedded-ai-mpus#software_amp_tools).  
To use the *DRP-AI Support Package*, *Linux Package* is required.  
*Linux Package* can be found in the page of *DRP-AI Support Package*.    

2. **Build image/SDK** according to the *DRP-AI Support Package Release Note* to generate following files.  

| Name | Filename |Usage|  
|----|---|---|  
| SDK Installer | poky-*.sh |Used when compiling model and building the application.|  
| Kernel Image | Image-*.bin |Used when booting kernel on the RZ/V evaluation board.|  
| Device Tree File | *.dtb |Used when booting kernel on the RZ/V evaluation board.|  
| Root Filesystem | *.tar.bz2 |Used when booting kernel on the RZ/V evaluation board.|  
| Bootloaders | i.e., *.bin, *.srec| See Linux Package Release Note for more details.<br>Used when booting the RZ/V evaluation board.|  

3. Install SDK according to the *DRP-AI Support Package Release Note*.  

### 2. Clone the respository
```sh
git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git drp-ai_tvm
```

### 3. Set environment variables
Run the following commands to set environment variables.
Note that environment variables must be set every time when opening the terminal.
```sh
export TVM_ROOT=<.../drp-ai_tvm>                    # Your own path to the cloned repository.
export TVM_HOME=${TVM_ROOT}/tvm
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=</opt/poky/3.1.14>                       # Your own Linux SDK path.
export TRANSLATOR=<.../drp-ai_translator_release/>  # Your own DRP-AI Translator path.
export PRODUCT=<V2MA>                               # Product name (V2L, V2M, or V2MA)
```
Please set the values in the table below to the SDK and PRODUCT variables according to Renesas Evaluation Board Kit you use.

| Renesas Evaluation Board Kit | SDK              | PRODUCT  |
|------------------------------|:----------------:|:--------:|
| RZ/V2L  Evaluation Board Kit | /opt/poky/3.1.17 |   V2L    |
| RZ/V2M  Evaluation Board Kit | /opt/poky/3.1.14 |   V2M    |
| RZ/V2MA Evaluation Board Kit | /opt/poky/3.1.14 |   V2MA   |

### 4. Install the minimal pre-requisites
```sh
# Install packagess
apt update
DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
add-apt-repository ppa:ubuntu-toolchain-r/test
apt update
DEBIAN_FRONTEND=noninteractive apt install -y build-essential cmake \
libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev \
libxml2-dev llvm-8-dev g++-9 gcc-9 wget

apt-get install -y python3-pip
pip3 install --upgrade pip
apt-get -y install unzip vim
pip3 install decorator attrs scipy numpy==1.23.5 pytest
pip3 install torch==1.8.0 torchvision==0.9.0

# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
mv /tmp/onnxruntime-linux-x64-1.8.1/ /opt/
```
### 5. Setup DRP-AI TVM[^1] environment
```sh
cd <.../drp-ai_tvm>
bash setup/make_drp_env.sh
```

## Installing DRP-AI TVM[^1] with Docker
### 1. Preparation
Before installing DRP-AI TVM[^1] with Docker, please prepare the following files in the working directory using the software listed in [Requirements](#requirements).
- DRP-AI_Translator-v*-Linux-x86_64-Install
    - DRP-AI Translator installer
    - Download from the Software section in [DRP-AI](https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software)
- poky*.sh
    - SDK installer
    - Intermediate generated file when building image/SDK.
    - See [Build RZ/V Software](#build-rzv-software) below.
    

#### Build RZ/V Software
Download the *DRP-AI Support Package* from [Renesas Web Page](https://www.renesas.com/products/microcontrollers-microprocessors/rz-mpus/rzv-embedded-ai-mpus#software_amp_tools).  
To use the *DRP-AI Support Package*, *Linux Package* is required.  
*Linux Package* can be found in the page of *DRP-AI Support Package*.    

After you donloaded the packages, **build image/SDK** according to the *DRP-AI Support Package Release Note* to generate following files.  

| Name | Filename |Usage|  
|----|---|---|  
| SDK Installer | poky-*.sh |Used when compiling model and building the application.|  
| Kernel Image | Image-*.bin |Used when booting kernel on the RZ/V evaluation board.|  
| Device Tree File | *.dtb |Used when booting kernel on the RZ/V evaluation board.|  
| Root Filesystem | *.tar.bz2 |Used when booting kernel on the RZ/V evaluation board.|  
| Bootloaders | i.e., *.bin, *.srec| See Linux Package Release Note for more details.<br>Used when booting the RZ/V evaluation board.|  


### 2. Download Dockerfile
```sh
wget https://raw.githubusercontent.com/renesas-rz/rzv_drp-ai_tvm/main/Dockerfile
```

### 3. Build docker image
```sh
docker build -t drp-ai_tvm_v2ma_image --build-arg SDK="/opt/poky/3.1.14" --build-arg PRODUCT="V2MA" .
```
Please set the values in the table below to the SDK and PRODUCT variables according to Renesas Evaluation Board Kit you use.

| Renesas Evaluation Board Kit | SDK              | PRODUCT  |
|------------------------------|:----------------:|:--------:|
| RZ/V2L  Evaluation Board Kit | /opt/poky/3.1.17 |   V2L    |
| RZ/V2M  Evaluation Board Kit | /opt/poky/3.1.14 |   V2M    |
| RZ/V2MA Evaluation Board Kit | /opt/poky/3.1.14 |   V2MA   |

### 4. Run docker image
```sh
mkdir data
docker run -it --name drp-ai_tvm_v2ma_container -v $(pwd)/data:/drp-ai_tvm/data drp-ai_tvm_v2ma_image
```
The local `$(pwd)/data` is mounted to `/drp-ai_tvm/data` on the Docker container by the above command option.  
For example, you can use this directory to copy files created on the Docker container to your local environment.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
