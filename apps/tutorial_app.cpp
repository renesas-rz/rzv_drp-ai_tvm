/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2022
 *　
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA Compiler
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
* File Name    : tutorial_app.cpp
* Version      : 0.10
* Description  : DRP-AI TVM[*1] Application Example
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>

#include "MeraDrpRuntimeWrapper.h"
#include "opencv2/opencv.hpp"

/*****************************************
* Macro
******************************************/
#define MODEL_IN_H  (224)
#define MODEL_IN_W  (224)
#define MODEL_IN_C  (3)

/* Edgecortex Functions */
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
* Function Name     : load_label_file
* Description       : Load label list text file and return the label list that contains the label.
* Arguments         : label_file_name = filename of label list. must be in txt format
* Return value      : std::map<int, std::string> list = list text file which contains labels
*                     empty if error occured
******************************************/
std::map<int, std::string> load_label_file(std::string label_file_name)
{
    int n = 0;
    std::map<int, std::string> list;
    std::ifstream infile(label_file_name);

    if (!infile.is_open())
    {
        return list;
    }

    std::string line;
    while (getline(infile,line))
    {
        list[n++] = line;
        if (infile.fail())
        {
            std::map<int, std::string> empty;
            return empty;
        }
    }

    return list;
}

/*****************************************
* Function Name : softmax
* Description   : Function for Post Processing
* Arguments     : val[] = array to be computed Softmax
*                 size = size of array
* Return value  : -
******************************************/
void softmax(float* val, int32_t size)
{
    float max_num = -INT_MAX;
    float sum = 0;
    int32_t i;
    for ( i = 0 ; i<size ; i++ )
    {
        max_num = std::max(max_num, val[i]);
    }

    for ( i = 0 ; i<size ; i++ )
    {
        val[i]= (float) exp(val[i] - max_num);
        sum+= val[i];
    }

    for ( i = 0 ; i<size ; i++ )
    {
        val[i]= val[i]/sum;
    }
    return;
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
    /* ResNet Information */
    float mean[] = { 0.485, 0.456, 0.406 };
    float std[] = { 0.229, 0.224, 0.225 };
    /* Variables required for Transpose&Normalize in pre-processing */
    std::vector<float> chw;
    float val = 0;
    /* Label list file for ImageNet*/
    std::string labels = "synset_words_imagenet.txt";
    /* Map to store label list */
    std::map<int, std::string> label_file_map;
    /* Map list to store the classification result. */
    std::map<float,int> result;
    int result_cnt = 0;
    /* Time Measurement */
    struct timespec start_time, end_time;
    double diff = 0;
    /* Input image file */
    std::string filename = "sample.bmp";
    /* DRP-AI TVM[*1] Runtime object */
    MeraDrpRuntimeWrapper runtime;

    /* Model Binary */
    std::string model_dir = "resnet18_onnx";

    /* Load Label list */
    label_file_map = load_label_file(labels);
    if (label_file_map.empty())
    {
        std::cerr << "[ERROR] Label file : failed to load "<< labels << std::endl;
        return 0;
    }

    /*Load model_dir structure and its weight to runtime object */
    runtime.LoadModel(model_dir);

    /*Get input data */
    auto input_data_type = runtime.GetInputDataType(0);

    /*Load input data */
    /*Input data type can be either FLOAT32 or FLOAT16, which depends on the model */
    if (InOutDataType::FLOAT32 == input_data_type)
    {
        /* Pre-processing */
        /* Read image data from file */
        cv::Mat image = cv::imread(filename);
        if (image.empty())
        {
            std::cerr << "[ERROR] Input image : failed to load "<< filename << std::endl;
            return 0;
        }
        /* Convert image data from BGR to RGB (cv reads image as BGR) */
        cv::cvtColor(image, image, CV_BGR2RGB);
        /* Resize the image to appropriate size for model input */
        cv::resize(image, image, cv::Size(MODEL_IN_W, MODEL_IN_H), 0, 0, CV_INTER_LINEAR);
        /* Convert the pixel values to [0, 1]*/
        image.convertTo(image, CV_32FC3, 1.0/255.0);
        /* Normalize with mean and std and Transpose from HWC to CHW */
        for (int c = 0; c < MODEL_IN_C ; c++)
        {
            for (int y = 0; y < MODEL_IN_H ; y++)
            {
                for (int x = 0; x < MODEL_IN_W; x++)
                {
                    val = (image.at<cv::Vec3f>(y, x)[c]  - mean[c])/std[c];
                    chw.push_back(val);
                }
            }
        }
        /*Load image data to runtime object */
        runtime.SetInput(0, chw.data());
    }
    else if (InOutDataType::FLOAT16 == input_data_type)
    {
        std::cerr << "[ERROR] Input data type : FP16." << std::endl;
        /*If your model input data type is FP16, use std::vector<uint16_t> for reading input data. */
        return 0;
    }
    else
    {
        std::cerr << "[ERROR] Input data type : neither FP32 nor FP16." << std::endl;
        return 0;
    }

    timespec_get(&start_time, TIME_UTC);
    std::cout << "Running tvm runtime" << std::endl;
    runtime.Run();
    timespec_get(&end_time, TIME_UTC);

    /* Print Inference processing time */
    diff = timedifference_msec(start_time, end_time);
    std::cout<<"[TIME] AI Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

    /* Get the number of output of the target model. For ResNet, 1 output. */
    auto output_num = runtime.GetNumOutput();
    if(output_num != 1)
    {
        std::cerr << "[ERROR] Output size : not 1." << std::endl;
        return 0;
    }

    /* Comparing output with reference.*/
    /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
    auto output_buffer = runtime.GetOutput(0);
    if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
    {
        /* Extract data in FP16 <uint16_t>. */
        uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
        int64_t out_size = std::get<2>(output_buffer);

        /* Array to store the FP32 output data from inference. */
        float floatarr[out_size];

        /* Post-processing */
        /* Clear the classification result. */
        result.clear();
        /* Cast FP16 output data to FP32. */
        for (int n = 0; n < out_size; n++)
        {
            floatarr[n] = float16_to_float32(data_ptr[n]);
        }
        /* Softmax 1000 class scores. */
        softmax(&floatarr[0], out_size);
        /* Sort in decending order. */
        for (int n = 0; n < out_size; n++)
        {
            result[floatarr[n]] = n;
        }

        result_cnt = 0;
        /* Print Top-5 results. */
        std::cout << "Result ----------------------- "<< std::endl;
        for (auto it = result.rbegin(); it != result.rend(); it++)
        {
            result_cnt++;
            if (result_cnt > 5) break;
            std::cout << "  Top "<< result_cnt << " ["
                << std::right << std::setw(5) << std::fixed << std::setprecision(1) << (float)(*it).first*100
                <<"%] : [" << label_file_map[(*it).second] << "]" <<std::endl;
        }
    }
    else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
    {
        std::cerr << "[ERROR] Output data type : FP32." << std::endl;
        /* If the output of inference is FP32, use <float*> to extract the data. */
    }
    else
    {
        std::cerr << "[ERROR] Output data type : not floating point type." << std::endl;
    }
    return 0;
}
