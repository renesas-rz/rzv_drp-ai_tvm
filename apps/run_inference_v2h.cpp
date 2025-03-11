/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2025
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
* File Name    : run_inference_v2h.cpp
* Version      : 1.0.0
* Description  : DRP-AI TVM[*1] Application Example
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <linux/drpai.h>
#include <builtin_fp16.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <climits>
#include <getopt.h>

#include "MeraDrpRuntimeWrapper.h"
#include "PreRuntime.h"

/*****************************************
* Function Name     : float16_to_float32
* Description       : Function by Edgecortex. Cast uint16_t a into float value.
* Arguments         : a = uint16_t number
* Return value      : float = float32 number
******************************************/
float float16_to_float32(uint16_t a)
{
    return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

/*****************************************
* Function Name     : LoadBinary
* Description       : Function by Edgecortex. Load bin file into std::vector.
* Arguments         : bin_file = *.bin filename to be read
* Return value      : std::vector<T> = file content
******************************************/
template <typename T>
static std::vector<T> LoadBinary(const std::string &bin_file)
{
    std::ifstream file(bin_file.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LOG(FATAL) << "unable to open file " + bin_file;
    }

    file.seekg(0, file.end);
    const uint32_t file_size = static_cast<uint32_t>(file.tellg());
    file.seekg(0, file.beg);

    const auto file_buffer = std::unique_ptr<char>(new char[file_size]);
    file.read(file_buffer.get(), file_size);

    if (file.bad() || file.fail())
    {
        LOG(FATAL) << "error occured while reading the file";
    }

    file.close();

    auto ptr = reinterpret_cast<T *>(file_buffer.get());
    const auto num_elements = file_size / sizeof(T);
    return std::vector<T>(ptr, ptr + num_elements);
}

/*****************************************
* Function Name     : SaveBinary (FP32 mode)
* Description       : Function by Edgecortex. Save inference result as bin file
* Arguments         : bin_file = *.bin filename to be written
*                   : T = inference result (=output)
*                   : size = output data length
* Return value      : -
******************************************/
template <typename T>
void SaveBinary(const std::string &bin_file, 
                const T* const tvm_result, 
                size_t size)
{
    std::ofstream ofs(bin_file, std::ios::out | std::ios::binary);
    ofs.write((char *)tvm_result, size*4);
    std::cout << "Finish file out" << bin_file << std::endl;
}
/*****************************************
* Function Name     : SaveBinary (FP16 mode)
* Description       : Function by Edgecortex. Save inference result as bin file
* Arguments         : bin_file = *.bin filename to be written
*                   : T = inference result (=output)
*                   : size = output data length
* Return value      : -
******************************************/
void SaveBinary(const std::string &bin_file, 
                const uint16_t* const tvm_result, 
                size_t size)
{
    std::ofstream ofs(bin_file, std::ios::out | std::ios::binary);
    ofs.write((char *)tvm_result, size*2);
    std::cout << "Finish file out" << bin_file << std::endl;
}

/*****************************************
* Function Name : get_drpai_start_addr
* Description   : Function to get the start address of DRPAImem.
* Arguments     : -
* Return value  : uint32_t = DRPAImem start address in 32-bit.
******************************************/
uint64_t get_drpai_start_addr()
{
    int fd  = 0;
    int ret = 0;
    drpai_data_t drpai_data;

    errno = 0;

    fd = open("/dev/drpai0", O_RDWR);
    if (0 > fd)
    {
        LOG(FATAL) << "[ERROR] Failed to open DRP-AI Driver : errno=" << errno;
        return (uint64_t)NULL;
    }

    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(fd, DRPAI_GET_DRPAI_AREA, &drpai_data);
    if (-1 == ret)
    {
        LOG(FATAL) << "[ERROR] Failed to get DRP-AI Memory Area : errno=" << errno ;
        return (uint64_t)NULL;
    }
    close(fd);

    return drpai_data.address;
}

/*****************************************
* Function Name : timedifference_msec
* Description   : Function to compute the processing time in mili-seconds
* Arguments     : t0 = processing start time
*                 t1 = processing end time
* Return value  : processing time in mili-seconds
******************************************/
static double timedifference_msec(struct timespec t0, struct timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1000.0 / 1000.0;
}

int main(int argc, char **argv)
{
    // Get argument info
    auto usage_log = [] {
    std::cout << "Usage: run_inference_v2h <model dir>" << std::endl;
    std::cout << "   model_dir means specific deployment folder" << std::endl;
    };
    
    /* Set path to model directory*/
    std::string model_dir = std::string(argv[1]);

    /* Error check */
    if (model_dir.empty()) {
        usage_log();
        return 0;
    }

    /* Time Measurement */
    struct timespec start_time, end_time;
    double diff = 0;
    
    /* DRP-AI TVM[*1] Runtime object */
    MeraDrpRuntimeWrapper runtime;
    uint64_t drpaimem_addr_start = 0;

    /* Currently, the start address can only use the head of the area managed by the DRP-AI. */
    drpaimem_addr_start = get_drpai_start_addr();
    if (drpaimem_addr_start == (uint64_t)NULL) return 0;

    /*Load model_dir structure and its weight to runtime object */
    runtime.LoadModel(model_dir, drpaimem_addr_start);

    /* Set input data*/
    // Please save binary file as <model_dir>/input_0.bin file.
    auto input_num = runtime.GetNumInput(model_dir);
    if(input_num==0){
        std::cout << "[ERROR]There is no input file" << std::endl;
        std::cout << "Please save input binary file under the model directory." << std::endl;
        std::cout << "<model_dir>" << std::endl;
        std::cout << "  |- input_0.bin <-- Add this file" << std::endl;
        std::cout << "  |- deploy.json" << std::endl;
        std::cout << "  |- deploy.params" << std::endl;
        std::cout << "  |- deploy.so" << std::endl;
        return 0;
    }
    for (int i = 0; i < input_num; i++) {
        std::string filename = model_dir + "/input_" + std::to_string(i) + ".bin";
        std::cout << "Input bin file : " << filename << std::endl;
        auto input_data_type = runtime.GetInputDataType(i);
        if (InOutDataType::FLOAT32 == input_data_type) {
            auto input = LoadBinary<float>(filename);
            runtime.SetInput(i, input.data());
        } else {
            auto input = LoadBinary<uint16_t>(filename);
            runtime.SetInput(i, input.data());
        }
    }
    /* Warm up for runtime measurement*/
    std::cout << "Warming up..." << std::endl;
    for (int i = 0; i < 3; i++) {
        runtime.Run();
    }

    /* Run inference and get the time*/
    std::cout << "Running tvm runtime" << std::endl;
    timespec_get(&start_time, TIME_UTC);
    runtime.Run();
    timespec_get(&end_time, TIME_UTC);

    /* Print Inference processing time */
    diff = timedifference_msec(start_time, end_time);
    std::cout << "[TIME] AI Processing Time: " << std::fixed << std::setprecision(2) << diff << " msec." << std::endl;

    /* Save inference result as binary file(s) */
    // Inference result is saved to <model_dir>/data_out_0_fp32.bin or data_out_0_fp16.bin
    auto output_num = runtime.GetNumOutput();
    for (int i = 0; i < output_num; i++) {
        auto output_buffer = runtime.GetOutput(i);
        if (InOutDataType::FLOAT32 == std::get<0>(output_buffer)) {
            float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
            int64_t out_size = std::get<2>(output_buffer);
            std::string out_bin_file(model_dir + "data_out_" + std::to_string(i) + "_fp32.bin");
            SaveBinary(out_bin_file, data_ptr, out_size);
        
        } else {
            uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
            int64_t out_size = std::get<2>(output_buffer);
            std::string out_bin_file(model_dir + "data_out_" + std::to_string(i) + "_fp16.bin");
            SaveBinary(out_bin_file, data_ptr, out_size);
        }
    }
    return 0;
}
