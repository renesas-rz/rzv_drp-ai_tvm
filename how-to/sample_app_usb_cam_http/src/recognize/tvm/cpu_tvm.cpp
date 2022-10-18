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
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : cpu_tvm.cpp
* Version      : 1.0.1
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "cpu_tvm.h"
CPU_TVMModel::CPU_TVMModel() : IRecognizeModel(128 * 1024, TVM_MODEL_DIR.data(), MODEL_NAME.data(), TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_DEEPPOSE_CPU)
{
    std::cout << "TVM model" << std::endl;
}

int32_t CPU_TVMModel::inf_post_process(float* arg)
{
    postproc_result.clear();
    post_process(postproc_result, arg);
    return 0;

}
int32_t CPU_TVMModel::print_result()
{
    /*Displays AI Inference results & Processing Time on console*/
    int32_t id = 0;
    for (pos_t p : postproc_result)
    {
        printf("  ID %d: (%d, %d)\n", id, p.X, p.Y);
        id++;
    }
    return 0;
}

shared_ptr<PredictNotifyBase> CPU_TVMModel::get_command()
{
    PoseDetection* ret = new PoseDetection();
    for (pos_t p : postproc_result)
    {
        ret->predict.push_back(p);
    }
    return shared_ptr<PredictNotifyBase>(move(ret));
}

/**
 * @brief post_process
 * @details implementation post process
 * @param result reference to store result(X,Y)
 * @param floatarr DRP-AI result
 * @return int8_t
 */
int8_t CPU_TVMModel::post_process(vector<pos_t> &result, float* floatarr)
{
    /* Render skeleton on image and print their details */
    int32_t posx = 0;
    int32_t posy = 0;
    result.clear();

    for (int i = 0; i < TVM_NUM_OUTPUT_KEYPNT; i++)
    {
        posx = (int32_t)(floatarr[2*i] * TVM_DRPAI_IN_WIDTH);
        posy = (int32_t)(floatarr[2*i+1] * TVM_DRPAI_IN_HEIGHT);

        pos_t p;
        p.X = posx;
        p.Y = posy;
        result.push_back(p);
    }

    return 0;
}
