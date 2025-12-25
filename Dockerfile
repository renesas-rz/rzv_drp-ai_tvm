FROM ubuntu:22.04

ARG PRODUCT="V2H"

# Update gcc/g++ compiler
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3-pip unzip
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common
RUN apt install -y ca-certificates gpg wget lsb-release
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor > /usr/share/keyrings/kitware-archive-keyring.gpg
RUN echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main" > /etc/apt/sources.list.d/kitware.list
RUN DEBIAN_FRONTEND=noninteractive apt-get update --fix-missing
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential cmake-data=3.28.1-* cmake=3.28.1-*
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libgl1 libjpeg-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git vim llvm-14 file g++-13 gcc-13

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1
RUN update-alternatives --config python

# Install SDK
COPY ./*.sh /opt
RUN chmod a+x /opt/*.sh
RUN cd /opt && yes "" | ./*.sh
RUN rm /opt/*.sh
RUN ln -s `find /opt -name "cortexa55-poky-linux"` `find /opt -name "cortexa55-poky-linux"`/../aarch64-poky-linux

# Install DRP-AI Translator
COPY ./DRP-AI_Translator*-Linux*-x86_64-Install  /opt
RUN chmod a+x /opt/DRP-AI_Translator*-Linux*-x86_64-Install
WORKDIR /opt
RUN yes | DEBIAN_FRONTEND=noninteractive /opt/DRP-AI_Translator*-Linux*-x86_64-Install
#RUN rm /opt/DRP-AI_Translator*-Linux*-x86_64-Install

RUN pip3 install psutil numpy==1.26.4
RUN pip3 install cython==3.0.11
RUN pip3 install decorator attrs
RUN pip3 install tensorflow==2.18.1 tflite tqdm


# Clone repository
ENV TVM_ROOT="/drp-ai_tvm"
RUN git clone --recursive https://github.com/renesas-rz/rzv_drp-ai_tvm.git  ${TVM_ROOT}

# Set environment variables
WORKDIR $TVM_ROOT
RUN echo 'export SDK="`find /opt/ -name "sysroots"`/../"' >> ~/.bashrc
ENV PYTHONPATH="/opt/DRP-AI_Translator_i8/drpAI_Quantizer/"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$LIBRARY_PATH"
ENV LIBRARY_PATH="$LD_LIBRARY_PATH"

WORKDIR ${TVM_ROOT}/obj/pip_package
RUN apt-get -y purge python3-yaml
RUN pip3 install --upgrade pip
#RUN pip3 install *.whl
RUN pip3 install mera2_r*
RUN pip3 install tvm-*
RUN pip3 install mera2_c*
#RUN pip3 install tensorflow-cpu==2.9.0

WORKDIR $TVM_ROOT/package
#ENV TVM_HOME="${TVM_ROOT}/tvm"
#ENV TRANSLATOR="/opt/DRP-AI_Translator_i8/translator/"
RUN echo 'export TRANSLATOR="`find /opt/ -name "python_api"`/../../"' >> ~/.bashrc
ENV QUANTIZER="/opt/DRP-AI_Translator_i8/drpAI_Quantizer/"
ENV PRODUCT="${PRODUCT}"


WORKDIR $TVM_ROOT/3rdparty
RUN git clone https://github.com/gabime/spdlog.git
RUN git clone https://github.com/chriskohlhoff/asio.git

RUN cp $TVM_ROOT/setup/include/*.h $TVM_ROOT/tvm/include/tvm/runtime/

# Set WORKDIR
WORKDIR /$TVM_ROOT/tutorials