/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : box.h
* Version      : 1.00
* Description  : For RZ/V2H DRP-AI Sample Application with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef BOX_H
#define BOX_H

#include <vector>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*****************************************
* Box : Bounding box coordinates and its size
******************************************/
typedef struct
{
    float x, y, w, h;
} Box;

/*****************************************
* detection : Detected result
******************************************/
typedef struct detection
{
    Box bbox;
    int32_t c;
    float prob;
} detection;

/*****************************************
* Functions
******************************************/
float box_iou(Box a, Box b);
float overlap(float x1, float w1, float x2, float w2);
float box_intersection(Box a, Box b);
float box_union(Box a, Box b);
void filter_boxes_nms(std::vector<detection> &det, int32_t size, float th_nms);

#endif
