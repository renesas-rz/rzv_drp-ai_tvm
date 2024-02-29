/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define_color.h
* Version      : 0.90
* Description  : RZ/V2H DRP-AI Sample Application for Megvii-Base Detection YOLOX with MIPI/USB Camera
***********************************************************************************************************************/
#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H

/*****************************************
* color
******************************************/
/* Pascal VOC dataset label list */
const static std::vector<std::string> label_file_map = 
{ 
    "aeroplane",
    "bicycle",
    "bird",
    "boat",
    "bottle",
    "bus",
    "car",
    "cat",
    "chair",
    "cow",
    "diningtable",
    "dog",
    "horse",
    "motorbike",
    "person",
    "pottedplant", 
    "sheep",
    "sofa",
    "train",
    "tvmonitor" 
};

/* box color list */
const static unsigned int box_color[] =
{
    (0xFFFF00u),
    (0xFF0000u),
    (0xC0C0C0u),
    (0xFFA07Au),
    (0xFF1493u),
    (0x006400u),
    (0x00BFFFu),
    (0xDAA520u),
    (0xFF00FFu),
    (0xFFC0CBu),
    (0x008000u),
    (0x800080u),
    (0xFFA500u),
    (0x1E90FFu),
    (0x7CFC00u),
    (0xF000F0u),
    (0xF000FFu),
    (0xFF00FFu),
    (0xFF00FFu),
    (0xFF0FFFu)
};


#endif

