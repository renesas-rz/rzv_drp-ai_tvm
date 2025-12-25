/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : box.cpp
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for mmpose Detection YOLOX Pose with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "box.h"
#include "box_yolox_pose.h"

/*****************************************
* Function Name : filter_boxes_nms_pose
* Description   : Apply Non-Maximum Suppression (NMS) to get rid of overlapped rectangles.
* Arguments     : det = detected rectangles
*                 size = number of detections stored in det
*                 th_nms = threshold for nms
* Return value  : -
******************************************/
void filter_boxes_nms_pose(std::vector<pose_detection> &det, int32_t size, float th_nms)
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
