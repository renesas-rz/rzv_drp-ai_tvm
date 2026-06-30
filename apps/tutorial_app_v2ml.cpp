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
* File Name    : tutorial_app_v2ml.cpp
* Version      : 2.8.0
* Description  : DRP-AI TVM[*1] Application Example for RZ/V2M, RZ/V2MA, and RZ/V2L
***********************************************************************************************************************/

/*****************************************
* includes
******************************************/
#include <linux/drpai.h>
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>
#include <climits>

#include "MeraDrpRuntimeWrapper.h"
#include "PreRuntime.h"

extern "C" {
#include "mmngr_user_public.h"
}

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

/*****************************************
* Image buffer
******************************************/
typedef struct
{
    uint8_t* virt_addr;
    uint32_t phy_addr;
    uint32_t size;
    MMNGR_ID mmngr_id;
} image_buffer_t;

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
* Function Name : read_bmp
* Description   : Function to load BMP file into buffer
* NOTE          : This is just the simplest example to read Windows Bitmap v3 file.
*                 This function does not have header check.
* Arguments     : filename = name of BMP file to be read
*                 width  = BMP image width
*                 height = BMP image height
*                 channel = BMP image color channel
*                 buffer = pointer to buffer to store image data
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t read_bmp(std::string filename, uint32_t width, uint32_t height, uint32_t channel, uint8_t* buffer)
{
    int32_t i = 0;
    FILE *fp = NULL;
    size_t ret = 0;
    uint32_t header_size = FILEHEADERSIZE + INFOHEADERSIZE_W_V3;
    /*  Read header for Windows Bitmap v3 file. */
    uint8_t bmp_header[header_size];
    uint8_t * bmp_line_data;

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
        std::cerr << "[ERROR] Failed to run fread(): errno="<<(uint8_t) errno << std::endl;
        fclose(fp);
        return -1;
    }
    /* Single row image data */
    bmp_line_data = (uint8_t *) malloc(sizeof(uint8_t) * line_width);
    if (NULL == bmp_line_data)
    {
        std::cerr << "[ERROR] Failed to malloc for fread(): errno="<<(uint8_t) errno << std::endl;
        fclose(fp);
        return -1;
    }

    for (i = height-1; i >= 0; i--)
    {
        errno = 0;
        ret = fread(bmp_line_data, sizeof(uint8_t), line_width, fp);
        if (!ret)
        {
            std::cerr << "[ERROR] Failed to run fread(): errno="<<(uint8_t) errno << std::endl;
            free(bmp_line_data);
            fclose(fp);
            return -1;
        }
        memcpy(buffer+i*width*channel, bmp_line_data, sizeof(uint8_t)*width*channel);
    }

    free(bmp_line_data);
    fclose(fp);
    return 0;
}

/*****************************************
* Function Name : get_drpai_start_addr
* Description   : Function to get the start address of DRPAImem.
* Arguments     : -
* Return value  : uint32_t = DRPAImem start address in 32-bit.
******************************************/
uint32_t get_drpai_start_addr()
{
    int fd  = 0;
    int ret = 0;
    drpai_data_t drpai_data;

    errno = 0;

    fd = open("/dev/drpai0", O_RDWR);
    if (0 > fd )
    {
        std::cerr << "[ERROR] Failed to open DRP-AI Driver : errno=" << errno << std::endl;
        return (uint32_t)NULL;
    }

    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(fd , DRPAI_GET_DRPAI_AREA, &drpai_data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to get DRP-AI Memory Area : errno=" << errno << std::endl;
        return (uint32_t)NULL;
    }

    if (0 < fd)
    {
        errno = 0;
        ret = close(fd);
        if (0 != ret)
        {
            std::cerr << "[ERROR] Failed to close DRP-AI Driver: errno=" << errno << std::endl;
            return (uint32_t)NULL;
        }
    }

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

/*****************************************
* Function Name : alloc_image_buffer
* Description   : Allocate image buffer using mmngr.
* Arguments     : buffer = pointer to image buffer struct
*                 size   = requested size
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
static int8_t alloc_image_buffer(image_buffer_t* buffer, uint32_t size)
{
    if (nullptr == buffer)
    {
        return -1;
    }

#ifdef MMNGR_PHYS_ADDR_64BIT
    unsigned long phard_addr = 0;
#else
    unsigned int  phard_addr = 0;
#endif
    void* puser_virt_addr = nullptr;

    buffer->virt_addr = nullptr;
    buffer->phy_addr  = 0;
    buffer->size      = 0;
    buffer->mmngr_id  = 0;

    int ret = mmngr_alloc_in_user_ext(&buffer->mmngr_id,
                                      size,
                                      &phard_addr,
                                      &puser_virt_addr,
                                      MMNGR_VA_SUPPORT_CACHED,
                                      NULL);
    if (0 != ret)
    {
        std::cerr << "[ERROR] Failed to allocate mmngr memory: ret=" << ret << std::endl;
        return -1;
    }

    if (nullptr == puser_virt_addr)
    {
        std::cerr << "[ERROR] mmngr_alloc_in_user_ext returned null virtual address" << std::endl;
        mmngr_free_in_user_ext(buffer->mmngr_id);
        return -1;
    }

    memset(puser_virt_addr, 0, size);

    /* On RZ/V2M/V2MA the mmngr reserved region may be above 4GB.
     * DRP-AI input path uses the lower 32 bits of the physical address. */
    buffer->virt_addr = static_cast<uint8_t*>(puser_virt_addr);
    buffer->phy_addr  = static_cast<uint32_t>(phard_addr & 0xFFFFFFFFUL);
    buffer->size      = size;

    return 0;
}

/*****************************************
* Function Name : flush_image_buffer
* Description   : Flush image buffer cache.
* Arguments     : buffer = pointer to image buffer struct
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
static int8_t flush_image_buffer(image_buffer_t* buffer)
{
    if (nullptr == buffer)
    {
        return -1;
    }

    int ret = mmngr_flush(buffer->mmngr_id, 0, buffer->size);
    if (0 != ret)
    {
        std::cerr << "[ERROR] Failed to flush mmngr cache: ret=" << ret << std::endl;
        return -1;
    }

    return 0;
}

/*****************************************
* Function Name : free_image_buffer
* Description   : Free image buffer.
* Arguments     : buffer = pointer to image buffer struct
******************************************/
static void free_image_buffer(image_buffer_t* buffer)
{
    if (nullptr == buffer)
    {
        return;
    }

    if (buffer->virt_addr != nullptr)
    {
        mmngr_free_in_user_ext(buffer->mmngr_id);
        buffer->virt_addr = nullptr;
        buffer->phy_addr  = 0;
        buffer->size      = 0;
        buffer->mmngr_id  = 0;
    }
}

int main(int argc, char** argv)
{
    uint8_t ret = 0;
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
    std::string pre_dir = model_dir + "/preprocess";

    /* Input image file */
    std::string filename = "sample.bmp";

    /* Image buffer */
    image_buffer_t img_buffer_obj = {};
    
    uint32_t drpaimem_addr_start = 0;
    uint32_t img_buffer_size = INPUT_IMAGE_H*INPUT_IMAGE_W*INPUT_IMAGE_C;
    
    /* Load Label list */
    label_file_map = load_label_file(labels);
    if (label_file_map.empty())
    {
        std::cerr << "[ERROR] Label file : failed to load "<< labels << std::endl;
        return 0;
    }

    /* Get DRP-AI memory start address */
    drpaimem_addr_start = get_drpai_start_addr();
    if (drpaimem_addr_start == (uint32_t)NULL) return 0;

    /* Define memory allocation constants */
    const uint32_t MEMORY_ALIGNMENT = 0x1000000; // 16MB alignment

    /* First, load the runtime model at the start of DRP-AI memory */
    uint32_t runtime_start_addr = drpaimem_addr_start;
    /* Ensure runtime_start_addr is aligned to 16MB boundary */
    runtime_start_addr = (runtime_start_addr + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);

    /* Load the runtime model */
    runtime.LoadModel(model_dir, runtime_start_addr);

    /* Get the last address used by runtime */
    uint64_t runtime_last_addr = runtime.GetLastAddress();
    std::cout << "Runtime memory usage: " << std::hex 
              << "start=0x" << runtime_start_addr 
              << ", end=0x" << runtime_last_addr << std::dec << std::endl;

    /* Calculate the start address for preruntime */
    uint32_t preruntime_start_addr;
    if (runtime_last_addr == 0) {
        /* CPU-only model case - use the original DRP-AI memory start address */
        preruntime_start_addr = drpaimem_addr_start;
        std::cout << "Maybe CPU-only model. Using head of DRP-AI memory for preruntime." << std::endl;
    } else {
        /* DRP-AI model case - allocate memory after runtime */
        preruntime_start_addr = (runtime_last_addr + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);
    }
    std::cout << "Preruntime memory allocation: start=0x" << std::hex << preruntime_start_addr << std::dec << std::endl;

    /* Load pre_dir object to DRP-AI */
    ret = preruntime.Load(pre_dir, preruntime_start_addr);
    if (0 < ret)
    {
        std::cerr << "[ERROR] Failed to run Pre-processing Runtime Load()." << std::endl;
        return 0;
    }

    /*Get input data */
    auto input_data_type = runtime.GetInputDataType(0);

    /* Allocate image buffer using mmngr */
    std::cout << "Allocating image buffer..." << std::endl;
    ret = alloc_image_buffer(&img_buffer_obj, img_buffer_size);
    if (0 != ret)
    {
        std::cerr << "[ERROR] Failed to allocate image buffer." << std::endl;
        return 0;
    }
    
    std::cout << "Image buffer allocated successfully" << std::endl;
    std::cout << "  Virtual address: " << (void*)img_buffer_obj.virt_addr << std::endl;
    std::cout << "  Physical address: 0x" << std::hex << img_buffer_obj.phy_addr << std::dec << std::endl;

    /*Load input data */
    /*Input data type can be either FLOAT32 or FLOAT16, which depends on the model */
    if (InOutDataType::FLOAT32 == input_data_type)
    {
        /* Pre-processing */
        /* Read image data from file */
        ret = read_bmp(filename, INPUT_IMAGE_W, INPUT_IMAGE_H, INPUT_IMAGE_C, img_buffer_obj.virt_addr);
        if (ret > 0)
        {
            std::cerr << "[ERROR] Failed to read image :"<<filename << std::endl;
            free_image_buffer(&img_buffer_obj);
            return 0;
        }
        
        /* Flush cache to ensure data is written to physical memory */
        ret = flush_image_buffer(&img_buffer_obj);
        if (0 != ret)
        {
            std::cerr << "[ERROR] Failed to flush cache." << std::endl;
            free_image_buffer(&img_buffer_obj);
            return 0;
        }
        
        /*Define parameter to be changed in Pre-processing Runtime*/
        s_preproc_param_t in_param;
        in_param.pre_in_addr    = img_buffer_obj.phy_addr;
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
        void* output_ptr;
        uint32_t out_size;

        timespec_get(&start_time, TIME_UTC);
        /*Run pre-processing*/
        ret = preruntime.Pre(&in_param, &output_ptr, &out_size);
        if (0 < ret)
        {
            std::cerr << "[ERROR] Failed to run Pre-processing Runtime Pre()." << std::endl;
            free_image_buffer(&img_buffer_obj);
            return 0;
        }
        timespec_get(&end_time, TIME_UTC);
        /* Print Inference processing time */
        diff = timedifference_msec(start_time, end_time);
        std::cout<<"[TIME] Pre Processing Time: "<< std::fixed << std::setprecision(2)<< diff << " msec." << std::endl;

        /*Set Pre-processing output to be inference input. */
        runtime.SetInput(0, (float*)output_ptr);
    }
    else if (InOutDataType::FLOAT16 == input_data_type)
    {
        std::cerr << "[ERROR] Input data type : FP16." << std::endl;
        /*If your model input data type is FP16, use std::vector<uint16_t> for reading input data. */
        free_image_buffer(&img_buffer_obj);
        return 0;
    }
    else
    {
        std::cerr << "[ERROR] Input data type : neither FP32 nor FP16." << std::endl;
        free_image_buffer(&img_buffer_obj);
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
        free_image_buffer(&img_buffer_obj);
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
        free_image_buffer(&img_buffer_obj);
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

    free_image_buffer(&img_buffer_obj);
    return 0;
}
