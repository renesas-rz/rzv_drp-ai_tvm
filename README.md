# Extension package of TVM Deep Learning Complier for renesas DRP-AI accelerators powered by MERA&trade; compiler stack   

[TVM Documentation](https://tvm.apache.org/docs) |
[TVM Community](https://tvm.apache.org/community) |
[TVM github](https://github.com/apache/tvm) |


DRP-AI TVM[^1] is Machine Learning Compiler plugin for [Apache TVM](https://github.com/apache/tvm/) provided by Renesas Electronics Corporation.

## License
(C) Copyright EdgeCortix, Inc. 2022   
(C) Copyright Renesas Electronics Corporation 2022   
Contributors Licensed under an Apache-2.0 license.   

## Supported Embedded Platforms
- Renesas RZ/V2M Evaluation Board Kit

## Introduction
### Overview   
This compiler stack is an extension of the DRP-AI Translator to the TVM backend. CPU and DRP-AI can work together for the inference processing of the AI models.  

<img src=./img/tool_stack.png width=350>   


### File Configuration
| Directory | Details |
|:---|:---|
|tutorials |Sample compile script|
|apps |Sample inference application on the target board|
|setup | Setup scripts for building a TVM environment |
|obj |Pre-build runtime binaries|
|tvm | TVM repository from github |
|3rd party | 3rd party tools |   
|how-to |Sample to solve specific problems, i.e., How to run validation between x86 and DRP-AI|


## Installation
### Requirements
Required software is listed below.  

- Ubuntu 18.04  
- Python 3.6  
- git
- [DRP-AI Translator v1.60](#drp-ai-translator)  
- [RZ/V2M Linux Package v1.2.0](#rzv-software)  
- [RZ/V2M DRP-AI Support Package v6.00](#rzv-software)

##### DRP-AI Translator
Download the DRP-AI Translator v1.60 from the Software section in [RZ/V2M Software](https://www.renesas.com/rzv2m)　and install it by following the *User's Manual*.   

##### RZ/V Software
Download the *RZ/V2M Linux Package* and *DRP-AI Support Package* from the Software section in [RZ/V2M Software](https://www.renesas.com/rzv2m) and **build image/SDK** according to the *DRP-AI Support Package Release Note* *1.  

*1 OpenCV library is required to run application example provided in this repository ([Application Example](./apps)).  
To install OpenCV, please see [How to install OpenCV](./apps#how-to-install-opencv-to-linux-package) in Application Example page.  

### Installing DRP-AI TVM[^1]  
Before installing DRP-AI TVM[^1], please install the software listed in [Requirements](#requirements) and build image/SDK with RZ/V2M Linux Package and DRP-AI Support Package.  

#### 1. Clone the respository.   
```sh
git clone --recursive -b v0.1 <git url.> drp-ai_tvm 
```

#### 2. Set environment variables.  
Run the following commands to set environment variables.
Note that environment variables must be set every time when opening the terminal.
```sh
export TVM_HOME=<.../drp-ai_tvm>/tvm                # Your own path to the cloned repository.
export PYTHONPATH=$TVM_HOME/python:${PYTHONPATH}
export SDK=</opt/poky/2.4.3>                        # Your own RZ/V2M Linux SDK path.
export TRANSLATOR=<.../drp-ai_translator/>          # Your own DRP-AI Translator path.
```
#### 3. Install the minimal pre-requisites.   
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
```
If an already recent gcc/g++ compiler is already the default one in the system then the next step is not necessary. If the system default version is **9 or higher** then we are ready. If that is not the case then we can make sure that gcc/g++-9 will be the default compiler by running:
```sh
# Update gcc to 9.4
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 10
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 20
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 10
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 20
update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
update-alternatives --set cc /usr/bin/gcc
update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30
update-alternatives --set c++ /usr/bin/g++
update-alternatives --set gcc "/usr/bin/gcc-9"
update-alternatives --set g++ "/usr/bin/g++-9"
```
Installing ONNX Runtime Library from precompiled release package.
```sh
# Install onnx runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz -O /tmp/onnxruntime.tar.gz
tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/
mv /tmp/onnxruntime-linux-x64-1.8.1/ /opt/
```
Setup DRP-AI TVM[^1] environment.
```sh
cd <.../drp-ai_tvm>
bash setup/make_drp_env.sh
```

----

## Deploy AI models on DRP-AI
![drawing](./img/deploy_flow.png)  

To deploy the AI model to DRP-AI on the target board, you need to compile the model with DRP-AI TVM[^1] to generate Runtime Model Data (Compile).  
SDK generated from RZ/V Linux Package and DRP-AI Support Package is required to compile the model.  

After compiled the model, you need to copy the file to the target board (Deploy).  
You also need to copy the C++ inference application and DRP-AI TVM[^1] Runtime Library to run the AI model inference.  
Moreover, since DRP-AI TVM[^1] does not support pre/post-processing of AI inference, OpenCV Library is essential.  

Following pages show the example to compile the ResNet18 model and run it on the target board.  

### Compile model with DRP-AI TVM[^1]
Please see [Tutorial](./tutorials).  

### Run inference on board
Please see [Application Example](./apps) page.  

### How-to
Pages above only show the example for ResNet.  
To find more examples, please see [How-to](./how-to) page.  
It includes the sample to solve specific problems, i.e.;  
- how to run application with camera;  
- validation between x86 and DRP-AI, etc.  

----
For any enquiries, please contact Renesas.  

[^1]: DRP-AI TVM is powered by EdgeCortix MERA Compiler

