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
* File Name    : functions.h
* Version      : 1.0.2
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H
/*****************************************
* Includes
******************************************/
#include "../../includes.h"
using namespace std;
class CommonFunc
{
public :
    /**
     * @brief sigmoid
     * @details  Helper function for YOLO Post Processing
     * @param x
     * @return double
     */
	static double sigmoid(double x)
	{
		return 1.0 / (1.0 + exp(-x));
	}

   /**
    * @brief softmax
    * @details Helper function for YOLO Post Processing
    * @param val array to be computed softmax
    * @param num_class number of classes
    */
    static void softmax(float* val, int32_t num_class)
    {
        float max_num = -INT_MAX;
        float sum = 0;
        int32_t i = 0;
        for (i = 0; i < num_class; i++)
        {
            max_num = std::max(max_num, val[i]);
        }

        for (i = 0; i < num_class; i++)
        {
            val[i] = (float)exp(val[i] - max_num);
            sum += val[i];
        }

        for (i = 0; i < num_class; i++)
        {
            val[i] = val[i] / sum;
        }
        return;
    }
};

#endif
