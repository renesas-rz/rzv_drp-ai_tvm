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
* File Name    : system_analyzer.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "system_analyzer.h"

LinuxSystemAnalyzer::LinuxSystemAnalyzer()
{
    firstTick_ = true;
    preTime_ = times(NULL);
}

/**
 * @brief get_cpu_usage
 * @details Get cpu usag (each core)
 * @param nCPU cpu core num
 * @return std::string cpu usage separated by space. (e.g.  10 20)
 */
std::string LinuxSystemAnalyzer::get_cpu_usage(int32_t nCPU)
{
    std::string cpuUsage;
    int32_t usage;
    int32_t i;

    char buf[1024];
    int32_t usr;
    int32_t nice;
    int32_t sys;
    std::vector<int32_t> tmp(7);

    FILE* infile = fopen("/proc/stat", "r");
    if (NULL == infile)
    {
        std::cout << "[get_cpu_usage]<<Cannot open /proc/stat" << std::endl;
        return 0;
    }

    clock_t now = times(NULL);

    if (firstTick_ == true)
    {
        preTick_.resize(nCPU);

    }

    for (i = 0; i <= nCPU; i++)
    {
        int32_t result = fscanf(infile, "%s %d %d %d %d %d %d %d %d %d %d",
            buf, &usr, &nice, &sys, &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6]);

        if (result == -1)
        {
            std::cout << "[get_cpu_usage]<<Cannot read fscanf" << std::endl;
            return 0;
        }

        if (i == 0) {
            continue;
        }

        if (firstTick_ == true)
        {
            cpuUsage.append("0 ");
        }
        else
        {
            usage = ((double)(usr + nice + sys - preTick_[i - 1]) / (now - preTime_)) * 100.0;
            cpuUsage.append(std::to_string(usage));
            cpuUsage.append(" ");
        }

        preTick_[i - 1] = usr + nice + sys;
    }

    fclose(infile);

    if (firstTick_ == true)
    {
        firstTick_ = false;
    }

    preTime_ = now;

    return cpuUsage;
}


/**
 * @brief get_memory_usage
 * @details get memory usage (by all system)
 * @return uint32_t
 */
uint32_t LinuxSystemAnalyzer::get_memory_usage()
{
    struct sysinfo info;
    sysinfo(&info);

    unsigned long totalram = (info.totalram * info.mem_unit) / 1024;
    unsigned long freeram = (info.freeram * info.mem_unit) / 1024;

    uint32_t memoryUsage = (double)(totalram - freeram) / (double)totalram * 100;

    return memoryUsage;
}
