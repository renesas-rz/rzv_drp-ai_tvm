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
cd "$SCRIPT_FILE_DIR"

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

    if [ -d "ultralytics" ]; then
        pr_err 'The `ultralytics` folder is found.'
        pr_err 'Please remove the `ultralytics` folder before running this script.'
        exit 1
    fi
}


function download_base_repo
{
    pr_info "Download the base repositories..."

    git clone -b v8.3.163 https://github.com/ultralytics/ultralytics.git
    cd ultralytics
}


function setup_python_lib
{
    pr_info "Setup python libraries..."
    pip install -e .
    pip install onnxruntime-gpu==1.17.1 
}


function patching_src
{
    pr_info "Patching source code..."

    echo -n > ./retrain_with_pruning.py 
    patch -u ./retrain_with_pruning.py  $SCRIPT_FILE_DIR/patch/retrain_with_pruning.py.patch
    
    echo -n > ./torch2onnx.py 
    patch -u ./torch2onnx.py $SCRIPT_FILE_DIR/patch/torch2onnx.py.patch

    echo -n > ./eval_with_pruned_model.py 
    patch -u ./eval_with_pruned_model.py  $SCRIPT_FILE_DIR/patch/eval_with_pruned_model.py.patch

    echo -n > ./onnx_validate.py 
    patch -u ./onnx_validate.py  $SCRIPT_FILE_DIR/patch/onnx_validate.py.patch

    # Fix issue incompatible with pruning tool
    patch -u $SCRIPT_FILE_DIR/ultralytics/ultralytics/nn/autobackend.py $SCRIPT_FILE_DIR/patch/autobackend.py.patch
    patch -u $SCRIPT_FILE_DIR/ultralytics/ultralytics/nn/tasks.py $SCRIPT_FILE_DIR/patch/tasks.py.patch
    patch -u $SCRIPT_FILE_DIR/ultralytics/ultralytics/utils/torch_utils.py $SCRIPT_FILE_DIR/patch/torch_utils.py.patch
    patch -u $SCRIPT_FILE_DIR/ultralytics/ultralytics/utils/loss.py $SCRIPT_FILE_DIR/patch/loss.py.patch
    patch -u $SCRIPT_FILE_DIR/ultralytics/ultralytics/engine/trainer.py $SCRIPT_FILE_DIR/patch/trainer.py.patch
    
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
