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
* File Name    : image_converter.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "image_converter.h"

using namespace std;

/**
 * @brief compress_jpeg_turbo
 * @details Jpeg compress func by libjpeg-turbo
 * @param input input yuyv image
 * @param width input image width
 * @param height input image height
 * @param output output jpeg image
 * @param quality jpeg compress quality
 */
void ImageConverter::compress_jpeg_turbo(uint8_t* input,
    uint32_t width,
    uint32_t height,
    vector<uint8_t>& output,
    int32_t quality)
{
#define PADDING     (1)

    tjhandle tjh;
    uint8_t* jpegBuf = NULL;
    uint64_t jpegSize;
    unsigned i;
    uint32_t h;
    uint32_t w = width * BYTE_PER_PIX;

    /* YUV(4:2:2) Buffer */
    vector<uint8_t> all_array((height * width) + ((height * width) / BYTE_PER_PIX) + ((height * width) / BYTE_PER_PIX));
    int32_t yoff = 0;
    int32_t uoff = height * width;
    int32_t voff = uoff + ((height * width) / BYTE_PER_PIX);
    uint8_t* p_input = 0;

    tjh = tjInitCompress();
    if (NULL == tjh) {
        printf("tjInitCompress error '%s'\n", tjGetErrorStr());
        return;
    }

    {
        Measuretime mm("YUV extract time");
        for (h = 0; h < height; h++) {

            /* align to 4:2:2
             YYYYYYYYY
             YYYYYYYYY
             .....
             UUUUUUU
             VVVVVVV */
            p_input = &input[h*w];
            for (i = 0; i < w; i += 4)
            {
                all_array[yoff++] = *(p_input++);
                all_array[uoff++] = *(p_input++);
                all_array[yoff++] = *(p_input++);
                all_array[voff++] = *(p_input++);
            }
        }
    }

    {
        Measuretime mm("tuebo jpeg comps time");
        if (tjCompressFromYUV(tjh, all_array.data(), width, PADDING, height,
            TJSAMP_422, &jpegBuf, &jpegSize, quality, 0))
        {
            printf("tjCompressFromYUV error '%s'\n", tjGetErrorStr());
        }
    }

    output = vector<uint8_t>(jpegBuf, jpegBuf + jpegSize);
    tjFree(jpegBuf);
    tjDestroy(tjh);
}
