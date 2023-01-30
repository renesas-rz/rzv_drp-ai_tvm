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
* File Name    : measure_time.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef MEASURE_TIME_H
#define MEASURE_TIME_H

//#define TIME_PRINT_ON
/*****************************************
* Includes
******************************************/

#include <stdio.h>
#include <thread>

using namespace std;
class Measuretime
{
public:
    Measuretime(string itemname)
    {
        timespec_get(&start_time, TIME_UTC);
        item = itemname;
    }
    /**
     * @brief get_elapsed_ms
     * @details Get the elapsed ms
     * @return float
     */
    float get_elapsed_ms()
    {
        timespec tmp;
        timespec_get(&tmp, TIME_UTC);
        return  (tmp.tv_sec - start_time.tv_sec) * 1000.0 + (tmp.tv_nsec - start_time.tv_nsec) / 1000000.0;
    }
    ~Measuretime()
    {
#ifdef TIME_PRINT_ON
        timespec_get(&end_time, TIME_UTC);
        float diff_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0;
        printf("[MeasLog],%s, %.1f, [ms], id=>%d \n", item.c_str(), diff_time, std::this_thread::get_id());
#endif
    }
private:
    string item;
    timespec start_time;
    timespec end_time;

};
#endif
