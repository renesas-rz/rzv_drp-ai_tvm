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
* File Name    : tvm_drpai_emotionfp.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once

#ifndef DRP_TVM_MODEL_EMOTIONFP_H
#define DRP_TVM_MODEL_EMOTIONFP_H

/*****************************************
* Includes
******************************************/
#include "../irecognize_model.h"
#include "../../includes.h"
#include "../common/functions.h"
#include "../command/classification.h"
#include "../command/object_detection.h"
#include "opencv2/opencv.hpp"

class TVM_EmotionFP_DRPAI : public IRecognizeModel
{
private:
    constexpr static string_view TVM_MODEL_DIR = "emotion_fp_onnx";
    constexpr static int32_t TVM_DRPAI_IN_WIDTH = (640);
    constexpr static int32_t TVM_DRPAI_IN_HEIGHT = (480);
    constexpr static int32_t TVM_DRPAI_IN_CHANNEL = (2);

    /*GoogleNet Related*/
    constexpr static string_view MODEL_NAME = "DRP-AI TVM Emotion FER Plus (DRP-AI)";
    /*DRP-AI Input image information*/
    constexpr static int32_t TVM_MODEL_IN_C = (1);
    constexpr static int32_t TVM_MODEL_IN_W = (64);
    constexpr static int32_t TVM_MODEL_IN_H = (64);
    /*DRP-AI Output information*/
    constexpr static int32_t TVM_MODEL_OUT_C = (8);
    constexpr static int32_t TOP_NUM = (1);

public:
    TVM_EmotionFP_DRPAI();
    virtual int32_t inf_pre_process
        (uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size);
    virtual int32_t inf_post_process(float* arg);
    virtual shared_ptr<PredictNotifyBase> get_command();
    virtual int32_t print_result();

private:
    int8_t pre_process_cpu(uint8_t* input_data, float** output_buf, uint32_t* buf_size);
    int8_t post_process(std::map<float, int32_t>& result, float* floatarr);

private:
    int32_t num_class = TVM_MODEL_OUT_C;
    std::vector<std::string> emotion_table = 
    {
        "neutral", 
        "happiness", 
        "surprise", 
        "sadness", 
        "anger", 
        "disgust", 
        "fear", 
        "contempt"
    };
    /* OpenCV Mat for pre-processing */
    cv::Mat image;
    cv::Mat image_gray;
    cv::Mat image_resize;
    /* Variables required for Transpose&Normalize in pre-processing */
    std::vector<float> chw;

    /* Post-processing result */
    std::map<float, int32_t> postproc_result;
    std::vector<float> overall_result_prob;
    std::vector<int> overall_result_class;
};
#endif //DRP_TVM_MODEL_EMOTIONFP_H
