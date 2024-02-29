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
* Version      : 1.1.0
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
#define NUM_OBJ_FILE    (6)
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
#define PRE_ERROR_UI        (-1)

/* Library Name */
#define LIB_CONVYUV2RGB     ("conv_yuv2rgb")
#define LIB_RESIZE_HWC      ("resize_hwc")
#define LIB_IMAGESCALER     ("imagescaler")
#define LIB_TRANSPOSE       ("transpose")
#define LIB_CASTFP16_FP32   ("cast_fp16_fp32")
#define LIB_CONVX2GRAY      ("conv_x2gray")
#define LIB_CROP            ("crop")
#define LIB_ARGMINMAX       ("argminmax")

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
#define P_WADDR             ("waddr")
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
#define P_CROP_POS_X        ("CROP_POS_X")
#define P_CROP_POS_Y        ("CROP_POS_Y")
#define P_DIN_FORMAT        ("DIN_FORMAT")
#define P_DOUT_RGB_FORMAT   ("DOUT_RGB_FORMAT")
#define P_IMG_ICH           ("IMG_ICH")
#define P_IMG_OCH           ("IMG_OCH")

/* Other related values */
#define FORMAT_YUYV_422     (0x0000)
#define FORMAT_YVYU_422     (0x0001)
#define FORMAT_UYUV_422     (0x0002)
#define FORMAT_VUYY_422     (0x0003)
#define FORMAT_YUYV_420     (0x1000)
#define FORMAT_UYVY_420     (0x1001)
#define FORMAT_YV12_420     (0x1002)
#define FORMAT_IYUV_420     (0x1003)
#define FORMAT_NV12_420     (0x1004)
#define FORMAT_NV21_420     (0x1005)
#define FORMAT_IMC1_420     (0x1006)
#define FORMAT_IMC2_420     (0x1007)
#define FORMAT_IMC3_420     (0x1008)
#define FORMAT_IMC4_420     (0x1009)
#define FORMAT_GRAY         (0xFFFC)
#define FORMAT_BGR          (0xFFFD)
#define FORMAT_RGB          (0xFFFE)
#define FORMAT_UNKNOWN      (0xFFFF)

/* Format in string. Only used when DEBUG_LOG is ON */
#define FORMAT_YUYV_422_STR ("YUYV_422")
#define FORMAT_YVYU_422_STR ("YVYU_422")
#define FORMAT_UYUV_422_STR ("UYUV_422")
#define FORMAT_VUYY_422_STR ("VUYY_422")
#define FORMAT_YUYV_420_STR ("YVYU_420")
#define FORMAT_UYVY_420_STR ("UYVY_420")
#define FORMAT_YV12_420_STR ("YV12_420")
#define FORMAT_IYUV_420_STR ("IYUV_420")
#define FORMAT_NV12_420_STR ("NV12_420")
#define FORMAT_NV21_420_STR ("NV21_420")
#define FORMAT_IMC1_420_STR ("IMC1_420")
#define FORMAT_IMC2_420_STR ("IMC2_420")
#define FORMAT_IMC3_420_STR ("IMC3_420")
#define FORMAT_IMC4_420_STR ("IMC4_420")
#define FORMAT_GRAY_STR     ("GRAY")
#define FORMAT_BGR_STR      ("BGR")
#define FORMAT_RGB_STR      ("RGB")
#define FORMAT_UNKNOWN_STR  ("UNKNOWN")
/* Format in string. Only used when DEBUG_LOG is ON */
static const std::unordered_map<uint16_t, std::string> format_string_table = 
{ 
    {FORMAT_YUYV_422, FORMAT_YUYV_422_STR},
    {FORMAT_YVYU_422 , FORMAT_YVYU_422_STR},
    {FORMAT_UYUV_422 , FORMAT_UYUV_422_STR},
    {FORMAT_VUYY_422 , FORMAT_VUYY_422_STR},
    {FORMAT_YUYV_420 , FORMAT_YUYV_420_STR},
    {FORMAT_UYVY_420 , FORMAT_UYVY_420_STR},
    {FORMAT_YV12_420 , FORMAT_YV12_420_STR},
    {FORMAT_IYUV_420 , FORMAT_IYUV_420_STR},
    {FORMAT_NV12_420 , FORMAT_NV12_420_STR},
    {FORMAT_NV21_420 , FORMAT_NV21_420_STR},
    {FORMAT_IMC1_420 , FORMAT_IMC1_420_STR},
    {FORMAT_IMC2_420 , FORMAT_IMC2_420_STR},
    {FORMAT_IMC3_420 , FORMAT_IMC3_420_STR},
    {FORMAT_IMC4_420 , FORMAT_IMC4_420_STR},
    {FORMAT_GRAY , FORMAT_GRAY_STR},
    {FORMAT_BGR , FORMAT_BGR_STR},
    {FORMAT_RGB , FORMAT_RGB_STR},
    {FORMAT_UNKNOWN , FORMAT_UNKNOWN_STR}
};

/*If FORMAT_* >> BIT_YUV is 1, YUV420.
  If 0, YUV422. 
  >1 otherwise.*/
#define BIT_YUV             (12)

#define DIN_FORMAT_RGB      (0x1000)
#define DIN_FORMAT_BGR      (0x1001)

#define NUM_C_YUV           (2)
#define NUM_C_RGB_BGR       (3)
#define NUM_C_GRAY          (1)

#define ALG_NEAREST         (0)
#define ALG_BILINEAR        (1)
#define INVALID_ADDR        (0xFFFFFFFF)
#define INVALID_SHAPE       (0xFFFF)
#define INVALID_FORMAT      (FORMAT_UNKNOWN)
#define INVALID_RESIZE_ALG  (0xFF)

#define MIN_INPUT_W_BOUND   (0)
#define MIN_INPUT_H_BOUND   (0)
#define MIN_RESIZE_W_BOUND  (2)
#define MIN_RESIZE_H_BOUND  (2)
#define MAX_RESIZE_W_BOUND  (4096)
#define MAX_RESIZE_H_BOUND  (4096)
#define MIN_CROP_W_BOUND    (0)
#define MIN_CROP_H_BOUND    (0)

#define MODE_PRE            (0)
#define MODE_POST           (1)
/***********************************************************************************************************************
* Struct and related function
***********************************************************************************************************************/

/* For dynamic allocation support of DRP-AI Object files */
typedef struct
{
    std::string   directory_name;
    uint64_t      start_address;
    unsigned long object_files_size;
    unsigned long data_in_addr;
    unsigned long data_in_size;
    unsigned long data_out_addr;
    unsigned long data_out_size;
} st_drpai_data_t;

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
    unsigned long aimac_param_cmd_addr;
    unsigned long aimac_param_cmd_size;
    unsigned long aimac_param_desc_addr;
    unsigned long aimac_param_desc_size;
    unsigned long aimac_cmd_addr;
    unsigned long aimac_cmd_size;
} st_addr_info_t;


typedef struct
{
    int             drpai_fd = -1;
    st_drpai_data_t data_inout;
    st_addr_info_t  drpai_address;
} drpai_handle_t;

typedef struct
{
    uint16_t pre_in_shape_w = INVALID_SHAPE;
    uint16_t pre_in_shape_h = INVALID_SHAPE;
    uint64_t pre_in_addr    = INVALID_ADDR;
    uint16_t pre_in_format  = INVALID_FORMAT;
    uint16_t pre_out_format = INVALID_FORMAT;
    uint8_t resize_alg      = INVALID_RESIZE_ALG;
    uint16_t resize_w       = INVALID_SHAPE;
    uint16_t resize_h       = INVALID_SHAPE;
    float cof_add[3]        = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    float cof_mul[3]        = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    uint16_t crop_tl_x      = INVALID_SHAPE;
    uint16_t crop_tl_y      = INVALID_SHAPE;
    uint16_t crop_w         = INVALID_SHAPE;
    uint16_t crop_h         = INVALID_SHAPE;
} s_preproc_param_t;

typedef struct
{
    std::string name;
    uint32_t value;
    uint16_t offset;
    uint16_t size;
} s_op_param_t;

typedef struct
{
    std::string name;
    std::string lib;
    uint16_t offset;
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

static std::string setW(std::string const &str, int n)
{
    std::ostringstream oss;
    oss << std::left<<std::setw(n) << str;
    return oss.str();
}
static void print_preproc_param(const s_preproc_param_t data, uint8_t mode=MODE_PRE)
{
    std::cout <<"PreProcessing Parameter List " <<std::endl;
    std::cout <<"  pre_in_shape_w = "<<std::setw(8)<<std::dec<<data.pre_in_shape_w <<std::endl;
    std::cout <<"  pre_in_shape_h = "<<std::setw(8)<<std::dec<<data.pre_in_shape_h <<std::endl;
    std::cout <<"  pre_in_addr    = "<<std::setw(8)<<std::hex <<data.pre_in_addr <<std::endl;
    if (!mode)
    {
        std::cout <<"  pre_in_format  = "<<std::setw(8)<<std::hex <<data.pre_in_format <<"("<<format_string_table.at(data.pre_in_format)<<")"<<std::endl;
        std::cout <<"  pre_out_format = "<<std::setw(8)<<std::hex <<data.pre_out_format <<"("<<format_string_table.at(data.pre_out_format)<<")"<<std::endl;
        std::cout <<"  resize_alg     = "<<std::setw(8)<<std::dec <<(int) data.resize_alg <<std::endl;
        std::cout <<"  resize_w       = "<<std::setw(8)<<std::dec <<(int) data.resize_w <<std::endl;
        std::cout <<"  resize_h       = "<<std::setw(8)<<std::dec <<(int) data.resize_h <<std::endl;
        std::cout <<"  cof_add        = ";
        std::cout << std::fixed<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[0];
        if (FORMAT_GRAY !=data.pre_out_format)
        {
            std::cout <<", "<< std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[1];
            std::cout <<", "<< std::setw(5)<<std::setprecision(4)<<(float)data.cof_add[2];
        }
        std::cout << std::endl <<"  cof_mul        = ";
        std::cout << std::fixed<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[0];
        if (FORMAT_GRAY !=data.pre_out_format)
        {
            std::cout << ", "<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[1];
            std::cout << ", "<<std::setw(5)<<std::setprecision(4)<<(float)data.cof_mul[2];
        }
        std::cout << std::endl;
        std::cout <<"  crop_tl_x      = "<<std::setw(8)<<std::dec <<(int) data.crop_tl_x <<std::endl;
        std::cout <<"  crop_tl_y      = "<<std::setw(8)<<std::dec <<(int) data.crop_tl_y <<std::endl;
        std::cout <<"  crop_w         = "<<std::setw(8)<<std::dec <<(int) data.crop_w <<std::endl;
        std::cout <<"  crop_h         = "<<std::setw(8)<<std::dec <<(int) data.crop_h <<std::endl;
    }
}

/***********************************************************************************************************************
* PreRuntime Class
***********************************************************************************************************************/
class PreRuntime {
    public:
        PreRuntime();
        ~PreRuntime();

        uint8_t Load(const std::string pre_dir, uint64_t start_addr);
        uint8_t Load(const std::string pre_dir, uint32_t start_addr = INVALID_ADDR, uint8_t mode = MODE_PRE);
        int     SetInput(void *indata);
        uint8_t Pre(s_preproc_param_t* param, void** out_ptr, uint32_t* out_size);
        uint8_t Pre(void** out_ptr, uint32_t* out_size, uint64_t phyaddr);
        int     Occupied_size;

    private:
        /*Internal parameter value holder*/
        s_preproc_param_t internal_param_val;
        /*Internal output buffer*/
        void* internal_buffer = NULL;
        /*Internal output buffer size*/
        uint32_t internal_buffer_size = 0;
        /*DRP-AI Driver dynamic allocation function*/
        drpai_handle_t drpai_obj_info;
        drpai_data_t drpai_data0;
        std::string obj_prefix = "pp";
        /*Buffer to store drp_param.bin data*/
        std::vector<uint8_t> param_data;
        /*Buffer to store weight.dat data*/
        std::vector<uint8_t> weight_data;
        /*List to store parmeter information, i.e., address, offset, size.*/
        std::vector<s_op_t> param_info;
        uint8_t run_mode;
        /*Variables for internal in/out information */
        uint16_t pre_out_shape_w = (uint16_t) INVALID_SHAPE;
        uint16_t pre_out_shape_h = (uint16_t) INVALID_SHAPE;
        uint16_t pre_in_shape_c   = (uint16_t) FORMAT_UNKNOWN;
        uint16_t pre_out_shape_c  = (uint16_t) FORMAT_UNKNOWN;
        uint8_t pre_in_type_size  = 0;
        uint8_t pre_out_type_size = 0;
        /*Flags to figure out whether operators are included in Pre-Runtime Object files*/
        bool crop_included      = false;
        bool resize_included    = false;
        bool normalize_included = false;
        
        /*Since ADRCONV cannot delete just any entry, a means to reconfigure everything became necessary.*/
        uint64_t start_addr_v2h;
        uint64_t mapped_in_addr_v2h;

        /*Supported Format*/
        const uint16_t supported_format_in[17] = 
        { 
            FORMAT_YUYV_422,
            FORMAT_YVYU_422,
            FORMAT_UYUV_422,
            FORMAT_VUYY_422,
            FORMAT_YUYV_420,
            FORMAT_UYVY_420,
            FORMAT_YV12_420,
            FORMAT_IYUV_420,
            FORMAT_NV12_420,
            FORMAT_NV21_420,
            FORMAT_IMC1_420,
            FORMAT_IMC2_420,
            FORMAT_IMC3_420,
            FORMAT_IMC4_420,
            FORMAT_GRAY,
            FORMAT_RGB,
            FORMAT_BGR 
        };
        const uint16_t supported_format_out[3] = 
        { 
            FORMAT_GRAY, 
            FORMAT_RGB, 
            FORMAT_BGR 
        };
        /*Functions*/
        uint8_t ReadAddrmapTxt(std::string addr_file);
        uint8_t WritePrerunData(const std::string dir);
        uint8_t LoadFileToMemDynamic(std::string data, unsigned long offset, unsigned long size, uint32_t file_type);
        uint8_t LoadFileToMemDynamic(std::string data, unsigned long offset, unsigned long size);
        uint8_t LoadDataToMem(std::vector<uint8_t> *data, unsigned long from, unsigned long size);
        uint8_t LoadDataToMem(std::vector<uint8_t> data, unsigned long from, unsigned long size);
        uint8_t ReadFileData(std::vector<uint8_t> *data, std::string file, unsigned long size);
        uint8_t ReadFileData(std::vector<uint8_t> &data, std::string file, unsigned long size);
        uint8_t GetResult(unsigned long output_addr, unsigned long output_size);
        uint8_t ParseParamInfo(const std::string info_file);
        uint8_t LoadParamInfo();
        uint8_t UpdateParamToDynamic(uint32_t start_addr);
        
        int8_t  UpdateParamData(const s_preproc_param_t param);
        int8_t  UpdateWeightData(const s_preproc_param_t param);

        void    UpdateInputShape(const uint16_t w, const uint16_t h);
        void    UpdateResizeShape(const uint16_t w, const uint16_t h);
        void    UpdateResizeAlg(const uint8_t val);
        void    UpdateFormat(const uint16_t input_val, const uint16_t output_val);
        uint8_t UpdateCoefficient(const float* cof_add, const float* cof_mul);
        void    UpdateCropParam(const uint16_t tl_x, const uint16_t tl_y, const uint16_t w, const uint16_t h);

        bool    IsDifferentFmInternal(const float* cof_add, const float* cof_mul);
        void    WriteValue(uint16_t offset, uint32_t value, uint8_t size);
        bool    IsInSupportedList(uint16_t format, uint8_t is_input);
        bool    IsSupportedFormat(const s_preproc_param_t param, uint16_t format_in, uint16_t format_out);
        uint32_t GetStartAddress(uint32_t addr, drpai_data_t drpai_data);
        uint64_t GetStartAddress(uint64_t addr, drpai_data_t drpai_data);
        bool    StartsWith(std::string str, std::string prefix);
        double  timedifference_msec(struct timespec t0, struct timespec t1);
};

#endif //PRERUNTIME_H
