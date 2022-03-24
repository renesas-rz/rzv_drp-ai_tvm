FROM ubuntu:18.04

RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt install -y vim sudo build-essential build-essential cmake \
    libomp-dev libgtest-dev libgoogle-glog-dev libtinfo-dev zlib1g-dev libedit-dev \
    libxml2-dev llvm-8-dev g++-9 gcc-9 wget

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 10
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 20
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 10
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 20
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
RUN update-alternatives --set cc /usr/bin/gcc
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30
RUN update-alternatives --set c++ /usr/bin/g++
RUN update-alternatives --set gcc "/usr/bin/gcc-9"
RUN update-alternatives --set g++ "/usr/bin/g++-9"
RUN which python3.6

RUN DEBIAN_FRONTEND=noninteractive apt install -y python3-pip
RUN pip3 install virtualenv
RUN virtualenv -p python3.6 drp-venv
RUN . drp-venv/bin/activate && pip3 install decorator attrs scipy numpy pytest onnx==1.9.0

COPY DRP-AI_Translator-v1.60-Linux-x86_64-Install .
RUN . drp-venv/bin/activate && ./DRP-AI_Translator-v1.60-Linux-x86_64-Install
RUN . drp-venv/bin/activate && pip3 install torch==1.8.0 torchvision==0.9.0

WORKDIR /opt
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz
RUN tar xvzf onnxruntime-linux-x64-1.8.1.tgz

COPY poky-glibc-x86_64-core-image-bsp-aarch64-toolchain-2.4.3.sh .
RUN chmod +x poky-glibc-x86_64-core-image-bsp-aarch64-toolchain-2.4.3.sh
RUN ./poky-glibc-x86_64-core-image-bsp-aarch64-toolchain-2.4.3.sh -y

COPY drp-tvm /drp-tvm
WORKDIR /drp-tvm
RUN mkdir build
WORKDIR /drp-tvm/build
RUN cp ../cmake/config.cmake .
RUN echo "set(USE_OPENMP gnu)" >> config.cmake && \
    echo "set(USE_LLVM ON)" >> config.cmake && \
    echo "set(USE_MERA_DRP ON)" >> config.cmake
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
RUN make -j$(nproc)

WORKDIR /drp-tvm/tutorials/mera/drp/
RUN wget https://github.com/onnx/models/raw/master/vision/classification/resnet/model/resnet18-v1-7.onnx

RUN echo "export TVM_HOME=/drp-tvm" >> ~/.bashrc && \
    echo "export PYTHONPATH=\$TVM_HOME/python:\$TVM_HOME/topi/python:\${PYTHONPATH}" >> ~/.bashrc && \
    echo "source /drp-venv/bin/activate" >> ~/.bashrc
