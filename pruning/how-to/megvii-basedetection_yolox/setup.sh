#!/bin/bash
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
# Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
#######################################################################################################################

set -eu

SCRIPT_FILE_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
cd $SCRIPT_FILE_DIR

function pr_info
{
  local message="[INFO] $@"
  echo "${message}"  
}

function pr_err
{
  local message="[ERROR] $@"
  echo "${message}"  
}


function validate_current_dir
{
    pr_info "Validate current direcotry..."

    if [ ! -d "patch" ]; then
        pr_err 'The `patch` folder could not be found.'
        pr_err 'Please make sure that the `patch` folder is in the `'$SCRIPT_FILE_DIR'` directory.'
        exit 1
    fi

    if [ -d "YOLOX" ]; then
        pr_err 'The `YOLOX` folder is found.'
        pr_err 'Please remove the `YOLOX` folder before running this script.'
        exit 1
    fi
}


function download_base_repo
{
    pr_info "Download the base repositories..."

    git clone -b 0.3.0 https://github.com/Megvii-BaseDetection/YOLOX.git
    cd YOLOX
}


function setup_python_lib
{
    pr_info "Setup python libraries..."

    python3 -m pip install cython matplotlib urllib3==1.21.1 \
                           scikit-learn==1.2.0 scikit-image==0.21.0 \
                           onnx==1.8.1 onnxruntime==1.8.0 wheel==0.40.0
    python3 -m pip install -r requirements.txt
    python3 -m pip install -e .
    python3 -m pip install protobuf==3.20.1 numpy==1.21.1
}


function patching_src
{
    pr_info "Patching source code..."

    cp tools/eval.py eval_with_pruned_model.py
    patch -u eval_with_pruned_model.py $SCRIPT_FILE_DIR/patch/eval_with_pruned_model.py.patch

    cp tools/export_onnx.py torch2onnx.py
    patch -u torch2onnx.py $SCRIPT_FILE_DIR/patch/torch2onnx.py.patch

    cp yolox/evaluators/voc_evaluator.py onnx_voc_evaluator.py
    patch -u onnx_voc_evaluator.py $SCRIPT_FILE_DIR/patch/onnx_voc_evaluator.py.patch

    echo -n > yolox_voc_s_pruning_conf.py
    patch -u yolox_voc_s_pruning_conf.py $SCRIPT_FILE_DIR/patch/yolox_voc_s_pruning_conf.py.patch
}

function main
{

    # Validate current directory
    validate_current_dir

    # Download base repository
    download_base_repo

    # Install python libraries
    setup_python_lib

    # Patching source
    patching_src

    pr_info "Success setup."
}


main "$@";