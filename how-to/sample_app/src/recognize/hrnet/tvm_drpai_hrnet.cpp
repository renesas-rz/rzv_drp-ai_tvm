/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : tvm_drpai_hrnet.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_hrnet.h"
TVM_HRNET_DRPAI::TVM_HRNET_DRPAI(uint8_t id) :
    IRecognizeModel(0,
        TVM_MODEL_DIR.data(), MODEL_NAME.data(),
        TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL,
        TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, id)      
{
    preruntime.Load(pre_dir);

    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
    in_param.cof_add[0]= -255*mean[0];//-123.675;
    in_param.cof_add[1]= -255*mean[1];//-116.28;
    in_param.cof_add[2]= -255*mean[2];//-103.53;
    in_param.cof_mul[0]= 1/(stdev[0]*255);//0.01712475;
    in_param.cof_mul[1]= 1/(stdev[1]*255);//0.017507;
    in_param.cof_mul[2]= 1/(stdev[2]*255);//0.01742919;

    if (id == MODE_TVM_HRNET_DRPAI)
    {
        model_dir = TVM_MODEL_DIR;
        model_name = MODEL_NAME;
        _model_w = TVM_MODEL_IN_W;
        _model_h = TVM_MODEL_IN_H;
        _model_c = TVM_MODEL_IN_C;
        std::cout << "DRP-AI TVM HRNet model" << std::endl;

        pre_cropped_image_left = CROPPED_IMAGE_LEFT;
        pre_cropped_image_top = CROPPED_IMAGE_TOP;
        pre_cropped_image_width = CROPPED_IMAGE_WIDTH;
        pre_cropped_image_height =  CROPPED_IMAGE_HEIGHT;
        
        /*Define pre-processing parameter*/
        in_param.pre_in_shape_w = pre_cropped_image_width;
        in_param.pre_in_shape_h = pre_cropped_image_height;
        in_param.pre_in_format = INPUT_YUYV;
        in_param.resize_w = TVM_MODEL_IN_W;
        in_param.resize_h = TVM_MODEL_IN_H;
        in_param.resize_alg = ALG_BILINEAR;
        
        num_inf_out = NUM_OUTPUT_W * NUM_OUTPUT_H * NUM_OUTPUT_C;

        hrnet_num_output_c = NUM_OUTPUT_C;
        hrnet_num_output_w = NUM_OUTPUT_W;
        hrnet_num_output_h = NUM_OUTPUT_H;
        hrnet_th_kpt = TH_KPT;

        hrnet_output_width = OUTPUT_WIDTH;
        hrnet_output_left = OUTPUT_LEFT;
        hrnet_output_adj_x = OUTPUT_ADJ_X;
        hrnet_output_height = OUTPUT_HEIGHT;
        hrnet_output_top = OUTPUT_TOP;
        hrnet_output_adj_y = OUTPUT_ADJ_Y;
    }
    else if (id == MODE_TVM_HRNETV2_DRPAI)
    {
        model_dir = TVM_MODEL_DIR_V2;
        model_name = MODEL_NAME_V2;
        _model_w = TVM_MODEL_IN_W_V2;
        _model_h = TVM_MODEL_IN_H_V2;
        _model_c = TVM_MODEL_IN_C_V2;
        std::cout << "DRP-AI TVM HRNetv2 model" << std::endl;

        pre_cropped_image_left = CROPPED_IMAGE_LEFT_V2;
        pre_cropped_image_top = CROPPED_IMAGE_TOP_V2;
        pre_cropped_image_width = CROPPED_IMAGE_WIDTH_V2;
        pre_cropped_image_height =  CROPPED_IMAGE_HEIGHT_V2;
        
        /*Define pre-processing parameter*/
        in_param.pre_in_shape_w = pre_cropped_image_width;
        in_param.pre_in_shape_h = pre_cropped_image_height;
        in_param.pre_in_format = INPUT_YUYV;
        in_param.resize_w = TVM_MODEL_IN_W_V2;
        in_param.resize_h = TVM_MODEL_IN_H_V2;
        in_param.resize_alg = ALG_BILINEAR;
        
        num_inf_out = NUM_OUTPUT_W_V2 * NUM_OUTPUT_H_V2 * NUM_OUTPUT_C_V2;

        hrnet_num_output_c = NUM_OUTPUT_C_V2;
        hrnet_num_output_w = NUM_OUTPUT_W_V2;
        hrnet_num_output_h = NUM_OUTPUT_H_V2;
        hrnet_th_kpt = TH_KPT_V2;

        hrnet_output_width = OUTPUT_WIDTH_V2;
        hrnet_output_left = OUTPUT_LEFT_V2;
        hrnet_output_adj_x = OUTPUT_ADJ_X_V2;
        hrnet_output_height = OUTPUT_HEIGHT_V2;
        hrnet_output_top = OUTPUT_TOP_V2;
        hrnet_output_adj_y = OUTPUT_ADJ_Y_V2;
    }
     
    outBuffSize = num_inf_out;
#ifdef TENTATIVE
    /* Obtain udmabuf memory area starting address */
    int8_t fd = 0;
    char addr[1024];
    int32_t read_ret = 0;
    errno = 0;
    fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY);
    if (0 > fd)
    {
        fprintf(stderr, "[ERROR] Failed to open udmabuf0/phys_addr : errno=%d\n", errno);
        return;
    }
    read_ret = read(fd, addr, 1024);
    if (0 > read_ret)
    {
        fprintf(stderr, "[ERROR] Failed to read udmabuf0/phys_addr : errno=%d\n", errno);
        close(fd);
        return;
    }
    sscanf(addr, "%lx", &udmabuf_crop_addr);
    close(fd);
    /* Filter the bit higher than 32 bit */
    udmabuf_crop_addr &= 0xFFFFFFFF;
    /*Add capture buffer offset to udmabuf_crop_addr*/
    udmabuf_crop_addr += TVM_DRPAI_IN_WIDTH*TVM_DRPAI_IN_HEIGHT*TVM_DRPAI_IN_CHANNEL*4;
    size = pre_cropped_image_width * pre_cropped_image_height * TVM_DRPAI_IN_CHANNEL;

    /*Mmap udmabuf for cropped image*/
    udmabuf_fd = open("/dev/udmabuf0", O_RDWR );
    if (udmabuf_fd < 0)
    {
        fprintf(stderr, "[ERROR] Failed to open udmabuf.\n");
        return;
    }
    crop_out_ptr =(uint8_t*) mmap(NULL, size ,PROT_READ|PROT_WRITE, MAP_SHARED,  udmabuf_fd, TVM_DRPAI_IN_WIDTH*TVM_DRPAI_IN_HEIGHT*TVM_DRPAI_IN_CHANNEL*4 ); 

    if (crop_out_ptr == MAP_FAILED)
    {
        fprintf(stderr, "[ERROR] Failed to mmap udmabuf.\n");
        close(udmabuf_fd);
        return;
    }
    /* Write once to allocate physical memory to u-dma-buf virtual space.
    * Note: Do not use memset() for this.
    *       Because it does not work as expected. */
    {
        for (int i = 0 ; i < size; i++)
        {
            crop_out_ptr[i] = 0;
        }
    }
#endif
}
#ifdef TENTATIVE
TVM_HRNET_DRPAI::~TVM_HRNET_DRPAI()
{
    munmap(crop_out_ptr, size);
    if (udmabuf_fd > 0)
    {
        close(udmabuf_fd);
    }
}
#endif
/**
 * @brief inf_pre_process
 * @details Run pre-processing.
 * @details For CPU input, use input_data for input data.
 * @details For DRP-AI input, use addr for input data stored address
 * @param input_data Input data pointer
 * @param width new input data width.
 * @param height new input data width.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI::inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
{
    /*Update width and height*/
    if ((width != _capture_w) || (height != _capture_h)) 
    {
        _capture_w = width;
        _capture_h = height;
        in_param.pre_in_shape_w = _capture_w;
        in_param.pre_in_shape_h = _capture_h;
    }

    pre_process(input_data, addr, arg, buf_size);
    return 0;
}
/**
 * @brief inf_post_process
 * @details Run post-processing
 * @param arg Inference output data pointer
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI::inf_post_process(float* arg)
{
    postproc_result.clear();
    post_process(postproc_result, arg);
    return 0;
}
/**
 * @brief print_result
 * @details Print AI result on console
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI::print_result()
{
    /*Displays AI Inference results on console*/
    int32_t id = 0;
    for (pos_t p : postproc_result)
    {
        printf("  ID %d: (%.2d, %.2d): %.2f%% \n", id, p.X, p.Y, p.preds);
        id++;
    }
    return 0;
}
/**
* @brief  hrnet_offset
* @details Get the offset number to access the HRNet attributes
* @param  b = Number to indicate which region [0~17] (HRNet) [0~21] (HRNetv2)
* @param  y = Number to indicate which region [0~64]
* @param  x = Number to indicate which region [0~48] (HRNet) [0~64] (HRNetv2)
* @return int32_t offset to access the HRNet attributes.
*/
int32_t TVM_HRNET_DRPAI::hrnet_offset(int32_t b, int32_t y, int32_t x)
{
    return b * hrnet_num_output_w * hrnet_num_output_h + y * hrnet_num_output_w + x;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Pose detection result data
 */
shared_ptr<PredictNotifyBase> TVM_HRNET_DRPAI::get_command()
{
    PoseDetection* ret = new PoseDetection();
    for (pos_t p : postproc_result)
    {
        ret->predict.push_back(p);
    }
    return shared_ptr<PredictNotifyBase>(move(ret));
}
/**
 * @brief pre_process
 * @details implementation pre process using Pre-processing Runtime and CPU.
 * @param input_data Input data pointer
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_HRNET_DRPAI::pre_process(uint8_t* input_data, uint32_t addr, float** arg, uint32_t* buf_size)
{
#ifdef TENTATIVE
    uint8_t err_crop = 0;
    uint32_t x;
    uint32_t y;
    uint32_t top    = pre_cropped_image_top;
    uint32_t bottom = top + pre_cropped_image_height;
    uint32_t left   = pre_cropped_image_left / 2;
    uint32_t right  = left + pre_cropped_image_width / 2;
    uint32_t index  = 0;
    drpai_data_t drpai_data;
    for (y = top; y < bottom; y++)
    {
        for (x = left; x < right; x++)
        {
            *((uint32_t *)&crop_out_ptr[index]) = *((uint32_t *)&input_data[y * TVM_DRPAI_IN_WIDTH * YUY2_NUM_CHANNEL + x * YUY2_NUM_DATA]);
            index += YUY2_NUM_DATA;
        }
    }
#endif
    in_param.pre_in_addr = (uintptr_t) udmabuf_crop_addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, arg, buf_size);
    return 0;
}
/**
* @brief sign
* @details Get the sign of the input value
* @param   x = input value
* @return int8_t  1 if positive -1 if not
*/
int8_t TVM_HRNET_DRPAI::sign(int32_t x)
{
    return x > 0 ? 1 : -1;
}
/**
 * @brief coord_convert
 * @details HRNet coord convert to original image size
 * @param result reference to store result
 * @param preds postproce result
 */
void TVM_HRNET_DRPAI::coord_convert(vector<pos_t> &result, vector<vector<float>> &preds)
{
    /* Render skeleton on image and print their details */
    int32_t posx = 0;
    int32_t posy = 0;
    int8_t i = 0;
    result.clear();
    for (i = 0; i < hrnet_num_output_c; i++)
    {
        /* 0.5 is round */
        posx = (int32_t)(preds.at(i).at(0) / HRNET_CROPPED_IMAGE_WIDTH * hrnet_output_width + 0.5) + hrnet_output_left + hrnet_output_adj_x;
        posy = (int32_t)(preds.at(i).at(1) / HRNET_CROPPED_IMAGE_HEIGHT * hrnet_output_height + 0.5) + hrnet_output_top + hrnet_output_adj_y;

        pos_t p;
        p.X = posx;
        p.Y = posy;
        p.preds = preds.at(i).at(2) * 100;
        result.push_back(p);
    }
    return;
}
/**
 * @brief post_process
 * @details implementation post process
 * @param det reference to store bounding box
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_HRNET_DRPAI::post_process(vector<pos_t> &result,float* floatarr)
{
    float     lowest_kpt_score = 0;

    float score = 0;
    int32_t b = 0;
    int32_t y = 0;
    int32_t x = 0;
    int32_t i = 0;
    int32_t offs = 0;

    float center[] = { HRNET_CROPPED_IMAGE_WIDTH / 2 - 1, HRNET_CROPPED_IMAGE_HEIGHT / 2 - 1 };
    int8_t ind_x = -1;
    int8_t ind_y = -1;
    float max_val = -1;
    float scale_x, scale_y, coords_x, coords_y;
    vector<vector<float>> hrnet_preds(hrnet_num_output_c, vector<float>(3));

    for (b = 0; b < hrnet_num_output_c; b++)
    {
        float scale[] = { HRNET_CROPPED_IMAGE_WIDTH / 200.0, HRNET_CROPPED_IMAGE_HEIGHT / 200.0 };
        ind_x = -1;
        ind_y = -1;
        max_val = -1;
        for (y = 0; y < hrnet_num_output_h; y++)
        {
            for (x = 0; x < hrnet_num_output_w; x++)
            {
                offs = hrnet_offset(b, y, x);
                if (floatarr[offs] > max_val)
                {
                    /*Update the maximum value and indices*/
                    max_val = floatarr[offs];
                    ind_x = x;
                    ind_y = y;
                }
            }
        }
        if (0 > max_val)
        {
            ind_x = -1;
            ind_y = -1;
            lowest_kpt_score = 0;
            return -1 ;
        }
        hrnet_preds.at(b).at(0) = float(ind_x);
        hrnet_preds.at(b).at(1) = float(ind_y);
        hrnet_preds.at(b).at(2) = max_val;
        offs = hrnet_offset(b, ind_y, ind_x);
        if (ind_y > 1 && ind_y < hrnet_num_output_h - 1)
        {
            if (ind_x > 1 && ind_x < hrnet_num_output_w - 1)
            {
                float diff_x = floatarr[offs + 1] - floatarr[offs - 1];
                float diff_y = floatarr[offs + hrnet_num_output_w] - floatarr[offs - hrnet_num_output_w];
                hrnet_preds.at(b).at(0) += sign(diff_x) * 0.25;
                hrnet_preds.at(b).at(1) += sign(diff_y) * 0.25;
            }
        }

        /*transform_preds*/
        scale[0] *= 200;
        scale[1] *= 200;
        /* udp (Unbiased Data Processing) = False */
        scale_x = scale[0] / (hrnet_num_output_w);
        scale_y = scale[1] / (hrnet_num_output_h);
        coords_x = hrnet_preds.at(b).at(0);
        coords_y = hrnet_preds.at(b).at(1);
        hrnet_preds.at(b).at(0) = coords_x * scale_x + center[0] - scale[0] * 0.5;
        hrnet_preds.at(b).at(1) = coords_y * scale_y + center[1] - scale[1] * 0.5;
    }
    /* Clear the score in preparation for the update. */
    lowest_kpt_score = 0;
    score = 1;
    for (i = 0; i < hrnet_num_output_c; i++)
    {
        /* Adopt the lowest score. */
        if (hrnet_preds.at(i).at(2) < score)
        {
            score = hrnet_preds.at(i).at(2);
        }
    }
    /* Update the score for display thread. */
    lowest_kpt_score = score;

    if (hrnet_th_kpt < lowest_kpt_score)
    {
        coord_convert(result, hrnet_preds);
    }

    return 0;
}
