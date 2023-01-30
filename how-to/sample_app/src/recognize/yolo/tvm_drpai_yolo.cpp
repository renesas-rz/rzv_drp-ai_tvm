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
* File Name    : tvm_drpai_yolo.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "tvm_drpai_yolo.h"
TVM_YOLO_DRPAI::TVM_YOLO_DRPAI(uint8_t id) : IRecognizeModel(0, TVM_MODEL_DIR_YOLOV3.data(), MODEL_NAME.data(), TVM_DRPAI_IN_WIDTH, TVM_DRPAI_IN_HEIGHT, TVM_DRPAI_IN_CHANNEL, TVM_MODEL_IN_W, TVM_MODEL_IN_H, TVM_MODEL_IN_C, id)
{
    preruntime.Load(pre_dir);
    /*Define pre-processing parameter*/
    in_param.pre_in_shape_w = TVM_DRPAI_IN_WIDTH;
    in_param.pre_in_shape_h = TVM_DRPAI_IN_HEIGHT;
    in_param.pre_in_format = INPUT_YUYV;
    in_param.resize_w = TVM_MODEL_IN_W;
    in_param.resize_h = TVM_MODEL_IN_H;
    in_param.resize_alg = ALG_BILINEAR;
    /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
    in_param.cof_add[0]= -255*mean[0];
    in_param.cof_add[1]= -255*mean[1];
    in_param.cof_add[2]= -255*mean[2];
    in_param.cof_mul[0]= 1/(stdev[0]*255);
    in_param.cof_mul[1]= 1/(stdev[1]*255);
    in_param.cof_mul[2]= 1/(stdev[2]*255);

    /*Load label list for YOLOv3/TinyYOLOv3 */
    if (id == MODE_TVM_YOLOV3_DRPAI || id == MODE_TVM_TINYYOLOV3_DRPAI )
    {
        label_file_map = CommonFunc::load_label_file(LABEL_LIST.data());
    }
    num_class = label_file_map.size();

    if (id == MODE_TVM_YOLOV3_DRPAI)
    {
        model_dir = TVM_MODEL_DIR_YOLOV3;
        std::cout << "DRP-AI TVM YOLOv3 model" << std::endl;
        /* init num_grids*/
        num_grids = { 13,26,52 };
        /*init num_inf_out*/
        num_inf_out = (num_class + 5) * YOLOV3_NUM_BB * num_grids[0] * num_grids[0]
            + (num_class + 5) * YOLOV3_NUM_BB * num_grids[1] * num_grids[1]
            + (num_class + 5) * YOLOV3_NUM_BB * num_grids[2] * num_grids[2];
        num_out_layer = YOLOV3_NUM_INF_OUT_LAYER;
        /* init anchors*/
        anchors =
        {
            10, 13,
            16, 30,
            33, 23,
            30, 61,
            62, 45,
            59, 119,
            116, 90,
            156, 198,
            373, 326
        };
    }
    else if (id == MODE_TVM_TINYYOLOV3_DRPAI)
    {
        model_dir = TVM_MODEL_DIR_TINYYOLOV3;
        std::cout << "DRP-AI TVM TinyYOLOv3 model" << std::endl;
        /* init num_grids*/
        num_grids = { 13,26 };
        /*init num_inf_out*/
        num_inf_out = (num_class + 5) * YOLOV3_NUM_BB * num_grids[0] * num_grids[0]
            + (num_class + 5) * YOLOV3_NUM_BB * num_grids[1] * num_grids[1];
        num_out_layer = TINYYOLOV3_NUM_INF_OUT_LAYER;
        /* init anchors*/
        anchors =
        {
            10, 14,
            23, 27,
            37, 58,
            81, 82,
            135, 169,
            344, 319
        };
    }
    else if (id == MODE_TVM_YOLOV2_DRPAI)
    {
        model_dir = TVM_MODEL_DIR_YOLOV2;
        std::cout << "DRP-AI TVM YOLOv2 model" << std::endl;
        /* init anchors*/
        anchors =
        {
            1.3221, 1.73145,
            3.19275, 4.00944,
            5.05587, 8.09892,
            9.47112, 4.84053,
            11.2364, 10.0071
        };
        /* init num_grids*/
        num_grids = { 13 };
         /*init num_inf_out*/
        num_inf_out = (num_class + 5) * YOLOV2_NUM_BB * num_grids[0] * num_grids[0];
        num_out_layer = YOLOV2_NUM_INF_OUT_LAYER;
    }
    else if (id == MODE_TVM_TINYYOLOV2_DRPAI)
    {
        model_dir = TVM_MODEL_DIR_TINYYOLOV2;
        std::cout << "DRP-AI TVM TinyYOLOv2 model" << std::endl;
        /* init anchors*/
        anchors =
        {
            1.08,   1.19,
            3.42,   4.41,
            6.63,   11.38,
            9.42,   5.11,
            16.62,  10.52
        };
        /* init num_grids*/
        num_grids = { 13 };
         /*init num_inf_out*/
        num_inf_out = (num_class + 5) * YOLOV2_NUM_BB * num_grids[0] * num_grids[0];
        num_out_layer = TINYYOLOV2_NUM_INF_OUT_LAYER;
    }

    outBuffSize = num_inf_out;
}
/**
 * @brief inf_pre_process
 * @details Run pre-processing.
 * @details For CPU input, use input_data for input data.
 * @details For DRP-AI input, use addr for input data stored address
 * @param input_data Input data pointer
 * @param width new input data width.
 * @param height new input data width.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_YOLO_DRPAI:: inf_pre_process(uint8_t* input_data, uint32_t width, uint32_t height,  uint32_t addr, float** arg, uint32_t* buf_size)
{
    /*Update width and height*/
    if ((width != _capture_w) || (height != _capture_h)) 
    {
        _capture_w = width;
        _capture_h = height;
        in_param.pre_in_shape_w = _capture_w;
        in_param.pre_in_shape_h = _capture_h;
    }

    pre_process_drpai(addr, arg, buf_size);
    return 0;
}
/**
 * @brief inf_post_process
 * @details Run post-processing
 * @param arg Inference output data pointer
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_YOLO_DRPAI::inf_post_process(float* arg)
{
    postproc_data.clear();
    post_process(postproc_data, arg);
    return 0;

}
/**
 * @brief print_result
 * @details Print AI result on console
 * @return int32_t success:0 error: != 0
 */
int32_t TVM_YOLO_DRPAI::print_result()
{
    ObjectDetectionFunc::print_boxes(postproc_data, label_file_map);
    return 0;
}
/**
 * @brief get_command
 * @details Prepare the command to send via HTTP
 * @return shared_ptr<PredictNotifyBase> Result data
 */
shared_ptr<PredictNotifyBase> TVM_YOLO_DRPAI::get_command()
{
    ObjectDetection* ret = new ObjectDetection();
    for (detection det : postproc_data)
    {
        if (det.prob == 0)
        {
            continue;
        }
        else
        {
            bbox_t dat;
            dat.name = label_file_map[det.c].c_str();
            dat.X = (int32_t)(det.bbox.x - (det.bbox.w / 2));
            dat.Y = (int32_t)(det.bbox.y - (det.bbox.h / 2));
            dat.W = (int32_t)det.bbox.w;
            dat.H = (int32_t)det.bbox.h;
            dat.pred = det.prob * 100.0;
            ret->predict.push_back(dat);
        }
    }
    return shared_ptr<PredictNotifyBase>(move(ret));
}
/**
 * @brief pre_process_drpai
 * @details implementation pre process using Pre-processing Runtime.
 * @param addr Physical address of input data buffer
 * @param out output_buf Output data buffer pointer holder
 * @param out buf_size Output data buffer size holder
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_YOLO_DRPAI::pre_process_drpai(uint32_t addr, float** output_buf, uint32_t* buf_size)
{
    in_param.pre_in_addr = (uintptr_t) addr;
    /*Run pre-processing*/
    preruntime.Pre(&in_param, output_buf, buf_size);
    return 0;
}
/**
 * @brief post_process
 * @details implementation post process
 * @param det reference to store bounding box
 * @param floatarr DRP-AI result
 * @return int8_t success:0 error: != 0
 */
int8_t TVM_YOLO_DRPAI::post_process(std::vector<detection>& det, float* floatarr)
{
    /* Following variables are required for correct_yolo/region_boxes in Darknet implementation*/
    /* Note: This implementation refers to the "darknet detector test" */
    float new_w, new_h;
    float correct_w = 1.;
    float correct_h = 1.;
    if ((float)(TVM_MODEL_IN_W / correct_w) < (float)(TVM_MODEL_IN_H / correct_h))
    {
        new_w = (float)TVM_MODEL_IN_W;
        new_h = correct_h * TVM_MODEL_IN_W / correct_w;
    }
    else
    {
        new_w = correct_w * TVM_MODEL_IN_H / correct_h;
        new_h = TVM_MODEL_IN_H;
    }
    int32_t n = 0;
    int32_t b = 0;
    int32_t y = 0;
    int32_t x = 0;
    int32_t offs = 0;
    int32_t i = 0;
    float tx = 0;
    float ty = 0;
    float tw = 0;
    float th = 0;
    float tc = 0;
    float center_x = 0;
    float center_y = 0;
    float box_w = 0;
    float box_h = 0;
    float objectness = 0;
    uint8_t num_grid = 0;
    uint8_t anchor_offset = 0;
    float classes[num_class];
    float max_pred = 0;
    int32_t pred_class = -1;
    float probability = 0;
    detection d;
    /*Number of bounding box for each grid. Default is YOLOv3 values.*/
    uint8_t num_bb = YOLOV3_NUM_BB;
    /*If YOLOv2/TinyYOLOv2, update the number of bounding box for each grid. */
    if (_id == MODE_TVM_YOLOV2_DRPAI ||_id == MODE_TVM_TINYYOLOV2_DRPAI )
    {
        num_bb = YOLOV2_NUM_BB;
    }
    for (n = 0; n < num_out_layer; n++)
    {
        num_grid = num_grids[n];
        anchor_offset = 2 * num_bb * (num_out_layer - (n + 1));
        for (b = 0; b < num_bb; b++)
        {
            for (y = 0; y < num_grid; y++)
            {
                for (x = 0; x < num_grid; x++)
                {
                    offs = ObjectDetectionFunc::yolo_offset(n, b, y, x, num_grids.data(), num_bb, label_file_map.size());
                    tx = floatarr[offs];
                    ty = floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 1)];
                    tw = floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 2)];
                    th = floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 3)];
                    tc = floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 4)];

                    /* Compute the bounding box */
                    /*get_yolo_box/get_region_box in paper implementation*/
                    center_x = ((float)x + CommonFunc::sigmoid(tx)) / (float)num_grid;
                    center_y = ((float)y + CommonFunc::sigmoid(ty)) / (float)num_grid;
                    if (_id == MODE_TVM_YOLOV3_DRPAI ||_id == MODE_TVM_TINYYOLOV3_DRPAI )
                    {
                        box_w = (float) exp(tw) * anchors[anchor_offset + 2 * b + 0] / (float)TVM_MODEL_IN_W;
                        box_h = (float) exp(th) * anchors[anchor_offset + 2 * b + 1] / (float)TVM_MODEL_IN_W;
                    }
                    else // For YOLOv2/TinyYOLOv2
                    {
                        box_w = (float) exp(tw) * anchors[anchor_offset + 2 * b + 0] / (float) num_grid;
                        box_h = (float) exp(th) * anchors[anchor_offset + 2 * b + 1] / (float) num_grid;
                    }

                    /* Adjustment for VGA size */
                    /* correct_yolo/region_boxes */
                    center_x = (center_x - (TVM_MODEL_IN_W - new_w) / 2. / TVM_MODEL_IN_W) / ((float)new_w / TVM_MODEL_IN_W);
                    center_y = (center_y - (TVM_MODEL_IN_H - new_h) / 2. / TVM_MODEL_IN_H) / ((float)new_h / TVM_MODEL_IN_H);
                    box_w *= (float)(TVM_MODEL_IN_W / new_w);
                    box_h *= (float)(TVM_MODEL_IN_H / new_h);

                    center_x = round(center_x * TVM_DRPAI_IN_WIDTH);
                    center_y = round(center_y * TVM_DRPAI_IN_HEIGHT);
                    box_w = round(box_w * TVM_DRPAI_IN_WIDTH);
                    box_h = round(box_h * TVM_DRPAI_IN_HEIGHT);

                    objectness = CommonFunc::sigmoid(tc);

                    Box bb = { center_x, center_y, box_w, box_h };
                    /* Get the class prediction */
                    {
                        for (i = 0; i < num_class; i++)
                        {
                            if (_id == MODE_TVM_YOLOV3_DRPAI ||_id == MODE_TVM_TINYYOLOV3_DRPAI )
                            {
                                classes[i] = CommonFunc::sigmoid(floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 5 + i)]);
                            }
                            else // For YOLOv2/TinyYOLOv2
                            {
                                classes[i] = floatarr[ObjectDetectionFunc::yolo_index(num_grid, offs, 5 + i)];
                            }
                        }
                    }
                    if (_id == MODE_TVM_YOLOV2_DRPAI ||_id == MODE_TVM_TINYYOLOV2_DRPAI )
                    {
                        CommonFunc::softmax(classes, num_class);
                    }

                    max_pred = 0;
                    pred_class = -1;
                    for (i = 0; i < num_class; i++)
                    {
                        if (classes[i] > max_pred)
                        {
                            pred_class = i;
                            max_pred = classes[i];
                        }
                    }

                    /* Store the result into the list if the probability is more than the threshold */
                    probability = max_pred * objectness;
                    if (probability > YOLO_TH_PROB)
                    {
                        d = { bb, pred_class, probability };
                        det.push_back(d);
                    }
                }
            }
        }
    }
    /* Non-Maximum Supression filter */
    filter_boxes_nms(det, det.size(), YOLO_TH_NMS);
    return 0;
}
