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
* Version      : 0.10
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
    free(internal_buffer);
    /*Close DRP-AI Driver*/
    if (0 <= drpai_fd)
    {
        errno = 0;
        if (PRE_SUCCESS != close(drpai_fd))
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
    std::string str;
    uint32_t l_addr = 0;
    uint32_t l_size = 0;
    std::string element, a, s;
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
            address.drp_config_addr = l_addr;
            address.drp_config_size = l_size;
        }
        else if ("desc_aimac" == element)
        {
            address.desc_aimac_addr = l_addr;
            address.desc_aimac_size = l_size;
        }
        else if ("desc_drp" == element)
        {
            address.desc_drp_addr = l_addr;
            address.desc_drp_size = l_size;
        }
        else if ("drp_param" == element)
        {
            address.drp_param_addr = l_addr;
            address.drp_param_size = l_size;
        }
        else if ("weight" == element)
        {
            address.weight_addr = l_addr;
            address.weight_size = l_size;
        }
        else if ("data_in" == element)
        {
            address.data_in_addr = l_addr;
            address.data_in_size = l_size;
        }
        else if ("data" == element)
        {
            address.data_addr = l_addr;
            address.data_size = l_size;
        }
        else if ("data_out" == element)
        {
            address.data_out_addr = l_addr;
            address.data_out_size = l_size;
        }
        else if ("work" == element)
        {
            address.work_addr = l_addr;
            address.work_size = l_size;
        }
        else
        {
            /*Ignore other space*/
        }
    }
    ifs.close();
    return PRE_SUCCESS;
}

/*****************************************
* Function Name : LoadFileToMem
* Description   : Loads a file to memory via DRP-AI Driver
* Arguments     : data = filename to be written to memory
*                 drpai_fd = file descriptor of DRP-AI Driver
*                 from = memory start address where the data is written
*                 size = data size to be written
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadFileToMem(std::string data, unsigned long from, unsigned long size)
{
    int8_t ret_load_data = PRE_SUCCESS;
    uint8_t obj_fd = 0;
    uint8_t drpai_buf[BUF_SIZE];
    drpai_data_t drpai_data;
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
    drpai_data.address = from;
    drpai_data.size = size;

    errno = 0;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        ret_load_data = PRE_ERROR;
        goto end;
    }

    for (i = 0 ; i<(drpai_data.size/BUF_SIZE) ; i++)
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, BUF_SIZE);
        if ( 0 > ret)
        {
            std::cerr << "[ERROR] Failed to read "<<data<<" : errno=" << errno << std::endl;
            ret_load_data = PRE_ERROR;
            goto end;
        }
        ret = write(drpai_fd, drpai_buf,  BUF_SIZE);
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
        ret = write(drpai_fd, drpai_buf, (drpai_data.size % BUF_SIZE));
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
*                 drpai_fd = file descriptor of DRP-AI Driver
*                 from = memory start address where the data is written
*                 size = data size to be written
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::LoadDataToMem(std::vector<uint8_t> data, unsigned long from, unsigned long size)
{
    uint8_t ret;
    drpai_data_t drpai_data;
    errno = 0;

    drpai_data.address = from;
    drpai_data.size = size;

    errno = 0;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    for (int i = 0 ; i<(drpai_data.size/BUF_SIZE) ; i++)
    {
        ret = write(drpai_fd, &data[BUF_SIZE*i],  BUF_SIZE);
        if ( -1 == ret )
        {
            std::cerr << "[ERROR] Failed to write via DRP-AI Driver : errno=" << errno << std::endl;
            return PRE_ERROR;
        }
    }

    if ( 0 != (drpai_data.size%BUF_SIZE))
    {
        ret = write(drpai_fd,  &data[BUF_SIZE*(int)(drpai_data.size/BUF_SIZE)], (drpai_data.size % BUF_SIZE));
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
* Arguments     : data = filename to be written to memory
*                 drpai_fd = file descriptor of DRP-AI Driver
*                 from = memory start address where the data is written
*                 size = data size to be written
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
    std::string str             = "";
    std::string element         = "";
    std::string str_value       = "";
    uint8_t val_u8      = 0;
    uint32_t val_u32    = 0;
    s_op_t tmp_op;
    s_op_param_t tmp_param;
    bool first_itr_done = false;
    errno = 0;

    /* Open param info file */
    std::ifstream param_file(info_file);
    if (param_file.fail())
    {
        std::cerr << "[ERROR] Failed to open Param Info file "<<info_file<<": errno=" << errno << std::endl;
        return -1;
    }
    while(getline(param_file, str))
    {
        /*Remove all spaces*/
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
        std::istringstream iss(str);
        /*Get first word*/
        getline(iss, element, ',');
        /*Operator info*/
        if (element.size() >= offset_add.size() &&
            std::equal(std::begin(offset_add), std::end(offset_add), std::begin(element)))
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
            val_u8 = static_cast<uint8_t>(std::stoi(str_value));
            tmp_op.offset = val_u8;
            /*Get op info*/
            while(getline(iss, element, ','))
            {
                /* layer_name */
                if (element.size() >= layer_name.size() &&
                    std::equal(std::begin(layer_name), std::end(layer_name), std::begin(element)))
                {
                    str_value = element.substr(layer_name.size());
                    tmp_op.name = str_value;
                }
                /* drp_lib */
                else if (element.size() >= drp_lib.size() &&
                    std::equal(std::begin(drp_lib), std::end(drp_lib), std::begin(element)))
                {
                    str_value = element.substr(drp_lib.size());
                    tmp_op.lib = str_value;
                }
            }
        }
        /*Parameter info*/
        else if (element.size() >= param_head.size() &&
            std::equal(std::begin(param_head), std::end(param_head), std::begin(element)))
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
                if (element.size() >= param_value.size() &&
                    std::equal(std::begin(param_value), std::end(param_value), std::begin(element)))
                {
                    str_value = element.substr(param_value.size());
                    val_u32 =  static_cast<uint32_t>(std::stoul(str_value));
                    tmp_param.value = val_u32;
                }
                /* offset */
                else if (element.size() >= param_offset.size() &&
                    std::equal(std::begin(param_offset), std::end(param_offset), std::begin(element)))
                {
                    str_value = element.substr(param_offset.size());
                    val_u8 = static_cast<uint8_t>(std::stoi(str_value));
                    tmp_param.offset = val_u8;
                }
                /* size */
                else if (element.size() >= param_size.size() &&
                    std::equal(std::begin(param_size), std::end(param_size), std::begin(element)))
                {
                    str_value = element.substr(param_size.size());
                    val_u8 = static_cast<uint8_t>(std::stoi(str_value));
                    tmp_param.size = val_u8;
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
    std::string dout_rgb_order      = P_DOUT_RGB_ORDER;
    std::string resize_alg          = P_RESIZE_ALG;

    /*pre_in_shape_w, pre_in_shape_h, pre_in_addr*/
    tmp_op = &param_info[0];
    for (i = 0; i<tmp_op->param_list.size(); i++)
    {
        /*pre_in_shape_w*/
        if (tmp_op->param_list[i].name == img_iwidth)
        {
            internal_param_val.pre_in_shape_w = (uint16_t) tmp_op->param_list[i].value;
        }
        /*pre_in_shape_h*/
        else if (tmp_op->param_list[i].name == img_iheight)
        {
            internal_param_val.pre_in_shape_h = (uint16_t) tmp_op->param_list[i].value;
        }
        /*pre_in_addr*/
        else if (tmp_op->param_list[i].name == readaddr)
        {
            internal_param_val.pre_in_addr = (uint32_t) tmp_op->param_list[i].value;
        }
    }

    /*pre_in_format*/
    /*TODO: consider transpose_chw_hwc, conv_x2gray, memcpy, crop, NV12 */
    tmp_op = &param_info[0];
    for (i = 0; i<tmp_op->param_list.size(); i++)
    {
        if (tmp_op->lib ==  lib_convyuv2rgb && tmp_op->param_list[i].name == input_yuv_format )
        {
            internal_param_val.pre_in_format = (uint16_t) tmp_op->param_list[i].value;
            break;
        }
        else
        {
            internal_param_val.pre_in_format = (uint16_t) INPUT_UNKNOWN;
        }
    }
    if (internal_param_val.pre_in_format  ==  INPUT_UNKNOWN )
    {
        if (tmp_op->lib ==  lib_resize_hwc || tmp_op->lib ==  lib_imagescaler)
        {
            for (i = 0; i<param_info.size(); i++)
            {
                tmp_op = &param_info[i];
                if (tmp_op->lib ==  lib_imagescaler )
                {
                    for (j = 0; j<tmp_op->param_list.size(); j++)
                    {
                        if (tmp_op->param_list[i].name == dout_rgb_order )
                        {
                            internal_param_val.pre_in_format = (uint16_t)INPUT_UNKNOWN - 1 - tmp_op->param_list[i].value;
                            /*Ignore after ops*/
                            break;
                        }
                    }
                    /*Ignore after ops*/
                    break;
                }
            }
        }
    }

    /*resize_alg*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_resize_hwc)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                if (tmp_op->param_list[j].name == resize_alg )
                {
                    internal_param_val.resize_alg = (uint8_t) tmp_op->param_list[j].value;
                    /*Ignore after ops*/
                    break;
                }
            }
            break;
        }
    }

    /*resize_w, resize_h*/
    for (i = 0; i<param_info.size(); i++)
    {
        tmp_op = &param_info[i];
        if (tmp_op->lib ==  lib_resize_hwc)
        {
            for (j = 0; j<tmp_op->param_list.size(); j++)
            {
                /*resize_w*/
                if (tmp_op->param_list[j].name == img_owidth)
                {
                    internal_param_val.resize_w = (uint16_t)tmp_op->param_list[j].value;
                }
                /*resize_h*/
                else if (tmp_op->param_list[j].name == img_oheight)
                {
                    internal_param_val.resize_h = (uint16_t)tmp_op->param_list[j].value;
                }
            }
        }
    }


    /*cof_add, cof_mul*/
    int index, mod;
    uint8_t float_cvt[2];
    float a;
    uint8_t float_num = sizeof(weight_data)/sizeof(float)+1;
    uint8_t cof_num = 3;
    uint8_t cood_num = 2;
    /*Minimum size of coefficient data. cof_add(6bytes)+padding(2bytes)+cof_mul(6bytes)*/
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
                a = float16_to_float32(float_cvt[1]*16*16+float_cvt[0]);
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
#ifdef DEBUG_LOG
    print_preproc_param(internal_param_val);
#endif
    return PRE_SUCCESS;
}

/*****************************************
* Function Name : Load
* Description   : Loads PreRuntime Object data.
* Arguments     : pre_dir = folder name to be loaded.
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t PreRuntime::Load(const std::string pre_dir)
{
    uint8_t ret = 0;
    struct stat statBuf;
    std::string tmp_dir = "/";
    std::string dir = pre_dir;
    uint32_t addr = 0;
    uint32_t size = 0;
    uint8_t i = 0;

    /* Delete unnecessary slush */
    if (dir.size() >= tmp_dir.size() &&
            dir.find(tmp_dir, dir.size() - tmp_dir.size()) != std::string::npos)
    {
        dir = dir.erase(dir.size()-1);
    }
    /* Check whether directory exists*/
    if (stat(dir.c_str(),&statBuf)!=0)
    {
        std::cerr << "[ERROR] Directory " << dir << " not found." << std::endl;
        return PRE_ERROR;
    }

    /*Define necessary filename*/
    const std::string address_file = dir+"/"+dir+"_addrmap_intm.txt";
    const std::string drpai_file_path[6] =
    {
        "",
        dir+"/drp_desc.bin",
        dir+"/"+dir+"_drpcfg.mem",
        dir+"/drp_param.bin",
        dir+"/aimac_desc.bin",
        dir+"/"+dir+"_weight.dat",
    };
    const std::string drpai_param_file = dir + "/drp_param_info.txt";

    errno = 0;
    /* Read Address Map List file */
    ret = ReadAddrmapTxt(address_file);
    if (PRE_SUCCESS < ret)
    {
        std::cerr << "[ERROR] Failed to read Address Map List " << address_file  << std::endl;
        return 1;
    }

    /*Open DRP-AI Driver*/
    drpai_fd = open("/dev/drpai0", O_RDWR);
    if (PRE_SUCCESS > drpai_fd)
    {
        std::cerr << "[ERROR] Failed to open DRP-AI Driver : errno=" << errno << std::endl;
        return 1;
    }

    /* Read PreRuntime Object and write it to memory via DRP-AI Driver */
    for (i = 1;i<6 ;i++)
    {
        switch (i)
        {
            case (INDEX_W):
                addr = address.weight_addr;
                size = address.weight_size;
                break;
            case (INDEX_C):
                addr = address.drp_config_addr;
                size = address.drp_config_size;
                break;
            case (INDEX_P):
                addr = address.drp_param_addr;
                size = address.drp_param_size;
                break;
            case (INDEX_A):
                addr = address.desc_aimac_addr;
                size = address.desc_aimac_size;
                break;
            case (INDEX_D):
                addr = address.desc_drp_addr;
                size = address.desc_drp_size;
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
                return ret;
            }
            ret = LoadDataToMem(param_data, addr, size);
        }
        else if (INDEX_W == i)
        {
            /*Read and Hold weight.dat data internally in weight_data array. */
            ret = ReadFileData(weight_data, drpai_file_path[i], size);
            if ( PRE_SUCCESS < ret )
            {
                std::cerr << "[ERROR] Failed to read file "<< drpai_file_path[i] << std::endl;
                return ret;
            }
            ret = LoadDataToMem(weight_data, addr, size);
        }
        else
        {
            /*Read data from file and write to memory */
            ret = LoadFileToMem(drpai_file_path[i], addr, size);
        }
        /*Error Check*/
        if ( PRE_SUCCESS < ret )
        {
            std::cerr << "[ERROR] Failed to load data to memory: "<< drpai_file_path[i] << std::endl;
            return ret;
        }
    }
    /*Parse drp_param_info.txt*/
    ret = ParseParamInfo(drpai_param_file);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read param info file: "<< drpai_param_file << std::endl;
        return ret;
    }
    /*Load param data*/
    ret = LoadParamInfo();
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to load param info. " << std::endl;
        return ret;
    }
    return PRE_SUCCESS;
}

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
void PreRuntime::UpdateInputShape(const uint16_t w, const uint16_t h)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string img_iwidth      = P_IMG_IWIDTH;
    std::string img_iheight     = P_IMG_IHEIGHT;
    std::string img_owidth      = P_IMG_OWIDTH;
    std::string img_oheight     = P_IMG_OHEIGHT;
    std::string lib_resize_hwc  = LIB_RESIZE_HWC;
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
        if (tmp_op->lib == lib_resize_hwc)
        {
            /*Ignore all op after resize_hwc */
            break;
        }
    }
    internal_param_val.pre_in_shape_w = w;
    internal_param_val.pre_in_shape_h = h;
    return;
}

/*****************************************
* Function Name     : UpdateResizeShape
* Description       : Function to update resize_w and resize_h in param data.
* Arguments         : w = uint16_t number. Width to be resized.
*                     h = uint16_t number. Height to be resized.
* Return value      : -
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
* Function Name     : UpdateInputFormat
* Description       : Function to update pre_in_format in param data.
* Arguments         : val = uint16_t number. New input format.
* Return value      : 0 if succeeded
*                     not 0 otherwise
******************************************/
uint8_t PreRuntime::UpdateInputFormat(const uint16_t val)
{
    s_op_t* tmp_op;
    s_op_param_t* tmp_param;
    std::string in_format_name = P_INPUT_YUV_FORMAT;
    std::string lib_convyuv2rgb = LIB_CONVYUV2RGB;
    std::string lib_imagescaler = LIB_IMAGESCALER;
    std::string out_rgb_format = P_DOUT_RGB_FORMAT;
    std::string out_rgb_order  = P_DOUT_RGB_ORDER;
    uint8_t i = 0;
    uint8_t j = 0;
    uint16_t offset = 0;
    if (INPUT_YUYV == val)
    {
        for (i = 0; i<param_info.size(); i++)
        {
            tmp_op = &param_info[i];
            if (tmp_op->lib == lib_convyuv2rgb)
            {
                pre_in_shape_c = NUM_C_YUV422;
                pre_out_shape_c = NUM_C_RGB;
                for (j = 0; j<tmp_op->param_list.size(); j++)
                {
                    tmp_param = &tmp_op->param_list[j];
                    if ( tmp_param->name == in_format_name )
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
    }
    else
    {
        std::cerr << "[ERROR] Wrong input format: 0x"<<std::hex <<val<<std::endl;
        return PRE_ERROR;
    }
    internal_param_val.pre_in_format = val;
    return PRE_SUCCESS;
}

/*****************************************
* Function Name     : UpdateResizeAlg
* Description       : Function to update resize_alg in param data.
* Arguments         : val = uint8_t number. New resize algorithm.
* Return value      : -
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
* Function Name     : UpdateCoefficient
* Description       : Function to update cof_add and cof_mul in weight data.
* Arguments         : new_cof_add = float array. New cof_add.
*                     new_cof_mul = float array. New cof_mul.
* Return value      : 0 if succeeded
*                     not 0 otherwise
******************************************/
uint8_t PreRuntime::UpdateCoefficient(const float* new_cof_add, const float* new_cof_mul)
{
    uint8_t cof_num = 3;
    uint8_t cood_num = 2;
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
                    weight_offset = tmp_param->value - address.weight_addr;
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

    for (i = weight_offset;i< weight_size+weight_offset;i++)
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
* Function Name     : IsDifferentFmInternal
* Description       : Function to update cof_add and cof_mul in weight data.
* Arguments         : new_cof_add = float array. New cof_add.
*                     new_cof_mul = float array. New cof_mul.
* Return value      : true if argument is different from internal_param_val
*                     false if not
******************************************/
bool PreRuntime::IsDifferentFmInternal(const float* new_cof_add, const float* new_cof_mul)
{
    uint8_t i = 0;
    uint8_t size = sizeof(new_cof_add)/sizeof(float);
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
* Function Name     : IsSupportedFormat
* Description       : Function to check whether input format is supported or not.
* Arguments         : format = input format
* Return value      : true if argument is supported format
*                     false if not
******************************************/
bool PreRuntime::IsSupportedFormat(uint16_t format)
{
    uint8_t i = 0;
    uint8_t size = sizeof(supported_format)/sizeof(uint16_t);
    for (i = 0; i<size ; i++)
    {
        if (supported_format[i] == format)
        {
            return true;
        }
    }
    return false;
}

/*****************************************
* Function Name     : UpdateParamData
* Description       : Check whether input parameter value is different from current parameter value,
*                     and if different, change the drp_param data according to the input parameter value.
* Arguments         : param = input parameter value.
* Return value      : true if input parameter value is different from current value.
*                     false if not.
******************************************/
bool PreRuntime::UpdateParamData(const s_preproc_param_t param)
{
    bool different = false;

    /*Input image size (shape) */
    uint16_t new_pre_in_shape_w = param.pre_in_shape_w;
    uint16_t new_pre_in_shape_h = param.pre_in_shape_h;

    /* If one of them is INVALID_SHAPE, use the current shape */
    if (new_pre_in_shape_w == INVALID_SHAPE) new_pre_in_shape_w = internal_param_val.pre_in_shape_w;
    if (new_pre_in_shape_h == INVALID_SHAPE) new_pre_in_shape_h = internal_param_val.pre_in_shape_h;
    /* Size restriction check */
    if (MIN_INPUT_WIDTH > new_pre_in_shape_w || MAX_INPUT_WIDTH < new_pre_in_shape_w)
    {
        std::cerr<<"[WARNING] Invalid parameter: pre_in_shape_w="<<new_pre_in_shape_w<<std::endl;
        std::cerr<<"          Current value will be used instead:"<<internal_param_val.pre_in_shape_w<<std::endl;
        new_pre_in_shape_w = internal_param_val.pre_in_shape_w;
    }
    if (MIN_INPUT_HEIGHT > new_pre_in_shape_h || MAX_INPUT_HEIGHT < new_pre_in_shape_h )
    {
        std::cerr<<"[WARNING] Invalid parameter: pre_in_shape_h="<<new_pre_in_shape_h<<std::endl;
        std::cerr<<"          Current value will be used instead:"<<internal_param_val.pre_in_shape_h<<std::endl;
        new_pre_in_shape_h = internal_param_val.pre_in_shape_h;
    }
    if (new_pre_in_shape_w != internal_param_val.pre_in_shape_w
        || new_pre_in_shape_h != internal_param_val.pre_in_shape_h)
    {
        UpdateInputShape(new_pre_in_shape_w, new_pre_in_shape_h);
        different = true;
#ifdef DEBUG_LOG
        std::cout<<"[INFO] Changed pre_in_shape_w: "<<new_pre_in_shape_w<<std::endl;
        std::cout<<"[INFO] Changed pre_in_shape_h: "<<new_pre_in_shape_h<<std::endl;
#endif
    }

    /*Input format */
    uint16_t new_pre_in_format = param.pre_in_format;
    if (new_pre_in_format == INVALID_FORMAT)
    {
        /*param.pre_in_format not defined*/
    }
    else if (!IsSupportedFormat(new_pre_in_format))
    {
        std::cerr<<"[WARNING] Invalid parameter: pre_in_format="<<new_pre_in_format<<std::endl;
        std::cerr<<"          Current value will be used instead: 0x"<<std::hex<<internal_param_val.pre_in_format<<std::endl;
    }
    else if (new_pre_in_format != internal_param_val.pre_in_format)
    {
        UpdateInputFormat(new_pre_in_format);
        different = true;
#ifdef DEBUG_LOG
        std::cout<<"[INFO] Changed pre_in_format: 0x"<<std::hex<<new_pre_in_format<<std::endl;
#endif
    }

    /*Resize output data size */
    uint16_t new_resize_w = param.resize_w;
    uint16_t new_resize_h = param.resize_h;

    /* If one of them is INVALID_SHAPE, use the current shape */
    if (new_resize_w == INVALID_SHAPE) new_resize_w = internal_param_val.resize_w;
    if (new_resize_h == INVALID_SHAPE) new_resize_h = internal_param_val.resize_h;
    /* Size restriction check */
    if (MIN_RESIZE_WIDTH > new_resize_w || MAX_RESIZE_WIDTH < new_resize_w)
    {
        std::cerr<<"[WARNING] Invalid parameter: resize_w="<<new_resize_w<<std::endl;
        std::cerr<<"          Current value will be used instead:"<<internal_param_val.resize_w<<std::endl;
        new_resize_w = internal_param_val.resize_w;
    }
    if (MIN_RESIZE_HEIGHT > new_resize_h || MAX_RESIZE_HEIGHT < new_resize_h )
    {
        std::cerr<<"[WARNING] Invalid parameter: resize_h="<<new_resize_h<<std::endl;
        std::cerr<<"          Current value will be used instead:"<<internal_param_val.resize_h<<std::endl;
        new_resize_h = internal_param_val.resize_h;
    }
    if (new_resize_w != internal_param_val.resize_w
        || new_resize_h != internal_param_val.resize_h)

    {
        UpdateResizeShape(new_resize_w, new_resize_h);
        different = true;
#ifdef DEBUG_LOG
        std::cout<<"[INFO] Changed resize_w: "<<new_resize_w<<std::endl;
        std::cout<<"[INFO] Changed resize_h: "<<new_resize_h<<std::endl;
#endif
    }

    /*Resize algorithm */
    uint8_t new_resize_alg = param.resize_alg;

    if (new_resize_alg == INVALID_RESIZE_ALG)
    {
        /*param.resize_alg not defined*/
    }
    else if ( 1 < new_resize_alg)
    {
        std::cerr<<"[WARNING] Invalid parameter: resize_alg="<<new_resize_alg<<std::endl;
        std::cerr<<"          Current value will be used instead:"<<internal_param_val.resize_alg<<std::endl;
    }
    else if (new_resize_alg != internal_param_val.resize_alg)
    {
        UpdateResizeAlg(new_resize_alg);
        different = true;
#ifdef DEBUG_LOG
        std::cout<<"[INFO] Changed resize_alg: "<<(int) new_resize_alg<<std::endl;
#endif
    }

    /*Update data_in & data_out size*/
    address.data_in_size = new_pre_in_shape_w * new_pre_in_shape_h * pre_in_shape_c;
    address.data_out_size = new_resize_w * new_resize_h * pre_out_shape_c *sizeof(float);

    return different;
}
/*****************************************
* Function Name     : UpdateWeightData
* Description       : Check whether input parameter value is different from current parameter value,
*                     and if different, change the weight data according to the input parameter value.
* Arguments         : param = input parameter value.
* Return value      : true if input parameter value is different from current value.
*                     false if not.
******************************************/
bool PreRuntime::UpdateWeightData(const s_preproc_param_t param)
{
    bool different = false;
    uint8_t size = sizeof(param.cof_add)/sizeof(float);

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
            return false;
        }
        different = true;
#ifdef DEBUG_LOG
        /*Print changed parameters*/
        std::cout<<"[INFO] Changed cof_add:";
        for (int i = 0;i<size;i++)
        {
            std::cout <<" "<<std::setprecision(4)<< new_cof_add[i];
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
    return different;
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
    internal_buffer = (float*) malloc(drpai_data.size);
    if(internal_buffer == NULL)
    {
        std::cerr << "[ERROR] Failed to malloc PreRuntime internal buffer." <<std::endl;
        return PRE_ERROR;
    }
    internal_buffer_size = drpai_data.size/sizeof(float);

    errno = 0;
    /* Assign the memory address and size to be read */
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN: errno=" << errno << std::endl;
        return PRE_ERROR;
    }

    /* Read the memory via DRP-AI Driver and store the output to buffer */
    errno = 0;
    ret = read(drpai_fd, internal_buffer, drpai_data.size);
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
uint8_t PreRuntime::Pre(s_preproc_param_t* param, float** out_ptr, uint32_t* out_size)
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
    bool param_modified = false;
    bool weight_modified = false;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);

    /* Update the parameter pre_in_addr */
    if (param->pre_in_addr != internal_param_val.pre_in_addr && param->pre_in_addr != INVALID_IN_ADDR)
    {
        internal_param_val.pre_in_addr = param->pre_in_addr;
    }
#ifdef DEBUG_LOG
    timespec_get(&ts_start, TIME_UTC);
#endif

    param_modified = UpdateParamData(*param);

#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] UpdateParamData() Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif

    weight_modified = UpdateWeightData(*param);

#ifdef DEBUG_LOG
    timespec_get(&ts_end, TIME_UTC);
    diff = timedifference_msec(ts_start, ts_end);
    std::cout<<"[TIME] UpdateWeightData() Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    timespec_get(&ts_start, TIME_UTC);
#endif

    /* Write updated drp_param.bin to memory */
    if (param_modified)
    {
        ret = LoadDataToMem(param_data, address.drp_param_addr, address.drp_param_size);
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
        ret = LoadDataToMem(weight_data, address.weight_addr, address.weight_size);
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
#endif
    /* Start DRP-AI */
    proc[DRPAI_INDEX_INPUT].address = internal_param_val.pre_in_addr;
    proc[DRPAI_INDEX_INPUT].size = internal_param_val.pre_in_shape_w*internal_param_val.pre_in_shape_h*pre_in_shape_c;
    proc[DRPAI_INDEX_DRP_CFG].address = address.drp_config_addr;
    proc[DRPAI_INDEX_DRP_CFG].size = address.drp_config_size;
    proc[DRPAI_INDEX_DRP_PARAM].address = address.drp_param_addr;
    proc[DRPAI_INDEX_DRP_PARAM].size = address.drp_param_size;
    proc[DRPAI_INDEX_AIMAC_DESC].address = address.desc_aimac_addr;
    proc[DRPAI_INDEX_AIMAC_DESC].size = address.desc_aimac_size;
    proc[DRPAI_INDEX_DRP_DESC].address = address.desc_drp_addr;
    proc[DRPAI_INDEX_DRP_DESC].size = address.desc_drp_size;
    proc[DRPAI_INDEX_WEIGHT].address = address.weight_addr;
    proc[DRPAI_INDEX_WEIGHT].size = address.weight_size;
    proc[DRPAI_INDEX_OUTPUT].address = address.data_out_addr;
    proc[DRPAI_INDEX_OUTPUT].size = address.data_out_size;

#ifdef DEBUG_LOG
    /*Start Timer */
    timespec_get(&ts_start, TIME_UTC);
#endif

    errno = 0;
    if ( PRE_SUCCESS != ioctl(drpai_fd, DRPAI_START, &proc[0]))
    {
        std::cerr << "[ERROR] Failed to run DRPAI_START : errno=" <<  errno << std::endl;
        return PRE_ERROR;
    }
    /* Wait till DRP-AI ends */
    FD_ZERO(&rfds);
    FD_SET(drpai_fd, &rfds);
    tv.tv_sec = 5;
    tv.tv_nsec = 0;

    ret_drpai = pselect(drpai_fd+1, &rfds, NULL, NULL, &tv, &sigset);

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

    if (FD_ISSET(drpai_fd, &rfds))
    {
        errno = 0;
        ret_drpai = ioctl(drpai_fd, DRPAI_GET_STATUS, &drpai_status);
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
    ret = GetResult(address.data_out_addr, address.data_out_size);
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
