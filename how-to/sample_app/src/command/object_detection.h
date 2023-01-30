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
* File Name    : object_detection.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

/*****************************************
* Includes
******************************************/

#include "predict_notify_base.h"
#include "bbox_t.h"

using namespace std;
class ObjectDetection :public PredictNotifyBase
{
public:
    ObjectDetection() :PredictNotifyBase("object_detection")
    {

    }
    virtual ~ObjectDetection() {}

    virtual std::string CreateRequest(void)
    {
        return CommandCreateHelper::SerializeCommandBody<ObjectDetection>(*this);
    }
    template<class Archive>
    void save(Archive& archive) const
    {
        /* regist params */
        archive(
            CEREAL_NVP(img),
            CEREAL_NVP(img_org_w),
            CEREAL_NVP(img_org_h),
            CEREAL_NVP(predict),
            CEREAL_NVP(drp_time),
            CEREAL_NVP(post_time),
            CEREAL_NVP(pre_time));
    }



public:
    vector<bbox_t> predict;
};

#endif
