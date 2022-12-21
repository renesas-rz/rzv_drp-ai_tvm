# Installation
## Requirements
Requirements are listed below.  
- OS : Ubuntu 20.04  
- Python : 3.8  
- Package : git
- Evaluation Board: RZ/V2MA EVK
- Related Software Version:
    - [DRP-AI Translator v1.80](#drp-ai-translator)  
    - [RZ/V2MA Linux Package v1.0.0](#rzv-software)  
    - [RZ/V2MA DRP-AI Support Package v7.20](#rzv-software)  

#### DRP-AI Translator
Download the DRP-AI Translator v1.80 from the Software section in [DRP-AI](https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software) and install it by following the *User's Manual*.  

#### RZ/V Software
Download the *RZ/V2MA DRP-AI Support Package* from [Renesas Web Page](https://www.renesas.com/products/microcontrollers-microprocessors/rz-mpus/rzv-embedded-ai-mpus#software_amp_tools).  
To use the *DRP-AI Support Package*, *Linux Package* is required.  
*RZ/V2MA Linux Package* can be found in the page of *DRP-AI Support Package*.    

After you donloaded the packages, **build image/SDK** according to the *DRP-AI Support Package Release Note* *1.  

## Installing DRP-AI TVM[^1]  
Before installing DRP-AI TVM[^1], please install the software listed in [Requirements](#requirements) and build image/SDK with RZ/V2MA Linux Package and DRP-AI Support Package.  

### 1. Clone the respository.   
```sh
git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git drp-ai_tvm
```

### 2. Set environment variables.  
Run the following commands to set environment variables.
Note that environment variables must be set every time when opening the terminal.
```sh
export TVM_HOME=<.../drp-ai_tvm>/tvm                # Your own path to the cloned repository.
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=</opt/poky/3.1.14>                       # Your own RZ/V2MA Linux SDK path.
export TRANSLATOR=<.../drp-ai_translator_release/>  # Your own DRP-AI Translator path.
```
### 3. Install the minimal pre-requisites.   
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
pip3 install decorator attrs scipy numpy pytest onnx==1.9.0
pip3 install torch==1.8.0 torchvision==0.9.0

# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
mv /tmp/onnxruntime-linux-x64-1.8.1/ /opt/
```
### 4. Setup DRP-AI TVM[^1] environment.
```sh
cd <.../drp-ai_tvm>
bash setup/make_drp_env.sh
```

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
