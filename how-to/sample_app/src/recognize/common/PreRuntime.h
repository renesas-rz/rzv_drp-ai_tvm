/*
 * Original Code (C) Copyright Renesas Electronics Corporation 2023
 *　
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

/***********************************************************************************************************************
* File Name    : PreRuntime.h
* Version      : 0.10
* Description  : PreRuntime Header file
***********************************************************************************************************************/
#pragma once

#ifndef PRERUNTIME_H
#define PRERUNTIME_H
/***********************************************************************************************************************
* Include
***********************************************************************************************************************/
#include <linux/drpai.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <cstring>
#include <float.h>
#include <signal.h>
#include <cmath>

#include <builtin_fp16.h>
/***********************************************************************************************************************
* Macro
***********************************************************************************************************************/
#define BUF_SIZE        (1024)
#define INDEX_I         (0)
#define INDEX_D         (1)
#define INDEX_C         (2)
#define INDEX_P         (3)
#define INDEX_A         (4)
#define INDEX_W         (5)
#define DRPAI_TIMEOUT   (5)

/*Uncomment to enable displaying the debug console log*/
// #define DEBUG_LOG

/*Error List*/
#define PRE_SUCCESS         (0)
#define PRE_ERROR           (1)

/* Library Name */
#define LIB_CONVYUV2RGB     ("conv_yuv2rgb")
#define LIB_RESIZE_HWC      ("resize_hwc")
#define LIB_IMAGESCALER     ("imagescaler")
#define LIB_TRANSPOSE       ("transpose")
#define LIB_CASTFP16_FP32   ("cast_fp16_fp32")

/* Param Info ID */
#define OP_HEAD             ("OFFSET_ADD:")
#define OP_LAYER_NAME       ("layer_name:")
#define OP_LIB              ("drp_lib:")
#define PRAM_HEAD           ("Param:")
#define PARAM_VALUE         ("Value:")
#define PARAM_OFFSET        ("offset:")
#define PARAM_SIZE          ("size:")

/* Param name */
#define P_RADDR             ("raddr")
#define P_IMG_IWIDTH        ("IMG_IWIDHT")
#define P_IMG_IHEIGHT       ("IMG_IHEIGHT")
#define P_IMG_OWIDTH        ("IMG_OWIDTH")
#define P_IMG_OHEIGHT       ("IMG_OHEIGHT")
#define P_INPUT_YUV_FORMAT  ("INPUT_YUV_FORMAT")
#define P_DOUT_RGB_FORMAT   ("DOUT_RGB_FORMAT")
#define P_RESIZE_ALG        ("RESIZE_ALG")
#define P_DATA_TYPE         ("DATA_TYPE")
#define P_ADD_ADDR          ("ADD_ADDR")
#define P_MUL_ADDR          ("MUL_ADDR")
#define P_DOUT_RGB_ORDER    ("DOUT_RGB_ORDER")
#define P_WORD_SIZE         ("WORD_SIZE")
#define P_IS_CHW2HWC        ("IS_CHW2HWC")
#define P_CAST_MODE         ("CAST_MODE")

/* Other related values */
#define INPUT_YUYV          (0x0000)
#define INPUT_NV12          (0x1004)
#define INPUT_NV21          (0x1005)
#define INPUT_UNKNOWN       (0xFFFF) //RFU
#define INPUT_RGB           (0xFFFE) //RFU
#define INPUT_BGR           (0xFFFD) //RFU
#define NUM_C_YUV422        (2)
#define NUM_C_YUV420        (2)  //Currently not used
#define NUM_C_RGB           (3)
#define NUM_C_BGR           (3)  //Currently not used
#define ALG_NEAREST         (0)
#define ALG_BILINEAR        (1)
#define INVALID_IN_ADDR     (0xFFFFFFFF)
#define INVALID_SHAPE       (0xFFFF)
#define INVALID_FORMAT      (INPUT_UNKNOWN)
#define INVALID_RESIZE_ALG  (0xFF)

#define MAX_INPUT_WIDTH     (4096)
#define MIN_INPUT_WIDTH     (0)
#define MAX_INPUT_HEIGHT    (2160)
#define MIN_INPUT_HEIGHT    (0)
#define MAX_RESIZE_WIDTH    (640)
#define MIN_RESIZE_WIDTH    (0)
#define MAX_RESIZE_HEIGHT   (640)
#define MIN_RESIZE_HEIGHT   (0)

/***********************************************************************************************************************
* Struct and related function
***********************************************************************************************************************/
typedef struct
{
    unsigned long desc_aimac_addr;
    unsigned long desc_aimac_size;
    unsigned long desc_drp_addr;
    unsigned long desc_drp_size;
    unsigned long drp_param_addr;
    unsigned long drp_param_size;
    unsigned long data_in_addr;
    unsigned long data_in_size;
    unsigned long data_addr;
    unsigned long data_size;
    unsigned long work_addr;
    unsigned long work_size;
    unsigned long data_out_addr;
    unsigned long data_out_size;
    unsigned long drp_config_addr;
    unsigned long drp_config_size;
    unsigned long weight_addr;
    unsigned long weight_size;
} addr;

typedef struct
{
    uint16_t pre_in_shape_w = INVALID_SHAPE;
    uint16_t pre_in_shape_h = INVALID_SHAPE;
    uint32_t pre_in_addr = INVALID_IN_ADDR;
    uint16_t pre_in_format = INVALID_FORMAT;
    uint8_t resize_alg = INVALID_RESIZE_ALG;
    uint16_t resize_w = INVALID_SHAPE;
    uint16_t resize_h= INVALID_SHAPE;
    float cof_add[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    float cof_mul[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
} s_preproc_param_t;

typedef struct
{
    std::string name;
    uint32_t value;
    uint8_t offset;
    uint8_t size;
} s_op_param_t;

typedef struct
{
    std::string name;
    std::string lib;
    uint8_t offset;
    std::vector<s_op_param_t> param_list;
} s_op_t;

static void clear_param(s_op_param_t* data)
{
    data->name = "";
    data->value = 0;
    data->offset = 0;
    data->size = 0;
}

static void clear_op(s_op_t* data)
{
    data->name = "";
    data->lib = "";
    data->offset = 0;
    data->param_list.clear();
}

static void print_preproc_param(const s_preproc_param_t data)
{
    std::cout <<"PreProcessing Parameter List " <<std::endl;
    std::cout <<"  pre_in_shape_w = "<<std::setw(8)<<data.pre_in_shape_w <<std::endl;
    std::cout <<"  pre_in_shape_h = "<<std::setw(8)<<data.pre_in_shape_h <<std::endl;
    std::cout <<"  pre_in_addr    = "<<std::setw(8)<<std::hex << data.pre_in_addr <<std::endl;
    std::cout <<"  pre_in_format  = "<<std::setw(8)<<std::hex <<data.pre_in_format <<std::endl;
    std::cout <<"  resize_alg     = "<<std::setw(8)<<std::dec <<(int) data.resize_alg <<std::endl;
    std::cout <<"  resize_w       = "<<std::setw(8)<<std::dec <<(int) data.resize_w <<std::endl;
    std::cout <<"  resize_h       = "<<std::setw(8)<<std::dec <<(int) data.resize_h <<std::endl;
    std::cout <<"  cof_add        = ";
    std::cout << std::fixed<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[0]<<", ";
    std::cout << std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[1]<<", ";
    std::cout << std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[2] <<std::endl;
    std::cout <<"  cof_mul        = ";
    std::cout << std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[0];
    std::cout << ", "<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[1];
    std::cout << ", "<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[2]<<std::endl;
}

/***********************************************************************************************************************
* PreRuntime Class
***********************************************************************************************************************/
class PreRuntime {
    public:
        PreRuntime();
        ~PreRuntime();

        uint8_t Load(const std::string pre_dir);
        uint8_t Pre(s_preproc_param_t* param, float** out_ptr, uint32_t* out_size);

    private:
        /*Internal parameter value holder*/
        s_preproc_param_t internal_param_val;
        /*Internal output buffer*/
        float* internal_buffer = NULL;
        /*Internal output buffer size*/
        uint32_t internal_buffer_size = 0;
        /*DRP-AI Driver file descriptor*/
        int8_t drpai_fd = -1;
        /*Buffer to store drp_param.bin data*/
        std::vector<uint8_t> param_data;
        /*Buffer to store weight.dat data*/
        std::vector<uint8_t> weight_data;
        /*List to store parmeter information, i.e., address, offset, size.*/
        std::vector<s_op_t> param_info;
        /*Address map information*/
        addr address;
        /*Variables to store the number of color channel of input/output */
        int8_t pre_in_shape_c = NUM_C_YUV422;
        int8_t pre_out_shape_c = NUM_C_RGB;
        /*Supported Input Format*/
        const float supported_format[1] = { INPUT_YUYV };
        /*Functions*/
        uint8_t ReadAddrmapTxt(std::string addr_file);
        uint8_t LoadFileToMem(std::string data, unsigned long from, unsigned long size);
        uint8_t LoadDataToMem(std::vector<uint8_t> data, unsigned long from, unsigned long size);
        uint8_t ReadFileData(std::vector<uint8_t> &data, std::string file, unsigned long size);
        uint8_t GetResult(unsigned long output_addr, unsigned long output_size);
        uint8_t ParseParamInfo(const std::string info_file);
        uint8_t LoadParamInfo();

        bool UpdateParamData(const s_preproc_param_t param);
        bool UpdateWeightData(const s_preproc_param_t param);

        void UpdateInputShape(const uint16_t w, const uint16_t h);
        void UpdateResizeShape(const uint16_t w, const uint16_t h);
        void UpdateResizeAlg(const uint8_t val);
        uint8_t UpdateInputFormat(const uint16_t val);
        uint8_t UpdateCoefficient(const float* cof_add, const float* cof_mul);

        bool IsDifferentFmInternal(const float* cof_add, const float* cof_mul);
        void WriteValue(uint16_t offset, uint32_t value, uint8_t size);
        bool IsSupportedFormat(uint16_t format);

        double timedifference_msec(struct timespec t0, struct timespec t1);
};

#endif //PRERUNTIME_H
