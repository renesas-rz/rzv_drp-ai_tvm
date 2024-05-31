/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define_color.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for PyTorch DeepLabv3 with MIPI/USB Camera
***********************************************************************************************************************/
#ifndef DEFINE_COLOR_H
#define DEFINE_COLOR_H

/*****************************************
* color
******************************************/

#define NUM_CLASS                (22)

/*class color (RGB)*/
#define CLASS_COLOR_0    (0x000000u)
#define CLASS_COLOR_1    (0xFFFF00u)
#define CLASS_COLOR_2    (0xFF0000u)
#define CLASS_COLOR_3    (0xC0C0C0u)
#define CLASS_COLOR_4    (0xFFA07Au)
#define CLASS_COLOR_5    (0xFF1493u)
#define CLASS_COLOR_6    (0x006400u)
#define CLASS_COLOR_7    (0x00BFFFu)
#define CLASS_COLOR_8    (0xDAA520u)
#define CLASS_COLOR_9    (0xFF00FFu)
#define CLASS_COLOR_10   (0xFFC0CBu)
#define CLASS_COLOR_11   (0x008000u)
#define CLASS_COLOR_12   (0x800080u)
#define CLASS_COLOR_13   (0xFFA500u)
#define CLASS_COLOR_14   (0x1E90FFu)
#define CLASS_COLOR_15   (0x7CFC00u)
#define CLASS_COLOR_16   (0xF000F0u)
#define CLASS_COLOR_17   (0xF000FFu)
#define CLASS_COLOR_18   (0xFF00FFu)
#define CLASS_COLOR_19   (0xFF00FFu)
#define CLASS_COLOR_20   (0xFF0FFFu)
#define CLASS_COLOR_21   (0xFFF00Fu)

/*class name*/
#define CLASS_NAME_0    "background"
#define CLASS_NAME_1    "aeroplane"
#define CLASS_NAME_2    "bicycle"
#define CLASS_NAME_3    "bird"
#define CLASS_NAME_4    "boat"
#define CLASS_NAME_5    "bottle"
#define CLASS_NAME_6    "bus"
#define CLASS_NAME_7    "car"
#define CLASS_NAME_8    "cat"
#define CLASS_NAME_9    "chair"
#define CLASS_NAME_10   "cow"
#define CLASS_NAME_11   "diningtable"
#define CLASS_NAME_12   "dog"
#define CLASS_NAME_13   "horse"
#define CLASS_NAME_14   "motorbike"
#define CLASS_NAME_15   "person"
#define CLASS_NAME_16   "pottedplant"
#define CLASS_NAME_17   "sheep"
#define CLASS_NAME_18   "sofa"
#define CLASS_NAME_19   "train"
#define CLASS_NAME_20   "tvmonitor"
#define CLASS_NAME_21   "void"

#endif

