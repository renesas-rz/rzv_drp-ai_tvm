/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for Lightnet YOLOv2 with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef DEFINE_MACRO_H
#define DEFINE_MACRO_H

/*****************************************
* includes
******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <vector>
#include <map>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <cstring>
#include <float.h>
#include <atomic>
#include <semaphore.h>
#include <numeric>
/*****************************************
* Macro for YOLOv2
******************************************/
/* Input Camera support */
/* n = 0: USB Camera, n = 1: eCAM22 */
#define INPUT_CAM_TYPE 0

/* Output Camera Size */
#define CAM_INPUT_FHD
#define IMAGE_OUTPUT_FHD
#define MIPI_CAM_RES "1920x1080"

/*Time Measurement Flag*/
//#define DEBUG_TIME_FLG

/*Display AI frame rate*/
#undef DISP_AI_FRAME_RATE

/* DRP-AI memory offset for model object file*/
#define DRPAI_MEM_OFFSET            (0X38E0000)
/*****************************************
* Static Variables for YOLOv2
* Following variables need to be changed in order to custormize the AI model
*  - drpai_prefix0 = directory name of DRP-AI Object files (DRP-AI Translator output)
******************************************/
/* Model Binary */
const static std::string model_dir = "yolov2_cam";
/* Pre-processing Runtime Object */
const static std::string pre_dir = model_dir + "/preprocess";
/* Anchor box information */
const static double anchors[] =
{
    1.3221, 1.73145,
    3.19275, 4.00944,
    5.05587, 8.09892,
    9.47112, 4.84053,
    11.2364, 10.0071
};

/*****************************************
* Macro for YOLOv2
******************************************/
/* Number of class to be detected */
#define NUM_CLASS                   (20)
/* Number of grids in the image */
#define NUM_GRID_X                  (13)
#define NUM_GRID_Y                  (13)
/* Number for [region] layer num parameter */
#define NUM_BB                      (5)
/* Thresholds */
#define TH_PROB                     (0.5f)
#define TH_NMS                      (0.5f)
/* Size of input image to the model */
#define MODEL_IN_W                  (416)
#define MODEL_IN_H                  (416)
#define INF_OUT_SIZE                ((NUM_CLASS + 5)* NUM_BB * NUM_GRID_X * NUM_GRID_Y)


/*****************************************
* Macro for Application
******************************************/
/*Maximum DRP-AI Timeout threshold*/
#define DRPAI_TIMEOUT               (5)
/* DRP_MAX_FREQ and DRPAI_FREQ are the   */
/* frequency settings for DRP-AI.        */
/* Basically use the default values      */

#define DRP_MAX_FREQ                (2)
/* DRP_MAX_FREQ can be set from 2 to 127 */
/* 2: 420MHz                             */
/* 3: 315MHz                             */
/* ...                                   */
/* 127: 9.84MHz                          */
/* Calculation Formula:                  */
/*     1260MHz /(DRP_MAX_FREQ + 1)       */

#define DRPAI_FREQ                  (2)
/* DRPAI_FREQ can be set from 1 to 127   */
/* 1,2: 1GHz                             */
/* 3: 630MHz                             */
/* 4: 420MHz                             */
/* 5: 315MHz                             */
/* ...                                   */
/* 127: 10MHz                            */
/* Calculation Formula:                  */
/*     1260MHz /(DRPAI_FREQ - 1)         */
/*     (When DRPAI_FREQ = 3 or more.)    */

/*Camera:: Capture Image Information*/
#ifdef CAM_INPUT_VGA
#define CAM_IMAGE_WIDTH             (640)
#define CAM_IMAGE_HEIGHT            (480)
#else /* CAM_INPUT_FHD */
#define CAM_IMAGE_WIDTH             (1920)
#define CAM_IMAGE_HEIGHT            (1080)
#endif

#define CAM_IMAGE_CHANNEL_YUY2      (2)
#define CAM_IMAGE_SIZE              (CAM_IMAGE_WIDTH * CAM_IMAGE_HEIGHT * CAM_IMAGE_CHANNEL_YUY2)

/*Camera:: Capture Information */
#if INPUT_CAM_TYPE == 1
#define CAP_BUF_NUM                 (6)
#define INPUT_CAM_NAME              "MIPI Camera"
#else /* INPUT_CAM_TYPE */
#define CAP_BUF_NUM                 (3)
#define INPUT_CAM_NAME              "USB Camera"
#endif /* INPUT_CAM_TYPE */

/*DRP-AI Input image information*/
#define DRPAI_IN_WIDTH              (CAM_IMAGE_WIDTH)
#define DRPAI_IN_HEIGHT             (CAM_IMAGE_HEIGHT)
#define DRPAI_IN_CHANNEL_YUY2       (CAM_IMAGE_CHANNEL_YUY2)

/*Wayland:: Wayland Information */
#ifdef IMAGE_OUTPUT_HD
#define IMAGE_OUTPUT_WIDTH          (1280)
#define IMAGE_OUTPUT_HEIGHT         (720)
#else /* IMAGE_OUTPUT_FHD */
#define IMAGE_OUTPUT_WIDTH          (1920)
#define IMAGE_OUTPUT_HEIGHT         (1080)
#endif

/*DRP-AI Input image information*/
#ifdef CAM_INPUT_VGA
#define DRPAI_OUT_WIDTH             (960)
#define DRPAI_OUT_HEIGHT            (720)
#else
#define DRPAI_OUT_WIDTH             (IMAGE_OUTPUT_WIDTH)
#define DRPAI_OUT_HEIGHT            (IMAGE_OUTPUT_HEIGHT)
#endif

#define IMAGE_CHANNEL_BGRA          (4)
#define WL_BUF_NUM                  (2)

/*input image memory area Information*/
#define IMG_AREA_ORG_ADDRESS        (0xD0000000)    /* Note: Don't change this address */
#define IMG_AREA_CNV_ADDRESS        (0x58000000)    /* CMA area start address used by mmngr */
#define IMG_AREA_SIZE               (0x20000000)    /* CMA area size */

/*Image:: Text information to be drawn on image*/
#define CHAR_SCALE_LARGE            (0.8)
#define CHAR_SCALE_SMALL            (0.7)
#define CHAR_THICKNESS              (2)
#define LINE_HEIGHT                 (30) /*in pixel*/
#define LINE_HEIGHT_OFFSET          (20) /*in pixel*/
#define TEXT_WIDTH_OFFSET           (10) /*in pixel*/
#define BOX_LINE_SIZE               (3)  /*in pixel*/
#define BOX_HEIGHT_OFFSET           (30) /*in pixel*/
#define BOX_TEXT_HEIGHT_OFFSET      (8)  /*in pixel*/
#define CHAR_SCALE_FONT             (0.8)
#define WHITE_DATA                  (0xFFFFFFu) /* in RGB */
#define BLACK_DATA                  (0x000000u)

/*Waiting Time*/
#define WAIT_TIME                   (1000) /* microseconds */

/*Timer Related*/
#define CAPTURE_TIMEOUT             (20)  /* seconds */
#define AI_THREAD_TIMEOUT           (20)  /* seconds */
#define DISPLAY_THREAD_TIMEOUT      (20)  /* seconds */
#define KEY_THREAD_TIMEOUT          (5)   /* seconds */
#define TIME_COEF                   (1)

/*Buffer size for writing data to memory via DRP-AI Driver.*/
#define BUF_SIZE                    (1024)

/*Array size*/
#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

/*****************************************
* For image.cpp
******************************************/
/*For drawing the bounding box label on image*/
#define FONTDATA_WIDTH              (6)
#define FONTDATA_HEIGHT             (8)

#endif
