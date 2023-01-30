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
* File Name    : tvm_drpai_yolo.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once

#ifndef DRP_TVM_MODEL_YOLO_H
#define DRP_TVM_MODEL_YOLO_H

/*****************************************
* Includes
******************************************/
#include "../irecognize_model.h"
#include "../../includes.h"
#include "../common/box.h"
#include "../common/functions.h"
#include "../common/object_detection.h"
#include "../common/PreRuntime.h"
#include "../command/object_detection.h"

class TVM_YOLO_DRPAI : public IRecognizeModel
{
private:
    constexpr static string_view TVM_MODEL_DIR_YOLOV3 = "yolov3_onnx";
    constexpr static string_view TVM_MODEL_DIR_TINYYOLOV3 = "tinyyolov3_onnx";
    constexpr static string_view TVM_MODEL_DIR_YOLOV2 = "yolov2_onnx";
    constexpr static string_view TVM_MODEL_DIR_TINYYOLOV2 = "tinyyolov2_onnx";
    constexpr static  int32_t TVM_DRPAI_IN_WIDTH = (640);
    constexpr static  int32_t TVM_DRPAI_IN_HEIGHT = (480);

    /*YOLO Common*/
    constexpr static string_view MODEL_NAME = "DRP-AI TVM YOLO (DRP-AI)";
    constexpr static float  YOLO_TH_PROB = 0.5f;
    constexpr static float  YOLO_TH_NMS = 0.5f;

    /*YOLOv3 Related*/
    constexpr static  string_view LABEL_LIST = "coco-labels-2014_2017.txt";
    constexpr static  int32_t  YOLOV3_NUM_BB = 3;
    constexpr static int32_t  YOLOV3_NUM_INF_OUT_LAYER = 3;
    constexpr static int32_t  TINYYOLOV3_NUM_INF_OUT_LAYER = 2;
    /*YOLOv2 Related*/
    constexpr static  int32_t  YOLOV2_NUM_BB = 5;
    constexpr static int32_t  YOLOV2_NUM_INF_OUT_LAYER = 1;
    constexpr static int32_t  TINYYOLOV2_NUM_INF_OUT_LAYER = 1;

    /*DRP-AI Input image information*/
    constexpr static int32_t TVM_DRPAI_IN_CHANNEL = (2);
    constexpr static int32_t  TVM_MODEL_IN_C = (3);
    constexpr static int32_t  TVM_MODEL_IN_W = (416);
    constexpr static int32_t  TVM_MODEL_IN_H = (416);

public:
    TVM_YOLO_DRPAI();
    TVM_YOLO_DRPAI(uint8_t id);
    virtual int32_t inf_pre_process
        (uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size);
    virtual int32_t inf_post_process(float* arg);
    virtual shared_ptr<PredictNotifyBase> get_command();
    virtual int32_t print_result();

private:
    int8_t pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size);
    int8_t post_process(std::vector<detection>& det, float* floatarr);

private:
    /* Pre-processing Runtime variables for pre-processing */
    PreRuntime preruntime;
    s_preproc_param_t in_param;
    const std::string pre_dir = "preprocess_tvm_v2ma";
    float mean[3] = { 0.0, 0.0, 0.0 };
    float stdev[3] = { 1.0, 1.0, 1.0 };

    /* Class labels to be classified */
    /* YOLOv2/TinyYOLOv2 label list.  For YOLOv3/TinyYOLOv3, vector will be loaded from label_list later.*/
    std::vector<std::string> label_file_map =
    {
        "aeroplane",    "bicycle",
        "bird",         "boat",
        "bottle",       "bus",
        "car",          "cat",
        "chair",        "cow",
        "diningtable",  "dog",
        "horse",        "motorbike",
        "person",       "pottedplant",
        "sheep",        "sofa",
        "train",        "tvmonitor"
    };

    /* Number of grids in the image. The length of this array MUST match with the NUM_INF_OUT_LAYER */
    vector<uint8_t> num_grids;
    /* Number of DRP-AI output */
    uint32_t num_inf_out;
    /* Number of output layers */
    uint8_t num_out_layer;
    /* Anchor box information */
    vector<double> anchors;
    /* Number of detection labels */
    int32_t num_class;

    /* Post-processing result */
    vector<detection> postproc_data;

};

#endif //DRP_TVM_MODEL_YOLO_H
