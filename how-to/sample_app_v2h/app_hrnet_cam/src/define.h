/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for MMPose HRNet with MIPI/USB Camera
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
#include <signal.h>
#include <vector>
#include <map>
#include <fstream>
#include <errno.h>
#include <math.h>
#include <iomanip>
#include <atomic>
#include <semaphore.h>
#include <cstring>
#include <float.h>
#include <numeric>
/*****************************************
* Macro for HRNet/YOLOX
******************************************/
/* Input Camera support */
/* n = 0: USB Camera, n = 1: eCAM22 */
#define INPUT_CAM_TYPE 0
#if INPUT_CAM_TYPE == 0
    #define CAM_INPUT_VGA
#else
    #define CAM_INPUT_FHD
#endif
/* Output Camera Size */
#define IMAGE_OUTPUT_FHD
#define MIPI_CAM_RES "1920x1080"

/*Time Measurement Flag*/
//#define DEBUG_TIME_FLG

/* Enable demonstration mode for combination with GUI Demo system */
#define END_DET_TYPE                (0)

/*Display AI frame rate*/
#undef DISP_AI_FRAME_RATE

/* Padding input mode to maintain the aspect ratio of DRP-AI input image.
   This mode requires the DRP-AI object file having the squared input size CAM_IMAGE_WIDTH x CAM_IMAGE_WIDTH.
   0: No padding, 
   1: With padding (maintains the aspect ratio) */
#define DRPAI_INPUT_PADDING (1)

/* DRP-AI memory offset for model object file*/
#define DRPAI_MEM_OFFSET            (0X38E0000)
/* YOLOX Skip Function
   0: YOLOX Running
   1: YOLOX Skip        */
#define INF_YOLOX_SKIP (1)

/*****************************************
* Static Variables for MMPose HRNet
* Indices for skeleton to tell which keypoint is connected to which
******************************************/
/* Directory name of DRP-AI Object files (DRP-AI Translator output) */
//const static std::string drpai_prefix0 = "hrnet_cam";
const static uint8_t skeleton[][2] =
{
    {15, 13},
    {13, 11},
    {16, 14},
    {14, 12},
    {11, 12},
    {5, 11},
    {6, 12},
    {5, 6},
    {5, 7},
    {6, 8},
    {7, 9},
    {8, 10},
    {1, 2},
    {0, 1},
    {0, 2},
    {1, 3},
    {2, 4},
    {3, 5},
    {4, 6}
};

/*****************************************
* Static Variables for YOLOX
* Following variables need to be changed in order to custormize the AI model
*  - drpai_prefix0 = directory name of DRP-AI Object files (DRP-AI Translator output)
******************************************/
/* Anchor box information */
const static double anchors[] =
{
    1.08,   1.19,
    3.42,   4.41,
    6.63,   11.38,
    9.42,   5.11,
    16.62,  10.52
};

/*****************************************
* Macro for MMPose HRNet pre YOLOX
******************************************/
/*Inference Related Parameters*/

/* Model Binary */
const static std::string model_dir = "hrnet_cam";
/* Pre-processing Runtime Object */
const static std::string pre_dir = model_dir + "/preprocess";

/*Cropping Image Related*/
#ifdef CAM_INPUT_VGA
#define CROPPED_IMAGE_WIDTH         (360)
#else /* CAM_INPUT_FHD */
#define CROPPED_IMAGE_WIDTH         (810)
#endif
#define CROPPED_IMAGE_HEIGHT        (CAM_IMAGE_HEIGHT)
/*HRNet Related*/
#define NUM_OUTPUT_W                (48)
#define NUM_OUTPUT_H                (64)
#define NUM_OUTPUT_C                (17)
#define INF_OUT_SIZE                (NUM_OUTPUT_W*NUM_OUTPUT_H*NUM_OUTPUT_C)
#ifdef CAM_INPUT_VGA
#define OUTPUT_LEFT                 (140)
#define OUTPUT_TOP                  (0)
#else /* CAM_INPUT_FHD */
#define OUTPUT_LEFT                 (555)
#define OUTPUT_TOP                  (0)
#endif
/*Graphic Drawing Settings Related*/
#define KEY_POINT_SIZE              (2)
#define NUM_LIMB                    (19)

/*CAMERA & ISP Settings Related*/
#ifdef CAM_INPUT_VGA
#define MIPI_WIDTH                (960)
#define IMREAD_IMAGE_WIDTH        (640)
#define IMREAD_IMAGE_HEIGHT       (480)
#else /* CAM_INPUT_FHD */
#define MIPI_WIDTH                (2240)  // 1920 + 320
#define IMREAD_IMAGE_WIDTH        (1920)
#define IMREAD_IMAGE_HEIGHT       (1080)
#endif


/*HRNet Post Processing & Drawing Related*/
#define TH_KPT                    (0.00f)
#define TH_KPT_YOLOX_SKIP         (0.10f)
#define OUTPUT_ADJ_X              (2)
#define OUTPUT_ADJ_Y              (0)
#define NUM_MAX_PERSON            (7)
#define CROP_ADJ_X                (20)
#define CROP_ADJ_Y                (20)

/* YOLOX class = person */
#define PERSON_LABEL_NUM          (14)

/*****************************************
* Macro for YoloX
******************************************/
/* Number of class to be detected */
#define NUM_CLASS                   (20)
/* Number for [region] layer num parameter */
#define NUM_BB                      (1)
/* Number of output layers. This value MUST match with the length of num_grids[] below */
#define NUM_INF_OUT_LAYER   (3)
/* Number of grids in the image. The length of this array MUST match with the NUM_INF_OUT_LAYER */
const static uint8_t num_grids[] = { 80,40,20 };
/* Number of DRP-AI output */
const static uint32_t num_inf_out =  (NUM_CLASS + 5) * NUM_BB * num_grids[0] * num_grids[0]
                                + (NUM_CLASS + 5) * NUM_BB * num_grids[1] * num_grids[1]
                                + (NUM_CLASS + 5) * NUM_BB * num_grids[2] * num_grids[2];/* Thresholds */

#define TH_PROB                     (0.5f)
#define TH_NMS                      (0.5f)
/* Size of input image to the model */
#define MODEL_IN_W                  (640)
#define MODEL_IN_H                  (640)
#define INPUT_W                    (640)
#define INPUT_H                    (640)

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
#if (1) == DRPAI_INPUT_PADDING
/*** DRP-AI input is assigned to the buffer having the size of CAM_IMAGE_WIDTH^2 */
#define DRPAI_IN_WIDTH              (CAM_IMAGE_WIDTH)
#define DRPAI_IN_HEIGHT             (CAM_IMAGE_WIDTH) 
#define DRPAI_IN_CHANNEL_YUY2       (CAM_IMAGE_CHANNEL_YUY2)
#else
/** DRP-AI input is assigned to the buffer having the size of camera image. */
#define DRPAI_IN_WIDTH              (CAM_IMAGE_WIDTH)
#define DRPAI_IN_HEIGHT             (CAM_IMAGE_HEIGHT)
#define DRPAI_IN_CHANNEL_YUY2       (CAM_IMAGE_CHANNEL_YUY2)
#endif

/*Wayland:: Wayland Information */
#ifdef IMAGE_OUTPUT_HD
#define IMAGE_OUTPUT_WIDTH          (1280)
#define IMAGE_OUTPUT_HEIGHT         (720)
#else /* IMAGE_OUTPUT_FHD */
#define IMAGE_OUTPUT_WIDTH          (1920)
#define IMAGE_OUTPUT_HEIGHT         (1080)
#endif

/*Camera image size displayed on HDMI image.*/
#ifdef CAM_INPUT_VGA
#define CAM_RESIZED_WIDTH        (CAM_IMAGE_WIDTH*2)
#define CAM_RESIZED_HEIGHT       (CAM_IMAGE_HEIGHT*2)
#else /* CAM_INPUT_FHD */
#define CAM_RESIZED_WIDTH        (IMAGE_OUTPUT_WIDTH)
#define CAM_RESIZED_HEIGHT       (IMAGE_OUTPUT_HEIGHT)
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
#define TEXT_WIDTH_OFFSET_L         (50) /*in pixel*/
#define TEXT_WIDTH_OFFSET_R         (10) /*in pixel*/
#define BOX_LINE_SIZE               (3)  /*in pixel*/
#define BOX_HEIGHT_OFFSET           (30) /*in pixel*/
#define BOX_TEXT_HEIGHT_OFFSET      (8)  /*in pixel*/
#define CHAR_SCALE_FONT             (0.8)
#define YELLOW_DATA                 (0xFF00FFu) /* in RGB */
#define BLUE_DATA                   (0x00FF00u) /* in RGB */
#define RED_DATA                    (0x0000FFu) /* in RGB */
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

/*****************************************
* ERROR CODE
******************************************/
#define SUCCESS                   (0x00)
#define ERR_ADDRESS_CMA           (0x01)
#define ERR_OPEN                  (0x10)
#define ERR_FORMAT                (0x11)
#define ERR_READ                  (0x12)
#define ERR_MMAP                  (0x13)
#define ERR_MLOCK                 (0x14)
#define ERR_MALLOC                (0x15)
#define ERR_GET_TIME              (0x16)
#define ERR_DRPAI_TIMEOUT         (0x20)
#define ERR_DRPAI_START           (0x21)
#define ERR_DRPAI_ASSIGN          (0x22)
#define ERR_DRPAI_WRITE           (0x23)
#define ERR_DRPAI_ASSIGN_PARAM    (0x24)
#define ERR_SYSTEM_ID             (-203)

#endif
