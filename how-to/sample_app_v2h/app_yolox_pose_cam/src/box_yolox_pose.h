/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : box.h
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for mmpose Detection YOLOX Pose with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef BOX_YOLOX_S_POSE_H
#define BOX_YOLOX_S_POSE_H

#include "box.h"
using namespace std;

/*****************************************
* Kpts : Keypoints coordinates and its size
******************************************/
typedef struct Kpts
{
    float c, x, y;
} Kpts;

/*****************************************
* pose_detection : Detected result
******************************************/
typedef struct pose_detection
{
    Box bbox;
    vector<Kpts> kpts;
    float prob;
} pose_detection;

/*****************************************
* Functions
******************************************/
void filter_boxes_nms_pose(std::vector<pose_detection> &det, int32_t size, float th_nms);

#endif
