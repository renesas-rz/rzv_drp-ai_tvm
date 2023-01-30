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
* File Name    : recognize_define.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#ifndef RECOGNIE_DEFINE_H
#define RECOGNIE_DEFINE_H

#define JPEG_QUALUTY (50)           /* JPEG 0 - 100 */
#define INFERENE_ON                 /* Enable inference.*/
#define SEND_INFERENCE_RESULT_ON    /* Enable send inference image*/
#define SEND_CAMERA_ON              /* Enable send through image */
// #define COUT_INFERENCE_RESULT_ON    /* Enable print infference result to console */
#define MODEL_VGA                   /* ON=VGA Capture & Inference. OFF=FHD Capture & Inference */
//#define SEQUENCTCIAL                /* Enable sync capture and inference*/
#define TENTATIVE                   /*Will be deleted in the future */

/*Define Mode*/
#define MODE_TVM_UNKNOWN             (0b00000000)
/*For DRP-AI TVM, value must be more than or equal to 0b10000000(128) */
#define MODE_TVM_MIN                (0b10000000)
/*For DRP-AI TVM DRP-AI mode, LSB must be 0. */
/*For DRP-AI TVM CPU mode, LSB must be 1. */
#define MODE_TVM_DEEPPOSE_DRPAI     (0b10000000)
#define MODE_TVM_DEEPPOSE_CPU       (0b10000001)
#define MODE_TVM_YOLOV3_DRPAI       (0b10000010)
#define MODE_TVM_YOLOV3_CPU         (0b10000011)
#define MODE_TVM_TINYYOLOV3_DRPAI   (0b10000100)
#define MODE_TVM_TINYYOLOV3_CPU     (0b10000101)
#define MODE_TVM_YOLOV2_DRPAI       (0b10000110)
#define MODE_TVM_YOLOV2_CPU         (0b10000111)
#define MODE_TVM_TINYYOLOV2_DRPAI   (0b10001000)
#define MODE_TVM_TINYYOLOV2_CPU     (0b10001001)
#define MODE_TVM_HRNET_DRPAI        (0b10001010)
#define MODE_TVM_HRNET_CPU          (0b10001011)
#define MODE_TVM_ULTRAFACE_DRPAI    (0b10001100)
#define MODE_TVM_ULTRAFACE_CPU      (0b10001101)
#define MODE_TVM_HRNETV2_DRPAI      (0b10001110)
#define MODE_TVM_HRNETV2_CPU        (0b10001111)
#define MODE_TVM_GOOGLENET_DRPAI    (0b10010000)
#define MODE_TVM_GOOGLENET_CPU      (0b10010001)
#define MODE_TVM_EMOTIONFP_DRPAI    (0b10010010)
#define MODE_TVM_EMOTIONFP_CPU      (0b10010011)

#endif // !RECOGNIE_DEFINE_H
