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
* File Name    : tvm_drpai_googlenet.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_googlenet.h"
TVM_GoogleNet_DRPAI::TVM_GoogleNet_DRPAI() : 
    IRecognizeModel(0, 
        TVM_MODEL_DIR.data(), MODEL_NAME.data(), 
        TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, 
        TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_GOOGLENET_DRPAI)
{
    preruntime.Load(pre_dir);
    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = TVM_DRPAI_IN_WIDTH;
    in_param.pre_in_shape_h = TVM_DRPAI_IN_HEIGHT;
    in_param.pre_in_format = INPUT_YUYV;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/scale */
    in_param.cof_add[0]= mean[0];
    in_param.cof_add[1]= mean[1];
    in_param.cof_add[2]= mean[2];
    in_param.cof_mul[0]= scale[0];
    in_param.cof_mul[1]= scale[1];
    in_param.cof_mul[2]= scale[2];
    
    /*Load label for GoogleNet */
    label_file_map = CommonFunc::load_label_file(LABEL_LIST.data());
    if (label_file_map.empty())
    {
        std::cerr << "[ERROR] Failed to load label file: "<<LABEL_LIST.data()<<std::endl;
    } 
    num_class = label_file_map.size();
    outBuffSize = num_class;
}

/**
 * @brief inf_pre_process
 * @details Run pre-processing.
 * @details For CPU input, use input_data for input data.
 * @details For DRP-AI input, use addr for input data stored address
 * @param input_data Input data pointer
 * @param width new input data width.
 * @param height new input data width.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_GoogleNet_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
{
    /*Update width and height*/
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
int32_t TVM_GoogleNet_DRPAI::inf_post_process(float* arg)
{
    postproc_result.clear();
    post_process(postproc_result, arg);
    return 0;
}
/**
 * @brief print_result
 * @details Print AI result on console
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_GoogleNet_DRPAI::print_result()
{
    int32_t result_cnt = 0;
    for (reverse_iterator it = postproc_result.rbegin(); it != postproc_result.rend(); it++)
    {
        result_cnt++;
        if (result_cnt > TOP_NUM) break;
        printf("  Top %d [%5.1f%%] : [%s]\n", result_cnt, it->first * 100, label_file_map[it->second].c_str());
    }
    return 0;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Result data
 */
shared_ptr<PredictNotifyBase> TVM_GoogleNet_DRPAI::get_command()
{
    Classification* ret = new Classification();
    int32_t cnt=0;
    classify_t dat;
    for (reverse_iterator it = postproc_result.rbegin(); it != postproc_result.rend(); it++)
    {
        if (cnt == TOP_NUM)break;
        cnt++;
        dat.names = label_file_map[it->second];
        dat.pred = it->first * 100;
        ret->predict.push_back(dat);
    }

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
int8_t TVM_GoogleNet_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, output_buf, buf_size);

#ifdef TENTATIVE
    /*RGB to BGR*/
    /*Run by CPU since currently DRP-AI Pre-processing Runtime does not support BGR output*/
    int32_t i = 0;
    int32_t j = 0;
    int32_t w = TVM_MODEL_IN_W;
    int32_t h = TVM_MODEL_IN_H;
    int32_t c = TVM_MODEL_IN_C;
    float tmp_val = 0;
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {
            /*Store R value in tmp_val*/
            tmp_val = (*output_buf)[i*w + j];
            /*R -> B value*/
            (*output_buf)[i*w + j] = (*output_buf)[2*h*w + i*w + j];
            /*B -> R value in tmp_val*/
            (*output_buf)[2*h*w + i*w + j] = tmp_val;
        }
    }
#endif
    return 0;
}


/**
 * @brief post_process
 * @details implementation post process
 * @param result reference to store the classification result
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_GoogleNet_DRPAI::post_process(std::map<float, int32_t>& result, float* floatarr)
{
    int32_t i = 0;

    /* Post-processing */
    /* Note that softmax has been done in ONNX inference. */
    /* Sort the score */
    for (i = 0; i < num_class; i++)
    {
        result[floatarr[i]] = i;
    }

    return 0;
}

