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
    unsigned long l_addr = 0;
    unsigned long l_size = 0;
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

    }

    ifs.close();
    return PRE_SUCCESS;
}

uint8_t PreRuntime::WritePrerunData(const std::string dir)
{
    std::string str;
    std::string element, a, s;
    uint8_t ret;
    errno = 0;


    std::vector<uint8_t> config_data;
    std::string fname = dir+"/drp_config.mem";
    ret = ReadFileData(&config_data, fname, drpai_obj_info.drpai_address.drp_config_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/drp_config.mem" << std::endl;
        return PRE_ERROR;
    }
    /*Load weight data to memory using non-dynamic function.*/
    ret = LoadDataToMem(&config_data, drpai_obj_info.drpai_address.drp_config_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.drp_config_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/drp_config.mem" << std::endl;
        return PRE_ERROR;
    }


    std::vector<uint8_t> aimac_desc_data;
    ret = ReadFileData(&aimac_desc_data, dir+"/aimac_desc.bin", drpai_obj_info.drpai_address.desc_aimac_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/aimac_desc.bin" << std::endl;
        return PRE_ERROR;
    }
    /*Load weight data to memory using non-dynamic function.*/
    ret = LoadDataToMem(&aimac_desc_data, drpai_obj_info.drpai_address.desc_aimac_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.desc_aimac_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/aimac_desc.bin" << std::endl;
        return PRE_ERROR;
    }

    std::vector<uint8_t> drp_desc_data;
    ret = ReadFileData(&drp_desc_data, dir+"/drp_desc.bin", drpai_obj_info.drpai_address.desc_drp_size);
    if ( PRE_SUCCESS < ret )
    {
       std::cerr << "[ERROR] Failed to read file "<< dir+"/drp_desc.bin" << std::endl;
       return PRE_ERROR;
    }
    ret = LoadDataToMem(&drp_desc_data, drpai_obj_info.drpai_address.desc_drp_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.desc_drp_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/drp_desc.bin" << std::endl;
        return PRE_ERROR;
    }
       

    std::vector<uint8_t> drp_param_data;
    ret = ReadFileData(&drp_param_data, dir+"/drp_param.bin", drpai_obj_info.drpai_address.drp_param_size);
    if ( PRE_SUCCESS < ret )
    {
       std::cerr << "[ERROR] Failed to read file "<< dir+"/drp_param.bin" << std::endl;
       return PRE_ERROR;
    }
    ret = LoadDataToMem(&drp_param_data, drpai_obj_info.drpai_address.drp_param_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.drp_param_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/drp_param.bin" << std::endl;
        return PRE_ERROR;
    }

    std::vector<uint8_t> weight_data;
    ret = ReadFileData(&weight_data, dir+"/weight.bin", drpai_obj_info.drpai_address.weight_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/weight.bin" << std::endl;
        return PRE_ERROR;
    }
    ret = LoadDataToMem(&weight_data, drpai_obj_info.drpai_address.weight_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.weight_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/weight.bin" << std::endl;
        return PRE_ERROR;
    }

    std::vector<uint8_t> aimac_param_cmd_data;
    ret = ReadFileData(&aimac_param_cmd_data, dir+"/aimac_param_cmd.bin", drpai_obj_info.drpai_address.aimac_param_cmd_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/aimac_param_cmd.bin" << std::endl;
        return PRE_ERROR;
    }
    ret = LoadDataToMem(&aimac_param_cmd_data, drpai_obj_info.drpai_address.aimac_param_cmd_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.aimac_param_cmd_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/aimac_param_cmd.bin" << std::endl;
        return PRE_ERROR;
    }

    std::vector<uint8_t> aimac_param_desc_data;
    ret = ReadFileData(&aimac_param_desc_data, dir+"/aimac_param_desc.bin", drpai_obj_info.drpai_address.aimac_param_desc_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/aimac_param_desc.bin" << std::endl;
        return PRE_ERROR;
    }
    ret = LoadDataToMem(&aimac_param_desc_data, drpai_obj_info.drpai_address.aimac_param_desc_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.aimac_param_desc_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/aimac_param_desc.bin" << std::endl;
        return PRE_ERROR;
    }
 
    std::vector<uint8_t> aimac_cmd_data;
    ret = ReadFileData(&aimac_cmd_data, dir+"/aimac_cmd.bin", drpai_obj_info.drpai_address.aimac_cmd_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< dir+"/aimac_cmd.bin" << std::endl;
        return PRE_ERROR;
    }
    ret = LoadDataToMem(&aimac_cmd_data, drpai_obj_info.drpai_address.aimac_cmd_addr + drpai_obj_info.data_inout.start_address, drpai_obj_info.drpai_address.aimac_cmd_size);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write file data "<< dir+"/aimac_cmd.bin" << std::endl;
        return PRE_ERROR;
    }

    return PRE_SUCCESS;
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

    errno = 0;
    drpai_data.address = from;
    drpai_data.size    = size;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN : errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    ret = write(drpai_fd, data->data(), size);
    if ( -1 == ret )
    {
        std::cerr << "[ERROR] Failed to write with DRPAI_ASSIGN : errno=" << errno << std::endl;
        return PRE_ERROR;
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
uint8_t PreRuntime::ReadFileData(std::vector<uint8_t> *data, std::string file, unsigned long size)
{
    errno = 0;
    data->resize(size);
    data->clear();

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
        data->push_back(*it);
    }
    /* Check the param_data size is appropriate */
    if (size != data->size())
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
    std::string str         = "";
    std::string str_return  = "";
    std::string element     = "";
    std::string str_value   = "";
    s_op_param_t tmp_param;
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
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN_PARAM : errno="<< errno << __FILE__ << __LINE__ << std::endl;
        return PRE_ERROR;
    }
    
    /* Open param info file */
    std::vector<uint8_t> param_info_data;
    int ret;
    ret = ReadFileData(&param_info_data, info_file, drp_param_info_size);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read file "<< info_file << std::endl;
        return PRE_ERROR;
    }
    uint8_t param_info_array[param_info_data.size()];
    std::copy(param_info_data.begin(),param_info_data.end(),param_info_array);
    if ( 0 > write(drpai_fd, param_info_array, drp_param_info_size))
    {
        std::cerr << "[ERROR] Failed to write PARAM_INFO to DRP-AI Driver : errno="<<errno << std::endl;
        return PRE_ERROR;
    }
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
uint8_t PreRuntime::Load(const std::string pre_dir, uint32_t start_addr, uint8_t mode)
{
    return Load(pre_dir, (uint64_t)start_addr);
}

uint8_t PreRuntime::Load(const std::string pre_dir, uint64_t start_addr)
{
    uint8_t ret = 0;
    struct stat statBuf;
    std::string tmp_dir = "/";
    std::string dir = pre_dir;
    std::string tmp_address_file = dir+"/addr_map.txt";
    const std::string drpai_param_file = dir + "/drp_param_info.txt";

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
    const std::string address_file = dir+"/addr_map.txt";

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

    /* Read Address Map List file */
    ret = ReadAddrmapTxt(address_file);
    if (PRE_SUCCESS < ret)
    {
        std::cerr << "[ERROR] Failed to read Address Map List " << address_file  << std::endl;
        return PRE_ERROR;
    }

    if(start_addr == INVALID_ADDR)
    {
        this->Occupied_size = drpai_obj_info.drpai_address.desc_drp_addr + drpai_obj_info.drpai_address.desc_drp_size;
        this->Occupied_size = (Occupied_size + 0xffffff) & 0xff000000;
        start_addr = drpai_data0.address + drpai_data0.size - Occupied_size;
    }
    else
    {
        this->Occupied_size = drpai_data0.size - (start_addr - drpai_data0.address);
    }

    /* Set the DRP virtual start address to 0, indicating the actual 40-bit physical address. */
    errno = 0;
    drpai_adrconv_t drpai_adrconv;
    this->start_addr_v2h = start_addr;
    drpai_adrconv.conv_address = start_addr;
    drpai_adrconv.org_address  = drpai_obj_info.drpai_address.data_in_addr; //Currently, data_in_addr contained the actual starting address.
    drpai_adrconv.size         = this->Occupied_size;
    drpai_adrconv.mode         = DRPAI_ADRCONV_MODE_REPLACE;
    if ( PRE_SUCCESS != ioctl(drpai_obj_info.drpai_fd , DRPAI_SET_ADRCONV, &drpai_adrconv))
    {
       std::cerr << "[ERROR] Failed to run DRPAI_SET_ADRCONV : errno=" <<  errno << std::endl;
       return PRE_ERROR;
    }

    /*Define the start address.*/
    /*drpai_obj_info.drpai_address.data_in_addr maybe 0x00000000. */
    drpai_obj_info.data_inout.start_address = drpai_adrconv.conv_address - drpai_obj_info.drpai_address.data_in_addr;

    /*Parse drp_param_info.txt*/
    ret = ParseParamInfo(drpai_param_file);
    if ( PRE_SUCCESS < ret )
    {
        std::cerr << "[ERROR] Failed to read param info file: "<< drpai_param_file << std::endl;
        return PRE_ERROR;
    }
    
    /*Write binary parameters for drpai.*/
    ret = WritePrerunData(dir);
    if ( PRE_SUCCESS != ret )
    {
        std::cerr << "[ERROR] Failed to write parameters. "<< std::endl;
        return PRE_ERROR;
    }

    return PRE_SUCCESS;
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

    /*Free internal buffer if its memory is already allocated */
    if(internal_buffer != NULL )
    {
        free(internal_buffer);
    }

    internal_buffer = (uint32_t*) malloc(drpai_data.size);

    if(internal_buffer == NULL)
    {
        std::cerr << "[ERROR] Failed to malloc PreRuntime internal buffer." <<std::endl;
        return PRE_ERROR;
    }
    internal_buffer_size = (uint32_t) (drpai_data.size);

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
* Function Name : SetInput
* Description   : 
*                 
* Arguments     : indata = pointer to input data
*              
* Return value  : true if succeeded
*                 false error;
******************************************/
int PreRuntime::SetInput(void *indata)
{
    uint8_t ret = 0;
    /* Write input data to data_in_addr.*/
    drpai_data_t input;
    input.address = drpai_obj_info.drpai_address.data_in_addr + drpai_obj_info.data_inout.start_address;
    input.size    = drpai_obj_info.drpai_address.data_in_size;
    ret = ioctl(drpai_obj_info.drpai_fd , DRPAI_ASSIGN, &input);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to run DRPAI_ASSIGN in SetInput(): errno=" << errno << std::endl;
        return PRE_ERROR;
    }
    ret = write(drpai_obj_info.drpai_fd , indata, input.size);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to write via DRP-AI Driver in SetInput(): errno=" << errno << std::endl;
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
    if((param->pre_in_addr & 0x800000000000) != 0)
    {
        /* Virtual address in linux user space */
        SetInput((void *)param->pre_in_addr);

        #ifndef WITH_V2H_DEV
        return Pre(out_ptr, out_size, (uint64_t)(drpai_obj_info.drpai_address.data_in_addr + drpai_obj_info.data_inout.start_address));
        #else
        // If you don't have rzv2h-dev, don't care.
        return Pre(out_ptr, out_size, (uint64_t)(drpai_obj_info.drpai_address.data_in_addr));
        #endif
    }
    else
    {

        if(this->mapped_in_addr_v2h != param->pre_in_addr){
	    int ret = 0;

            drpai_adrconv_t drpai_adrconv;
            drpai_adrconv.conv_address = this->start_addr_v2h;
            drpai_adrconv.org_address  = drpai_obj_info.drpai_address.data_in_addr; //Currently, data_in_addr contained the actual starting address.
            drpai_adrconv.size         = this->Occupied_size;
            drpai_adrconv.mode         = DRPAI_ADRCONV_MODE_REPLACE;
            ret = ioctl(drpai_obj_info.drpai_fd, DRPAI_SET_ADRCONV, &drpai_adrconv);
            if (0 != ret)
            {
                std::cerr << "[ERROR] Failed to run SET_ADRCONV(1) for DRP-AI input image : errno=" <<  errno << std::endl;
                return PRE_ERROR;
            }

            this->mapped_in_addr_v2h = param->pre_in_addr;
            drpai_adrconv.conv_address  = param->pre_in_addr & 0xffffff000000;
            drpai_adrconv.org_address   = 0xD0000000;
            drpai_adrconv.size          = 0x20000000; /*(drpai_obj_info.drpai_address.data_in_size + 0xffffff) & 0xff000000;*/
            drpai_adrconv.mode          = DRPAI_ADRCONV_MODE_ADD;
            ret = ioctl(drpai_obj_info.drpai_fd, DRPAI_SET_ADRCONV, &drpai_adrconv);
    
            if (0 != ret)
            {
                std::cerr << "[ERROR] Failed to run SET_ADRCONV(2) for DRP-AI input image : errno=" <<  errno << std::endl;
                return PRE_ERROR;
            }
        }

        #ifndef WITH_V2H_DEV
        return Pre(out_ptr, out_size, (uint64_t)param->pre_in_addr);
        #else
        // If you don't have rzv2h-dev, don't care.
        return Pre(out_ptr, out_size, (uint64_t)0xD0000000);
        #endif
    }
}

uint8_t PreRuntime::Pre(void** out_ptr, uint32_t* out_size, uint64_t phyaddr)
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


    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);

    proc[DRPAI_INDEX_INPUT].address            = phyaddr;
    proc[DRPAI_INDEX_INPUT].size               = drpai_obj_info.drpai_address.data_in_size;
    proc[DRPAI_INDEX_DRP_CFG].address          = drpai_obj_info.drpai_address.drp_config_addr + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_CFG].size             = drpai_obj_info.drpai_address.drp_config_size;
    proc[DRPAI_INDEX_DRP_PARAM].address        = drpai_obj_info.drpai_address.drp_param_addr  + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_PARAM].size           = drpai_obj_info.drpai_address.drp_param_size;
    proc[DRPAI_INDEX_AIMAC_DESC].address       = drpai_obj_info.drpai_address.desc_aimac_addr + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_AIMAC_DESC].size          = drpai_obj_info.drpai_address.desc_aimac_size;
    proc[DRPAI_INDEX_DRP_DESC].address         = drpai_obj_info.drpai_address.desc_drp_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_DRP_DESC].size            = drpai_obj_info.drpai_address.desc_drp_size;
    proc[DRPAI_INDEX_WEIGHT].address           = drpai_obj_info.drpai_address.weight_addr     + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_WEIGHT].size              = drpai_obj_info.drpai_address.weight_size;
    proc[DRPAI_INDEX_OUTPUT].address           = drpai_obj_info.drpai_address.data_out_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_OUTPUT].size              = drpai_obj_info.drpai_address.data_out_size;
    proc[DRPAI_INDEX_AIMAC_CMD].address        = drpai_obj_info.drpai_address.aimac_param_cmd_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_AIMAC_CMD].size           = drpai_obj_info.drpai_address.aimac_param_cmd_size;
    proc[DRPAI_INDEX_AIMAC_PARAM_DESC].address = drpai_obj_info.drpai_address.aimac_param_desc_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_AIMAC_PARAM_DESC].size    = drpai_obj_info.drpai_address.aimac_param_desc_size;
    proc[DRPAI_INDEX_AIMAC_PARAM_CMD].address  = drpai_obj_info.drpai_address.aimac_cmd_addr   + drpai_obj_info.data_inout.start_address;
    proc[DRPAI_INDEX_AIMAC_PARAM_CMD].size     = drpai_obj_info.drpai_address.aimac_cmd_size;

#ifdef DEBUG_LOG
    float diff = 0;
    timespec_get(&ts_start, TIME_UTC);
#endif

    /* Wait till DRP-AI ends */
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
    ret = GetResult(proc[DRPAI_INDEX_OUTPUT].address, proc[DRPAI_INDEX_OUTPUT].size);
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
