# Installation

## Requirements

Requirements are listed below.

- OS : Ubuntu 22.04  
- Python : 3.10
- Package : git
- Evaluation Board: RZ/V2H EVK or RZ/V2N EVK
- Related Software Version:
  - [DRP-AI Translator i8 v1.10][def2]
    - DRP-AI_Translator_i8-v1.10-Linux-x86_64-Install or later.
  - SDK
    - [RZ/V2H AI SDK v5.20](https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit)
      - RTK0EF0180F05200SJ.zip or later.
    - [RZ/V2N AI SDK v6.00](https://www.renesas.com/us/en/software-tool/rzv2n-ai-software-development-kit)
      - RTK0EF0189F06000SJ.zip or lator.

To install DRP-AI TVM[^1] without Dockerfile, see [Installing DRP-AI TVM](#installing-drp-ai-tvm1-rzv2h-and-rzv2n)[^1].

To install DRP-AI TVM[^1] with Dockerfile, see [Installing DRP-AI TVM](#installing-drp-ai-tvm1-with-docker-rzv2h-and-rzv2n)[^1] [with Docker for RZ/V2H and RZ/V2N](#installing-drp-ai-tvm1-with-docker-rzv2h-and-rzv2n).

## Installing DRP-AI TVM[^1] (RZ/V2H and RZ/V2N)

## Installing DRP-AI TVM[^1]

### 1. Preparation

Before installing DRP-AI TVM[^1], please follow the instruction below to install the software listed in [Requirements](#requirements).  

#### Install DRP-AI Translator

Download the DRP-AI Translator from the Software section in [DRP-AI](invalid-https://www.renesas.com/application/key-technology/artificial-intelligence/ai-accelerator-drp-ai#software) and install it by following the *User's Manual*.  
The following example shows a case where downloaded software is stored under /tmp/ and run as the **root** user.

```bash
cd /opt
apt update && DEBIAN_FRONTEND=noninteractive apt install -y git wget unzip build-essential python3-pip libgl1-mesa-dev file
chmod +x /tmp/DRP-AI_Translator_i8-*-Install
/tmp/DRP-AI_Translator_i8-*-Install

export PYTHONPATH=${PWD}/DRP-AI_Translator_i8/drpAI_Quantizer:${PYTHONPATH}
```

#### Install RZ/V Software

1. Download the *RZ/V2H AI SDK or RZ/V2N* from [Renesas Web Page for RZ/V2H](https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit) or [Renesas Web Page for RZ/V2N](https://www.renesas.com/us/en/software-tool/rzv2n-ai-software-development-kit).  

2. Install SDK .  
The following example shows a case where downloaded software is stored under /tmp/ .

```bash
cd /tmp
unzip RTK0EF018?F0??00SJ.zip
mv `find ./ -name "*toolchain*sh"` .
chmod a+x *toolchain-*.sh 
./*toolchain-*.sh  -y
find /opt -name "cortexa55-poky-linux" | grep -q .
if [ $? -eq 0 ]; then
  ln -s `find /opt -name "cortexa55-poky-linux"` `find /opt -name "cortexa55-poky-linux"`/../aarch64-poky-linux
fi
```

### 2. Install the minimal pre-requisites

Please follow the steps below by root or sudo user.

```sh
# Install packagess
apt update
DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
add-apt-repository ppa:ubuntu-toolchain-r/test
apt update
DEBIAN_FRONTEND=noninteractive apt install -y build-essential cmake llvm-14-dev
apt install -y libgl1-mesa-dev

pip3 install --upgrade pip
pip3 install psutil numpy==1.26.4
pip3 install cython==3.0.11
pip3 install decorator attrs
pip3 install tensorflow tflite tqdm

# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.18.1/onnxruntime-linux-x64-1.18.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
mv /tmp/onnxruntime-linux-x64-1.18.1/ /opt/
```

### 3. Clone the respository

```sh
cd ~/YOURWORKDIR

git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git drp-ai_tvm

cd drp-ai_tvm
```

### 4. Set environment variables

Run the following commands to set environment variables.
Note that environment variables must be set every time when opening the terminal.

```sh
export TVM_ROOT=$PWD                                        # or path to your own cloned repository.
export TVM_HOME=${TVM_ROOT}/tvm
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=`find /opt -type d -name "sysroots"`/../         # or path to your own Linux SDK.
export TRANSLATOR=/opt/DRP-AI_Translator_i8/translator/     # or path to your own DRP-AI Translator.
export QUANTIZER=/opt/DRP-AI_Translator_i8/drpAI_Quantizer/ # or path to your own DRP-AI Quantizer.
export PRODUCT=V2N #or V2H

```

### 5. Setup DRP-AI TVM[^1] environment

```sh
cd $TVM_ROOT
bash setup/make_drp_env.sh
```

## Installing DRP-AI TVM[^1] with Docker (RZ/V2H and RZ/V2N)

### 1. Preparation for Docker

Before installing DRP-AI TVM[^1] with Docker, please prepare the following files in the working directory using the software listed in [Requirements](#requirements).

### 2. Download Dockerfile

```sh
wget https://raw.githubusercontent.com/renesas-rz/rzv_drp-ai_tvm/main/DockerfileV2H -O DockerfileV2H
```

### 3. Build docker image

```bash
#export PRODUCT=V2H
export PRODUCT=V2N
unzip RTK0EF018?F0??00SJ.zip
mv `find ./ -name "*toolchain*sh"` .
docker build -t drp-ai_tvm_${PRODUCT,,}_image_${USER} -f Dockerfile* --build-arg PRODUCT=${PRODUCT} .
```

**Tip:** If you encounter an error like "404 Not Found," try adding the `--no-cache` option with "`docker build`".

### 4. Run docker image

```sh
docker run -it --name drp-ai_tvm_${PRODUCT,,}_container_${USER} drp-ai_tvm_${PRODUCT,,}_image_${USER}
```

The local `$(pwd)/data` is mounted to `/drp-ai_tvm/data` on the Docker container by the above command option.  
For example, you can use this directory to copy files created on the Docker container to your local environment.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.

[def]: https://www.renesas.com/us/en/software-tool/rzv2h-ai-software-development-kit
[def2]: https://www.renesas.com/software-tool/drp-ai-translator-i8
