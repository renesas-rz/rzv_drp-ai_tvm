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
* File Name    : tvm_drpai_emotionfp.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_emotionfp.h"
TVM_EmotionFP_DRPAI::TVM_EmotionFP_DRPAI() : 
    IRecognizeModel(TVM_MODEL_OUT_C, 
        TVM_MODEL_DIR.data(), MODEL_NAME.data(), 
        TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, 
        TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_EMOTIONFP_DRPAI)
{
    /*Initialize opencv container*/
    image.create(TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_WIDTH, CV_8UC2);
    image_gray.create(TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_WIDTH, CV_8UC1);
    image_resize.create(TVM_MODEL_IN_H, TVM_MODEL_IN_W, CV_8UC1);
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
int32_t TVM_EmotionFP_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
{
    /*Update width and height*/
    if ((width != _capture_w) || (height != _capture_h)) 
    {
        _capture_w = width;
        _capture_h = height;
        image.release();
        image.create(_capture_h, _capture_w, CV_8UC2);
        image_gray.release();
        image_gray.create(_capture_h, _capture_w, CV_8UC1);
    }

    pre_process_cpu(input_data, arg, buf_size);
    return 0;
}
/**
 * @brief inf_post_process
 * @details Run post-processing
 * @param arg Inference output data pointer
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_EmotionFP_DRPAI::inf_post_process(float* arg)
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
int32_t TVM_EmotionFP_DRPAI::print_result()
{
    float x, y, w, h;
    uint32_t i = 0;
    detection det;
    std::string name;
    for (i = 0;i<detected_data.size();i++)
    {
        det = detected_data[i];
        x = det.bbox.x;
        y = det.bbox.y;
        w = det.bbox.w;
        h = det.bbox.h;
        printf(" Bounding Box Number: %d\n", i+1);
        printf(" Bounding Box       : (X, Y, W, H) = (%.0f, %.0f, %.0f, %.0f)\n", x, y, w, h);
        name = emotion_table[overall_result_class[i]];
        printf("\x1b[32;1m Detected  Class    : %s (%d)\n\x1b[0m", name.c_str(), overall_result_class[i]);
        printf("  Probability       : %5.1f %%\n", overall_result_prob[i] * 100);
        i++;
    }
    printf(" Bounding Box Count   : %d\n", i+1);
    return 0;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Result data
 */
shared_ptr<PredictNotifyBase> TVM_EmotionFP_DRPAI::get_command()
{
    ObjectDetection* ret = new ObjectDetection();
    detection det;
    bbox_t dat;
    /*Prepare the command*/
    for (int32_t i = 0;i<detected_data.size();i++)
    {
        det = detected_data[i];
        dat.X = (int32_t)det.bbox.x;
        dat.Y = (int32_t)det.bbox.y;
        dat.W = (int32_t)det.bbox.w;
        dat.H = (int32_t)det.bbox.h;
        dat.name = emotion_table[overall_result_class[i]];
        dat.pred = overall_result_prob[i]* 100; //Use the classification score as probability
        ret->predict.push_back(dat);
    }
    /*Clear overall vectors.*/
    overall_result_prob.clear();
    overall_result_class.clear();
    return shared_ptr<PredictNotifyBase>(move(ret));
}

/**
 * @brief pre_process_cpu
 * @details implementation pre process for OpenCV
 * @param input_data input data buffer
 * @param out output_buf output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_EmotionFP_DRPAI::pre_process_cpu(uint8_t* input_data, float** output_buf, uint32_t* buf_size)
{
    float val = 0;
    chw.clear();
    /*Loop variant*/
    int32_t c = 0;
    int32_t y = 0;
    int32_t x = 0;

    /*Load input image to opencv container*/
    image = cv::Mat(_capture_h, _capture_w, CV_8UC2, (void*)input_data);
    /*Color conversion*/
    cv::cvtColor(image, image_gray, cv::COLOR_YUV2GRAY_YUYV);
    /*Resize*/
    cv::resize(image_gray, image_resize, cv::Size(_model_w, _model_h), 0, 0, cv::INTER_AREA);
    /*Cast to float and Transpose*/
    for (c = 0; c < _model_c ; c++)
    {
        for (y = 0; y < _model_h ; y++)
        {
            for (x = 0; x < _model_w ; x++)
            {
                val = (float) image_resize.at<uchar>(y, x);
                chw.push_back(val);
            }
        }
    }
    /*Copy output pointer to output_buf*/
    *output_buf = chw.data();
    return 0;
}

/**
 * @brief post_process
 * @details implementation post process
 * @param result reference to store the classification result
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_EmotionFP_DRPAI::post_process(std::map<float, int32_t>& result, float* floatarr)
{
    int32_t i = 0;
    /* Post-processing */
    CommonFunc::softmax(floatarr, num_class);
    /* Sort the score */
    for (i = 0; i < num_class; i++)
    {
        result[floatarr[i]] = i;
    }

    /*Store the top result into overall vectors.*/
    i = 0;
    for (reverse_iterator it = result.rbegin(); it != result.rend(); it++)        
    {
        if (i >= TOP_NUM) break;
        overall_result_prob.push_back(it->first);
        overall_result_class.push_back(it->second);
        i++;
    }
    return 0;
}

