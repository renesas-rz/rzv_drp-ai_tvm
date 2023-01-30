/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2023
 *　
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
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
* Version      : 1.0.0
* Description  : DRP-AI TVM[*1] Application Example
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>
#include <climits>
#include <cstdlib>
#include <cstring>

#include "MeraDrpRuntimeWrapper.h"
#include "PreRuntime.h"

/*****************************************
* Macro
******************************************/
/*Input image info*/
#define INPUT_IMAGE_H       (480)
#define INPUT_IMAGE_W       (640)
#define INPUT_IMAGE_C_YUY2  (2)
/*Model input info*/
#define MODEL_IN_H          (224)
#define MODEL_IN_W          (224)
#define MODEL_IN_C          (3)

/* Image buffer (u-dma-buf) */
unsigned char * img_buffer;

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
* Function Name : read_bin
* Description   : Function to load YUY2 bin file into img_buffer[]
*                 Make sure img_buffer is allocated for size of buffer_size beforehand.
* Arguments     : filename = name of bin file to be read
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t read_bin(std::string filename, uint32_t buffer_size)
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == NULL)
    {
        std::cerr << "[ERROR] Failed to open binary file: "<<filename << std::endl;
        return -1;
    }

    if (!fread(&img_buffer[0], sizeof(unsigned char), buffer_size, fp))
    {
        std::cerr << "[ERROR] Failed to read binary file: "<<filename << std::endl;
        fclose(fp);
        return -1;
    }
    fclose(fp);

    return 0;
}

/*****************************************
* Function Name : get_udmabuf_addr
* Description   : Function to obtain the u-dma-buf start address.
* Arguments     : -
* Return value  : uint32_t = u-dma-buf start address in 32-bit.
******************************************/
uint32_t get_udmabuf_addr()
{
    int8_t fd = 0;
    char addr[1024];
    int32_t read_ret = 0;
    uint32_t udmabuf_addr_start = 0;
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
    /* Filter the bit heigher than 32 bit */
    udmabuf_addr_start &=0xFFFFFFFF;
    return udmabuf_addr_start;
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
    uint8_t ret = 0;
    /* ResNet Information */
    float mean[] = { 0.485, 0.456, 0.406 };
    float std[] = { 0.229, 0.224, 0.225 };
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
    /* DRP-AI TVM[*1] Runtime object */
    MeraDrpRuntimeWrapper runtime;
    /* Pre-processing Runtime object */
    PreRuntime preruntime;


    /* Model Binary */
    std::string model_dir = "resnet18_onnx";
    /* Pre-processing Runtime Object */
    const char prod_v2l[]="V2L";
    const char prod_v2m[]="V2M";
    const char prod_v2ma[]="V2MA";
    const char *env_var = std::getenv("PRODUCT");
    if (env_var == nullptr )
    {
        std::cerr << "[ERROR] PRODUCT variable is not set." << std::endl << "E.g., for V2MA, run \"export PRODUCT=V2MA\"." << std::endl;
        return 0;
    }	    

    std::string pre_dir = "";
    if ( 0 == std::strcmp(env_var, prod_v2l))
    {
        pre_dir += "preprocess_tvm_v2l";
    }
	else if ( 0 == std::strcmp(env_var, prod_v2m))
    {
        pre_dir += "preprocess_tvm_v2m";
    }
    else if ( 0 == std::strcmp(env_var, prod_v2ma))
    {
        pre_dir += "preprocess_tvm_v2ma";
    }
    else
    {
        std::cerr << "[ERROR] PRODUCT variable is unsupported value." << std::endl;
        return 0;
    }

    /* Input image file */
    std::string filename = "sample.yuv";

    /* About u-dma-buf
        Pre-processing Runtime requires the input buffer to be allocated in continuous memory area.
        This application uses imagebuf (u-dma-buf) memory area.
        Refer to RZ/V2MA DRP-AI Support Package for imagebuf details. */
    /*File descriptor for u-dma-buf*/
    int8_t udmabuf_fd = 0;
    uint32_t udmabuf_size = INPUT_IMAGE_H*INPUT_IMAGE_W*INPUT_IMAGE_C_YUY2;
    /* u-dma-buf start addres */
    uint64_t udmabuf_addr_start = 0;

    /* Load Label list */
    label_file_map = load_label_file(labels);
    if (label_file_map.empty())
    {
        std::cerr << "[ERROR] Label file : failed to load "<< labels << std::endl;
        return 0;
    }

    /*Load pre_dir object to DRP-AI */
    ret = preruntime.Load(pre_dir);
    if (0 < ret)
    {
        std::cerr << "[ERROR] Failed to run Pre-processing Runtime Load()." << std::endl;
        return 0;
    }

    /*Load model_dir structure and its weight to runtime object */
    runtime.LoadModel(model_dir);

    /*Get input data */
    auto input_data_type = runtime.GetInputDataType(0);

    /*Obtain u-dma-buf memory area starting address*/
    udmabuf_addr_start = get_udmabuf_addr();
    if (0 == udmabuf_addr_start)
    {
        std::cerr << "[ERROR] Failed to get u-dma-buf." << std::endl;
        return 0;
    }
    /* Allocate image buffer in u-dma-buf memory area */
    udmabuf_fd = open("/dev/udmabuf0", O_RDWR );
    if (0 > udmabuf_fd)
    {
        std::cerr << "[ERROR] Failed to open udmabuf " << std::endl;
        return 0;
    }
    img_buffer =(unsigned char*) mmap(NULL, udmabuf_size ,PROT_READ|PROT_WRITE, MAP_SHARED,  udmabuf_fd, 0);
    if (MAP_FAILED == img_buffer)
    {
        std::cerr << "[ERROR] Failed to run mmap: udmabuf " << std::endl;
        close(udmabuf_fd);
        return 0;
    }

    /* Write once to allocate physical memory to u-dma-buf virtual space.
    * Note: Do not use memset() for this.
    *       Because it does not work as expected. */
    {
        for(int i = 0 ; i <  udmabuf_size; i++)
        {
            img_buffer[i] = 0;
        }
    }
    /*Load input data */
    /*Input data type can be either FLOAT32 or FLOAT16, which depends on the model */
    if (InOutDataType::FLOAT32 == input_data_type)
    {
        /* Pre-processing */
        /* Read image data from file */
        ret = read_bin(filename, INPUT_IMAGE_W*INPUT_IMAGE_H*INPUT_IMAGE_C_YUY2);
        if (ret > 0)
        {
            std::cerr << "[ERROR] Failed to read bin " << std::endl;
            munmap(img_buffer, udmabuf_size);
            close(udmabuf_fd);
            return 0;
        }
        /*Define parameter to be changed in Pre-processing Runtime*/
        s_preproc_param_t in_param;
        in_param.pre_in_addr = udmabuf_addr_start;
        in_param.pre_in_shape_w = INPUT_IMAGE_W;
        in_param.pre_in_shape_h = INPUT_IMAGE_H;
        in_param.pre_in_format = INPUT_YUYV;
        in_param.resize_w = MODEL_IN_W;
        in_param.resize_h = MODEL_IN_H;
        in_param.resize_alg = ALG_BILINEAR;
        /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
        in_param.cof_add[0]= -255*mean[0];
        in_param.cof_add[1]= -255*mean[1];
        in_param.cof_add[2]= -255*mean[2];
        in_param.cof_mul[0]= 1/(std[0]*255);
        in_param.cof_mul[1]= 1/(std[1]*255);
        in_param.cof_mul[2]= 1/(std[2]*255);
        /*Output variables for Pre-processing Runtime */
        float* output_ptr;
        uint32_t out_size;

        timespec_get(&start_time, TIME_UTC);
        /*Run pre-processing*/
        ret = preruntime.Pre(&in_param, &output_ptr, &out_size);
        if (0 < ret)
        {
            std::cerr << "[ERROR] Failed to run Pre-processing Runtime Pre()." << std::endl;
            munmap(img_buffer, udmabuf_size);
            close(udmabuf_fd);
            return 0;
        }
        timespec_get(&end_time, TIME_UTC);

        /* Print Inference processing time */
        diff = timedifference_msec(start_time, end_time);
        std::cout<<"[TIME] Pre Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

        /*Set Pre-processing output to be inference input. */
        runtime.SetInput(0, output_ptr);
    }
    else if (InOutDataType::FLOAT16 == input_data_type)
    {
        std::cerr << "[ERROR] Input data type : FP16." << std::endl;
        /*If your model input data type is FP16, use std::vector<uint16_t> for reading input data. */
        munmap(img_buffer, udmabuf_size);
        close(udmabuf_fd);
        return 0;
    }
    else
    {
        std::cerr << "[ERROR] Input data type : neither FP32 nor FP16." << std::endl;
        munmap(img_buffer, udmabuf_size);
        close(udmabuf_fd);
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
        munmap(img_buffer, udmabuf_size);
        close(udmabuf_fd);
        return 0;
    }

    /* Comparing output with reference.*/
    /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
    auto output_buffer = runtime.GetOutput(0);
    int64_t out_size = std::get<2>(output_buffer);
    /* Array to store the FP32 output data from inference. */
    float floatarr[out_size];

    /* Clear the classification result. */
    result.clear();

    if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
    {
        std::cout << "Output data type : FP16." << std::endl;
        /* Extract data in FP16 <uint16_t>. */
        uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));

        /* Post-processing for FP16 */
        /* Cast FP16 output data to FP32. */
        for (int n = 0; n < out_size; n++)
        {
            floatarr[n] = float16_to_float32(data_ptr[n]);
        }
    }
    else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
    {
        std::cout << "Output data type : FP32." << std::endl;
        /* Extract data in FP32 <float>. */
        float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
        /*Copy output data to buffer for post-processing. */
        for (int n = 0; n < out_size; n++)
        {
            floatarr[n] = data_ptr[n];
        }
    }
    else
    {
        std::cerr << "[ERROR] Output data type : not floating point type." << std::endl;
        /*End application*/
        munmap(img_buffer, udmabuf_size);
        close(udmabuf_fd);
        return 0;
    }

    /*Post-processing: common for FP16/FP32*/
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

    munmap(img_buffer, udmabuf_size);
    close(udmabuf_fd);
    return 0;
}
