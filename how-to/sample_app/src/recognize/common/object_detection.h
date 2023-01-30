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
* File Name    : object_detection.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef COMMON_OBJECT_DETECTION_H
#define COMMON_OBJECT_DETECTION_H

/*****************************************
* Includes
******************************************/

#include "../../includes.h"
class ObjectDetectionFunc
{
public:
   /**
    * @brief yolo_index
    * @details  Get the index of the bounding box attributes based on the input offset.
    * @param num_grid number of grid
    * @param offs  offset to access the bounding box attributesd.
    * @param channel channel to access each bounding box attribute.
    * @return int32_t index to access the bounding box attribute.
    */
    static int32_t yolo_index(uint8_t num_grid, int32_t offs, int32_t channel)
    {
        return offs + channel * num_grid * num_grid;
    }

   /**
    * @brief yolo_offset
    * @details Get the offset nuber to access the bounding box attributes
    *          To get the actual value of bounding box attributes, use yolo_index() after this function.
    * @param n output layer number [0~2].
    * @param b Number to indicate which bounding box in the region [0~2]
    * @param y Number to indicate which region [0~13]
    * @param x Number to indicate which region [0~13]
    * @param num_grids Number of grids
    * @param numBB Number of BB
    * @param numClass Number of classes
    * @return int32_t offset to access the bounding box attributes.
    */
    static int32_t yolo_offset(uint8_t n, int32_t b, int32_t y, int32_t x, const uint8_t* num_grids, int32_t numBB, int32_t numClass)
    {
        uint8_t num = num_grids[n];
        uint32_t prev_layer_num = 0;
        int32_t i = 0;

        for (i = 0; i < n; i++)
        {
            prev_layer_num += numBB * (numClass + 5) * num_grids[i] * num_grids[i];
        }
        return prev_layer_num + b * (numClass + 5) * num * num + y * num + x;
    }

/**
 * @brief print_boxes
 * @details Function to printout details of detected bounding boxes to standard output
 * @param det detected boxes details
 * @param labelList class labels
 */
    static void print_boxes(std::vector<detection>& det, vector<string> labelList)
    {
        uint32_t real_count = 0;
        float x, y, w, h;
        uint32_t i = 0;

        for (i = 0; i < det.size(); i++)
        {
            if (det[i].prob == 0)
            {
                continue;
            }
            else
            {
                real_count++;
                x = det[i].bbox.x;
                y = det[i].bbox.y;
                w = det[i].bbox.w;
                h = det[i].bbox.h;
                printf(" Bounding Box Number: %d\n", real_count);
                printf(" Bounding Box       : (X, Y, W, H) = (%.0f, %.0f, %.0f, %.0f)\n", x, y, w, h);
                string className;
                if (labelList.size() < (uint32_t)det[i].c)
                {
                    className = "unknown(class no = det[i].c)";
                }
                else
                {
                    className = labelList[det[i].c];
                }
                printf("\x1b[32;1m Detected  Class    : %s (%d)\n\x1b[0m", className.c_str(), det[i].c);
                printf("  Probability       : %5.1f %%\n", det[i].prob * 100);
            }
        }
        printf(" Bounding Box Count   : %d\n", real_count);
    }
};
#endif // !COMMON_OBJECT_DETECTION_H
