/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : box.cpp
* Version      : 1.00
* Description  : For RZ/V2H DRP-AI Sample Application with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "box.h"

/*****************************************
* Function Name : overlap
* Description   : Function to compute the overlapped data between coordinate x with size w
* Arguments     : x1 = 1-dimensional coordinate of first line
*                 w1 = size of fist line
*                 x2 = 1-dimensional coordinate of second line
*                 w2 = size of second line
* Return value  : overlapped line size
******************************************/
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

/*****************************************
* Function Name : box_intersection
* Description   : Function to compute the area of intersection of Box a and b
* Arguments     : a = Box 1
*                 b = Box 2
* Return value  : area of intersection
******************************************/
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

/*****************************************
* Function Name : box_union
* Description   : Function to compute the area of union of Box a and b
* Arguments     : a = Box 1
*                 b = Box 2
* Return value  : area of union
******************************************/
float box_union(Box a, Box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

/*****************************************
* Function Name : box_iou
* Description   : Function to compute the Intersection over Union (IoU) of Box a and b
* Arguments     : a = Box 1
*                 b = Box 2
* Return value  : IoU
******************************************/
float box_iou(Box a, Box b)
{
    return box_intersection(a, b)/box_union(a, b);
}

/*****************************************
* Function Name : filter_boxes_nms
* Description   : Apply Non-Maximum Suppression (NMS) to get rid of overlapped rectangles.
* Arguments     : det= detected rectangles
*                 size = number of detections stored in det
*                 th_nms = threshold for nms
* Return value  : -
******************************************/
void filter_boxes_nms(std::vector<detection> &det, int32_t size, float th_nms)
{
    int32_t count = size;
    int32_t i = 0;
    int32_t j = 0;
    Box a;
    Box b;
    float b_intersection = 0;

    /** Interation for the comparision source */
    for (i = 0; i < count; i++)
    {
        /** Skip i-th process if the comparision source is already suppressed. */
        if ( 0 == det[i].prob )
        {
            continue;
        }

        /** Load the coordinate of the comparision source. */
        a = det[i].bbox;
        
        /** Interation for the comparision destination.  */
        for (j = i+1; j < count; j++)
        {
            /** Skip j-th process if the comparision destination is already suppressed */
            if ( 0 == det[j].prob )
            {
                continue;
            }

            /** Skip j-th process if the classes between the source and destination are same. */
            if (det[i].c != det[j].c)
            {
                continue;
            }

            /** Load the coordinate of the comparision source. */
            b = det[j].bbox;

            /** Suppression check with NMS algorithm. */
            b_intersection = box_intersection(a, b);
            if ((box_iou(a, b)>th_nms) || (b_intersection >= a.h * a.w - 1) || (b_intersection >= b.h * b.w - 1))
            {
                /** The one having the lesser probability will be suppressed */
                if (det[i].prob > det[j].prob)
                {
                    /** Suppress the comparision destination. */
                    det[j].prob= 0;
                }
                else
                {
                    /** Suppress the comparision source. */
                    det[i].prob= 0;

                    /** Skip the loop of comparision destination if the comparision source is suppressed. */
                    break;
                }
            }
        }
    }
    return;
}
