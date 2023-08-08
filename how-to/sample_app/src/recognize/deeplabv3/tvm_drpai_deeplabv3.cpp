/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : tvm_drpai_deeplabv3.cpp
* Version      : 1.1.1
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_deeplabv3.h"
#include <cfloat>
TVM_DeepLabv3_DRPAI::TVM_DeepLabv3_DRPAI() : IRecognizeModel(0, TVM_MODEL_DIR_DEEPLABV3.data(), MODEL_NAME.data(), TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_DEEPLABV3_DRPAI)
{
    int8_t ret = 0; // Exit status
    constructor_err = 0;

    /*Load label list for Deeplabv3 */
    label_file_map = CommonFunc::load_label_file(LABEL_LIST.data());

    if (label_file_map.empty())
    {
        constructor_err = -1;
        err_str = "[ERROR] Failed to load label file: "+ std::string(LABEL_LIST.data());
        /*Error will be caught at RecognizeBase::recognize_start()*/
    }

    num_class = label_file_map.size();
    /* init num_buffer*/
    outBuffSize = num_class * TVM_MODEL_IN_W * TVM_MODEL_IN_H * TVM_MODEL_OUT_NUM;
    /*Load preprocess runtime*/
    model_dir = TVM_MODEL_DIR_DEEPLABV3;
    pre_dir = model_dir + pre_dir;
    ret = preruntime.Load(pre_dir);
    /*Init post-process containers*/
    for (int i = 0; i < postproc_data.size(); i++) { postproc_data[i] = 0; }

    if (0 != ret && 0 == constructor_err)
    {
        constructor_err = -1;
        err_str = "[ERROR] Failed to load DRP-AI Pre-processing Runtime Object: " + pre_dir;
        err_str = err_str + ".\nPrepare the Pre-processing Runtime Object";
        err_str = err_str + " according to the GitHub (https://github.com/renesas-rz/rzv_drp-ai_tvm).";
        /*Error will be caught at RecognizeBase::recognize_start()*/
    }

    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = TVM_DRPAI_IN_WIDTH;
    in_param.pre_in_shape_h = TVM_DRPAI_IN_HEIGHT;
    in_param.pre_in_format = FORMAT_YUYV_422;
    in_param.pre_out_format = FORMAT_RGB;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
    in_param.cof_add[0] = -255 * mean[0];
    in_param.cof_add[1] = -255 * mean[1];
    in_param.cof_add[2] = -255 * mean[2];
    in_param.cof_mul[0] = 1 / (stdev[0] * 255);
    in_param.cof_mul[1] = 1 / (stdev[1] * 255);
    in_param.cof_mul[2] = 1 / (stdev[2] * 255);
}

/**
 * @brief inf_pre_process
 * @details Run pre-processing.
 * @details For CPU input, use input_data for input data.
 * @details For DRP-AI input, use addr for input data stored address
 * @param input_data Input data pointer
 * @param width new input data width.
 * @param height new input data height.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_DeepLabv3_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
{
    if ((width != _capture_w) || (height != _capture_h))
    {
        _capture_w = width;
        _capture_h = height;
        in_param.pre_in_shape_w = _capture_w;
        in_param.pre_in_shape_h = _capture_h;
    }

    pre_process_drpai(addr, arg, buf_size);
    return 0;
}

/**
 * @brief inf_post_process
 * @details Run post-processing
 * @param arg Inference output data pointer
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_DeepLabv3_DRPAI::inf_post_process(float* arg)
{
    int32_t ret = 0;
    detected_labels.clear();
    ret = post_process(arg);
    return ret;
}

/**
 * @brief print_result
 * @details Print AI result on console
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_DeepLabv3_DRPAI::print_result()
{
    for (int i = 0; i < detected_labels.size(); i++)
    {
        printf("Class : %s\n", detected_labels[i].c_str());
    }

    return 0;
}

/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Result data
 */
shared_ptr<PredictNotifyBase> TVM_DeepLabv3_DRPAI::get_command()
{
    SemanticSegmentation* ret = new SemanticSegmentation();
    ret->predict = lwsock::b64encode(postproc_data.data(), postproc_data.size());
    ret->names = detected_labels;
    return shared_ptr<PredictNotifyBase>(move(ret));
}

/**
 * @brief pre_process_drpai
 * @details implementation pre process using Pre-processing Runtime.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_DeepLabv3_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    int8_t ret = 0;
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    ret = preruntime.Pre(&in_param, (void**) output_buf, buf_size);

    if (0 != ret)
    {
        std::cerr << "[ERROR] Failed to run DRP-AI Pre-processing Runtime."<<std::endl;
        return -1;
    }

    return 0;
}

/**
 * @brief post_process
 * @details implementation post process
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_DeepLabv3_DRPAI::post_process(float* floatarr)
{
    /*Check detected labels*/
    bool* is_detected_labels = new bool[num_class];
    for (int i = 0; i < num_class; i++) { is_detected_labels[i] = false; }

    /*Convert output probabilities to label indice array*/
    int offset = TVM_MODEL_IN_W * TVM_MODEL_IN_H * num_class; // Skip loss outputs

    for (int i = 0; i < TVM_MODEL_IN_W * TVM_MODEL_IN_H; i++) {
        /*Get maximum probability index*/
        uint8_t tmp_max_idx = 0;
        float tmp_max_value = -FLT_MAX;

        for (int ci = 0; ci < num_class; ci++) {
            if (tmp_max_value < floatarr[ci * TVM_MODEL_IN_W * TVM_MODEL_IN_H + i + offset]) {
                tmp_max_value = floatarr[ci * TVM_MODEL_IN_W * TVM_MODEL_IN_H + i + offset];
                tmp_max_idx = ci;
            }
        }
        
        postproc_data[i] = tmp_max_idx;

        /*Check if label index is not background*/
        if (tmp_max_idx > 0) {
            is_detected_labels[tmp_max_idx] = true;
        }
    }

    /*Store detected labels (ignore background label)*/
    for (int i = 1; i < num_class; i++) {
        if (is_detected_labels[i]) {
            detected_labels.push_back(label_file_map[i]);
        }
    }

    delete[] is_detected_labels;

    return 0;
}
