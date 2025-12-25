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
* File Name    : tutorial_app.cpp
* Version      : 2.7.0
* Description  : DRP-AI TVM[*1] Application Example
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <cstddef>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <fstream>

#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <getopt.h>
#include <builtin_fp16.h>

#include <linux/drpai.h>
#include <sys/time.h>
#include <climits>
#include "MeraDrpRuntimeWrapper.h"
#include "PreRuntime.h"
/*****************************************
* Macro
******************************************/
/*Input image info*/
#define INPUT_IMAGE_H       (480)
#define INPUT_IMAGE_W       (640)
#define INPUT_IMAGE_C       (3)
/*Model input info*/
#define MODEL_IN_H          (224)
#define MODEL_IN_W          (224)
#define MODEL_IN_C          (3)

/*BMP Header size for Windows Bitmap v3*/
#define FILEHEADERSIZE          (14)
#define INFOHEADERSIZE_W_V3     (40)

/* Edgecortex Functions */
std::ostream &operator<<(std::ostream &os, InOutDataType type)
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
    while (getline(infile, line))
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
void softmax(float *val, int32_t size)
{
    float max_num = -INT_MAX;
    float sum = 0;
    int32_t i;
    for (i = 0; i < size; i++)
    {
        max_num = std::max(max_num, val[i]);
    }

    for (i = 0; i < size; i++)
    {
        val[i] = (float)exp(val[i] - max_num);
        sum += val[i];
    }

    for (i = 0; i < size; i++)
    {
        val[i] = val[i] / sum;
    }
    return;
}

/*****************************************
* Function Name : read_bmp
* Description   : Function to load BMP file into img_buffer
* NOTE          : This is just the simplest example to read Windows Bitmap v3 file.
*                 This function does not have header check.
* Arguments     : filename = name of BMP file to be read
*                 width  = BMP image width
*                 height = BMP image height
*                 channel = BMP image color channel
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t read_bmp(std::string filename, uint32_t width, uint32_t height, uint32_t channel, uint8_t *buffer)
{
    FILE *fp = NULL;
    size_t ret = 0;
    uint32_t header_size = FILEHEADERSIZE + INFOHEADERSIZE_W_V3;
    /*  Read header for Windows Bitmap v3 file. */
    uint8_t bmp_header[header_size];

    /* Number of byte in single row */
    /* NOTE: Number of byte in single row of Windows Bitmap image must be aligned to 4 bytes. */
    int32_t line_width = width * channel + width % 4;

    fp = fopen(filename.c_str(), "rb");
    if (NULL == fp)
    {
        return -1;
    }
    /* Read all header */
    errno = 0;
    ret = fread(bmp_header, sizeof(uint8_t), header_size, fp);
    if (!ret)
    {
        std::cerr << "[ERROR] Failed to run fread(): errno=" << (uint8_t)errno << std::endl;
        fclose(fp);
        return -1;
    }
    /* Single row image data */
    uint8_t bmp_line_data[sizeof(uint8_t) * line_width];

    for (int i = height - 1; i >= 0; i--)
    {
        errno = 0;
        ret = fread(bmp_line_data, sizeof(uint8_t), line_width, fp);
        if (!ret)
        {
            std::cerr << "[ERROR] Failed to run fread(): errno=" << errno << std::endl;
            fclose(fp);
            return -1;
        }
        memcpy(buffer + i * width * channel, bmp_line_data, sizeof(uint8_t) * width * channel);
    }

    fclose(fp);
    return 0;
}

/*****************************************
* Function Name : get_drpai_start_addr
* Description   : Function to get the start address of DRPAImem.
* Arguments     : -
* Return value  : uint64_t = DRPAImem start address in 64-bit.
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

static void print_usage() {
  std::cout
    << "Usage: tutorial_app [-z] [-f <freq_index>]\n"
    << "  -z, --zero-copy     Use zero-copy I/O path (Get*Ptr APIs)\n"
    << "  -f, --freq-index N  DRP-AI frequency index (default 1)\n"
    << "  -h, --help          Show this help\n";
}

int main(int argc, char **argv)
{

    bool zero_copy = false;
    int  freq_index = 1;
    while (1) {
        static struct option long_opts[] = {
            {"zero-copy",  no_argument,       0, 'z'},
            {"freq-index", required_argument, 0, 'f'},
            {"help",       no_argument,       0, 'h'},
            {0,0,0,0}
        };
        int optidx = 0;
        int c = getopt_long(argc, argv, "zf:h", long_opts, &optidx);
        if (c == -1) break;
        switch (c) {
            case 'z': zero_copy = true; break;
            case 'f': {
                int v = std::atoi(optarg);
                freq_index = (v > 0 ? v : 1);
                break;
            }
            case 'h':
            default:  print_usage(); return 0;
        }
    }

    if (optind < argc) {
        int maybe_freq = std::atoi(argv[optind]);
        if (maybe_freq > 0) freq_index = maybe_freq;
    }

    /* Label list file for ImageNet*/
    std::string labels = "synset_words_imagenet.txt";
    /* Map to store label list */
    std::map<int, std::string> label_file_map;
    /* Map list to store the classification result. */
    std::map<float, int> result;
    int result_cnt = 0;
    /* Time Measurement */
    struct timespec start_time, end_time;
    double diff = 0;
    /* DRP-AI TVM[*1] Runtime object */
    MeraDrpRuntimeWrapper runtime;
    /* Pre-processing Runtime object */
    PreRuntime preruntime;

    /* Model Binary */
    std::string model_dir = "resnet18_onnx";
    /* Pre-processing Runtime Object */
    std::string pre_dir = model_dir + "/preprocess";

    /* Input image file */
    std::string filename = "sample.bmp";

    uint64_t drpaimem_addr_start = 0;
    /* Load Label list */
    label_file_map = load_label_file(labels);
    if (label_file_map.empty())
    {
        std::cerr << "[ERROR] Label file : failed to load " << labels << std::endl;
        return -1;
    }

    /*Load pre_dir object to DRP-AI */
    uint8_t ret_preld = preruntime.Load(pre_dir);
    if (ret_preld != 0)
    {
        std::cerr << "[ERROR] Failed to run Pre-processing Runtime Load()." << std::endl;
        return -1;
    }

    /*Load model_dir structure and its weight to runtime object */
    drpaimem_addr_start = get_drpai_start_addr();
    if (drpaimem_addr_start == (uint64_t)NULL) return 0;
    /* Currently, the start address can only use the head of the area managed by the DRP-AI. */
    runtime.LoadModel(model_dir, drpaimem_addr_start);

    /*Get input data */
    auto input_data_type = runtime.GetInputDataType(0);

    /*Obtain u-dma-buf memory area starting address*/
    int imgbuf_size = INPUT_IMAGE_H * INPUT_IMAGE_W * INPUT_IMAGE_C;
    uint8_t img_buffer[imgbuf_size];


    /*Load input data */
    /*Input data type can be either FLOAT32 or FLOAT16, which depends on the model */
    if (InOutDataType::FLOAT32 == input_data_type)
    {
        /* Pre-processing */
        /* Read image data from file */
        uint8_t ret_bmp = read_bmp(filename, INPUT_IMAGE_W, INPUT_IMAGE_H, INPUT_IMAGE_C, img_buffer);
        if (ret_bmp != 0)
        {
            std::cerr << "[ERROR] Failed to read image :" << filename << std::endl;
            return -1;
        }
        s_preproc_param_t in_param;
        in_param.pre_in_addr    = (uint64_t)img_buffer;

        in_param.pre_in_shape_w = INPUT_IMAGE_W;
        in_param.pre_in_shape_h = INPUT_IMAGE_H;
        in_param.pre_in_format  = FORMAT_BGR;
        in_param.pre_out_format = FORMAT_RGB;

        /*Crop parameters can be changed. Currently not used.*/
        /* 
        in_param.crop_tl_x = 185;
        in_param.crop_tl_y = 0;
        in_param.crop_w = 480;
        in_param.crop_h = 480;
        */

        /*Resize parameters can be changed. Currently not used.*/
        /*
        in_param.resize_w       = MODEL_IN_W;
        in_param.resize_h       = MODEL_IN_H;
        in_param.resize_alg     = ALG_BILINEAR;
        */

        /*Normalize parameters can be changed. Currently not used.*/
        /* Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
        /*
        float mean[] = { 0.485, 0.456, 0.406 };
        float std[] = { 0.229, 0.224, 0.225 };
        in_param.cof_add[0] = -255*mean[0];
        in_param.cof_add[1] = -255*mean[1];
        in_param.cof_add[2] = -255*mean[2];
        in_param.cof_mul[0] = 1/(std[0]*255);
        in_param.cof_mul[1] = 1/(std[1]*255);
        in_param.cof_mul[2] = 1/(std[2]*255);
        */

        /*Output variables for Pre-processing Runtime */
        void *output_ptr;
        uint32_t out_size;

        timespec_get(&start_time, TIME_UTC);

        /*Run pre-processing*/
	    uint8_t ret_prepre = preruntime.Pre(&in_param, &output_ptr, &out_size);
        if (ret_prepre != 0)
        {
            std::cerr << "[ERROR] Failed to run Pre-processing Runtime Pre()." << std::endl;
            return -1;
        }

        timespec_get(&end_time, TIME_UTC);
        /* Print Inference processing time */
        diff = timedifference_msec(start_time, end_time);
        std::cout << "[TIME] Pre Processing Time: " << std::fixed << std::setprecision(2) << diff << " msec." << std::endl;

        /*Set Pre-processing output to be inference input. */
        if (zero_copy) {
            auto input_info = runtime.GetInputInfo(); // {name, size(bytes), type}
            if (input_info.empty()) {
                std::cerr << "[ERROR] GetInputInfo returned empty." << std::endl;
                return -1;
            }
            const auto& info0 = input_info[0];
            const std::string& in_name = std::get<0>(info0);
            size_t in_bytes = std::get<1>(info0);
            InOutDataType in_type = std::get<2>(info0);
            if (in_type != InOutDataType::FLOAT32) {
                std::cerr << "[ERROR] Zero-copy expects FLOAT32 input, but got type=" << (int)in_type << std::endl;
                return -1;
            }
            void* in_ptr = runtime.GetInputPtr(in_name);
            if (!in_ptr) {
                std::cerr << "[ERROR] GetInputPtr returned nullptr." << std::endl;
                return -1;
            }
            size_t copy_bytes = (size_t)out_size * sizeof(float);
            if (copy_bytes > in_bytes) copy_bytes = in_bytes;
            std::memcpy(in_ptr, output_ptr, copy_bytes);
        } else {
            
            runtime.SetInput(0, (float *)output_ptr);
        }
    }
    else if (InOutDataType::FLOAT16 == input_data_type)
    {
        std::cerr << "[ERROR] Input data type : FP16." << std::endl;
        /*If your model input data type is FP16, use std::vector<uint16_t> for reading input data. */
        return -1;
    }
    else
    {
        std::cerr << "[ERROR] Input data type : neither FP32 nor FP16." << std::endl;
        return -1;
    }

    timespec_get(&start_time, TIME_UTC);
    std::cout << "Running tvm runtime (freq_index=" << freq_index
              << ", zero_copy=" << (zero_copy ? "on" : "off") << ")" << std::endl;
    runtime.Run(freq_index);
    timespec_get(&end_time, TIME_UTC);

    /* Print Inference processing time */
    diff = timedifference_msec(start_time, end_time);
    std::cout << "[TIME] AI Processing Time: " << std::fixed << std::setprecision(2) << diff << " msec." << std::endl;

    /* Get the number of output of the target model. For ResNet, 1 output. */
    auto output_num = runtime.GetNumOutput();
    if (output_num == 3)
    {
        std::cout << "[INFO] Output layer =3::maybe yolov3. End." << std::endl;
        return 0;
    }
    else if (output_num != 1)
    {
        std::cerr << "[ERROR] Output size : not 1." << std::endl;
        return 0;
    }

    /* Prepare output access (zero-copy vs. non zero-copy) */
    InOutDataType out_type;
    void*         out_addr;
    int64_t       out_elems;

    if (zero_copy) {
        auto out_info = runtime.GetOutputInfo(); // {name, size(bytes), type}
        if (out_info.size() != 1) {
            if (out_info.size() == 3) {
                std::cout << "[INFO] Output layer =3::maybe yolov3. End." << std::endl;
                return 0;
            }
            std::cerr << "[ERROR] Output size : not 1." << std::endl;
            return 0;
        }
        const auto& oi0 = out_info[0];
        const std::string& out_name = std::get<0>(oi0);
        size_t out_bytes = std::get<1>(oi0);
        out_type = std::get<2>(oi0);
        out_addr = runtime.GetOutputPtr(out_name);
        if (!out_addr) {
            std::cerr << "[ERROR] GetOutputPtr returned nullptr." << std::endl;
            return 0;
        }
        if (out_type == InOutDataType::FLOAT32)      out_elems = out_bytes / sizeof(float);
        else if (out_type == InOutDataType::FLOAT16) out_elems = out_bytes / sizeof(uint16_t);
        else if (out_type == InOutDataType::INT64)   out_elems = out_bytes / sizeof(int64_t);
        else if (out_type == InOutDataType::INT32)   out_elems = out_bytes / sizeof(int32_t);
        else { std::cerr << "[ERROR] Output data type : unsupported." << std::endl; return 0; }
    } else {
    /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
    auto output_buffer = runtime.GetOutput(0);
        out_type = std::get<0>(output_buffer);
        out_addr = std::get<1>(output_buffer);
        out_elems= std::get<2>(output_buffer);
    }

    /* Array to store the FP32 output data from inference. */
    float floatarr[out_elems];

    /* Clear the classification result. */
    result.clear();
    if (InOutDataType::FLOAT16 == out_type)
    {
        std::cout << "Output data type : FP16." << std::endl;
        /* Extract data in FP16 <uint16_t>. */
        uint16_t *data_ptr = reinterpret_cast<uint16_t *>(out_addr);

        /* Post-processing for FP16 */
        /* Cast FP16 output data to FP32. */
        for (int n = 0; n < out_elems; n++)
        {
            floatarr[n] = float16_to_float32(data_ptr[n]);
        }
    }
    else if (InOutDataType::FLOAT32 == out_type)
    {
        std::cout << "Output data type : FP32." << std::endl;
        /* Extract data in FP32 <float>. */
        float *data_ptr = reinterpret_cast<float *>(out_addr);
        /*Copy output data to buffer for post-processing. */
        for (int n = 0; n < out_elems; n++)
        {
            floatarr[n] = data_ptr[n];
        }
    }
    else if (InOutDataType::INT64 == out_type)
    {
        std::cout << "Output data type : INT64." << std::endl;
        int64_t* data_ptr = reinterpret_cast<int64_t*>(out_addr);

        std::cout << "[INFO] There are no prepost for INT64. End." << std::endl;
        return 0;
    }
    else if (InOutDataType::INT32 == out_type)
    {
        std::cout << "Output data type : INT32." << std::endl;
        int32_t* data_ptr = reinterpret_cast<int32_t*>(out_addr);
        for (int n = 0; n < out_elems; n++) {
            floatarr[n] = static_cast<float>(data_ptr[n]);
        }
    }
    else
    {
        std::cerr << "[ERROR] Output data type : not floating point type." << std::endl;
        /*End application*/
        return 0;
    }

    /*Post-processing: common for FP16/FP32*/
    /* Softmax 1000 class scores. */
    softmax(&floatarr[0], out_elems);
    /* Sort in decending order. */
    for (int n = 0; n < out_elems; n++)
    {
        result[floatarr[n]] = n;
    }

    result_cnt = 0;
    /* Print Top-5 results. */
    std::cout << "Result ---------------------- " << std::endl;
    for (auto it = result.rbegin(); it != result.rend(); it++)
    {
        result_cnt++;
        if (result_cnt > 5)
            break;
        std::cout << "  Top " << result_cnt << " ["
                  << std::right << std::setw(5) << std::fixed << std::setprecision(1) << (float)(*it).first * 100
                  << "%] : [" << label_file_map[(*it).second] << "]" << std::endl;
    }
    return 0;
}
