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
# Copyright (C) 2025 Renesas Electronics Corporation. All rights reserved.
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

    if [ -d "TopFormer" ]; then
        pr_err 'The `TopFormer` folder is found.'
        pr_err 'Please remove the `TopFormer` folder before running this script.'
        exit 1
    fi
}


function download_base_repo
{
    pr_info "Download the base repositories..."

    git clone https://github.com/hustvl/TopFormer.git
    cd TopFormer
    git checkout cc0f436
}


function setup_python_lib
{
    pr_info "Setup python libraries..."

    pip install -r requirements.txt 
    pip uninstall -y mmcv mmcv-full
    pip install yapf==0.30.0
    git clone https://github.com/open-mmlab/mmcv.git
    cd mmcv
    git checkout v1.4.8
    MMCV_WITH_OPS=1 FORCE_CUDA=1 pip install -e . --no-cache-dir
    cd ../
}


function patching_src
{
    pr_info "Patching source code..."
    # cd TopFormer
    cp ../__init__.py mmcv/mmcv/runner/hooks/__init__.py
    
    cp local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k.py \
       local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py    
    patch -u local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \
             $SCRIPT_FILE_DIR/patch/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py.patch

    cp tools/test.py  tools/eval_with_pruned_model.py
    patch -u tools/eval_with_pruned_model.py $SCRIPT_FILE_DIR/patch/eval_with_pruned_model.py.patch
    
    cp tools/deploy_test.py  tools/onnx_validate.py
    patch -u tools/onnx_validate.py $SCRIPT_FILE_DIR/patch/onnx_validate.py.patch
    
    cp tools/convert2onnx.py  tools/torch2onnx.py
    patch -u tools/torch2onnx.py $SCRIPT_FILE_DIR/patch/torch2onnx.py.patch
    
    echo -n > mmcv/mmcv/runner/hooks/pruning_hook.py
    patch -u mmcv/mmcv/runner/hooks/pruning_hook.py $SCRIPT_FILE_DIR/patch/pruning_hook.py.patch

    patch -u local_configs/topformer/topformer_base.py $SCRIPT_FILE_DIR/patch/topformer_base.py.patch
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