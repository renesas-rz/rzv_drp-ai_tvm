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
* File Name    : tvm_drpai_ultraface.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_ultraface.h"
TVM_UltraFace_DRPAI::TVM_UltraFace_DRPAI() : IRecognizeModel(TVM_MODEL_OUT_SIZE, TVM_MODEL_DIR.data(), MODEL_NAME.data(), TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_ULTRAFACE_DRPAI)
{
    preruntime.Load(pre_dir);
    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = TVM_DRPAI_IN_WIDTH;
    in_param.pre_in_shape_h = TVM_DRPAI_IN_HEIGHT;
    in_param.pre_in_format = INPUT_YUYV;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/scale_factor */
    in_param.cof_add[0]= -1.0*mean[0];
    in_param.cof_add[1]= -1.0*mean[1];
    in_param.cof_add[2]= -1.0*mean[2];
    in_param.cof_mul[0]= scale_factor[0];
    in_param.cof_mul[1]= scale_factor[1];
    in_param.cof_mul[2]= scale_factor[2];

    model_dir = TVM_MODEL_DIR;
    std::cout << "DRP-AI TVM UltraFace model" << std::endl;
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
int32_t TVM_UltraFace_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
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
 * @details  implementation post process
 * @param arg
 * @return int32_t
 */
int32_t TVM_UltraFace_DRPAI::inf_post_process(float* arg)
{
    postproc_data.clear();
    post_process(postproc_data, arg);
    return 0;

}
/**
 * @brief print_result
 * @details Print AI result on console
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_UltraFace_DRPAI::print_result()
{
    ObjectDetectionFunc::print_boxes(postproc_data, label_file_map);
    return 0;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Result data
 */
shared_ptr<PredictNotifyBase> TVM_UltraFace_DRPAI::get_command()
{
    ObjectDetection* ret = new ObjectDetection();
    for (detection det : postproc_data)
    {
        if (det.prob == 0)
        {
            continue;
        }
        else
        {
            bbox_t dat;
            dat.name = label_file_map[det.c].c_str();
            dat.X = (int32_t)det.bbox.x;
            dat.Y = (int32_t)det.bbox.y;
            dat.W = (int32_t)det.bbox.w;
            dat.H = (int32_t)det.bbox.h;
            dat.pred = det.prob * 100.0;
            ret->predict.push_back(dat);
        }
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
int8_t TVM_UltraFace_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, output_buf, buf_size);
    return 0;
}
/**
 * @brief post_process
 * @details implementation post process
 * @param det reference to store bounding box
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_UltraFace_DRPAI::post_process(std::vector<detection>& det, float* floatarr)
{
    float* score = floatarr;
    float* box = floatarr + TVM_MODEL_OUT_NUM*TVM_MODEL_OUT_NUM_SCORE;
    detection d;
    float tl_x = 0;
    float tl_y = 0;
    float br_x = 0;
    float br_y = 0;
    float width = 0;
    float height = 0;
    float probs = 0;
    det.clear();
    for (int i = 0;i<TVM_MODEL_OUT_NUM;i++)
    {
        for (int j = 1;j<TVM_MODEL_OUT_NUM_SCORE;j++)
        {
            probs =  score[i*TVM_MODEL_OUT_NUM_SCORE+j];
            if (probs > ULTRAFACE_TH_SCORE)
            {
                tl_x = round(box[i*4  ]*TVM_DRPAI_IN_WIDTH);
                tl_y = round(box[i*4+1]*TVM_DRPAI_IN_HEIGHT);
                br_x = round(box[i*4+2]*TVM_DRPAI_IN_WIDTH);
                br_y = round(box[i*4+3]*TVM_DRPAI_IN_HEIGHT);
                width = br_x - tl_x;
                height = br_y - tl_y;
                Box bb = { tl_x, tl_y, width, height };
                d = { bb, 0, probs };
                det.push_back(d);
            }
        }
    }
    /* Non-Maximum Supression filter */
    filter_boxes_nms(det, det.size(), ULTRAFACE_TH_NMS);

    /*For pre face detection */
    /*Note that for running single UltraFace, this process is not required. */
    detected_data.resize(det.size());
    std::copy(det.begin(), det.end(), detected_data.begin());
    
    return 0;
}
