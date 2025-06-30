#######################################################################################################################
# DISCLAIMER
# This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
# other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
# applicable laws, including copyright laws.
# THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
# THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
# EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
# SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
# THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
# this software. By using this software, you agree to the additional terms and conditions found by accessing the
# following link:
# http://www.renesas.com/disclaimer
#
# Copyright (C) 2022, 2023, 2024, 2025 Renesas Electronics Corporation. All rights reserved.
#######################################################################################################################
# Description : Dockerfile for DRP-AI Extension Pack for PyTorch
# Version     : 1.2.0
FROM ubuntu:22.04 as base

ENV NVARCH x86_64

################################################################################
# Install CUDA prerequirements (Create an environment similar to the *-base environment)
################################################################################
ENV NVIDIA_REQUIRE_CUDA "cuda>=11.8 brand=tesla,driver>=418,driver<419 brand=tesla,driver>=450,driver<451"
ENV NV_CUDA_CUDART_VERSION 11.8.89-1
ENV NV_CUDA_COMPAT_PACKAGE cuda-compat-11-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    gnupg2 curl ca-certificates && \
    curl -fsSL https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/${NVARCH}/3bf863cc.pub | apt-key add - && \
    echo "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/${NVARCH} /" > /etc/apt/sources.list.d/cuda.list && \
    apt-get purge --autoremove -y curl \
    && rm -rf /var/lib/apt/lists/*

    ENV CUDA_VERSION 11.8.0

# For libraries in the cuda-compat-* package: https://docs.nvidia.com/cuda/eula/index.html#attachment-a
RUN apt-get update && apt-get install -y --no-install-recommends \
    cuda-cudart-11-8=${NV_CUDA_CUDART_VERSION} \
    ${NV_CUDA_COMPAT_PACKAGE} \
    && rm -rf /var/lib/apt/lists/*

# Required for nvidia-docker v1
RUN echo "/usr/local/nvidia/lib" >> /etc/ld.so.conf.d/nvidia.conf \
    && echo "/usr/local/nvidia/lib64" >> /etc/ld.so.conf.d/nvidia.conf

ENV PATH /usr/local/nvidia/bin:/usr/local/cuda/bin:${PATH}
ENV LD_LIBRARY_PATH /usr/local/nvidia/lib:/usr/local/nvidia/lib64

# nvidia-container-runtime
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES compute,utility

################################################################################
# Install CUDA libraries (Create an environment similar to the *-runtime environment)
################################################################################
FROM base as runtime_env
ENV NV_CUDA_LIB_VERSION 11.8.0-1

ENV NV_NVTX_VERSION 11.8.86-1
ENV NV_LIBNPP_VERSION 11.8.0.86-1
ENV NV_LIBNPP_PACKAGE libnpp-11-8=${NV_LIBNPP_VERSION}
ENV NV_LIBCUSPARSE_VERSION 11.7.5.86-1

ENV NV_LIBCUBLAS_PACKAGE_NAME libcublas-11-8
ENV NV_LIBCUBLAS_VERSION 11.11.3.6-1
ENV NV_LIBCUBLAS_PACKAGE ${NV_LIBCUBLAS_PACKAGE_NAME}=${NV_LIBCUBLAS_VERSION}

ENV NV_LIBNCCL_PACKAGE_NAME libnccl2
ENV NV_LIBNCCL_PACKAGE_VERSION 2.15.5-1
ENV NCCL_VERSION 2.15.5-1
ENV NV_LIBNCCL_PACKAGE ${NV_LIBNCCL_PACKAGE_NAME}=${NV_LIBNCCL_PACKAGE_VERSION}+cuda11.8

RUN apt-get update && apt-get install -y --no-install-recommends \
    cuda-libraries-11-8=${NV_CUDA_LIB_VERSION} \
    ${NV_LIBNPP_PACKAGE} \
    cuda-nvtx-11-8=${NV_NVTX_VERSION} \
    libcusparse-11-8=${NV_LIBCUSPARSE_VERSION} \
    ${NV_LIBCUBLAS_PACKAGE} \
    ${NV_LIBNCCL_PACKAGE} \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Keep apt from auto upgrading the cublas and nccl packages. See https://gitlab.com/nvidia/container-images/cuda/-/issues/88
RUN apt-mark hold ${NV_LIBCUBLAS_PACKAGE_NAME} ${NV_LIBNCCL_PACKAGE_NAME}

################################################################################
# Install nvcc (Create an environment similar to the *-devel environment)
################################################################################
FROM runtime_env as devel_env

ENV NV_CUDA_LIB_VERSION "11.8.0-1"

ENV NV_CUDA_CUDART_DEV_VERSION 11.8.89-1
ENV NV_NVML_DEV_VERSION 11.8.86-1
ENV NV_LIBCUSPARSE_DEV_VERSION 11.7.5.86-1
ENV NV_LIBNPP_DEV_VERSION 11.8.0.86-1
ENV NV_LIBNPP_DEV_PACKAGE libnpp-dev-11-8=${NV_LIBNPP_DEV_VERSION}

ENV NV_LIBCUBLAS_DEV_VERSION 11.11.3.6-1
ENV NV_LIBCUBLAS_DEV_PACKAGE_NAME libcublas-dev-11-8
ENV NV_LIBCUBLAS_DEV_PACKAGE ${NV_LIBCUBLAS_DEV_PACKAGE_NAME}=${NV_LIBCUBLAS_DEV_VERSION}

ENV NV_NVPROF_VERSION 11.8.87-1
ENV NV_NVPROF_DEV_PACKAGE cuda-nvprof-11-8=${NV_NVPROF_VERSION}

ENV NV_LIBNCCL_DEV_PACKAGE_NAME libnccl-dev
ENV NV_LIBNCCL_DEV_PACKAGE_VERSION 2.15.5-1
ENV NCCL_VERSION 2.8.4-1
ENV NV_LIBNCCL_DEV_PACKAGE ${NV_LIBNCCL_DEV_PACKAGE_NAME}=${NV_LIBNCCL_DEV_PACKAGE_VERSION}+cuda11.8

RUN apt-get update && apt-get install -y --no-install-recommends \
    libtinfo5 libncursesw5 \
    cuda-cudart-dev-11-8=${NV_CUDA_CUDART_DEV_VERSION} \
    cuda-command-line-tools-11-8=${NV_CUDA_LIB_VERSION} \
    cuda-minimal-build-11-8=${NV_CUDA_LIB_VERSION} \
    cuda-libraries-dev-11-8=${NV_CUDA_LIB_VERSION} \
    cuda-nvml-dev-11-8=${NV_NVML_DEV_VERSION} \
    ${NV_NVPROF_DEV_PACKAGE} \
    ${NV_LIBNPP_DEV_PACKAGE} \
    libcusparse-dev-11-8=${NV_LIBCUSPARSE_DEV_VERSION} \
    ${NV_LIBCUBLAS_DEV_PACKAGE} \
    ${NV_LIBNCCL_DEV_PACKAGE} \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Keep apt from auto upgrading the cublas and nccl packages. See https://gitlab.com/nvidia/container-images/cuda/-/issues/88
RUN apt-mark hold ${NV_LIBCUBLAS_DEV_PACKAGE_NAME} ${NV_LIBNCCL_DEV_PACKAGE_NAME}
ENV LIBRARY_PATH /usr/local/cuda/lib64/stubs

################################################################################
# Install cudnn
################################################################################
FROM devel_env as devel_cudnn_env
ENV NV_CUDNN_VERSION 8.9.7.29
ENV NV_CUDNN_PACKAGE_NAME "libcudnn8"

ENV NV_CUDNN_PACKAGE "libcudnn8=$NV_CUDNN_VERSION-1+cuda11.8"
ENV NV_CUDNN_PACKAGE_DEV "libcudnn8-dev=$NV_CUDNN_VERSION-1+cuda11.8"

LABEL com.nvidia.cudnn.version="${NV_CUDNN_VERSION}"

RUN apt-get update && apt-get install -y --no-install-recommends \
    ${NV_CUDNN_PACKAGE} \
    ${NV_CUDNN_PACKAGE_DEV} \
    && apt-mark hold ${NV_CUDNN_PACKAGE_NAME} \
    && rm -rf /var/lib/apt/lists/*

################################################################################
# Install DRP-AI Extension Pack and dependencies
################################################################################
FROM devel_cudnn_env as dev_env

# Set environment varialble
ENV DEBIAN_FRONTEND noninteractive
ENV PYTHON_VERSION 3.10.16
ENV LANG C.UTF-8
ENV BASHRC /root/.bashrc
ENV PYENV_ROOT /usr/local/pyenv
ENV ENVIRONMENT_SETUP_PYENV /root/.environment-setup-pyenv
ENV VENV /py310
ENV DRPAI_EXT_ROOT /usr/local/drpai_ext
ENV PYTHONPATH $DRPAI_EXT_ROOT:$PYTHONPATH
ENV TF_USE_LEGACY_KERAS 1

# Install package to build python source
RUN apt-get update -y && \
    apt-get install -y --no-install-recommends \
    wget git unzip vim rsync tree libgl1-mesa-dev \
    build-essential libssl-dev zlib1g-dev \
    libbz2-dev libreadline-dev libsqlite3-dev llvm \
    libncursesw5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev cmake && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

# Set pyenv to install python
RUN git clone https://github.com/pyenv/pyenv.git $PYENV_ROOT

RUN touch $ENVIRONMENT_SETUP_PYENV && \
    echo 'export PYENV_ROOT="$PYENV_ROOT"' >> $ENVIRONMENT_SETUP_PYENV && \
    echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> $ENVIRONMENT_SETUP_PYENV && \
    echo 'eval "$(pyenv init -)"' >> $ENVIRONMENT_SETUP_PYENV && \
    echo '. $ENVIRONMENT_SETUP_PYENV' >> $BASHRC

# Install specified python and python library
RUN . $ENVIRONMENT_SETUP_PYENV && \
        pyenv install $PYTHON_VERSION && \
        pyenv shell $PYTHON_VERSION && \
        python3 -m venv $VENV && \
        echo '. $VENV/bin/activate' >> $BASHRC && \
        $VENV/bin/python3 -m pip install --upgrade pip && \
        $VENV/bin/python3 -m pip install --no-cache-dir \
                torch==1.13.1 \
                torchvision==0.14.1 \
                --extra-index-url https://download.pytorch.org/whl/cu118 && \
        $VENV/bin/python3 -m pip install --no-cache-dir \
                torchstat==0.0.7 \
                pandas==1.4.2 \
                onnx==1.14.0 \
                onnxruntime==1.18.1\
                mmcv==2.0.0

# Setup drpai extension pack
COPY ./drpai-extension-pack_ver*.tar.gz /tmp
RUN mkdir $DRPAI_EXT_ROOT && \
    tar xvf /tmp/drpai-extension-pack_ver*.tar.gz -C $DRPAI_EXT_ROOT && \
    rm /tmp/drpai-extension-pack_ver*.tar.gz

# Test installing drpai extension pack
RUN $VENV/bin/python3 -c "import drpai_compaction_tool; print(drpai_compaction_tool.__version__)"
