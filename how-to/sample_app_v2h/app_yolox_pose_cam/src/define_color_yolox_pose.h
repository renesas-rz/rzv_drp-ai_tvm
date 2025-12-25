/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define_color.h
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for mmpose Detection YOLOX Pose with MIPI/USB Camera
***********************************************************************************************************************/
#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H
#include <opencv2/opencv.hpp>

/*****************************************
* color
******************************************/
/* links connection list */
const static std::vector<std::pair<uint32_t, uint32_t>> links_connection = 
{ 
    {0, 1},
    {0, 2},
    {1, 3},
    {2, 4},
    {0, 5},
    {0, 6},
    {5, 7},
    {7, 9},
    {6, 8},
    {8, 10},
    {5, 6},
    {5, 11},
    {6, 12},
    {11, 12},
    {11, 13},
    {13, 15},
    {12, 14},
    {14, 16}
};

/* links color list */
const static cv::Scalar links_color[] =
{
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(254, 49, 255, 255),
    cv::Scalar(254, 49, 255, 255),
    cv::Scalar(254, 49, 255, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255)
};

/* keypoins color list */
const static cv::Scalar kpts_color[] =
{
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(0, 255, 0, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(47, 156, 252, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255),
    cv::Scalar(249, 125, 0, 255)
};

#endif