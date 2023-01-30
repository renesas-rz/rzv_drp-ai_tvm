FROM ubuntu:20.04

ARG SDK="/opt/poky/3.1.14"
ARG PRODUCT="V2MA"

# Install packages
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev libxml2-dev llvm-8-dev g++-9 gcc-9
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libgl1-mesa-dev libopencv-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y cmake wget git unzip vim locales
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3-pip
RUN locale-gen en_US.UTF-8
RUN pip3 install --upgrade pip
RUN pip3 install decorator attrs scipy numpy==1.23.5 pytest
RUN pip3 install torch==1.8.0 torchvision==0.9.0

# Install onnxruntime
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz -O /tmp/onnxruntime.tar.gz \
    && tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/ \
    && mv /tmp/onnxruntime-linux-x64-1.8.1/ /opt/
    
# Install SDK
COPY ./poky*.sh /opt
RUN chmod a+x /opt/poky*.sh
RUN cd /opt && yes "" | ./poky*.sh
RUN rm /opt/poky*.sh

# Install DRP-AI Translator
COPY ./DRP-AI_Translator-v*-Linux-x86_64-Install /opt
RUN chmod a+x /opt/DRP-AI_Translator-v*-Linux-x86_64-Install
RUN cd /opt && yes | ./DRP-AI_Translator-v*-Linux-x86_64-Install
RUN rm /opt/DRP-AI_Translator-v*-Linux-x86_64-Install

# Clone repository
RUN git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git /drp-ai_tvm

# Set environment variables
ENV TVM_ROOT="/drp-ai_tvm"
ENV TVM_HOME="${TVM_ROOT}/tvm"
ENV PYTHONPATH="$TVM_HOME/python:${PYTHONPATH}"
ENV SDK="${SDK}"
ENV TRANSLATOR="/opt/drp-ai_translator_release"
ENV PRODUCT="${PRODUCT}"

# Setup environment
RUN cd ${TVM_ROOT} && bash setup/make_drp_env.sh

# Set WORKDIR
WORKDIR ${TVM_ROOT}

