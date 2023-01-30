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
* File Name    : tvm_drpai_hrnet.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once

#ifndef DRP_TVM_MODEL_HRNET_H
#define DRP_TVM_MODEL_HRNET_H

/*****************************************
* Includes
******************************************/
#include "../irecognize_model.h"
#include "../../includes.h"
#include "../command/pose_detection.h"
#include "../common/PreRuntime.h"

/*****************************************
* Static Variables and Macro for HRNet
******************************************/

class TVM_HRNET_DRPAI : public IRecognizeModel
{
private:
    /*Common*/
    constexpr static  int32_t TVM_DRPAI_IN_WIDTH = (640);
    constexpr static  int32_t TVM_DRPAI_IN_HEIGHT = (480);

    /*HRNet Related*/
    constexpr static string_view TVM_MODEL_DIR = "hrnet_onnx";
    constexpr static string_view MODEL_NAME = "DRP-AI TVM HRNet (DRP-AI)";
    constexpr static  int32_t NUM_OUTPUT_C = (17);
    constexpr static  int32_t NUM_OUTPUT_W = (48);
    constexpr static  int32_t NUM_OUTPUT_H = (64);
    constexpr static  float TH_KPT = (0.1f);

    /*HRNetV2 Related*/
    constexpr static string_view TVM_MODEL_DIR_V2 = "hrnetv2_pt";
    constexpr static string_view MODEL_NAME_V2 = "DRP-AI TVM HRNetV2 (DRP-AI)";
    constexpr static  int32_t NUM_OUTPUT_C_V2 = (21);
    constexpr static  int32_t NUM_OUTPUT_W_V2 = (64);
    constexpr static  int32_t NUM_OUTPUT_H_V2 = (64);
    constexpr static  float TH_KPT_V2 = (0.15f);

    /*DRP-AI Input image information*/
    /*Common*/
    constexpr static int32_t TVM_DRPAI_IN_CHANNEL = (2);
    /*HRNet*/
    constexpr static int32_t  TVM_MODEL_IN_C = (3);
    constexpr static int32_t  TVM_MODEL_IN_W = (192);
    constexpr static int32_t  TVM_MODEL_IN_H = (256);
    /*HRNetv2*/
    constexpr static int32_t  TVM_MODEL_IN_C_V2 = (3);
    constexpr static int32_t  TVM_MODEL_IN_W_V2 = (256);
    constexpr static int32_t  TVM_MODEL_IN_H_V2 = (256);

    /*Cropping Image Related*/
    /*Common*/
    constexpr static int32_t YUY2_NUM_CHANNEL   = (2);
    constexpr static int32_t YUY2_NUM_DATA      = (4);
    constexpr static  float HRNET_CROPPED_IMAGE_WIDTH  = (TVM_DRPAI_IN_WIDTH);
    constexpr static  float HRNET_CROPPED_IMAGE_HEIGHT = (TVM_DRPAI_IN_HEIGHT);
    /*HRNet*/
    constexpr static int32_t CROPPED_IMAGE_LEFT = (184); //Only Even numbers can be set  
    constexpr static int32_t CROPPED_IMAGE_TOP = (0);  
    constexpr static int32_t CROPPED_IMAGE_WIDTH = (270); 
    constexpr static int32_t CROPPED_IMAGE_HEIGHT = (TVM_DRPAI_IN_HEIGHT); 
    /*HRNetv2*/
    constexpr static int32_t CROPPED_IMAGE_LEFT_V2 = (80); //Only Even numbers can be set
    constexpr static int32_t CROPPED_IMAGE_TOP_V2 = (0);
    constexpr static int32_t CROPPED_IMAGE_WIDTH_V2 = (480);
    constexpr static int32_t CROPPED_IMAGE_HEIGHT_V2 = (TVM_DRPAI_IN_HEIGHT);

    /*Post Processing & Drawing Related*/
    /*HRNet*/
    constexpr static  float OUTPUT_LEFT = (276 * (TVM_DRPAI_IN_WIDTH / 960.0f));
    constexpr static  float OUTPUT_TOP = (0);
    constexpr static  float OUTPUT_WIDTH = (405 * (TVM_DRPAI_IN_WIDTH / 960.0f));
    constexpr static  float OUTPUT_HEIGHT = (TVM_DRPAI_IN_HEIGHT); 
    constexpr static  float OUTPUT_ADJ_X = (2);
    constexpr static  float OUTPUT_ADJ_Y = (0);
    /*HRNetv2*/
    constexpr static  float OUTPUT_LEFT_V2 = (120 * (TVM_DRPAI_IN_WIDTH / 960.0f));
    constexpr static  float OUTPUT_TOP_V2 = (0);
    constexpr static  float OUTPUT_WIDTH_V2 = (720 * (TVM_DRPAI_IN_WIDTH / 960.0f));
    constexpr static  float OUTPUT_HEIGHT_V2 = (TVM_DRPAI_IN_HEIGHT);
    constexpr static  float OUTPUT_ADJ_X_V2 = (2);
    constexpr static  float OUTPUT_ADJ_Y_V2 = (0);
 

public:
    TVM_HRNET_DRPAI();
    TVM_HRNET_DRPAI(uint8_t id);
#ifdef TENTATIVE
    ~TVM_HRNET_DRPAI();
#endif
    virtual int32_t inf_pre_process
    (uint8_t* input_data, uint32_t width, uint32_t height, uint32_t addr, float** arg, uint32_t* buf_size);
    virtual int32_t inf_post_process(float* arg);
    virtual shared_ptr<PredictNotifyBase> get_command();
    virtual int32_t print_result();

    int32_t hrnet_offset(int32_t b, int32_t y, int32_t x);

private:
    int8_t pre_process(uint8_t* input_data, uint32_t addr, float** output_buf, uint32_t* buf_size);
    int8_t post_process(vector<pos_t>& result, float* floatarr);

private:
    /* Pre-processing Runtime variables for pre-processing */
    PreRuntime preruntime;
    s_preproc_param_t in_param;
    const std::string pre_dir = "preprocess_tvm_v2ma";
    float mean[3] = { 0.485, 0.456, 0.406 };
    float stdev[3] = { 0.229, 0.224, 0.225 };

    int8_t sign(int32_t x);
    void coord_convert(vector<pos_t>& result, vector<vector<float>>& preds);
#ifdef TENTATIVE
    int8_t udmabuf_fd = 0;
    uint8_t * crop_out_ptr;
    uint32_t size = 0;
    uint64_t udmabuf_crop_addr = 0; //required for tentative cropping
#endif
    /* Post-processing result */
    vector<pos_t> postproc_result;
    /* Number of DRP-AI output */
    uint32_t num_inf_out;

    int32_t pre_cropped_image_left;
    int32_t pre_cropped_image_top;
    int32_t pre_cropped_image_width;
    int32_t pre_cropped_image_height; 

    int32_t hrnet_num_output_c;
    int32_t hrnet_num_output_w;
    int32_t hrnet_num_output_h;
    float hrnet_th_kpt;

    float hrnet_output_width;
    float hrnet_output_left;
    float hrnet_output_adj_x;
    float hrnet_output_height;
    float hrnet_output_top;
    float hrnet_output_adj_y;
};

#endif //DRP_TVM_MODEL_HRNET_H
