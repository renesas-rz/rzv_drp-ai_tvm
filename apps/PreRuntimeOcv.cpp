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
#include <opencv2/opencv.hpp>

PreRuntime::PreRuntime()
{
}

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

#if 0
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
#endif

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
    std::string str;
    uint32_t l_addr = 0;
    uint32_t l_size = 0;
    std::string element, a, s;
    std::string dir;
    uint8_t cnt = 0;
    errno = 0;
    int index = 0;

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

	index++;
	strncpy(this->addrmap[index].name, element.c_str(), sizeof(addrmap[0].name));
	this->addrmap[index].addr_offset = l_addr;
	this->addrmap[index].size        = l_size;


        if ("drp_config" == element)
        {
            drpai_obj_info.drpai_address.drp_config_addr = l_addr;
            drpai_obj_info.drpai_address.drp_config_size = l_size;
        }
        else if ("desc_aimac" == element || "aimac_desc" == element)
        {
            drpai_obj_info.drpai_address.desc_aimac_addr = l_addr;
            drpai_obj_info.drpai_address.desc_aimac_size = l_size;
        }
        else if ("desc_drp" == element || "drp_desc" == element)
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
        else if ("aimac_param_cmd" == element)
        {
            drpai_obj_info.drpai_address.aimac_param_cmd_addr = l_addr;
            drpai_obj_info.drpai_address.aimac_param_cmd_size = l_size;
        }
        else if ("aimac_param_desc" == element)
        {
            drpai_obj_info.drpai_address.aimac_param_desc_addr = l_addr;
            drpai_obj_info.drpai_address.aimac_param_desc_size = l_size;
        }
        else if ("aimac_cmd" == element)
        {
            drpai_obj_info.drpai_address.aimac_cmd_addr = l_addr;
            drpai_obj_info.drpai_address.aimac_cmd_size = l_size;
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
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadFileToMemDynamic(std::string data, unsigned long offset, unsigned long size)
{
    int8_t  ret_load_data = PRE_SUCCESS;
    int     obj_fd = 0;
    uint8_t drpai_buf[BUF_SIZE];
    int     drpai_fd = drpai_obj_info.drpai_fd;
    drpai_data_t drpai_data;
    uint8_t ret = 0;
    int     i = 0;

    errno = 0;
    obj_fd = open(data.c_str(), O_RDONLY);
    if (0 > obj_fd )
    {
        std::cerr << "[ERROR] Failed to open " << data << ": errno=" << errno << std::endl;
        ret_load_data = PRE_ERROR;
        goto end;
    }
    drpai_data.address = drpai_obj_info.data_inout.start_address + offset;
    drpai_data.size = size;
    errno = 0;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        ret_load_data = PRE_ERROR;
        goto end;
    }

    for (i = 0 ; i < (int)(drpai_data.size/BUF_SIZE) ; i++)
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
    if ( 0 != (drpai_data.size%BUF_SIZE))
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, (drpai_data.size % BUF_SIZE));
        if ( 0 > ret)
        {
            std::cerr << "[ERROR] Failed to read "<<data<<" : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
        ret = write(drpai_fd , drpai_buf, (drpai_data.size % BUF_SIZE));
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
uint8_t PreRuntime::LoadDataToMem(std::vector<uint8_t> *data, unsigned long from, unsigned long size)
{
    int          drpai_fd = drpai_obj_info.drpai_fd;
    drpai_data_t drpai_data;
    uint8_t      ret = 0;
    int          i = 0;

    errno = 0;
    drpai_data.address = from;
    drpai_data.size = size;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    for (i = 0 ; i < (int)(drpai_data.size/BUF_SIZE) ; i++)
    {
        errno = 0;
        ret = write(drpai_fd, &((*data)[BUF_SIZE*i]), BUF_SIZE);
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }

    if ( 0 != (drpai_data.size%BUF_SIZE))
    {
        errno = 0;
        ret = write(drpai_fd, &((*data)[BUF_SIZE*(int)(drpai_data.size/BUF_SIZE)]), (drpai_data.size % BUF_SIZE));
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
    //uint32_t data_in_size;
    //uint32_t data_out_size;
    uint32_t in_size = 0;
    uint32_t out_size = 0;

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
    //data_in_size = drpai_obj_info.data_inout.data_in_size;
    //data_out_size = drpai_obj_info.data_inout.data_out_size;
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
uint64_t PreRuntime::GetStartAddress(uint64_t addr, drpai_data_t drpai_data)
{
    uint64_t drpai_mem_addr_end = drpai_data.address+drpai_data.size - 1;
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
uint8_t PreRuntime::Load(const std::string pre_dir, uint32_t area_size, uint8_t mode)
{
    uint8_t ret = 0;
    struct stat statBuf;
    std::string tmp_dir = "/";
    std::string dir = pre_dir;
    std::string pp_prefix = "pp";
    std::string tmp_address_file = dir+"/addr_map.txt";
    uint64_t addr = 0;
    uint32_t size = 0;
    int      i = 0;
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
    //if (0 == stat(tmp_address_file.c_str(),&statBuf))
    //{
    //    obj_prefix = dir;
    //}

    /*Define necessary filename*/
    const std::string address_file = dir+"/addr_map.txt";
    const std::string drpai_file_path[] =
    {
        "",
        dir+"/drp_desc.bin",
        dir+"/drp_config.mem",
        dir+"/drp_param.bin",
        dir+"/aimac_desc.bin",
        dir+"/weight.bin",
        dir+"/aimac_cmd.bin",
        dir+"/aimac_param_desc.bin",
        dir+"/aimac_param_cmd.bin",
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
    
    /* Set the DRP virtual start address to 0, indicating the actual 40-bit physical address. */
    errno = 0;
    drpai_adrconv_t drpai_adrconv;
    drpai_adrconv.org_address  = 0x0000000;
    drpai_adrconv.conv_address = drpai_data0.address + (drpai_data0.size - area_size);
    drpai_adrconv.size         = area_size;
    drpai_adrconv.mode         = DRPAI_ADRCONV_MODE_REPLACE;
    if ( PRE_SUCCESS != ioctl(drpai_obj_info.drpai_fd , DRPAI_SET_ADRCONV, &drpai_adrconv))
    {
       std::cerr << "[ERROR] Failed to run DRPAI_SET_ADRCONV : errno=" <<  errno << std::endl;
       return PRE_ERROR;
    }

    /*Define the start address.*/
    drpai_obj_info.data_inout.start_address = drpai_adrconv.conv_address;

    
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
    for (i = 1;i < (int)(sizeof(drpai_file_path)/sizeof(drpai_file_path[0])); i++)
    {
        switch (i)
        {
            case (INDEX_W):
                addr = drpai_obj_info.drpai_address.weight_addr;
                size = drpai_obj_info.drpai_address.weight_size;
                break;
            case (INDEX_C):
                addr = drpai_obj_info.drpai_address.drp_config_addr;
                size = drpai_obj_info.drpai_address.drp_config_size;
                break;
            case (INDEX_P):
                addr = drpai_obj_info.drpai_address.drp_param_addr;
                size = drpai_obj_info.drpai_address.drp_param_size;
                break;
            case (INDEX_A):
                addr = drpai_obj_info.drpai_address.desc_aimac_addr;
                size = drpai_obj_info.drpai_address.desc_aimac_size;
                break;
            case (INDEX_D):
                addr = drpai_obj_info.drpai_address.desc_drp_addr;
                size = drpai_obj_info.drpai_address.desc_drp_size;
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
            ret = LoadDataToMem(&param_data, addr+drpai_obj_info.data_inout.start_address, size);
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
            ret = LoadDataToMem(&weight_data, addr + drpai_obj_info.data_inout.start_address, size);
        }
        else
        {
	    int index;
            /*Read data from file and write to memory */
	    for(index = 1; index < 13; index++){
		    if(strstr(drpai_file_path[i].c_str(), addrmap[index].name) != NULL){
			    break;
		    }
	    }
	    if(index < 13){
	        addr = addrmap[index].addr_offset;
		size = addrmap[index].size;
                ret = LoadFileToMemDynamic(drpai_file_path[i], addrmap[index].addr_offset, addrmap[index].size);
	    } else {
	        std::cerr << "[ERROR] Failed to load data to memory--no match addrmap: "<< drpai_file_path[i] << std::endl;
                return PRE_ERROR;
	    }

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

    /*Free internal buffer if its memory is already allocated */
    if(internal_buffer != NULL )
    {
        free(internal_buffer);
	internal_buffer = NULL;
    }
    /*Allocate internal output buffer*/
    if(sizeof(uint8_t) == pre_out_type_size)
    {
        internal_buffer = (uint8_t*) malloc(output_size);
    }
    else if (sizeof(uint16_t) == pre_out_type_size)
    {
        internal_buffer = (uint16_t*) malloc(output_size);
    }
    else if (sizeof(uint32_t) == pre_out_type_size)
    {
        internal_buffer = (uint32_t*) malloc(output_size);
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
    internal_buffer_size = (uint32_t) (output_size/pre_out_type_size);

    return PRE_SUCCESS;
}
/*****************************************
* Function Name : get_udmabuf_addr
* Description   : Function to obtain the u-dma-buf start address.
* Arguments     : -
* Return value  : uint32_t = u-dma-buf start address in 32-bit.
******************************************/
namespace {
uint64_t get_udmabuf_addr()
{
    int     fd = 0;
    char    addr[1024];
    int32_t read_ret = 0;
    uint64_t udmabuf_addr_start = 0;
    errno = 0;

    fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY);
    if (0 > fd)
    {
        std::cerr << "[ERROR] Failed to open udmabuf phys_addr " << std::endl;
        return 0;
    }
    read_ret = read(fd, addr, 1024);
    if (0 > read_ret)
    {
        std::cerr << "[ERROR] Failed to read udmabuf phys_addr " << std::endl;
        close(fd);
        return 0;
    }
    sscanf(addr, "%lx", &udmabuf_addr_start);
    close(fd);

    return udmabuf_addr_start;
}
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
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    uint64_t udmabuf_addr_start;
    int udmabuf_fd;
    unsigned char * img_buffer;

    udmabuf_addr_start = get_udmabuf_addr();
    if(udmabuf_addr_start != param->pre_in_addr){
        std::cerr << "[ERROR] OCVerror input buffer is not udmabuf" << std::endl;
        return PRE_ERROR;
    }
    udmabuf_fd = open("/dev/udmabuf0", O_RDWR | O_SYNC );
    if (0 > udmabuf_fd)
    {
        std::cerr << "[ERROR] OCVerror Failed to open udmabuf at Preruntime::Pre" << std::endl;
        return 0;
    }
    img_buffer =(unsigned char*) mmap(NULL, 640*480*3 ,PROT_READ|PROT_WRITE, MAP_SHARED,  udmabuf_fd, 0);
    if (MAP_FAILED == img_buffer)
    {
        std::cerr << "[ERROR] Failed to run mmap: udmabuf at Preruntime::Pre " << std::endl;
        close(udmabuf_fd);
        return 0;
    }



    cv::Mat ocvimg(480, 640, CV_8UC3, img_buffer);
    close(udmabuf_fd);


    //cv::cvtColor(ocvimg, ocvimg, cv::COLOR_BGR2RGB);
    //cv::resize(ocvimg, ocvimg,cv::Size(341,256),0,0, cv::INTER_LINEAR );
    cv::resize(ocvimg, ocvimg,cv::Size(224,224),0,0, cv::INTER_LINEAR );

    //cv::getRectSubPix(ocvimg, cv::Size(224,224), cv::Point(170,127), ocvimg);
    cv::Mat focvimg(224, 224, CV_32FC3);
    ocvimg.convertTo(focvimg, CV_32FC3);
    
    /* Obtain result. Result is stored in internal_buffer */
    ret = GetResult(drpai_obj_info.data_inout.data_out_addr, drpai_obj_info.data_inout.data_out_size);
    if (PRE_SUCCESS < ret)
    {
        std::cerr << "[ERROR] Failed to get result." << std::endl;
        return PRE_ERROR;
    }

    float *ftmp = (float *)focvimg.data;
    int csize = focvimg.cols * focvimg.rows;
    for(int i = 0;  i < csize; i++){
	    ((float *)internal_buffer)[i]         = ((ftmp[i*3+2]/255.0) -0.485)/0.229;
	    ((float *)internal_buffer)[i+csize]   = ((ftmp[i*3+1]/255.0) -0.456)/0.224;
	    ((float *)internal_buffer)[i+csize*2] = ((ftmp[i*3+0]/255.0) -0.406)/0.225;
    }

    *out_ptr = internal_buffer;
    *out_size = internal_buffer_size;

    return PRE_SUCCESS;
}
