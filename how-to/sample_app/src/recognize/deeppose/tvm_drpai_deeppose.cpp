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
* File Name    : tvm_drpai_deeppose.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_deeppose.h"
TVM_DeepPose_DRPAI::TVM_DeepPose_DRPAI() :
    IRecognizeModel(TVM_NUM_OUTPUT_KEYPNT*TVM_NUM_OUTPUT_COORD_NUM*sizeof(float),
        TVM_MODEL_DIR.data(), MODEL_NAME.data(),
        TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL,
        TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_DEEPPOSE_DRPAI)
{
    /*Initialization for DRP-AI Pre-processing*/
    preruntime.Load(pre_dir);
    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = TVM_DRPAI_IN_WIDTH;
    in_param.pre_in_shape_h = TVM_DRPAI_IN_HEIGHT;
    in_param.pre_in_format = INPUT_YUYV;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
    in_param.cof_add[0]= -255*mean[0];//-123.675;
    in_param.cof_add[1]= -255*mean[1];//-116.28;
    in_param.cof_add[2]= -255*mean[2];//-103.53;
    in_param.cof_mul[0]= 1/(stdev[0]*255);//0.01712475;
    in_param.cof_mul[1]= 1/(stdev[1]*255);//0.017507;
    in_param.cof_mul[2]= 1/(stdev[2]*255);//0.01742919;
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
int32_t TVM_DeepPose_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
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
int32_t TVM_DeepPose_DRPAI::inf_post_process(float* arg)
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
int32_t TVM_DeepPose_DRPAI::print_result()
{
    /*Displays AI Inference results on console*/
    int32_t id = 0;
    for (pos_t p : postproc_result)
    {
        printf("  ID %d: (%d, %d)\n", id, p.X, p.Y);
        id++;
    }
    return 0;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> result data
 */
shared_ptr<PredictNotifyBase> TVM_DeepPose_DRPAI::get_command()
{
    PoseDetection* ret = new PoseDetection();
    for (pos_t p : postproc_result)
    {
        ret->predict.push_back(p);
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
int8_t TVM_DeepPose_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, output_buf, buf_size);
    return 0;
}
/**
 * @brief post_process
 * @details implementation post process
 * @param result reference to store result(X,Y)
 * @param floatarr inference result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_DeepPose_DRPAI::post_process(vector<pos_t> &result, float* floatarr)
{
    int32_t posx = 0;
    int32_t posy = 0;
    pos_t p;
    result.clear();

    for (int i = 0; i < TVM_NUM_OUTPUT_KEYPNT; i++)
    {
        posx = (int32_t)(floatarr[2*i] * TVM_DRPAI_IN_WIDTH);
        posy = (int32_t)(floatarr[2*i+1] * TVM_DRPAI_IN_HEIGHT);

        p.X = posx;
        p.Y = posy;
        result.push_back(p);
    }

    return 0;
}
