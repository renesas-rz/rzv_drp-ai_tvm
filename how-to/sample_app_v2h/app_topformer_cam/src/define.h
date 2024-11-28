/***********************************************************************************************************************
* Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for TopFormer with MIPI/USB Camera
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
#include <numeric>

/*****************************************
* Macro for TopFormer
******************************************/
/* Input Camera support */
/* n = 0: USB Camera, n = 1: eCAM22 */
#define INPUT_CAM_TYPE 0

/* Output Camera Size */
#define CAM_INPUT_VGA
#define IMAGE_OUTPUT_FHD

/*Time Measurement Flag*/
//#define DEBUG_TIME_FLG

/* Enable demonstration mode for combination with GUI Demo system */
#define END_DET_TYPE                (0)

/*Display AI frame rate*/
#undef DISP_AI_FRAME_RATE

/* DRP-AI memory offset for model object file*/
#define DRPAI_MEM_OFFSET            (0X38E0000)

/*Inference Related Parameters*/
/* Model Binary */
const static std::string model_dir = "topformer_cam";
/* Pre-processing Runtime Object */
const static std::string pre_dir = model_dir + "/preprocess";
#define IMAGE_IO_SIZE_TYPE          (0)

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

/* TVM_NUM_THREADS is a environment variable which specifies the number of threads for TVM.           */
/* If it is not set, this application set the TVM_NUM_THREADS to TVM_NUM_THREADS_DEFAULT macro value. */
#define TVM_NUM_THREADS_DEFAULT     (2)

/*Camera:: Capture Image Information*/
#ifdef CAM_INPUT_VGA
#define CAM_IMAGE_WIDTH             (640)
#define CAM_IMAGE_HEIGHT            (480)
#define MIPI_CAM_RES "640x480"
#else /* CAM_INPUT_FHD */
#define CAM_IMAGE_WIDTH             (1920)
#define CAM_IMAGE_HEIGHT            (1080)
#define MIPI_CAM_RES "1920x1080"
#endif

#define CAM_IMAGE_CHANNEL_YUY2      (2)
#define CAM_IMAGE_SIZE              (CAM_IMAGE_WIDTH * CAM_IMAGE_HEIGHT * CAM_IMAGE_CHANNEL_YUY2)

/*TVM: information*/
#define TVM_MODEL_IN_W              (512)
#define TVM_MODEL_IN_H              (512)
#define TVM_MODEL_OUT_NUM           (1)
#define TVM_MODEL_OUT_W             (64)
#define TVM_MODEL_OUT_H             (64)
#define NUM_CLASS                   (150)

/*Camera:: Capture Information */
#if INPUT_CAM_TYPE == 1
#define CAP_BUF_NUM                 (6)
#define INPUT_CAM_NAME              "MIPI Camera"
#else /* INPUT_CAM_TYPE */
#define CAP_BUF_NUM                 (3)
#define INPUT_CAM_NAME              "USB Camera"
#endif /* INPUT_CAM_TYPE */

/*Wayland:: Wayland Information */
#ifdef IMAGE_OUTPUT_HD
#define IMAGE_OUTPUT_WIDTH          (1280)
#define IMAGE_OUTPUT_HEIGHT         (720)
#else /* IMAGE_OUTPUT_FHD */
#define IMAGE_OUTPUT_WIDTH          (1920)
#define IMAGE_OUTPUT_HEIGHT         (1080)
#endif

/*Camera image size displayed on HDMI image. */
#if defined(CAM_INPUT_VGA) && defined(IMAGE_OUTPUT_HD)     
#define CAM_RESIZED_WIDTH        (CAM_IMAGE_WIDTH)
#define CAM_RESIZED_HEIGHT       (CAM_IMAGE_HEIGHT)
#define CAM_RESIZED_PADDING      (1)
#elif defined(CAM_INPUT_VGA) // && IMAGE_OUTPUT_FHD
#define CAM_RESIZED_WIDTH        (CAM_IMAGE_WIDTH*2)
#define CAM_RESIZED_HEIGHT       (CAM_IMAGE_HEIGHT*2)
#define CAM_RESIZED_PADDING      (1)
#else // CAM_INPUT_FHD && (IMAGE_OUTPUT_FHD || IMAGE_OUTPUT_HD)
#define CAM_RESIZED_WIDTH        (IMAGE_OUTPUT_WIDTH)
#define CAM_RESIZED_HEIGHT       (IMAGE_OUTPUT_HEIGHT)
#define CAM_RESIZED_PADDING      (0)
#endif

#define IMAGE_CHANNEL_BGRA          (4)
#define WL_BUF_NUM                  (2)

/*input image memory area Information*/
#define IMG_AREA_ORG_ADDRESS        (0xD0000000)    /* Note: Don't change this address */
#define IMG_AREA_CNV_ADDRESS        (0x58000000)    /* CMA area start address used by mmngr */
#define IMG_AREA_SIZE               (0x20000000)    /* CMA area size */

/*Image:: Text information to be drawn on image*/
#define CHAR_SCALE_LARGE            (0.8)
#define CHAR_SCALE_SMALL            (0.6)
#define CHAR_THICKNESS              (2)
#define LINE_HEIGHT                 (30) /*in pixel*/
#define LINE_HEIGHT_OFFSET          (20) /*in pixel*/
#define TEXT_WIDTH_OFFSET           (10) /*in pixel*/
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

#endif
