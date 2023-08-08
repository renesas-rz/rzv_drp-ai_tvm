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
* File Name    : PreRuntime.cpp
* Version      : 1.1.0
* Description  : PreRuntime Source file
***********************************************************************************************************************/

#include <fstream>
#include <regex>
#include <dirent.h>
#include "PreRuntime.h"

PreRuntime::PreRuntime()
{
};

PreRuntime::~PreRuntime()
{
    /*Free internal output buffer*/
    if(NULL != internal_buffer)
    {
        free(internal_buffer);
    }
    /*Close DRP-AI Driver*/
    if (0 <= drpai_obj_info.drpai_fd )
    {
        errno = 0;
        if (PRE_SUCCESS != close(drpai_obj_info.drpai_fd ))
        {
            std::cerr << "[ERROR] Failed to close DRP-AI Driver : errno=" << errno << std::endl;
        }
    }
}

/*****************************************
* Function Name     : float32_to_float16
* Description       : Cast float a into uint16_t value.
* Arguments         : a = float32 number
* Return value      : uint16_t = float16 number
******************************************/
static uint16_t float32_to_float16(float a)
{
    return __truncXfYf2__<float, uint32_t, 23 , uint16_t, uint16_t, 10>(a);
}

/*****************************************
* Function Name     : float16_to_float32
* Description       : Function by Edgecortex. Cast uint16_t a into float value.
* Arguments         : a = uint16_t number
* Return value      : float = float32 number
******************************************/
static float float16_to_float32(uint16_t a)
{
    return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

/*****************************************
* Function Name : timedifference_msec
* Description   : Function to compute the processing time in mili-seconds
* Arguments     : t0 = processing start time
*                 t1 = processing end time
* Return value  : processing time in mili-seconds
******************************************/
double PreRuntime::timedifference_msec(struct timespec t0, struct timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1000.0 / 1000.0;
}

/*****************************************
* Function Name : ReadAddrmapTxt
* Description   : Loads address and size of DRP-AI Object files into struct addr.
* Arguments     : addr_file = filename of addressmap file (from DRP-AI Object files)
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::ReadAddrmapTxt(std::string addr_file)
{
    size_t ret = 0;
    std::string str;
    uint32_t l_addr = 0;
    uint32_t l_size = 0;
    std::string element, a, s;
    std::string dir;
    uint8_t cnt = 0;
    errno = 0;

    std::ifstream ifs(addr_file);
    if (ifs.fail())
    {
        std::cerr << "[ERROR] Failed to open Address Map List "<<addr_file<<": errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    while(getline(ifs, str))
    {
        std::istringstream iss(str);
        iss >> element >> a >> s;
        l_addr = strtol(a.c_str(), NULL, 16);
        l_size = strtol(s.c_str(), NULL, 16);

        if ("drp_config" == element)
        {
            drpai_obj_info.drpai_address.drp_config_addr = l_addr;
            drpai_obj_info.drpai_address.drp_config_size = l_size;
        }
        else if ("desc_aimac" == element)
        {
            drpai_obj_info.drpai_address.desc_aimac_addr = l_addr;
            drpai_obj_info.drpai_address.desc_aimac_size = l_size;
        }
        else if ("desc_drp" == element)
        {
            drpai_obj_info.drpai_address.desc_drp_addr = l_addr;
            drpai_obj_info.drpai_address.desc_drp_size = l_size;
        }
        else if ("drp_param" == element)
        {
            drpai_obj_info.drpai_address.drp_param_addr = l_addr;
            drpai_obj_info.drpai_address.drp_param_size = l_size;
        }
        else if ("weight" == element)
        {
            drpai_obj_info.drpai_address.weight_addr = l_addr;
            drpai_obj_info.drpai_address.weight_size = l_size;
        }
        else if ("data_in" == element)
        {
            drpai_obj_info.drpai_address.data_in_addr = l_addr;
            drpai_obj_info.drpai_address.data_in_size = l_size;
        }
        else if ("data" == element)
        {
            drpai_obj_info.drpai_address.data_addr = l_addr;
            drpai_obj_info.drpai_address.data_size = l_size;
        }
        else if ("data_out" == element)
        {
            drpai_obj_info.drpai_address.data_out_addr = l_addr;
            drpai_obj_info.drpai_address.data_out_size = l_size;
        }
        else if ("work" == element)
        {
            drpai_obj_info.drpai_address.work_addr = l_addr;
            drpai_obj_info.drpai_address.work_size = l_size;
        }
        else
        {
            /*Ignore other space*/
        }

        /* Check the start address of Object files */
        if (0 == cnt)
        {
            if (0x00000000 != l_addr)
            {
                std::cerr << "[ERROR] Directory name="<<drpai_obj_info.data_inout.directory_name<<":This Object files do not support the Dynamic allocation function."<<std::endl;
                ifs.close();
                return PRE_ERROR;
            }
        }
        cnt++;
    }
    drpai_obj_info.data_inout.object_files_size = l_addr + l_size;
    drpai_obj_info.data_inout.data_in_addr      = drpai_obj_info.drpai_address.data_in_addr + drpai_obj_info.data_inout.start_address;
    drpai_obj_info.data_inout.data_in_size      = drpai_obj_info.drpai_address.data_in_size;
    drpai_obj_info.data_inout.data_out_addr     = drpai_obj_info.drpai_address.data_out_addr + drpai_obj_info.data_inout.start_address;
    drpai_obj_info.data_inout.data_out_size     = drpai_obj_info.drpai_address.data_out_size;

    ifs.close();
    return PRE_SUCCESS;
}

/*****************************************
* Function Name : StartsWith
* Description   : Check if the string starts with prefix
* Arguments     : str = string to be checked
*                 prefix = prefix to compare
* Return value  : true if string starts with prefix
*                 false if not
******************************************/
bool PreRuntime::StartsWith(std::string str, std::string prefix)
{
    if (str.size() >= prefix.size() &&
    std::equal(std::begin(prefix), std::end(prefix), std::begin(str)))
    {
        return true;
    }
    return false;
}
/*****************************************
* Function Name : LoadFileToMemDynamic
* Description   : Loads a file to memory via DRP-AI Driver
* Arguments     : data = filename to be written to memory
*                 drpai_fd = file descriptor of DRP-AI Driver
*                 offset = memory offset  where the data is written from start address
*                 size = data size to be written
*                 file_type = file type of DRP-AI Object files
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadFileToMemDynamic(std::string data, unsigned long offset, unsigned long size, uint32_t file_type)
{
    int8_t  ret_load_data = PRE_SUCCESS;
    int     obj_fd = 0;
    uint8_t drpai_buf[BUF_SIZE];
    int     drpai_fd = drpai_obj_info.drpai_fd;
    drpai_data_dynamic_t drpai_data_dynamic;
    uint8_t ret = 0;
    int32_t i = 0;

    errno = 0;
    obj_fd = open(data.c_str(), O_RDONLY);
    if (0 > obj_fd )
    {
        std::cerr << "[ERROR] Failed to open " << data << ": errno=" << errno << std::endl;
        ret_load_data = PRE_ERROR;
        goto end;
    }
    drpai_data_dynamic.start_address = drpai_obj_info.data_inout.start_address;
    drpai_data_dynamic.offset = offset;
    drpai_data_dynamic.size = size;
    drpai_data_dynamic.file_type = file_type;
    errno = 0;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN_DYNAMIC, &drpai_data_dynamic);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN_DYNAMIC : errno=" << errno << std::endl;
        ret_load_data = PRE_ERROR;
        goto end;
    }

    for (i = 0 ; i<(drpai_data_dynamic.size/BUF_SIZE) ; i++)
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, BUF_SIZE);
        if ( 0 > ret)
        {
            std::cerr << "[ERROR] Failed to read "<<data<<" : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
        ret = write(drpai_fd , drpai_buf,  BUF_SIZE);
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
    }
    if ( 0 != (drpai_data_dynamic.size%BUF_SIZE))
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, (drpai_data_dynamic.size % BUF_SIZE));
        if ( 0 > ret)
        {
            std::cerr << "[ERROR] Failed to read "<<data<<" : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
        ret = write(drpai_fd , drpai_buf, (drpai_data_dynamic.size % BUF_SIZE));
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
    }
    goto end;

end:
    if (0 < obj_fd)
    {
        close(obj_fd);
    }
    return ret_load_data;
}
/*****************************************
* Function Name : LoadDataToMem
* Description   : Loads a drp_param.bin to memory via DRP-AI Driver
* Arguments     : data = filename to be written to memory
*                 from = memory start address where the data is written
*                 size = data size to be written
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadDataToMem(std::vector<uint8_t> data, unsigned long from, unsigned long size)
{
    int          drpai_fd = drpai_obj_info.drpai_fd;
    drpai_data_t drpai_data;
    uint8_t      ret = 0;
    int32_t      i = 0;

    errno = 0;
    drpai_data.address = from;
    drpai_data.size = size;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    for (i = 0 ; i<(drpai_data.size/BUF_SIZE) ; i++)
    {
        errno = 0;
        ret = write(drpai_fd, &data[BUF_SIZE*i], BUF_SIZE);
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }

    if ( 0 != (drpai_data.size%BUF_SIZE))
    {
        errno = 0;
        ret = write(drpai_fd, &data[BUF_SIZE*(int)(drpai_data.size/BUF_SIZE)], (drpai_data.size % BUF_SIZE));
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }
    return PRE_SUCCESS;
}

/*****************************************
* Function Name : ReadFileData
* Description   : Loads a drp_param.bin
* Arguments     : data = container to store the file contents
*                 file = filename to be read
*                 size = data size to be read
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::ReadFileData(std::vector<uint8_t> &data, std::string file, unsigned long size)
{
    uint8_t obj_fd, ret;
    drpai_data_t drpai_data;
    errno = 0;
    data.resize(size);
    data.clear();

    std::ifstream ifs(file);
    if (!ifs)
    {
        std::cerr << "[ERROR] Failed to open " << file << ": errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    /* Store file data to internal vector */
    std::istreambuf_iterator<char> it(ifs);
    std::istreambuf_iterator<char> last;
    for (; it != last; ++it)
    {
        data.push_back(*it);
    }
    /* Check the param_data size is appropriate */
    if (size != data.size())
    {
        std::cerr << "[ERROR] Failed to read " << file << ": errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    ifs.close();

    return PRE_SUCCESS;
}
/*****************************************
* Function Name : UpdateParamToDynamic
* Description   : update drp_param.bin data with dynamic allocated address.
* Arguments     : start_addr = dynamic allocated start address
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::UpdateParamToDynamic(uint32_t start_addr)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string raddr = P_RADDR;
    std::string waddr = P_WADDR;
    std::string add_addr = P_ADD_ADDR;
    std::string mul_addr = P_MUL_ADDR;
    uint16_t offset = 0;

    errno = 0;
    uint8_t i, j;

    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        for (j = 0; j<tmp_op->param_list.size(); j++)
        {
            tmp_param = &tmp_op->param_list[j];
            if (tmp_param->name == raddr 
                || tmp_param->name == waddr 
                || tmp_param->name == add_addr 
                || tmp_param->name == mul_addr )
            {
                tmp_param->value += (uint32_t) start_addr;
                offset = tmp_op->offset + tmp_param->offset;
                WriteValue(offset, tmp_param->value, tmp_param->size);
            }
        }
    }
    return PRE_SUCCESS;
}


/*****************************************
* Function Name : ParseParamInfo
* Description   : Loads a drp_param_info.txt.
* Arguments     : info_file = filename to be loaded.
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::ParseParamInfo(const std::string info_file)
{
    const std::string offset_add      = OP_HEAD;
    const std::string layer_name      = OP_LAYER_NAME;
    const std::string drp_lib         = OP_LIB;
    const std::string param_head      = PRAM_HEAD;
    const std::string param_value     = PARAM_VALUE;
    const std::string param_offset    = PARAM_OFFSET;
    const std::string param_size      = PARAM_SIZE;
    std::string str         = "";
    std::string str_return  = "";
    std::string element     = "";
    std::string str_value   = "";
    uint16_t val_u16        = 0;
    uint32_t val_u32        = 0;
    s_op_t tmp_op;
    s_op_param_t tmp_param;
    bool first_itr_done = false;
    int  drpai_fd = drpai_obj_info.drpai_fd;
    drpai_assign_param_t drpai_param;
    uint32_t drp_param_info_size;
    errno = 0;
    
    /*Get param info file size*/
    std::ifstream param_file_for_size(info_file, std::ifstream::ate);
    drp_param_info_size = static_cast<uint32_t>(param_file_for_size.tellg());
    param_file_for_size.close();

    drpai_param.info_size = drp_param_info_size;
    drpai_param.obj.address = drpai_obj_info.drpai_address.drp_param_addr + drpai_obj_info.data_inout.start_address;
    drpai_param.obj.size = drpai_obj_info.drpai_address.drp_param_size;
    
    if (0 != ioctl(drpai_fd, DRPAI_ASSIGN_PARAM, &drpai_param))
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN_PARAM : errno="<<errno << std::endl;
        return PRE_ERROR;
    }
    
    /* Open param info file */
    std::ifstream param_file(info_file);
    if (param_file.fail())
    {
        std::cerr << "[ERROR] Failed to open Param Info file "<<info_file<<": errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    while(getline(param_file, str))
    {
        /*Add newline character at the end for DRP-AI Driver write().*/
        str_return = str + "\n"; 
        /*Write to DRP-AI Driver*/
        if ( 0 > write(drpai_fd, str_return.c_str(), str_return.size()))
        {
            std::cerr << "[ERROR] Failed to write to DRP-AI Driver : errno="<<errno << std::endl;
            param_file.close();
            return PRE_ERROR;
        }
        /*Remove all spaces*/
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
        std::istringstream iss(str);
        /*Get first word*/
        getline(iss, element, ',');
        /*Operator info*/
        if (StartsWith(element, offset_add))
        {
            if (first_itr_done)
            {
                param_info.push_back(tmp_op);
            }
            else
            {
                first_itr_done = true;
            }

            /*Clear temporary param*/
            clear_op(&tmp_op);

            /* OFFSET_ADD */
            str_value = element.substr(offset_add.size());
            val_u16 = static_cast<uint16_t>(std::stoi(str_value));
            tmp_op.offset = val_u16;
            /*Get op info*/
            while(getline(iss, element, ','))
            {
                /* layer_name */
                if (StartsWith(element, layer_name))
                {
                    str_value = element.substr(layer_name.size());
                    tmp_op.name = str_value;
                }
                /* drp_lib */
                else if (StartsWith(element, drp_lib))
                {
                    str_value = element.substr(drp_lib.size());
                    tmp_op.lib = str_value;
                }
                else
                {
                    /*Do nothing*/
                }
            }
        }
        /*Parameter info*/
        else if (StartsWith(element, param_head))
        {
            /*Clear temporary param*/
            clear_param(&tmp_param);
            /* Param */
            str_value = element.substr(param_head.size());
            tmp_param.name = str_value;

            /*Get param info*/
            while(getline(iss, element, ','))
            {
                /* Value */
                if (StartsWith(element, param_value))
                {
                    str_value = element.substr(param_value.size());
                    val_u32 =  static_cast<uint32_t>(std::stoul(str_value));
                    tmp_param.value = val_u32;
                }
                /* offset */
                else if (StartsWith(element, param_offset))
                {
                    str_value = element.substr(param_offset.size());
                    val_u16 = static_cast<uint16_t>(std::stoi(str_value));
                    tmp_param.offset = val_u16;
                }
                /* size */
                else if (StartsWith(element, param_size))
                {
                    str_value = element.substr(param_size.size());
                    val_u16 = static_cast<uint16_t>(std::stoi(str_value));
                    tmp_param.size = val_u16;
                }
                else
                {
                    /*Do nothing*/
                }

           }
           tmp_op.param_list.push_back(tmp_param);
        }
    }
    param_info.push_back(tmp_op);
    param_file.close();

    return PRE_SUCCESS;
}

/*****************************************
* Function Name : LoadParamInfo
* Description   : Loads a drp_param_info.txt.
* Arguments     : info_file = filename to be loaded.
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadParamInfo()
{
    uint8_t i = 0, j = 0;
    s_op_t* tmp_op;
    s_op_param_t tmp_op_param;
    std::string readaddr            = P_RADDR;
    std::string img_iwidth          = P_IMG_IWIDTH;
    std::string img_iheight         = P_IMG_IHEIGHT;
    std::string img_owidth          = P_IMG_OWIDTH;
    std::string img_oheight         = P_IMG_OHEIGHT;
    std::string input_yuv_format    = P_INPUT_YUV_FORMAT;
    std::string lib_convyuv2rgb     = LIB_CONVYUV2RGB;
    std::string lib_resize_hwc      = LIB_RESIZE_HWC;
    std::string lib_imagescaler     = LIB_IMAGESCALER;
    std::string lib_crop            = LIB_CROP;
    std::string lib_convx2gray      = LIB_CONVX2GRAY;
    std::string lib_argminmax       = LIB_ARGMINMAX;
    std::string dout_rgb_order      = P_DOUT_RGB_ORDER;
    std::string resize_alg          = P_RESIZE_ALG;
    std::string crop_pos_x          = P_CROP_POS_X;
    std::string crop_pos_y          = P_CROP_POS_Y;
    std::string x2gray_din_format   = P_DIN_FORMAT;
    std::string dout_rgb_format     = P_DOUT_RGB_FORMAT;
    std::string img_ich             = P_IMG_ICH;
    std::string img_och             = P_IMG_OCH;
    uint32_t data_in_size = 0;
    uint32_t data_out_size = 0;
    uint32_t in_size = 0;
    uint32_t out_size = 0;
    uint16_t num_ch = 0;

    /*pre_in_shape_w, pre_in_shape_h, pre_in_addr*/
    tmp_op = &param_info[0];
    for (i = 0; i<tmp_op->param_list.size(); i++)
    {
        /*pre_in_shape_w*/
        if (tmp_op->param_list[i].name == img_iwidth)
        {
            internal_param_val.pre_in_shape_w = (uint16_t) tmp_op->param_list[i].value;
            pre_out_shape_w = internal_param_val.pre_in_shape_w;
        }
        /*pre_in_shape_h*/
        else if (tmp_op->param_list[i].name == img_iheight)
        {
            internal_param_val.pre_in_shape_h = (uint16_t) tmp_op->param_list[i].value;
            pre_out_shape_h = internal_param_val.pre_in_shape_h;
        }
        /*pre_in_addr*/
        else if (tmp_op->param_list[i].name == readaddr)
        {
            internal_param_val.pre_in_addr = (uint32_t) tmp_op->param_list[i].value;
        }
    }

    /*pre_in_format, pre_out_format*/
    tmp_op = &param_info[0];
    internal_param_val.pre_in_format = (uint16_t) FORMAT_UNKNOWN;
    internal_param_val.pre_out_format = (uint16_t) FORMAT_UNKNOWN;
    
    for (i = 0; i<tmp_op->param_list.size(); i++)
    {

        /*conv_yuv2rgb*/
        if (tmp_op->lib ==  lib_convyuv2rgb)
        {
            pre_in_shape_c = (uint8_t) NUM_C_YUV;
            pre_out_shape_c = (uint8_t) NUM_C_RGB_BGR;
            if (tmp_op->param_list[i].name == input_yuv_format)
            {
                /*pre_in_format*/
                internal_param_val.pre_in_format = (uint16_t) tmp_op->param_list[i].value;
            }
            else if (tmp_op->param_list[i].name == dout_rgb_format)
            {
                /*pre_out_format*/
                if (0 == (uint16_t) tmp_op->param_list[i].value)
                {
                    internal_param_val.pre_out_format = (uint16_t) FORMAT_RGB;
                }
                else
                {
                    internal_param_val.pre_out_format = (uint16_t) FORMAT_BGR;
                }
                break;
            }
            else
            {
                /*Do nothing*/
            }
        }
        /*conv_x2gray*/
        else if (tmp_op->lib ==  lib_convx2gray && tmp_op->param_list[i].name == x2gray_din_format )
        {
            pre_in_shape_c = (uint8_t) NUM_C_YUV;
            pre_out_shape_c = (uint8_t) NUM_C_GRAY;
            /*pre_in_format/pre_out_format*/
            internal_param_val.pre_in_format = (uint16_t) tmp_op->param_list[i].value;
            if (DIN_FORMAT_RGB == tmp_op->param_list[i].value)
            {
                pre_in_shape_c = (uint8_t) NUM_C_RGB_BGR;
                internal_param_val.pre_in_format = FORMAT_RGB;

            }
            else if (DIN_FORMAT_BGR == tmp_op->param_list[i].value)
            {
                internal_param_val.pre_in_format = FORMAT_BGR;
                pre_in_shape_c = (uint8_t) NUM_C_RGB_BGR;
            }
            else
            {
                /*Do nothing*/
            }
            internal_param_val.pre_out_format = (uint16_t) FORMAT_GRAY;
            break;
        }
        /*Number of channel*/
        else if (tmp_op->param_list[i].name == img_ich)
        {
            pre_in_shape_c = (uint16_t) tmp_op->param_list[i].value;
            if (1 == pre_in_shape_c)
            {
                internal_param_val.pre_in_format = (uint16_t) FORMAT_GRAY;
                internal_param_val.pre_out_format = (uint16_t) FORMAT_GRAY;
                pre_out_shape_c = (uint8_t) NUM_C_GRAY; 
            }
            else if (3 == pre_in_shape_c)
            {
                /*Temporal format = RGB (possibly BGR)*/
                internal_param_val.pre_in_format = (uint16_t) FORMAT_RGB;
                internal_param_val.pre_out_format = (uint16_t) FORMAT_RGB;
                pre_out_shape_c = (uint8_t) NUM_C_RGB_BGR;
            }
            else 
            {
                /*Ignore others*/
            }
        }
        else 
        {
            /*Ignore others*/
        }
    }
    /*Check imagescaler to determine RGB/BGR for pre_in/out_format */
    /*Also if imagescaler exists, enable normalize_included.*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_imagescaler )
        {
            normalize_included = true;
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                if (tmp_op->param_list[j].name == dout_rgb_order )
                {
                    /*Change pre_out if dout_rgb_order is 1*/
                    /*Note that input format may be either RGB/BGR but here, we assume input is RGB.*/
                    if (1 == (uint16_t)tmp_op->param_list[j].value)
                    {
                        if (FORMAT_RGB == internal_param_val.pre_out_format)
                        {
                            internal_param_val.pre_out_format = (uint16_t) FORMAT_BGR;
                        }
                        else if (FORMAT_BGR == internal_param_val.pre_out_format)
                        {
                            internal_param_val.pre_out_format = (uint16_t) FORMAT_RGB;
                        }
                        else
                        {
                            /*Do nothing*/
                        }
                    }
                    /*Ignore after ops*/
                    break;
                }
            }
            /*Ignore after ops*/
            break;
        }
    }
    /*Check the output number of channel.*/
    tmp_op = &param_info[param_info.size()-1];
    for (i = 0; i<tmp_op->param_list.size(); i++)
    {
        if (tmp_op->param_list[i].name == img_och )
        {
            pre_out_shape_c = (uint16_t) tmp_op->param_list[i].value;
            break;
            /*Ignore other operators*/
        }
    }

    /*crop_tl_x, crop_tl_y, crop_w, crop_h*/
    /*Also if crop exists, enable crop_included.*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_crop)
        {
            crop_included = true;
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                /*crop_tl_x*/
                if (tmp_op->param_list[j].name == crop_pos_x)
                {
                    internal_param_val.crop_tl_x = (uint16_t)tmp_op->param_list[j].value;
                }
                /*crop_tl_y*/
                else if (tmp_op->param_list[j].name == crop_pos_y)
                {
                    internal_param_val.crop_tl_y = (uint16_t)tmp_op->param_list[j].value;
                }
                /*crop_w*/
                if (tmp_op->param_list[j].name == img_owidth)
                {
                    internal_param_val.crop_w = (uint16_t)tmp_op->param_list[j].value;
                    pre_out_shape_w = internal_param_val.crop_w;
                }
                /*crop_h*/
                else if (tmp_op->param_list[j].name == img_oheight)
                {
                    internal_param_val.crop_h = (uint16_t)tmp_op->param_list[j].value;
                    pre_out_shape_h = internal_param_val.crop_h;
                }
            }
        }
    }

    /*resize_w, resize_h, resize_alg*/
    /*Also if resize exists, enable resize_included.*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_resize_hwc)
        {
            resize_included = true;
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                /*resize_w*/
                if (tmp_op->param_list[j].name == img_owidth)
                {
                    internal_param_val.resize_w = (uint16_t)tmp_op->param_list[j].value;
                    pre_out_shape_w = internal_param_val.resize_w;
                }
                /*resize_h*/
                else if (tmp_op->param_list[j].name == img_oheight)
                {
                    internal_param_val.resize_h = (uint16_t)tmp_op->param_list[j].value;
                    pre_out_shape_h = internal_param_val.resize_h;
                }
                else if (tmp_op->param_list[j].name == resize_alg )
                {
                    internal_param_val.resize_alg = (uint8_t) tmp_op->param_list[j].value;
                    /*Ignore after ops*/
                    break;
                }
                else
                {
                    /*Ignore other parameters*/
                }
            }
            break;
        }
    }


    /*cof_add, cof_mul*/
    int index, mod;
    uint8_t float_cvt[2];
    float a;
    uint8_t float_num = sizeof(weight_data)/sizeof(float)+1;
    uint8_t cood_num = 2;
    uint8_t cof_num = (uint8_t)(weight_data.size() - cood_num)/sizeof(float);
    /*Minimum size of coefficient data. cof_add(2 or 6bytes)+padding(2bytes)+cof_mul(2 or 6bytes)*/
    uint8_t cof_size = 2*cof_num*cood_num + cood_num;
    for (i = 0; i<weight_data.size(); i++)
    {
        index = (int) i/cood_num;
        mod = i%cood_num;
        if ( cof_num != index && index <= float_num )
        {
            float_cvt[mod] = weight_data[i];
            if (1 == mod)
            {
                /*Convert to FP32*/
                a = (float)float16_to_float32(float_cvt[1]*16*16+float_cvt[0]);
                if (cof_num > index )
                {
                    internal_param_val.cof_add[index] = a;
                }
                else
                {
                    internal_param_val.cof_mul[index-(cof_num + 1)] = a;
                }
            }
        }
        /*If weight.dat contains other information other than cof_add/mul, break the loop*/
        if (cof_size <= i) break;
    }

    /*pre_out_shape_w, pre_out_shape_h*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_argminmax)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                /*pre_out_shape_w*/
                if (tmp_op->param_list[j].name == img_owidth)
                {
                    pre_out_shape_w = (uint16_t)tmp_op->param_list[j].value;
                }
                /*pre_out_shape_h*/
                else if (tmp_op->param_list[j].name == img_oheight)
                {
                    pre_out_shape_h = (uint16_t)tmp_op->param_list[j].value;
                }
                else
                {
                    /*Ignore other parameters*/
                }
            }
            break;
        }
    }
    /*pre_in_type_size, pre_out_type_size*/
    data_in_size = drpai_obj_info.data_inout.data_in_size;
    data_out_size = drpai_obj_info.data_inout.data_out_size;
    in_size = internal_param_val.pre_in_shape_w * internal_param_val.pre_in_shape_h * pre_in_shape_c;
    out_size = pre_out_shape_w * pre_out_shape_h * pre_out_shape_c;
    pre_in_type_size = (uint8_t) (drpai_obj_info.data_inout.data_in_size / in_size); 
    pre_out_type_size = (uint8_t) (drpai_obj_info.data_inout.data_out_size / out_size); 
#ifdef DEBUG_LOG
    std::cout<<"[INFO] Input  (H, W, C, unit byte_size)=(";
    std::cout<<std::dec<<internal_param_val.pre_in_shape_h<<", " <<internal_param_val.pre_in_shape_w<<", ";
    std::cout<<pre_in_shape_c<<", " <<+pre_in_type_size<<")"<<std::endl;
    std::cout<<"[INFO] Output (H, W, C, unit byte_size)=(";
    std::cout<<pre_out_shape_h<<", " <<pre_out_shape_w<<", ";
    std::cout<<pre_out_shape_c<<", " <<+pre_out_type_size<<")"<<std::endl;
    if (MODE_POST == run_mode)
    {
        print_preproc_param(internal_param_val, MODE_POST);
    }
    else
    {
        print_preproc_param(internal_param_val);
    }
#endif
    return PRE_SUCCESS;
}


/*****************************************
* Function Name : GetStartAddress
* Description   : Check the user input start_addr is valid and return the valid address
* Arguments     : addr = address to be checked.
*               : drpai_data = DRP-AI memory area details.
* Return value  : start address for Pre-Runtime Object files
*                 INVALID_ADDR if user input is invalid
******************************************/
uint32_t PreRuntime::GetStartAddress(uint32_t addr, drpai_data_t drpai_data)
{
    uint32_t drpai_mem_addr_end = drpai_data.address+drpai_data.size - 1;
    if (INVALID_ADDR == addr)
    {
        /*If user did not specify the start_addr, use DRP-AI memory area start address.*/
        return drpai_data.address;
    }
    if ((drpai_data.address > addr)|| drpai_mem_addr_end < addr ) 
    {
        std::cerr << "[ERROR] Not in DRP-AI memory area."<< std::endl;
        return INVALID_ADDR;
    }
    if (0 != (addr % 64))
    {
        std::cerr << "[ERROR] Not 64-byte aligned."<< std::endl;
        return INVALID_ADDR;
    }
    return addr;
}

/*****************************************
* Function Name : Load
* Description   : Loads PreRuntime Object data.
* Arguments     : pre_dir = folder name to be loaded.
*               : start_addr = start address that object files are dynamically allocated.
*                              default value is INVALID_ADDR.
*               : mode       = pre or post mode.
*                              default value is MODE_PRE.
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::Load(const std::string pre_dir, uint32_t start_addr, uint8_t mode)
{
    uint8_t ret = 0;
    struct stat statBuf;
    std::string tmp_dir = "/";
    std::string dir = pre_dir;
    std::string pp_prefix = "pp";
    std::string tmp_address_file = dir+"/"+dir+"_addrmap_intm.txt";
    uint32_t start_address = start_addr;
    uint32_t addr = 0;
    uint32_t size = 0;
    uint8_t i = 0;
    uint32_t file_type = 0;
    const std::string drpai_param_file = dir + "/drp_param_info.txt";

    run_mode = mode;

    /* Delete unnecessary slush */
    if (dir.size() >= tmp_dir.size() &&
            dir.find(tmp_dir, dir.size() - tmp_dir.size()) != std::string::npos)
    {
        dir = dir.erase(dir.size()-1);
    }
    /* Check whether directory exists*/
    if (0 != stat(dir.c_str(),&statBuf))
    {
        std::cerr << "[ERROR] Directory " << dir << " not found." << std::endl;
        return PRE_ERROR;
    }

    /*Check if PreRuntime Object files are generated from PreRuntime Compile Module*/
    if (0 == stat(tmp_address_file.c_str(),&statBuf))
    {
        obj_prefix = dir;
    }
    /*Define necessary filename*/
    const std::string address_file = dir+"/"+obj_prefix+"_addrmap_intm.txt";
    const std::string drpai_file_path[NUM_OBJ_FILE] =
    {
        "",
        dir+"/drp_desc.bin",
        dir+"/"+obj_prefix+"_drpcfg.mem",
        dir+"/drp_param.bin",
        dir+"/aimac_desc.bin",
        dir+"/"+obj_prefix+"_weight.dat",
    };

    drpai_obj_info.data_inout.directory_name = dir;
    errno = 0;
    /*Open DRP-AI Driver*/
    drpai_obj_info.drpai_fd = open("/dev/drpai0", O_RDWR);
    if (PRE_SUCCESS > drpai_obj_info.drpai_fd )
    {
        std::cerr << "[ERROR] Failed to open DRP-AI Driver : errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(drpai_obj_info.drpai_fd , DRPAI_GET_DRPAI_AREA, &drpai_data0);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to get DRP-AI Memory Area : errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    /*Define the start address*/
    drpai_obj_info.data_inout.start_address = GetStartAddress(start_address, drpai_data0);
    if (INVALID_ADDR == drpai_obj_info.data_inout.start_address)
    {
        std::cerr << "[ERROR] Invalid argument: start_addr = 0x" <<std::setw(8)<<std::hex <<start_address << std::endl;
        return PRE_ERROR;
    }
    
    /* Read Address Map List file */
    ret = ReadAddrmapTxt(address_file);
    if (PRE_SUCCESS < ret)
    {
        std::cerr << "[ERROR] Failed to read Address Map List " << address_file  << std::endl;
        return PRE_ERROR;
    }
    
    /*Parse drp_param_info.txt*/
    ret = ParseParamInfo(drpai_param_file);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read param info file: "<< drpai_param_file << std::endl;
        return PRE_ERROR;
    }

    /* Read PreRuntime Object and write it to memory via DRP-AI Driver */
    for (i = 1;i<NUM_OBJ_FILE; i++)
    {
        switch (i)
        {
            case (INDEX_W):
                addr = drpai_obj_info.drpai_address.weight_addr;
                size = drpai_obj_info.drpai_address.weight_size;
                file_type = DRPAI_FILE_TYPE_WEIGHT;
                break;
            case (INDEX_C):
                addr = drpai_obj_info.drpai_address.drp_config_addr;
                size = drpai_obj_info.drpai_address.drp_config_size;
                file_type = DRPAI_FILE_TYPE_DRP_CFG;
                break;
            case (INDEX_P):
                addr = drpai_obj_info.drpai_address.drp_param_addr;
                size = drpai_obj_info.drpai_address.drp_param_size;
                file_type = DRPAI_FILE_TYPE_DRP_PARAM;
                break;
            case (INDEX_A):
                addr = drpai_obj_info.drpai_address.desc_aimac_addr;
                size = drpai_obj_info.drpai_address.desc_aimac_size;
                file_type = DRPAI_FILE_TYPE_AIMAC_DESC;
                break;
            case (INDEX_D):
                addr = drpai_obj_info.drpai_address.desc_drp_addr;
                size = drpai_obj_info.drpai_address.desc_drp_size;
                file_type = DRPAI_FILE_TYPE_DRP_DESC;
                break;
            default:
                break;
        }
        if (INDEX_P == i)
        {
            /*Read and Hold drp_param.bin data internally in param_data vector. */
            ret = ReadFileData(param_data, drpai_file_path[i], size);
            if ( PRE_SUCCESS < ret )
            {
                std::cerr << "[ERROR] Failed to read file "<< drpai_file_path[i] << std::endl;
                return PRE_ERROR;
            }
            /*Update address in drp_param.bin for dynamic allocation*/
            UpdateParamToDynamic(drpai_obj_info.data_inout.start_address );
            ret = LoadDataToMem(param_data, addr+drpai_obj_info.data_inout.start_address, size);
        }
        else if (INDEX_W == i)
        {
            /*Read and Hold weight.dat data internally in weight_data array. */
            ret = ReadFileData(weight_data, drpai_file_path[i], size);
            if ( PRE_SUCCESS < ret )
            {
                std::cerr << "[ERROR] Failed to read file "<< drpai_file_path[i] << std::endl;
                return PRE_ERROR;
            }
            /*Load weight data to memory using non-dynamic function.*/
            ret = LoadDataToMem(weight_data, addr + drpai_obj_info.data_inout.start_address, size);
        }
        else
        {
            /*Read data from file and write to memory */
            ret = LoadFileToMemDynamic(drpai_file_path[i], addr, size, file_type);
        }
        /*Error Check*/
        if ( PRE_SUCCESS < ret )
        {
            std::cerr << "[ERROR] Failed to load data to memory: "<< drpai_file_path[i] << std::endl;
        return PRE_ERROR;
        }
    }

    /*Load param data*/
    ret = LoadParamInfo();
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to load param info. " << std::endl;
        return PRE_ERROR;
    }
    return PRE_SUCCESS;
}
/*****************************************
* Function Name : WriteValue
* Description   : Write specified value to specified offset in internal drp_param.bin data.
* Arguments     : offset = offset to be written in param_data.
*               : value  = value to be written.
*               : size   = size of value.
* Return value  : -
******************************************/
void PreRuntime::WriteValue(uint16_t offset, uint32_t value, uint8_t size)
{
    uint16_t address = offset;
    uint32_t val = value;
    uint8_t i = 0;

    for (i = 0;i<size;i++)
    {
        param_data[address+i] = (uint8_t) (val >> 8*i) & 0xFF;
    }
    return;
}
/*****************************************
* Function Name : UpdateInputShape
* Description   : Function to update pre_in_shape_w and pre_in_shape_h in param data.
* Arguments     : w = uint16_t number. New input shape width.
*                 h = uint16_t number. New input shape height.
* Return value  : -
******************************************/
void PreRuntime::UpdateInputShape(const uint16_t w, const uint16_t h)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string img_iwidth      = P_IMG_IWIDTH;
    std::string img_iheight     = P_IMG_IHEIGHT;
    std::string img_owidth      = P_IMG_OWIDTH;
    std::string img_oheight     = P_IMG_OHEIGHT;
    std::string lib_resize_hwc  = LIB_RESIZE_HWC;
    std::string lib_crop        = LIB_CROP;
    uint16_t offset = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        for (j = 0; j<tmp_op->param_list.size(); j++)
        {
            tmp_param = &tmp_op->param_list[j];
            if (tmp_param->name == img_iwidth ||
                (tmp_param->name == img_owidth && tmp_op->lib != lib_resize_hwc))
            {
                tmp_param->value = w;
                offset = tmp_op->offset + tmp_param->offset;
                WriteValue(offset, tmp_param->value, tmp_param->size);
            }
            if (tmp_param->name == img_iheight ||
                (tmp_param->name == img_oheight && tmp_op->lib != lib_resize_hwc))
            {
                tmp_param->value = h;
                offset = tmp_op->offset + tmp_param->offset;
                WriteValue(offset, tmp_param->value, tmp_param->size);
            }
        }
        
        if ((tmp_op->lib == lib_resize_hwc) || (tmp_op->lib == lib_crop))
        {
            /*Ignore all op after resize_hwc and crop*/
            break;
        }
    }
    internal_param_val.pre_in_shape_w = w;
    internal_param_val.pre_in_shape_h = h;
    return;
}

/*****************************************
* Function Name : UpdateResizeShape
* Description   : Function to update resize_w and resize_h in param data.
* Arguments     : w = uint16_t number. Width to be resized.
*                 h = uint16_t number. Height to be resized.
* Return value  : -
******************************************/
void PreRuntime::UpdateResizeShape(const uint16_t w, const uint16_t h)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string img_iwidth = P_IMG_IWIDTH;
    std::string img_iheight = P_IMG_IHEIGHT;
    std::string img_owidth = P_IMG_OWIDTH;
    std::string img_oheight = P_IMG_OHEIGHT;
    std::string lib_resize_hwc = LIB_RESIZE_HWC;
    bool after_resize = false;
    uint16_t offset = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib == lib_resize_hwc)
        {
            after_resize = true;
            for (int j = 0; j<tmp_op->param_list.size(); j++)
            {
                tmp_param = &tmp_op->param_list[j];
                if ( tmp_param->name == img_owidth )
                {
                    tmp_param->value = w;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                if ( tmp_param->name == img_oheight )
                {
                    tmp_param->value = h;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                    /*Ignore all parameter after IMAGE_OHEIGHT */
                    break;
                }
            }
        }
        else if (after_resize)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                tmp_param = &tmp_op->param_list[j];
                if ( tmp_param->name == img_iwidth || tmp_param->name == img_owidth )
                {
                    tmp_param->value = w;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                if ( tmp_param->name == img_iheight || tmp_param->name == img_oheight )
                {
                    tmp_param->value = h;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                /*Ignore all parameter after IMAGE_OHEIGHT */
                if (tmp_param->name == img_oheight) break;
            }
        }
        else
        {
            /*Ignore all op before resize_hwc */
        }
    }
    internal_param_val.resize_w = (uint16_t) w;
    internal_param_val.resize_h = (uint16_t) h;
    return;
}


/*****************************************
* Function Name : UpdateCropParam
* Description   : Function to update crop parameters in param data.
* Arguments     : tl_x = uint16_t number. Top-left X coordinate of crop area.
*                 tl_y = uint16_t number. Top-left Y coordinate of crop area.
*                 w = uint16_t number. Width to be cropped.
*                 h = uint16_t number. Height to be cropped.
* Return value  : -
******************************************/
void PreRuntime::UpdateCropParam(const uint16_t tl_x, const uint16_t tl_y, const uint16_t w, const uint16_t h)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string img_iwidth      = P_IMG_IWIDTH;
    std::string img_iheight     = P_IMG_IHEIGHT;
    std::string img_owidth      = P_IMG_OWIDTH;
    std::string img_oheight     = P_IMG_OHEIGHT;
    std::string crop_pos_x      = P_CROP_POS_X;
    std::string crop_pos_y      = P_CROP_POS_Y;
    std::string lib_crop        = LIB_CROP;
    std::string lib_resize_hwc  = LIB_RESIZE_HWC;
    bool after_crop = false;
    bool before_resize = false;
    uint16_t offset = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib == lib_crop)
        {
            after_crop = true;
            for (int j = 0; j<tmp_op->param_list.size(); j++)
            {
                tmp_param = &tmp_op->param_list[j];
                if ( tmp_param->name == img_owidth )
                {
                    tmp_param->value = w;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                if ( tmp_param->name == img_oheight )
                {
                    tmp_param->value = h;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                if ( tmp_param->name == crop_pos_x )
                {
                    tmp_param->value = tl_x;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                }
                if ( tmp_param->name == crop_pos_y )
                {
                    tmp_param->value = tl_y;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                    /*Ignore all parameter after CROP_POS_Y */
                    break;
                }
            }
        }
        else if (after_crop)
        {
            if (tmp_op->lib == lib_resize_hwc)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == img_iwidth )
                    {
                        tmp_param->value = w;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                    }
                    if ( tmp_param->name == img_iheight )
                    {
                        tmp_param->value = h;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                        /*Ignore all parameter after IMAGE_IHEIGHT */
                        break;
                    }
                }
                /*Ignore all op after resize_hwc */
                break;
            }
            else
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == img_iwidth || tmp_param->name == img_owidth )
                    {
                        tmp_param->value = w;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                    }
                    if ( tmp_param->name == img_iheight || tmp_param->name == img_oheight )
                    {
                        tmp_param->value = h;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                    }
                    /*Ignore all parameter after IMAGE_OHEIGHT */
                    if (tmp_param->name == img_oheight) break;
                }
            }
        }
        else
        {
            /*Ignore all op before crop */
        }
    }
    internal_param_val.crop_tl_x = (uint16_t) tl_x;
    internal_param_val.crop_tl_y = (uint16_t) tl_y;
    internal_param_val.crop_w    = (uint16_t) w;
    internal_param_val.crop_h    = (uint16_t) h;
    return;
}
/*****************************************
* Function Name : UpdateFormat
* Description   : Function to update pre_in_format in param data.
* Arguments     : input_val = uint16_t number. New input format.
*               : output_val = uint16_t number. New Output format.
* Return value  : -
******************************************/
void PreRuntime::UpdateFormat(const uint16_t input_val, const uint16_t output_val)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string din_yuv_format  = P_INPUT_YUV_FORMAT;
    std::string din_format      = P_DIN_FORMAT;
    std::string lib_convyuv2rgb = LIB_CONVYUV2RGB;
    std::string lib_imagescaler = LIB_IMAGESCALER;
    std::string lib_convx2gray  = LIB_CONVX2GRAY;
    std::string out_rgb_format  = P_DOUT_RGB_FORMAT;
    std::string out_rgb_order   = P_DOUT_RGB_ORDER;
    uint8_t i = 0;
    uint8_t j = 0;
    uint16_t offset = 0;
    if (1 >= (input_val >> BIT_YUV))
    {
        for (i = 0; i<param_info.size(); i++)
        {
            tmp_op = &param_info[i];
            if (tmp_op->lib == lib_convyuv2rgb)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == din_yuv_format )
                    {
                        /*DIN_YUV_FORMAT*/
                        tmp_param->value = input_val;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                    }
                    else if ( tmp_param->name == out_rgb_format )
                    {
                        /*DOUT_RGB_FORMAT*/
                        if (internal_param_val.pre_out_format != output_val)
                        {
                            if (FORMAT_RGB == output_val)
                            {
                                tmp_param->value = 0;
                            }
                            else if (FORMAT_BGR == output_val) 
                            {
                                tmp_param->value = 1;
                            }
                            else
                            {
                                /*Do nothing*/
                            }
                            offset = tmp_op->offset + tmp_param->offset;
                            WriteValue(offset, tmp_param->value, tmp_param->size);
                        }
                        break;
                    }
                }
                /*Do not break since stil need to check imagescaler > DOUT_RGB_ORDER*/
            }
            else if (tmp_op->lib == lib_convx2gray)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == din_format )
                    {
                        /*DIN_FORMAT*/
                        tmp_param->value = input_val;
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                        break;
                    }
                }
                break;
            }
            else if (tmp_op->lib == lib_imagescaler)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == out_rgb_order )
                    {
                        if (1 == tmp_param->value)
                        {
                            /*DOUT_RGB_ORDER*/
                            /*Change it to 0 so that no swap occurs.*/
                            tmp_param->value = 0;
                            offset = tmp_op->offset + tmp_param->offset;
                            WriteValue(offset, tmp_param->value, tmp_param->size);
                        }
                        break;
                    }
                }
                break;
            }
            else
            {
                /*Ignore other operators*/
            }
        }
    }
    else
    {
        /*RGB/BGR*/
        for (i = 0; i<param_info.size(); i++)
        {
            tmp_op = &param_info[i];
            if (tmp_op->lib == lib_convx2gray)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == din_format)
                    {
                        /*DIN_FORMAT*/
                        if (FORMAT_RGB == input_val)
                        {
                            tmp_param->value = DIN_FORMAT_RGB;
                        }
                        else if (FORMAT_BGR == input_val)
                        {
                            tmp_param->value = DIN_FORMAT_BGR;
                        }
                        else 
                        {
                            /*Do nothing*/
                        }
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                        break;
                    }
                }
                break;
            }
            else if (tmp_op->lib == lib_imagescaler)
            {
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == out_rgb_order )
                    {
                        /*DOUT_RGB_ORDER*/
                        if (input_val == output_val)
                        {
                            /*Change it to 0 so that no swap occurs.*/
                            tmp_param->value = 0;
                        }
                        else
                        {
                            /*Change it to 1 so that swap occurs.*/
                            tmp_param->value = 1;
                        }
                        offset = tmp_op->offset + tmp_param->offset;
                        WriteValue(offset, tmp_param->value, tmp_param->size);
                        break;
                    }
                }
                break;
            }
            else
            {
                /*Ignore other operators*/
            }
        }
    }
    internal_param_val.pre_in_format = input_val;
    internal_param_val.pre_out_format = output_val;
    return;
}

/*****************************************
* Function Name : UpdateResizeAlg
* Description   : Function to update resize_alg in param data.
* Arguments     : val = uint8_t number. New resize algorithm.
* Return value  : -
******************************************/
void PreRuntime::UpdateResizeAlg(const uint8_t val)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string resize_alg_name = P_RESIZE_ALG;
    std::string lib_resize_hwc = LIB_RESIZE_HWC;
    uint8_t i = 0, j = 0;
    uint16_t offset = 0;

    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib == lib_resize_hwc)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                tmp_param = &tmp_op->param_list[j];
                if ( tmp_param->name == resize_alg_name )
                {
                    tmp_param->value = val;
                    offset = tmp_op->offset + tmp_param->offset;
                    WriteValue(offset, tmp_param->value, tmp_param->size);
                    break;
                }
            }
            break;
        }
    }
    internal_param_val.resize_alg = val;
    return;
}

/*****************************************
* Function Name : UpdateCoefficient
* Description   : Function to update cof_add and cof_mul in weight data.
* Arguments     : new_cof_add = float array. New cof_add.
*                 new_cof_mul = float array. New cof_mul.
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::UpdateCoefficient(const float* new_cof_add, const float* new_cof_mul)
{
    uint8_t cood_num = 2;
    uint8_t cof_num = (uint8_t)(weight_data.size() - cood_num)/sizeof(float);
    uint8_t cof_size = cood_num*cof_num;
    uint16_t fp16_data_add, fp16_data_mul;
    uint8_t new_cof_add_char[cof_size];
    uint8_t new_cof_mul_char[cof_size];
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string add_addr        = P_ADD_ADDR;
    std::string lib_imagescaler = LIB_IMAGESCALER;
    uint16_t weight_offset = 0;
    uint8_t weight_size = weight_data.size();
    uint8_t empty_size = 2;
    uint8_t i = 0;
    uint8_t j = 0;

    /*Get offset*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib == lib_imagescaler)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                tmp_param = &tmp_op->param_list[j];
                if ( tmp_param->name == add_addr )
                {
                    weight_offset = tmp_param->value - (drpai_obj_info.drpai_address.weight_addr + drpai_obj_info.data_inout.start_address);
                    break;
                }
            }
            break;
        }
    }

    if (0 > weight_offset)
    {
        std::cerr << "[ERROR] Wrong address of weight area or wrong drp_param_info.txt. "<<std::endl;
        return PRE_ERROR;
    }

    for (i = 0;i<cof_num; i++)
    {
        fp16_data_add = float32_to_float16(new_cof_add[i]);
        fp16_data_mul = float32_to_float16(new_cof_mul[i]);
        new_cof_add_char[cood_num*i]   = (uint8_t) fp16_data_add           & 0xFF;
        new_cof_add_char[cood_num*i+1] = (uint8_t) (fp16_data_add >> 8)    & 0xFF;
        new_cof_mul_char[cood_num*i]   = (uint8_t) fp16_data_mul           & 0xFF;
        new_cof_mul_char[cood_num*i+1] = (uint8_t) (fp16_data_mul >> 8)    & 0xFF;
    }

    for (i = weight_offset;i< weight_size + weight_offset;i++)
    {
        uint8_t id = i - weight_offset;
        if (i < cof_size)
        {
            weight_data[i] = new_cof_add_char[id];
        }
        else if (i == cof_size || i == cof_size + 1)
        {
            weight_data[i] = 0x00;
        }
        else
        {
            weight_data[i] = new_cof_mul_char[id - (cof_size + empty_size)];
        }
    }
    /*Update the current parameters in internal_param_val*/
    memcpy(internal_param_val.cof_add, new_cof_add, cof_num*sizeof(float));
    memcpy(internal_param_val.cof_mul, new_cof_mul, cof_num*sizeof(float));
    return PRE_SUCCESS;
}


/*****************************************
* Function Name : IsDifferentFmInternal
* Description   : Function to update cof_add and cof_mul in weight data.
* Arguments     : new_cof_add = float array. New cof_add.
*                 new_cof_mul = float array. New cof_mul.
* Return value  : true if argument is different from internal_param_val
*                 false if not
******************************************/
bool PreRuntime::IsDifferentFmInternal(const float* new_cof_add, const float* new_cof_mul)
{
    uint8_t i = 0;
    uint8_t cood_num = 0;
    uint8_t size = (uint8_t)(weight_data.size() - cood_num)/sizeof(float);
    for (i = 0; i<size ; i++)
    {
        if (new_cof_add[i] != internal_param_val.cof_add[i] || new_cof_mul[i] != internal_param_val.cof_mul[i])
        {
            return true;
        }
    }
    return false;
}

/*****************************************
* Function Name : IsInSupportedList
* Description   : Function to check whether format is in the supported list or not.
* Arguments     : format = input format
*               : is_input = flag to figure out in/out. set 1 if it is input.
* Return value  : true if argument is supported format
*                 false if not
******************************************/
bool PreRuntime::IsInSupportedList(uint16_t format, uint8_t is_input)
{
    uint8_t i = 0;
    uint8_t size = 0;
    if (is_input)
    {
        size = sizeof(supported_format_in)/sizeof(uint16_t);
        for (i = 0; i<size ; i++)
        {
            if (supported_format_in[i] == format)
            {
                return true;
            }
        }
    }
    else
    {
        size = sizeof(supported_format_out)/sizeof(uint16_t);
        for (i = 0; i<size ; i++)
        {
            if (supported_format_out[i] == format)
            {
                return true;
            }
        }
    }
    
    return false;
}

/*****************************************
* Function Name : IsSupportedFormat
* Description   : Function to check whether in/out format is supported or not.
* Arguments     : param = current internal parameter lists.
                  format_in = input format
                  format_out = output format
* Return value  : true if argument is supported format
*                 false if not
******************************************/
bool PreRuntime::IsSupportedFormat(const s_preproc_param_t param, uint16_t format_in, uint16_t format_out)
{
    uint8_t i = 0;
    /* Check format is in the supported table*/
    if (! IsInSupportedList(format_in, 1)) 
    {
        std::cerr<<"[ERROR] Invalid parameter: pre_in_format="<<format_in<<std::endl;
        goto end_false;
    }
    if (! IsInSupportedList(format_out, 0)) 
    {
        std::cerr<<"[ERROR] Invalid parameter: pre_out_format="<<format_out<<std::endl;
        goto end_false;
    }

    /* Check parameter change is valid*/
    if ((internal_param_val.pre_in_format != format_in )|| (internal_param_val.pre_out_format != format_out))
    {
        /*Not supported combination In YUV420 & Out Gray*/
        if ((1 == (format_in >> BIT_YUV)) && (FORMAT_GRAY == format_out))
        {
            std::cerr << "[ERROR] Not supported format combination."<<std::endl;
            std::cerr << "        In=YUV420 & Out=Gray "<<std::endl;
            goto end_false;
        }
        /*When current format is Gray, it cannot be changed.*/
        if ((FORMAT_GRAY==internal_param_val.pre_in_format) && (FORMAT_GRAY!=format_in))
        {
            std::cerr << "[ERROR] Parameter pre_in_format change is not allowed"<<std::endl;
            std::cerr << "          if current pre_in_format is Gray."<<std::endl;
            goto print_err_format_in;
        }
        if ((FORMAT_GRAY==internal_param_val.pre_out_format) && (FORMAT_GRAY!=format_out))
        {
            std::cerr << "[ERROR] Parameter pre_out_format change is not allowed"<<std::endl;
            std::cerr << "          if current pre_out_format is Gray."<<std::endl;
            goto print_err_format_out;
        }
        /*When current format is not Gray, it cannot be changed to Gray*/
        if ((FORMAT_GRAY !=internal_param_val.pre_in_format) && (FORMAT_GRAY==format_in))
        {
            std::cerr << "[ERROR] Parameter pre_in_format change is not allowed"<<std::endl;
            std::cerr << "          for non-Gray pre_in_format to Gray."<<std::endl;
            goto print_err_format_in;
        }
        if ((FORMAT_GRAY !=internal_param_val.pre_out_format) && (FORMAT_GRAY==format_out))
        {
            std::cerr << "[ERROR] Parameter pre_out_format change is not allowed"<<std::endl;
            std::cerr << "          for non-Gray pre_out_format to Gray."<<std::endl;
            goto print_err_format_out;
        }
        /*When current input format is YUV, new format_in must be YUV.*/
        if ((1 >= (internal_param_val.pre_in_format >> BIT_YUV))&& (1 < (format_in >> BIT_YUV)))
        {
            std::cerr << "[ERROR] Parameter change is not allowed if In=YUV & New In=Not YUV."<<std::endl;
            goto print_err_format_in;
        }
        /*When current input format is not YUV, new format_in must not be YUV.*/
        if ((1 < (internal_param_val.pre_in_format >> BIT_YUV))&& (1 >= (format_in >> BIT_YUV)))
        {
            std::cerr << "[ERROR] Parameter change is not allowed if In=Not YUV & New In=YUV."<<std::endl;
            goto print_err_format_in;
        }
    }
    goto end_true;

print_err_format_in:
    std::cerr << "          current pre_in_format  =";
    std::cerr << std::setprecision(8)<<std::hex<<internal_param_val.pre_in_format<<std::endl;
    std::cerr << "          specified pre_in_format=";
    std::cerr << std::setprecision(8)<<std::hex<<format_in<<std::endl;
    goto end_false;
print_err_format_out:
    std::cerr << "          current pre_out_format  =";
    std::cerr << std::setprecision(8)<<std::hex<<internal_param_val.pre_out_format<<std::endl;
    std::cerr << "          specified pre_out_format=";
    std::cerr << std::setprecision(8)<<std::hex<<format_out<<std::endl;
    goto end_false;
end_false:
    return false;
end_true:
    return true;
}

/*****************************************
* Function Name : UpdateParamData
* Description   : Check whether input parameter value is different from current parameter value,
*                 and if different, change the drp_param data according to the input parameter value.
* Arguments     : param = input parameter value.
* Return value  : Number of updated parameters.
*                 PRE_ERROR_UI if error occurs.
******************************************/
int8_t PreRuntime::UpdateParamData(const s_preproc_param_t param)
{
    int8_t num_updated = 0;
    uint16_t new_pre_in_shape_w =  param.pre_in_shape_w;
    uint16_t new_pre_in_shape_h = param.pre_in_shape_h;
    uint16_t new_pre_out_shape_w = 0;
    uint16_t new_pre_out_shape_h = 0;
    uint16_t new_pre_in_format = param.pre_in_format;
    uint16_t new_pre_out_format = param.pre_out_format;
    uint16_t new_resize_w = param.resize_w;
    uint16_t new_resize_h = param.resize_h;
    uint8_t new_resize_alg = param.resize_alg;
    uint16_t new_crop_tl_x = param.crop_tl_x;
    uint16_t new_crop_tl_y = param.crop_tl_y;
    uint16_t new_crop_w = param.crop_w;
    uint16_t new_crop_h = param.crop_h;

    /*--------------------------*
     * Input image size (shape) *
     * ------------------------ */
    /* If one of them is INVALID_SHAPE, use the current shape */
    if (new_pre_in_shape_w == INVALID_SHAPE) new_pre_in_shape_w = internal_param_val.pre_in_shape_w;
    if (new_pre_in_shape_h == INVALID_SHAPE) new_pre_in_shape_h = internal_param_val.pre_in_shape_h;
    /* Size restriction check */
    if (MIN_INPUT_W_BOUND > new_pre_in_shape_w )
    {
        std::cerr<<"[ERROR] Invalid parameter: pre_in_shape_w="<<new_pre_in_shape_w<<std::endl;
        return (int8_t) PRE_ERROR_UI;
    }
    if (MIN_INPUT_H_BOUND > new_pre_in_shape_h )
    {
        std::cerr<<"[ERROR] Invalid parameter: pre_in_shape_h="<<new_pre_in_shape_h<<std::endl;
        return (int8_t) PRE_ERROR_UI;
    }
    if (new_pre_in_shape_w != internal_param_val.pre_in_shape_w
        || new_pre_in_shape_h != internal_param_val.pre_in_shape_h)
    {
        UpdateInputShape(new_pre_in_shape_w, new_pre_in_shape_h);
        num_updated++;
#ifdef DEBUG_LOG
        std::cout<<"[INFO] Changed pre_in_shape_w: "<<std::dec<<new_pre_in_shape_w<<std::endl;
        std::cout<<"[INFO] Changed pre_in_shape_h: "<<new_pre_in_shape_h<<std::endl;
#endif
    }

    /*--------------------------*
     * Input/Output format      *
     * ------------------------ */
    if (MODE_PRE == run_mode)
    {
        /* If one of them is INVALID, use the current value */
        if (new_pre_in_format == INVALID_FORMAT) new_pre_in_format = internal_param_val.pre_in_format;
        if (new_pre_out_format == INVALID_FORMAT) new_pre_out_format = internal_param_val.pre_out_format;
        /*Check whether it is supported*/
        if (!IsSupportedFormat(param, new_pre_in_format, new_pre_out_format))
        {
            return (int8_t)  PRE_ERROR_UI;
        }
        if ((new_pre_in_format != internal_param_val.pre_in_format) 
            || (new_pre_out_format != internal_param_val.pre_out_format))
        {
            UpdateFormat(new_pre_in_format, new_pre_out_format);
            num_updated++;
    #ifdef DEBUG_LOG
            std::cout<<"[INFO] Changed pre_in_format: 0x"<<std::hex<<new_pre_in_format<<"("<<format_string_table.at(new_pre_in_format)<<")"<<std::endl;
            std::cout<<"[INFO] Changed pre_out_format: 0x"<<std::hex<<new_pre_out_format<<"("<<format_string_table.at(new_pre_out_format)<<")"<<std::endl;
    #endif
        }
        /*--------------------------*
        * Crop parameters          *
        * ------------------------ */
        if (!crop_included)
        {
            /* Crop is not used in loaded Pre-runtime Object files.*/
            /* If there is any crop parameters in user input, print warning and ignore the user input. */
            if ((INVALID_SHAPE != new_crop_tl_x) 
                || (INVALID_SHAPE != new_crop_tl_y)
                || (INVALID_SHAPE != new_crop_w)
                || (INVALID_SHAPE != new_crop_h))
            {
                std::cerr<<"[WARNING] Crop parameters are specified in Pre(),"<<std::endl;
                std::cerr<<"          but no crop operator used in loaded Pre-runtime Object files."<<std::endl;
                std::cerr<<"          Specified crop_tl_x, crop_tl_y, crop_w and crop_h are ignored."<<std::endl;
            }
        }
        else
        {
            /* If one of them is INVALID_SHAPE, use the current shape */
            if (new_crop_tl_x == INVALID_SHAPE) new_crop_tl_x = internal_param_val.crop_tl_x;
            if (new_crop_tl_y == INVALID_SHAPE) new_crop_tl_y = internal_param_val.crop_tl_y;
            if (new_crop_w == INVALID_SHAPE) new_crop_w = internal_param_val.crop_w;
            if (new_crop_h == INVALID_SHAPE) new_crop_h = internal_param_val.crop_h;
            
            /* Size restriction check */
            {
                if ((MIN_CROP_W_BOUND > new_crop_tl_x) 
                    || ((internal_param_val.pre_in_shape_w-1) < new_crop_tl_x))
                {
                    std::cerr<<"[ERROR] Invalid parameter: crop_tl_x="<<new_crop_tl_x<<std::endl;
                    if ((internal_param_val.pre_in_shape_w-1) < new_crop_tl_x)
                    {
                        std::cerr<<"          Requirement: crop_tl_x <= pre_in_shape_w - 1."<<std::endl;
                    }
                    return (int8_t)  PRE_ERROR_UI;
                }
                if ((MIN_CROP_H_BOUND > new_crop_tl_y)
                    || ((internal_param_val.pre_in_shape_h-1) < new_crop_tl_y))
                {
                    std::cerr<<"[ERROR] Invalid parameter: crop_tl_y="<<new_crop_tl_y<<std::endl;
                    if ((internal_param_val.pre_in_shape_h-1) < new_crop_tl_y)
                    {
                        std::cerr<<"          Requirement: crop_tl_y <= pre_in_shape_h - 1."<<std::endl;
                    }
                    return (int8_t)  PRE_ERROR_UI;
                }
                if ((MIN_CROP_W_BOUND > new_crop_w)
                    || (internal_param_val.pre_in_shape_w < new_crop_w))
                {
                    std::cerr<<"[ERROR] Invalid parameter: crop_w="<<new_crop_w<<std::endl;
                    if (internal_param_val.pre_in_shape_h < new_crop_w)
                    {
                        std::cerr<<"          Requirement: crop_w <= pre_in_shape_w"<<std::endl;
                    }
                    return (int8_t)  PRE_ERROR_UI;
                }
                if ((MIN_CROP_H_BOUND > new_crop_h)
                    || (internal_param_val.pre_in_shape_h < new_crop_h))
                {
                    std::cerr<<"[ERROR] Invalid parameter: crop_h="<<new_crop_h<<std::endl;
                    if (internal_param_val.pre_in_shape_h < new_crop_h)
                    {
                        std::cerr<<"          Requirement: crop_h <= pre_in_shape_h"<<std::endl;
                    }
                    return (int8_t)  PRE_ERROR_UI;
                }
            }
            /*Update the crop parameters in param_data.*/
            if ((new_crop_tl_x != internal_param_val.crop_tl_x)
                || (new_crop_tl_y != internal_param_val.crop_tl_y)
                || (new_crop_w != internal_param_val.crop_w)
                || (new_crop_h != internal_param_val.crop_h))
            {
                UpdateCropParam(new_crop_tl_x, new_crop_tl_y, new_crop_w, new_crop_h);
                num_updated++;
                if (!resize_included) new_pre_out_shape_w = new_crop_w;
                if (!resize_included) new_pre_out_shape_h = new_crop_h;
#ifdef DEBUG_LOG
                std::cout<<"[INFO] Changed crop_tl_x: "<<std::dec<<new_resize_w<<std::endl;
                std::cout<<"[INFO] Changed crop_tl_y: "<<new_crop_tl_y<<std::endl;
                std::cout<<"[INFO] Changed crop_w: "<<new_crop_w<<std::endl;
                std::cout<<"[INFO] Changed crop_h: "<<new_crop_h<<std::endl;
#endif
            }
        }
        /*---------------------------------------*
        * Resize output data size and algorithm *
        * ------------------------------------- */
        if (!resize_included)
        {
            /* Resize is not used in loaded Pre-runtime Object files.*/
            /* If there is any resize parameters in user input, print warning and ignore the user input. */
            if ((INVALID_SHAPE != new_resize_w) 
                || (INVALID_SHAPE != new_resize_h)
                || (INVALID_RESIZE_ALG != new_resize_alg))
            {
                std::cerr<<"[WARNING] Resize parameters are specified in Pre(),"<<std::endl;
                std::cerr<<"          but no resize operator used in loaded Pre-runtime Object files."<<std::endl;
                std::cerr<<"          Specified resize_w, resize_h and resize_alg are ignored."<<std::endl;
            }
        }
        else
        {
            /* If one of them is INVALID_SHAPE, use the current shape */
            if (new_resize_w == INVALID_SHAPE) new_resize_w = internal_param_val.resize_w;
            if (new_resize_h == INVALID_SHAPE) new_resize_h = internal_param_val.resize_h;
            /* Size restriction check */
            if (MIN_RESIZE_W_BOUND >= new_resize_w || MAX_RESIZE_W_BOUND < new_resize_w)
            {
                std::cerr<<"[ERROR] Invalid parameter: resize_w="<<new_resize_w<<std::endl;
                std::cerr<<"          Requirement: "<<(uint32_t) MIN_RESIZE_W_BOUND <<" < resize_w <= "<<(uint32_t) MAX_RESIZE_W_BOUND <<std::endl;
                return (int8_t)  PRE_ERROR_UI;
            }
            if (MIN_RESIZE_H_BOUND >= new_resize_h || MAX_RESIZE_H_BOUND < new_resize_h)
            {
                std::cerr<<"[ERROR] Invalid parameter: resize_h="<<new_resize_h<<std::endl;
                std::cerr<<"          Requirement: "<<(uint32_t) MIN_RESIZE_H_BOUND <<" < resize_h <= "<<(uint32_t) MAX_RESIZE_H_BOUND <<std::endl;
                return (int8_t)  PRE_ERROR_UI;
            }
            if (new_resize_w != internal_param_val.resize_w
                || new_resize_h != internal_param_val.resize_h)

            {
                UpdateResizeShape(new_resize_w, new_resize_h);
                num_updated++;
                new_pre_out_shape_w = new_resize_w;
                new_pre_out_shape_h = new_resize_h;
#ifdef DEBUG_LOG
                std::cout<<"[INFO] Changed resize_w: "<<std::dec<<new_resize_w<<std::endl;
                std::cout<<"[INFO] Changed resize_h: "<<new_resize_h<<std::endl;
#endif
            }

            if (new_resize_alg == INVALID_RESIZE_ALG)
            {
                /*param.resize_alg not defined*/
            }
            else if ( 1 < new_resize_alg)
            {
                std::cerr<<"[ERROR] Invalid parameter: resize_alg="<<std::dec<<(int)new_resize_alg<<std::endl;
                return (int8_t)  PRE_ERROR_UI;
            }
            else if (new_resize_alg != internal_param_val.resize_alg)
            {
                UpdateResizeAlg(new_resize_alg);
                num_updated++;
#ifdef DEBUG_LOG
                std::cout<<"[INFO] Changed resize_alg: "<<std::dec<<(int) new_resize_alg<<std::endl;
#endif
            }
        }

    }
    if (0 == new_pre_out_shape_w) new_pre_out_shape_w = pre_out_shape_w;
    if (0 == new_pre_out_shape_h) new_pre_out_shape_h = pre_out_shape_h;

    pre_out_shape_w = new_pre_out_shape_w;
    pre_out_shape_h = new_pre_out_shape_h;
    /*Update data_in & data_out size*/
#ifdef DEBUG_LOG
    std::cout<<"[INFO] Input  (H, W, C, unit byte_size)=(";
    std::cout<<std::dec<<new_pre_in_shape_h<<", " <<new_pre_in_shape_w<<", ";
    std::cout<<pre_in_shape_c<<", " <<+pre_in_type_size<<")"<<std::endl;
    std::cout<<"[INFO] Output (H, W, C, unit byte_size)=(";
    std::cout<<new_pre_out_shape_h<<", " <<new_pre_out_shape_w<<", ";
    std::cout<<pre_out_shape_c<<", " <<+pre_out_type_size<<")"<<std::endl;
#endif
    drpai_obj_info.data_inout.data_in_size = new_pre_in_shape_w * new_pre_in_shape_h * pre_in_shape_c * pre_in_type_size;
    drpai_obj_info.data_inout.data_out_size = new_pre_out_shape_w * new_pre_out_shape_h * pre_out_shape_c * pre_out_type_size;

    return num_updated;
}
/*****************************************
* Function Name : UpdateWeightData
* Description   : Check whether input parameter value is different from current parameter value,
*                 and if different, change the weight data according to the input parameter value.
* Arguments     : param = input parameter value.
* Return value  : Number of updated parameters.
*                 PRE_ERROR_UI if error occurs.
******************************************/
int8_t PreRuntime::UpdateWeightData(const s_preproc_param_t param)
{
    int8_t num_updated = 0;
    uint8_t cood_num = 2;
    uint8_t size = (uint8_t)(weight_data.size() - cood_num)/sizeof(float);
    if (!normalize_included)
    {
        /* Normalize is not used in loaded Pre-runtime Object files.*/
        /* If there is any normalize parameters in user input, print warning and ignore the user input. */
        if ((-FLT_MAX != param.cof_add[0]) 
            || (-FLT_MAX != param.cof_mul[0]))
        {
            std::cerr<<"[WARNING] Normalize parameters are specified in Pre(),"<<std::endl;
            std::cerr<<"          but no normalize operator used in loaded Pre-runtime Object files."<<std::endl;
            std::cerr<<"          Specified cof_add and cof_mul are ignored."<<std::endl;
        }
        return num_updated;
    }
     
    float new_cof_add[size];
    float new_cof_mul[size];

    memcpy(new_cof_add, param.cof_add, size*sizeof(float));
    memcpy(new_cof_mul, param.cof_mul, size*sizeof(float));
    uint8_t ret = 0;
    uint8_t i = 0;
    for (i = 0; i<size ; i++)
    {
        if (new_cof_add[i] == -FLT_MAX)
        {
            new_cof_add[i] = internal_param_val.cof_add[i];
        }
        if (new_cof_mul[i] == -FLT_MAX)
        {
            new_cof_mul[i] = internal_param_val.cof_mul[i];
        }
    }

    if (IsDifferentFmInternal(new_cof_add, new_cof_mul))
    {
        ret = UpdateCoefficient(new_cof_add, new_cof_mul);
        if (PRE_SUCCESS < ret )
        {
            return (int8_t) PRE_ERROR_UI;
        }
        num_updated++;
#ifdef DEBUG_LOG
        /*Print changed parameters*/
        std::cout<<"[INFO] Changed cof_add:";
        for (int i = 0;i<size;i++)
        {
            std::cout <<" "<<std::dec<<std::setprecision(4)<< new_cof_add[i];
        }
        std::cout<<std::endl;
        std::cout<<"[INFO] Changed cof_mul:";
        for (int i = 0;i<size;i++)
        {
            std::cout <<" "<<std::setprecision(4)<< new_cof_mul[i];
        }
        std::cout<<std::endl;
#endif
    }
    return num_updated;
}

/*****************************************
* Function Name : GetResult
* Description   : Function to save the DRP-AI output. Uses DRP-AI Driver
* Arguments     : output_ptr = pointer to the buffer which stores DRP-AI output
*                 output_addr = memory address that DRP-AI output is stored.
*                 output_size = memory size of DRP-AI output
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::GetResult(unsigned long output_addr, unsigned long output_size)
{
    int8_t ret = 0;
    drpai_data_t drpai_data;
    drpai_data.address = output_addr;
    drpai_data.size = output_size;
#ifdef DEBUG_LOG
    struct timespec start_time, end_time;
    float diff = 0;
    timespec_get(&start_time, TIME_UTC);
#endif

    /*Free internal buffer if its memory is already allocated */
    if(internal_buffer != NULL )
    {
        free(internal_buffer);
    }
    /*Allocate internal output buffer*/
    if(sizeof(uint8_t) == pre_out_type_size)
    {
        internal_buffer = (uint8_t*) malloc(drpai_data.size);
    }
    else if (sizeof(uint16_t) == pre_out_type_size)
    {
        internal_buffer = (uint16_t*) malloc(drpai_data.size);
    }
    else if (sizeof(uint32_t) == pre_out_type_size)
    {
        internal_buffer = (uint32_t*) malloc(drpai_data.size);
    }
    else
    {
        /*Do nothing*/
    }

    if(internal_buffer == NULL)
    {
        std::cerr << "[ERROR] Failed to malloc PreRuntime internal buffer." <<std::endl;
        return PRE_ERROR;
    }
    internal_buffer_size = (uint32_t) (drpai_data.size/pre_out_type_size);

    errno = 0;
    /* Assign the memory address and size to be read */
    ret = ioctl(drpai_obj_info.drpai_fd , DRPAI_ASSIGN, &drpai_data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN: errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    /* Read the memory via DRP-AI Driver and store the output to buffer */
    errno = 0;
    ret = read(drpai_obj_info.drpai_fd , internal_buffer, drpai_data.size);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to read via DRP-AI Driver: errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    return PRE_SUCCESS;
}

/*****************************************
* Function Name : Pre
* Description   : Function to change paramter, run inference and get result.
*                 To use this function, all param values must be defined.
* Arguments     : param = pointer to parameter to be changed.
*                 out_ptr = pointer to store output buffer starting pointer
*                 out_size = size of output buffer
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::Pre(s_preproc_param_t* param, void** out_ptr, uint32_t* out_size)
{
    uint8_t ret = 0;
    drpai_data_t proc[DRPAI_INDEX_NUM];
    struct timespec ts_start, ts_end;
    drpai_status_t drpai_status;
    fd_set rfds;
    struct timespec tv;
    int8_t ret_drpai;
    double preproc_time = 0;
    sigset_t sigset;
    float diff = 0;
    int8_t param_modified = 0;
    int8_t weight_modified = 0;
    uint32_t addr = 0;
    uint32_t size = 0;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);

    /* Update the parameter pre_in_addr */
    if (param->pre_in_addr != internal_param_val.pre_in_addr && param->pre_in_addr != INVALID_ADDR)
    {
        internal_param_val.pre_in_addr = param->pre_in_addr;
    }
#ifdef DEBUG_LOG
    timespec_get(&ts_start, TIME_UTC);
#endif
    param_modified = (int8_t) UpdateParamData(*param);
    if (PRE_ERROR_UI == param_modified)
    {
        return PRE_ERROR;
    }

#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] UpdateParamData() Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif

    if (MODE_PRE == run_mode)
    {
        weight_modified = (int8_t) UpdateWeightData(*param);
        if (PRE_ERROR_UI == weight_modified)
        {
            return PRE_ERROR;
        }
    }

#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] UpdateWeightData() Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif

    /* Write updated drp_param.bin to memory */
    if (param_modified)
    {
        addr = drpai_obj_info.drpai_address.drp_param_addr + drpai_obj_info.data_inout.start_address;
        size = drpai_obj_info.drpai_address.drp_param_size;
        ret = LoadDataToMem(param_data, addr ,size );
        if ( PRE_SUCCESS < ret )
        {
            std::cerr << "[ERROR] Failed to load param data to memory : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }

#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] parammodified Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif

    if (weight_modified)
    {
        addr = drpai_obj_info.drpai_address.weight_addr + drpai_obj_info.data_inout.start_address;
        size = drpai_obj_info.drpai_address.weight_size;
        ret = LoadDataToMem(weight_data, addr, size);
        if ( PRE_SUCCESS < ret )
        {
            std::cerr << "[ERROR] Failed to load weight data to memory : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }
#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] weightmodified Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    if (MODE_POST == run_mode)
    {
        print_preproc_param(internal_param_val, MODE_POST);

    }
    else
    {
        print_preproc_param(internal_param_val);
    }
    /*Debug Only: Save drp_param.bin data. Affect processing time.*/
#endif

    /* Start DRP-AI */
    proc[DRPAI_INDEX_INPUT].address       = internal_param_val.pre_in_addr;
    proc[DRPAI_INDEX_INPUT].size          = drpai_obj_info.drpai_address.data_in_size;
    proc[DRPAI_INDEX_DRP_CFG].address     = drpai_obj_info.drpai_address.drp_config_addr + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_CFG].size        = drpai_obj_info.drpai_address.drp_config_size;
    proc[DRPAI_INDEX_DRP_PARAM].address   = drpai_obj_info.drpai_address.drp_param_addr  + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_PARAM].size      = drpai_obj_info.drpai_address.drp_param_size;
    proc[DRPAI_INDEX_AIMAC_DESC].address  = drpai_obj_info.drpai_address.desc_aimac_addr + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_AIMAC_DESC].size     = drpai_obj_info.drpai_address.desc_aimac_size;
    proc[DRPAI_INDEX_DRP_DESC].address    = drpai_obj_info.drpai_address.desc_drp_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_DESC].size       = drpai_obj_info.drpai_address.desc_drp_size;
    proc[DRPAI_INDEX_WEIGHT].address      = drpai_obj_info.drpai_address.weight_addr     + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_WEIGHT].size         = drpai_obj_info.drpai_address.weight_size;
    proc[DRPAI_INDEX_OUTPUT].address      = drpai_obj_info.drpai_address.data_out_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_OUTPUT].size         = drpai_obj_info.drpai_address.data_out_size;

#ifdef DEBUG_LOG
    /*Start Timer */
    timespec_get(&ts_start, TIME_UTC);
#endif
    errno = 0;
    if ( PRE_SUCCESS != ioctl(drpai_obj_info.drpai_fd , DRPAI_START, &proc[0]))
    {
        std::cerr << "[ERROR] Failed to run DRPAI_START : errno=" <<  errno << std::endl;
        return PRE_ERROR;
    }
    /* Wait till DRP-AI ends */
    FD_ZERO(&rfds);
    FD_SET(drpai_obj_info.drpai_fd , &rfds);
    tv.tv_sec = 5;
    tv.tv_nsec = 0;

    ret_drpai = pselect(drpai_obj_info.drpai_fd +1, &rfds, NULL, NULL, &tv, &sigset);

    if(0 == ret_drpai)
    {
        std::cerr << "[ERROR] DRP-AI timed out : errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    else if (-1 == ret_drpai)
    {
        std::cerr << "[ERROR] Failed to run DRP-AI Driver pselect : errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    if (FD_ISSET(drpai_obj_info.drpai_fd , &rfds))
    {
        errno = 0;
        ret_drpai = ioctl(drpai_obj_info.drpai_fd, DRPAI_GET_STATUS, &drpai_status);
        if (-1 == ret_drpai)
        {
            std::cerr << "[ERROR] Failed to run DRPAI_GET_STATUS : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }
#ifdef DEBUG_LOG
    /*Stop Timer */
    timespec_get(&ts_end, TIME_UTC);
    preproc_time = timedifference_msec(ts_start, ts_end);
    std::cout << "[TIME] PreRuntime DRP-AI processing time : " << std::fixed << std::setprecision(2)<< preproc_time << " msec" << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif
    /* Obtain result. Result is stored in internal_buffer */
    ret = GetResult(drpai_obj_info.data_inout.data_out_addr, drpai_obj_info.data_inout.data_out_size);
    if (PRE_SUCCESS < ret)
    {
        std::cerr << "[ERROR] Failed to get result." << std::endl;
        return PRE_ERROR;
    }
#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout << "[TIME] GetResult() Processing Time : " << std::fixed << std::setprecision(2)<< diff << " msec" << std::endl;
#endif

    *out_ptr = internal_buffer;
    *out_size = internal_buffer_size;

    return PRE_SUCCESS;
}
