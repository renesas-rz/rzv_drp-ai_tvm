/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 2.8.0
* Description  : RZ/V2H DRP-AI Sample Application for mediapipe FaceLandmarker with MIPI/USB Camera
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
// #define DEBUG_TIME_FLG

/* Enable demonstration mode for combination with GUI Demo system */
#define END_DET_TYPE                (0)

/*Display AI frame rate*/
#undef DISP_AI_FRAME_RATE

#if(1)  // TVM
/* DRP-AI memory offset for model object file*/
#define DRPAI_MEM_OFFSET            (0X38E0000)
#endif  // TVM

/*****************************************
* Macro for FaceLandmarker
******************************************/
#if(1)  // TVM
/* Model Binary */
const static std::string model_dir = "face_landmarker_cam";
/* Pre-processing Runtime Object */
const static std::string pre_dir = model_dir + "/preprocess";
#endif  // TVM

/*****************************************
* Macro for FaceLandmarker
******************************************/
/* Number of conf size. */
#define NUM_CONF_SIZE               (1)
/* Number for [region] layer num parameter. */
#define NUM_BB                      (1)
/* Number of keypoints. */
#define NUM_KPTS                    (468)
/* Number of coordinates (x, y, z) for each keypoint. */
#define NUM_KPT_COORDS              (3)
/* Number of DRP-AI output. */
const static uint32_t num_inf_out = NUM_CONF_SIZE + NUM_BB * NUM_KPTS * NUM_KPT_COORDS;

/* Thresholds */
#define TH_PROB                     (0.5f)

// /* Size of input image to the model */
#define MODEL_IN_W                  (192)
#define MODEL_IN_H                  (192)

#ifdef CAM_INPUT_VGA
#define RADIUS                      (3)
#define LINE_THICKNESS              (2)
#else /* CAM_INPUT_FHD */
#define RADIUS                      (4)
#define LINE_THICKNESS              (3)
#endif

/*****************************************
* Macro for Application
******************************************/
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
/** DRP-AI input is assigned to the buffer having the size of camera image. */
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

/*Camera image size displayed on HDMI image.*/
#ifdef CAM_INPUT_VGA
#define CAM_RESIZED_WIDTH           (CAM_IMAGE_WIDTH*2)
#define CAM_RESIZED_HEIGHT          (CAM_IMAGE_HEIGHT*2)
#else /* CAM_INPUT_FHD */
#define CAM_RESIZED_WIDTH           (IMAGE_OUTPUT_WIDTH)
#define CAM_RESIZED_HEIGHT          (IMAGE_OUTPUT_HEIGHT)
#endif

#define IMAGE_CHANNEL_BGRA          (4)
#define WL_BUF_NUM                  (2)

/*Image:: Text information to be drawn on image*/
#define CHAR_SCALE_LARGE            (0.8)
#define CHAR_SCALE_SMALL            (0.7)
#define CHAR_THICKNESS              (2)
#define LINE_HEIGHT                 (30) /*in pixel*/
#define LINE_HEIGHT_OFFSET          (20) /*in pixel*/
#define TEXT_WIDTH_OFFSET           (10) /*in pixel*/
#ifdef CAM_INPUT_VGA
#define CHAR_THICKNESS_BOX          (1)  /*in pixel*/
#define BOX_LINE_SIZE               (2)  /*in pixel*/
#define BOX_HEIGHT_OFFSET           (15) /*in pixel*/
#define BOX_TEXT_HEIGHT_OFFSET      (5)  /*in pixel*/
#define CHAR_SCALE_FONT             (0.5)
#else
#define CHAR_THICKNESS_BOX          (2)  /*in pixel*/
#define BOX_LINE_SIZE               (3)  /*in pixel*/
#define BOX_HEIGHT_OFFSET           (30) /*in pixel*/
#define BOX_TEXT_HEIGHT_OFFSET      (8)  /*in pixel*/
#define CHAR_SCALE_FONT             (0.8)
#endif
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
#define SIZE_OF_ARRAY(array)        (sizeof(array)/sizeof(array[0]))

/*****************************************
* For image.cpp
******************************************/
/*For drawing the bounding box label on image*/
#define FONTDATA_WIDTH              (6)
#define FONTDATA_HEIGHT             (8)

#endif
