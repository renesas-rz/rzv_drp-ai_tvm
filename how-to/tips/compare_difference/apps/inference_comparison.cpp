/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2022　
 * *1 DRP-AI TVM is powered by EdgeCortix MERA Compiler Framework
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
* File Name    : inference_comparison.cpp
* Version      : 1.1.1
* Description  : DRP-AI TVM[*1] Application Example for inference comparison
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>
#include "MeraDrpRuntimeWrapper.h"

std::ostream& operator<<(std::ostream& os, InOutDataType type)
{
    switch (type)
    {
        case InOutDataType::FLOAT32:
            os << "FLOAT32";
            break;
        case InOutDataType::FLOAT16:
            os << "FLOAT16";
            break;
        case InOutDataType::OTHER:
            os << "OTHER";
            break;
        default:
            ;
    }
    return os;
}

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
static std::vector<T> LoadBinary(const std::string& bin_file)
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

    auto ptr = reinterpret_cast<T*>(file_buffer.get());
    const auto num_elements = file_size / sizeof(T);
    return std::vector<T>(ptr, ptr + num_elements);
}

/*****************************************
* Function Name     : CheckOutput
* Description       : Compare the inference output with reference file for FP32 output.
* Arguments         : label_file_name = filename of label list. must be in txt format
* Return value      : std::map<int, std::string> list = list text file which contains labels
*                     empty if error occured
******************************************/
template <typename T>
void CheckOutput(const T* const ref_result, const T* const tvm_result, size_t size,
                 float atol = 1e-2, float rtol = 1e-3)
{
    int num_correct = 0;
    T max_abs_diff = std::numeric_limits<T>::min();
    T mean_abs_diff = 0;
    int64_t non_zero_count = 0;
    for (size_t i = 0; i < size; ++i)
    {
        auto abs_diff = std::abs(tvm_result[i] - ref_result[i]);
        max_abs_diff = std::max(abs_diff, max_abs_diff);
        mean_abs_diff += abs_diff;
        non_zero_count += static_cast<int64_t>(0.0f != tvm_result[i]);
        num_correct += static_cast<int>(static_cast<float>(abs_diff) <= atol + std::abs(rtol * ref_result[i]));
    }

    LOG(INFO) << "max abs diff: " << max_abs_diff;
    LOG(INFO) << "mean abs diff: " << mean_abs_diff / size;
    LOG(INFO) << "correct ratio: " << num_correct / (double) size << ", with absolute tolerance: " << atol << ", relative tolerance: " << rtol;
    LOG(INFO) << "Non-zero values: " << non_zero_count / (double) size * 100.0 << "(%)" << std::endl;
}

/*****************************************
* Function Name     : CheckOutput
* Description       : Compare the inference output with reference file for FP16 output.
* Arguments         : label_file_name = filename of label list. must be in txt format
* Return value      : std::map<int, std::string> list = list text file which contains labels
*                     empty if error occured
******************************************/
void CheckOutput(const uint16_t* const ref_result, const uint16_t* const tvm_result, size_t size,
                 float atol = 1e-2, float rtol = 1e-3)
{
    int num_correct = 0;
    float max_abs_diff = std::numeric_limits<float>::min();
    float mean_abs_diff = 0;
    int64_t non_zero_count = 0;
    for (size_t i = 0; i < size; ++i)
    {
        float tvm_result_f32 = float16_to_float32(tvm_result[i]);
        float ref_result_f32 = float16_to_float32(ref_result[i]);

        auto abs_diff = std::abs(tvm_result_f32 - ref_result_f32);
        max_abs_diff = std::max(abs_diff, max_abs_diff);
        mean_abs_diff += abs_diff;
        non_zero_count += static_cast<int64_t>(0.0f != tvm_result_f32);
        num_correct += static_cast<int>(abs_diff <= atol + std::abs(rtol * ref_result_f32));
    }
    LOG(INFO) << "max abs diff: " << max_abs_diff;
    LOG(INFO) << "mean abs diff: " << mean_abs_diff / size;
    LOG(INFO) << "correct ratio: " << num_correct / (double) size << ", with absolute tolerance: " << atol << ", relative tolerance: " << rtol;
    LOG(INFO) << "Non-zero values: " << non_zero_count / (double) size * 100.0 << "(%)" << std::endl;
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

int main(int argc, char** argv)
{
    /* Time Measurement */
    struct timespec start_time, end_time;
    float diff = 0;
    /* Comparison Threshold */
    float atol = 0.01, rtol = 0.001;
    /* Model Binary */
    std::string model_dir = "resnet18_onnx";

    if (argc > 1)
    {
        model_dir = argv[1];
    }

    MeraDrpRuntimeWrapper runtime;
    runtime.LoadModel(model_dir);

    auto input_num = runtime.GetNumInput(model_dir);
    const std::string input_name("input");
    for (int i = 0; i < input_num; i++)
    {
        std::string filename = model_dir + "/" + input_name + "_" + std::to_string(i) + ".bin";
        auto input_data_type = runtime.GetInputDataType(i);
        if (InOutDataType::FLOAT32 == input_data_type)
        {
            auto input = LoadBinary<float>(filename);
            runtime.SetInput(i, input.data());
        }
        else
        {
            CHECK_EQ(InOutDataType::FLOAT16, input_data_type);
            auto input = LoadBinary<uint16_t>(filename);
            runtime.SetInput(i, input.data());
        }
    }

    timespec_get(&start_time, TIME_UTC);
    runtime.Run();
    timespec_get(&end_time, TIME_UTC);

    /* Print Inference processing time */
    diff = timedifference_msec(start_time, end_time);
    std::cout<<"[TIME] AI Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    auto output_num = runtime.GetNumOutput();
    /* Comparing output with reference. */
    for (int i = 0; i < output_num; i++)
    {
        auto output_buffer = runtime.GetOutput(i);
        if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
        {
            float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
            int64_t out_size = std::get<2>(output_buffer);

            std::string filename(model_dir + "/" + "ref_result_" + std::to_string(i) + ".bin");
            auto ref_result = LoadBinary<float>(filename);

            CHECK_EQ(out_size, ref_result.size());
            CheckOutput(ref_result.data(), data_ptr, out_size, atol, rtol);
        }
        else
        {
            CHECK_EQ(InOutDataType::FLOAT16, std::get<0>(output_buffer));
            uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
            int64_t out_size = std::get<2>(output_buffer);

            std::string filename(model_dir + "/" + "ref_result_" + std::to_string(i) + ".bin");
            auto ref_result = LoadBinary<uint16_t>(filename);

            CHECK_EQ(out_size, ref_result.size());
            CheckOutput(ref_result.data(), data_ptr, out_size, atol, rtol);
        }
    }

    return 0;
}
