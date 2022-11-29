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
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : tvm_drpai_hrnet.cpp
* Version      : 1.0.2
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_hrnet.h"
TVM_HRNET_DRPAI::TVM_HRNET_DRPAI() :
    IRecognizeModel(NUM_INF_OUT*sizeof(float),
        TVM_MODEL_DIR.data(), MODEL_NAME.data(),
        TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL,
        TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, MODE_TVM_HRNET_DRPAI)
{
    preruntime.Load(pre_dir);
    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = PRE_CROPPED_IMAGE_WIDTH;
    in_param.pre_in_shape_h = PRE_CROPPED_IMAGE_HEIGHT;
    in_param.pre_in_format = INPUT_YUYV;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
    in_param.cof_add[0]= -255*mean[0];//-123.675;
    in_param.cof_add[1]= -255*mean[1];//-116.28;
    in_param.cof_add[2]= -255*mean[2];//-103.53;
    in_param.cof_mul[0]= 1/(stdev[0]*255);//0.01712475;
    in_param.cof_mul[1]= 1/(stdev[1]*255);//0.017507;
    in_param.cof_mul[2]= 1/(stdev[2]*255);//0.01742919;

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
    size = PRE_CROPPED_IMAGE_WIDTH * PRE_CROPPED_IMAGE_HEIGHT * TVM_DRPAI_IN_CHANNEL; 

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
 * @brief inf_pre_process_drpai
 * @details Run pre-processing using Pre-processing Runtime (DRP-AI)
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI::inf_pre_process_drpai(uint32_t addr, float** arg, uint32_t* buf_size)
{
    pre_process_drpai(addr, arg, buf_size);
    return 0;
}
#ifdef TENTATIVE
/**
 * @brief inf_pre_process_hrnet
 * @details Run pre-processing using Pre-processing Runtime (DRP-AI) and CPU. Will be deleted in the future.
 * @param input_data Input data pointer
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI::inf_pre_process_hrnet(uint8_t* input_data, uint32_t addr, float** arg, uint32_t* buf_size)
{
    pre_process_hrnet(input_data, addr, arg, buf_size);
    return 0;
}
#endif
/**
 * @brief inf_pre_process_cpu
 * @details Run pre-processing using CPU
 * @param input_data Input data pointer
 * @param out output_buf Output data buffer pointer holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_HRNET_DRPAI:: inf_pre_process_cpu(uint8_t* input_data, float** output_buf)
{
    /*Do nothing*/
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
* @param  b = Number to indicate which region [0~17]
* @param  y = Number to indicate which region [0~64]
* @param  x = Number to indicate which region [0~48]
* @return int32_t offset to access the HRNet attributes.
*/
int32_t TVM_HRNET_DRPAI::hrnet_offset(int32_t b, int32_t y, int32_t x)
{
    return b * HRNET_NUM_OUTPUT_W * HRNET_NUM_OUTPUT_H + y * HRNET_NUM_OUTPUT_W + x;
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
 * @brief pre_process_drpai
 * @details implementation pre process using Pre-processing Runtime.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_HRNET_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, output_buf, buf_size);
    return 0;
}

int8_t TVM_HRNET_DRPAI::pre_process_cpu(uint8_t* input_data, float** output_buf)
{
    return 0;
}

#ifdef TENTATIVE
/**
 * @brief pre_process_hrnet
 * @details implementation pre process using Pre-processing Runtime and CPU.
 * @param input_data Input data pointer
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_HRNET_DRPAI::pre_process_hrnet(uint8_t* input_data, uint32_t addr, float** arg, uint32_t* buf_size)
{
    uint8_t err_crop = 0;
    uint32_t x;
    uint32_t y;
    uint32_t top    = PRE_CROPPED_IMAGE_TOP;
    uint32_t bottom = top + PRE_CROPPED_IMAGE_HEIGHT;
    uint32_t left   = PRE_CROPPED_IMAGE_LEFT / 2;
    uint32_t right  = left + PRE_CROPPED_IMAGE_WIDTH / 2;
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

    in_param.pre_in_addr = (uintptr_t) udmabuf_crop_addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, arg, buf_size);
    return 0;
}
#endif
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
void TVM_HRNET_DRPAI::coord_convert(vector<pos_t> &result, float preds[][3])
{
    /* Render skeleton on image and print their details */
    int32_t posx = 0;
    int32_t posy = 0;
    int8_t i = 0;
    result.clear();
    for (i = 0; i < HRNET_NUM_OUTPUT_C; i++)
    {
        /* 0.5 is round */
        posx = (int32_t)(preds[i][0] / HRNET_CROPPED_IMAGE_WIDTH * HRNET_OUTPUT_WIDTH + 0.5) + HRNET_OUTPUT_LEFT + HRNET_OUTPUT_ADJ_X;
        posy = (int32_t)(preds[i][1] / HRNET_CROPPED_IMAGE_HEIGHT * HRNET_OUTPUT_HEIGHT + 0.5) + HRNET_OUTPUT_TOP + HRNET_OUTPUT_ADJ_Y;

        pos_t p;
        p.X = posx;
        p.Y = posy;
        p.preds = preds[i][2] * 100;
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
    float hrnet_preds[HRNET_NUM_OUTPUT_C][3];

    for (b = 0; b < HRNET_NUM_OUTPUT_C; b++)
    {
        float scale[] = { HRNET_CROPPED_IMAGE_WIDTH / 200.0, HRNET_CROPPED_IMAGE_HEIGHT / 200.0 };
        ind_x = -1;
        ind_y = -1;
        max_val = -1;
        for (y = 0; y < HRNET_NUM_OUTPUT_H; y++)
        {
            for (x = 0; x < HRNET_NUM_OUTPUT_W; x++)
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
        hrnet_preds[b][0] = float(ind_x);
        hrnet_preds[b][1] = float(ind_y);
        hrnet_preds[b][2] = max_val;
        offs = hrnet_offset(b, ind_y, ind_x);
        if (ind_y > 1 && ind_y < HRNET_NUM_OUTPUT_H - 1)
        {
            if (ind_x > 1 && ind_x < HRNET_NUM_OUTPUT_W - 1)
            {
                float diff_x = floatarr[offs + 1] - floatarr[offs - 1];
                float diff_y = floatarr[offs + HRNET_NUM_OUTPUT_W] - floatarr[offs - HRNET_NUM_OUTPUT_W];
                hrnet_preds[b][0] += sign(diff_x) * 0.25;
                hrnet_preds[b][1] += sign(diff_y) * 0.25;
            }
        }

        /*transform_preds*/
        scale[0] *= 200;
        scale[1] *= 200;
        /* udp (Unbiased Data Processing) = False */
        scale_x = scale[0] / (HRNET_NUM_OUTPUT_W);
        scale_y = scale[1] / (HRNET_NUM_OUTPUT_H);
        coords_x = hrnet_preds[b][0];
        coords_y = hrnet_preds[b][1];
        hrnet_preds[b][0] = coords_x * scale_x + center[0] - scale[0] * 0.5;
        hrnet_preds[b][1] = coords_y * scale_y + center[1] - scale[1] * 0.5;
    }
    /* Clear the score in preparation for the update. */
    lowest_kpt_score = 0;
    score = 1;
    for (i = 0; i < HRNET_NUM_OUTPUT_C; i++)
    {
        /* Adopt the lowest score. */
        if (hrnet_preds[i][2] < score)
        {
            score = hrnet_preds[i][2];
        }
    }
    /* Update the score for display thread. */
    lowest_kpt_score = score;

    if (HRNET_TH_KPT < lowest_kpt_score)
    {
        coord_convert(result, hrnet_preds);
    }

    return 0;
}
