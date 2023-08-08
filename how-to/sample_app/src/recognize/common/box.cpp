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
* File Name    : box.cpp
* Version      : 1.1.1
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "box.h"


/**
 * @brief overlap
 * @details Function to compute the overlapped data between coordinate x with size w
 * @param x1 1-dimensional coordinate of first line
 * @param w1 size of fist line
 * @param x2 1-dimensional coordinate of second line
 * @param w2 size of second line
 * @return float  overlapped line size
 */
float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

/**
 * @brief box_intersection
 * @details Function to compute the area of intersection of Box a and b
 * @param a Box 1
 * @param b Box 2
 * @return float  area of intersection
 */
float box_intersection(Box a, Box b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0)
    {
        return 0;
    }
    float area = w*h;
    return area;
}

/**
 * @brief box_union
 * @details  Function to compute the area of union of Box a and b
 * @param a Box 1
 * @param b Box 2
 * @return float area of union
 */
float box_union(Box a, Box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

/**
 * @brief box_iou
 * @details Function to compute the Intersection over Union (IoU) of Box a and b
 * @param Box 1
 * @param Box 2
 * @return float IoU
 */
float box_iou(Box a, Box b)
{
    return box_intersection(a, b)/box_union(a, b);
}

/**
 * @brief filter_boxes_nms
 * @details Apply Non-Maximum Suppression (NMS) to get rid of overlapped rectangles.
 * @param det detected rectangles
 * @param size number of detections stored in det
 * @param th_nms threshold for nms
 */
void filter_boxes_nms(std::vector<detection> &det, int32_t size, float th_nms)
{
    int32_t count = size;
    int32_t i = 0;
    int32_t j = 0;
    Box a;
    Box b;
    float b_intersection = 0;
    for (i = 0; i < count; i++)
    {
        a = det[i].bbox;
        for (j = 0; j < count; j++)
        {
            if (i == j)
            {
                continue;
            }
            if (det[i].c != det[j].c)
            {
                continue;
            }
            b = det[j].bbox;
            b_intersection = box_intersection(a, b);
            if ((box_iou(a, b)>th_nms) || (b_intersection >= a.h * a.w - 1) || (b_intersection >= b.h * b.w - 1))
            {
                if (det[i].prob > det[j].prob)
                {
                    det[j].prob= 0;
                }
                else
                {
                    det[i].prob= 0;
                }
            }
        }
    }
    return;
}
