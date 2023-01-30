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
* File Name    : recognize_proc.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include <iostream>
#include "recognize_proc.h"

/**
 * @brief start_recognize
 * @details Start recognize proc thread
 */
void RecognizeProc::start_recognize()
{
    predict_thread();
}

/**
 * @brief stop_recognize
 * @details Stop recognize proc thread
 */
void RecognizeProc::stop_recognize()
{
    p_recog_base->recognize_end();
}

/**
 * @brief switch DRP-AI Model
 * @details Finish current recognize and start next recognize thread.
 * @param model model name
 */
void RecognizeProc::switch_model(std::string model)
{
    std::cout << "model : " << model << std::endl;
    if ("TVM_DRPAI_DEEPPOSE" == model)
    {
        p_recog_base->initialize(new TVM_DeepPose_DRPAI());
    }
    else if ("TVM_CPU_DEEPPOSE" == model)
    {
        p_recog_base->initialize(new TVM_DeepPose_CPU());
    }
    else if ("TVM_DRPAI_YOLOV3" == model)
    {
        p_recog_base->initialize(new TVM_YOLO_DRPAI(MODE_TVM_YOLOV3_DRPAI));
    }
    else if ("TVM_DRPAI_TINYYOLOV3" == model)
    {
        p_recog_base->initialize(new TVM_YOLO_DRPAI(MODE_TVM_TINYYOLOV3_DRPAI));
    }
    else if ("TVM_DRPAI_YOLOV2" == model)
    {
        p_recog_base->initialize(new TVM_YOLO_DRPAI(MODE_TVM_YOLOV2_DRPAI));
    }
    else if ("TVM_DRPAI_TINYYOLOV2" == model)
    {
        p_recog_base->initialize(new TVM_YOLO_DRPAI(MODE_TVM_TINYYOLOV2_DRPAI));
    }
    else if ("TVM_DRPAI_ULTRAFACE" == model)
    {
        p_recog_base->initialize(new TVM_UltraFace_DRPAI());
    }
    else if ("TVM_DRPAI_HRNET" == model)
    {
        p_recog_base->initialize(new TVM_HRNET_DRPAI(MODE_TVM_HRNET_DRPAI));
    }
    else if ("TVM_DRPAI_HRNETV2" == model)
    {
        p_recog_base->initialize(new TVM_HRNET_DRPAI(MODE_TVM_HRNETV2_DRPAI));
    }
    else if ("TVM_DRPAI_GOOGLENET" == model)
    {
        p_recog_base->initialize(new TVM_GoogleNet_DRPAI());
    }
    else if ("TVM_DRPAI_EMOTIONFP" == model)
    {
        /* Run face detection and then run emotion estimation */
        p_recog_base->initialize(new TVM_UltraFace_DRPAI(), new TVM_EmotionFP_DRPAI());
    }

    p_recog_base->recognize_start();

}

/**
 * @brief run predict thread sync
 * @details Predict thread start and wait
 */
void RecognizeProc::predict_thread()
{
    blRunPredict = true;
    pthread_create(&thPredict, NULL, &predict_thread_wrapper, this);
    pthread_join(thPredict, NULL);
}

/**
 * @brief thread wrapper
 * @details thread proc wrapper
 * @param object pointer to itself
 * @return void*
 */
void* RecognizeProc::predict_thread_wrapper(void* object)
{
    run_predict(reinterpret_cast<RecognizeProc*>(object));
    return NULL;
}

/**
 * @brief wait for predict flag to false
 * @details keep alive while recognizee running.
 * @param arg pointer to itself
 */
void RecognizeProc::run_predict(RecognizeProc* arg)
{
    while (arg->blRunPredict)
    {
        usleep(100000);
    }
    std::cout << "All Finish" << std::endl;

}
