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
* File Name    : predict_notify_base.h
* Version      : 1.0.2
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef PREDICT_NOTIFY_BASE_H
#define PREDICT_NOTIFY_BASE_H
/*****************************************
* Includes
******************************************/

#include "../includes.h"
#include "command_base.h"
using namespace std;
class PredictNotifyBase :public CommandBase
{
public:
    PredictNotifyBase(string name) :CommandBase(name) {}
    virtual ~PredictNotifyBase() {}
    virtual string CreateRequest(void)
    {
        return CommandCreateHelper::SerializeCommandBody<PredictNotifyBase>(*this);
    }

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(CEREAL_NVP(img),
            CEREAL_NVP(img_org_w),
            CEREAL_NVP(img_org_h),
            CEREAL_NVP(drp_time),
            CEREAL_NVP(post_time),
            CEREAL_NVP(pre_time));
    }

public:
    string img;
    int32_t img_org_w;
    int32_t img_org_h;
    float drp_time;
    float post_time;
    float pre_time;
};

#endif
