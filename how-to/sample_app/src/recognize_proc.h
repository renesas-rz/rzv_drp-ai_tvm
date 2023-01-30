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
* File Name    : recognize_proc.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#ifndef RECOGNIZEPROC_H
#define RECOGNIZEPROC_H

#pragma once
/*****************************************
* Includes
******************************************/
#include "includes.h"
#include "recognize/recognize_base.h"
#include "recognize/deeppose/tvm_drpai_deeppose.h"
#include "recognize/deeppose/tvm_cpu_deeppose.h"
#include "recognize/yolo/tvm_drpai_yolo.h"
#include "recognize/ultraface/tvm_drpai_ultraface.h"
#include "recognize/hrnet/tvm_drpai_hrnet.h"
#include "recognize/googlenet/tvm_drpai_googlenet.h"
#include "recognize/emotionfp/tvm_drpai_emotionfp.h"

using namespace std;
class RecognizeProc
{
public:
    RecognizeProc(shared_ptr<WebsocketServer> server)
    {
        _switch = true;
        p_recog_base = shared_ptr<RecognizeBase>(move(new RecognizeBase(server)));
    }
    void start_recognize();
    void stop_recognize();
    void switch_model(std::string model);
    void finish_recognize_thread() { blRunPredict = false; }
private:
    pthread_t thPredict;

    /* thread control */
    void predict_thread();
    static void* predict_thread_wrapper(void* object);
    static void run_predict(RecognizeProc* arg);
    bool _switch;

    shared_ptr<RecognizeBase> p_recog_base;

private:
    bool blRunPredict;
};

#endif
