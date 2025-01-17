/***********************************************************************************************************************
* Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : class_info.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for TopFormer with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef CLASS_INFO_H
#define CLASS_INFO_H

#include "define.h"

/******************************************************************************
Macro Definitions
******************************************************************************/
extern const std::string g_ade_classes[];
extern uint8_t g_ade_palette[][3];
void class_ade_initialize();

#endif