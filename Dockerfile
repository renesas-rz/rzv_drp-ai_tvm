FROM ubuntu:20.04

ARG PRODUCT="V2L"

# Install packages
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential cmake
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libxml2-dev llvm-8-dev g++-9 gcc-9
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libgl1-mesa-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y wget git vim locales
RUN locale-gen en_US.UTF-8

# Install onnxruntime
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.18.1/onnxruntime-linux-x64-1.18.1.tgz -O /tmp/onnxruntime.tar.gz \
    && tar -xvzf /tmp/onnxruntime.tar.gz -C /tmp/ \
    && mv /tmp/onnxruntime-linux-x64-1.18.1/ /opt/
    
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

# Install Python packages
RUN pip3 install onnxruntime==1.18.1
RUN pip3 install scipy numpy==1.23.5 psutil pytest
RUN pip3 install decorator attrs
RUN pip3 install torchvision==0.9.0 --index-url https://download.pytorch.org/whl/cpu
RUN pip3 install tensorflow tflite typing-extensions==4.5.0


# Clone repository
ENV TVM_ROOT="/drp-ai_tvm"
RUN git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git  ${TVM_ROOT}

# Set environment variables
ENV TVM_HOME="${TVM_ROOT}/tvm"
ENV PYTHONPATH="$TVM_HOME/python"
RUN echo 'export SDK="/opt/poky/`ls /opt/poky/`"' >> ~/.bashrc
ENV TRANSLATOR="/opt/drp-ai_translator_release"
ENV PRODUCT="${PRODUCT}"

# Setup environment
RUN cd ${TVM_ROOT} && bash setup/make_drp_env.sh

# Set WORKDIR
WORKDIR ${TVM_ROOT}

