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
* File Name    : tvm_drpai_ultraface.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once

#ifndef DRP_TVM_MODEL_ULTRAFACE_H
#define DRP_TVM_MODEL_ULTRAFACE_H

/*****************************************
* Includes
******************************************/
#include "../irecognize_model.h"
#include "../../includes.h"
#include "../common/box.h"
#include "../common/functions.h"
#include "../common/object_detection.h"
#include "../common/PreRuntime.h"
#include "../command/object_detection.h"

class TVM_UltraFace_DRPAI : public IRecognizeModel
{
private:
    constexpr static string_view TVM_MODEL_DIR = "ultraface_onnx";
    constexpr static  int32_t TVM_DRPAI_IN_WIDTH = (640);
    constexpr static  int32_t TVM_DRPAI_IN_HEIGHT = (480);

    /*UltraFace*/
    constexpr static string_view MODEL_NAME = "DRP-AI TVM UltraFace (DRP-AI)";

    /*DRP-AI Input image information*/
    constexpr static int32_t TVM_DRPAI_IN_CHANNEL = (2);
    constexpr static int32_t  TVM_MODEL_IN_C = (3);
    constexpr static int32_t  TVM_MODEL_IN_W = (320);
    constexpr static int32_t  TVM_MODEL_IN_H = (240);
    constexpr static int32_t  TVM_MODEL_OUT_NUM = (4420);
    constexpr static int32_t  TVM_MODEL_OUT_NUM_SCORE = (2);
    constexpr static int32_t  TVM_MODEL_OUT_NUM_BOX = (4);
    constexpr static int32_t  TVM_MODEL_OUT_SIZE = (TVM_MODEL_OUT_NUM * TVM_MODEL_OUT_NUM_SCORE + TVM_MODEL_OUT_NUM * TVM_MODEL_OUT_NUM_BOX)* sizeof(float);

    constexpr static float ULTRAFACE_TH_SCORE   = (0.7);//from ONNX Model Zoo
    constexpr static float ULTRAFACE_TH_NMS     = (0.5);//from ONNX Model Zoo
public:
    TVM_UltraFace_DRPAI();
    virtual int32_t inf_pre_process
        (uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size);
    virtual int32_t inf_post_process(float* arg);
    virtual shared_ptr<PredictNotifyBase> get_command();
    virtual int32_t print_result();

private:
    int8_t pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size);
    int8_t post_process(std::vector<detection>& det, float* floatarr);

private:
    /* Pre-processing Runtime variables for pre-processing */
    PreRuntime preruntime;
    s_preproc_param_t in_param;
    const std::string pre_dir = "preprocess_tvm_v2ma";
    float mean[3] = { 127.0, 127.0, 127.0 };
    float scale_factor[3] = { 1.0/128.0, 1.0/128.0, 1.0/128.0 };

    /* Class labels to be classified */
    /* For UltraFace, use empty label.*/
    std::vector<std::string> label_file_map =
    {
        ""
    };
    /* Post-processing result */
    vector<detection> postproc_data;

};

#endif //DRP_TVM_MODEL_ULTRAFACE_H
