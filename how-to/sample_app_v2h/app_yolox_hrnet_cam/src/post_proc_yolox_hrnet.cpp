/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : post_proc.cpp
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for Megvii-Base Detection YOLOX and MMPose HRNet with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "post_proc_yolox_hrnet.h"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <mutex>

using namespace std;

PostProc::PostProc()
{

}

PostProc::~PostProc()
{

}

static mutex mtx;

/*****************************************
* Function Name : sigmoid
* Description   : Helper function for YOLO Post Processing
* Arguments     : x = input argument for the calculation
* Return value  : sigmoid result of input x
******************************************/
#if (0) == INF_YOLOX_SKIP
double PostProc::sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}
#endif

/*****************************************
* Function Name : softmax
* Description   : Helper function for YOLO Post Processing
* Arguments     : val[] = array to be computed softmax
* Return value  : -
******************************************/
#if (0) == INF_YOLOX_SKIP
void PostProc::softmax(float val[NUM_CLASS])
{
    float max_num = -FLT_MAX;
    float sum = 0;
    int32_t i;
    for (i = 0; i<NUM_CLASS; i++)
    {
        max_num = max(max_num, val[i]);
    }

    for (i = 0; i < NUM_CLASS; i++)
    {
        val[i] = (float) exp(val[i] - max_num);
        sum += val[i];
    }

    for (i = 0; i < NUM_CLASS; i++)
    {
        val[i] = val[i] / sum;
    }
    return;
}
#endif

/*****************************************
* Function Name : index
* Description   : Get the index of the bounding box attributes based on the input offset.
* Arguments     : n = output layer number.
*                 offs = offset to access the bounding box attributesd.
*                 channel = channel to access each bounding box attribute.
* Return value  : index to access the bounding box attribute.
******************************************/
int32_t PostProc::index(uint8_t n, int32_t offs, int32_t channel)
{
    uint8_t num_grid = num_grids[n];
    return offs + channel * num_grid * num_grid;
}

/*****************************************
* Function Name : offset
* Description   : Get the offset nuber to access the bounding box attributes
*                 To get the actual value of bounding box attributes, use index() after this function.
* Arguments     : n = output layer number [0~2].
*                 b = Number to indicate which bounding box in the region [0~2]
*                 y = Number to indicate which region [0~13]
*                 x = Number to indicate which region [0~13]
* Return value  : offset to access the bounding box attributes.
******************************************/
int32_t PostProc::offset(uint8_t n, int32_t b, int32_t y, int32_t x)
{
    uint8_t num = num_grids[n];
    uint32_t prev_layer_num = 0;
    int32_t i = 0;

    for (i = 0; i < n; i++)
    {
        prev_layer_num += NUM_BB * (NUM_CLASS + 5) * num_grids[i] * num_grids[i];
    }
    return prev_layer_num + b * (NUM_CLASS + 5) * num * num + y * num + x;
}

/*****************************************
* Function Name : sort_detection_by_prob
* Description   : -
* Arguments     : -
* Return value  : -
******************************************/
bool PostProc::sort_detection_by_prob(const detection& a, const detection& b)
{
    return a.prob > b.prob;
}

/*****************************************
* Function Name : remove_prob_0
* Description   : -
* Arguments     : -
* Return value  : -
******************************************/
bool PostProc::remove_prob_0(const detection& a)
{
    return a.prob == 0;
}

/*****************************************
* Function Name : R_Post_Proc
* Description   : Process CPU post-processing for YoloX
* Arguments     : floatarr = drpai output address
*                 det = detected boxes details
*                 ppl_count = total number of boxes
* Return value  : -
******************************************/
#if (0) == INF_YOLOX_SKIP
void PostProc::R_Post_Proc(float* floatarr, vector<detection>& det, uint32_t* ppl_count)
{
    uint32_t BoundingBoxCount = 0;
    /*Memory Access*/
    /* Following variables are required for correct_region_boxes in Darknet implementation*/
    /* Note: This implementation refers to the "darknet detector test" */
    vector<detection> det_buff;
    float new_w, new_h;
    float correct_w = 1.;
    float correct_h = 1.;
    if ((float)(MODEL_IN_W / correct_w) < (float)(MODEL_IN_H/correct_h))
    {
        new_w = (float)MODEL_IN_W;
        new_h = correct_h * MODEL_IN_W / correct_w;
    }
    else
    {
        new_w = correct_w * MODEL_IN_H / correct_h;
        new_h = (float)MODEL_IN_H;
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
    float classes[NUM_CLASS];
    float max_pred = 0;
    int32_t pred_class = -1;
    float probability = 0;
    detection d;
    /* Clear the detected result list */
    det.clear();

    /* YOLOX */
    int stride = 0;
    vector<int> strides = {8, 16, 32};

    for (n = 0; n < NUM_INF_OUT_LAYER; n++)
    {
        num_grid = num_grids[n];

        for (b = 0; b < NUM_BB; b++)
        {
           stride = strides[n];
            for (y = 0; y < num_grid; y++)
            {
                for (x = 0; x < num_grid; x++)
                {
                    offs = offset(n, b, y, x);
                    tc = floatarr[index(n, offs, 4)];

                    objectness = tc;

                    if (objectness > TH_PROB)
                    {
                        /* Get the class prediction */
                        for (i = 0; i < NUM_CLASS; i++)
                        {
                            classes[i] = floatarr[index(n, offs, 5+i)];
                        }

                        max_pred = 0;
                        pred_class = -1;
                        for (i = 0; i < NUM_CLASS; i++)
                        {
                            if (classes[i] > max_pred)
                            {
                                pred_class = i;
                                max_pred = classes[i];
                            }
                        }

                        /* Store the result into the list if the probability is more than the threshold */
                        probability = max_pred * objectness;
                        if (probability > TH_PROB)
                        {
                            if (pred_class == PERSON_LABEL_NUM)    //person = 14
                            {
                                tx = floatarr[offs];
                                ty = floatarr[index(n, offs, 1)];
                                tw = floatarr[index(n, offs, 2)];
                                th = floatarr[index(n, offs, 3)];

                                /* Compute the bounding box */
                                /* get_yolo_box/get_region_box in paper implementation */
                                center_x = (tx + float(x)) * stride;
                                center_y = (ty + float(y)) * stride;
                                center_x = center_x  / (float)MODEL_IN_W;
                                center_y = center_y  / (float)MODEL_IN_H;
                                box_w = exp(tw) * stride;
                                box_h = exp(th) * stride;
                                box_w = box_w / (float)MODEL_IN_W;
                                box_h = box_h / (float)MODEL_IN_H;
                                
                                /* Adjustment for size */
                                /* correct_yolo/region_boxes */
                                center_x = (center_x - (MODEL_IN_W - new_w) / 2. / MODEL_IN_W) / ((float)new_w / MODEL_IN_W);
                                center_y = (center_y - (MODEL_IN_H - new_h) / 2. / MODEL_IN_H) / ((float)new_h / MODEL_IN_H);
                                box_w *= (float)(MODEL_IN_W / new_w);
                                box_h *= (float)(MODEL_IN_H / new_h);

                                center_x = round(center_x * DRPAI_IN_WIDTH);
                                center_y = round(center_y * DRPAI_IN_HEIGHT);
                                box_w = round(box_w * DRPAI_IN_WIDTH);
                                box_h = round(box_h * DRPAI_IN_HEIGHT);

                                Box bb = {center_x, center_y, box_w, box_h};
                                d = {bb, pred_class, probability};
                                det_buff.push_back(d);
                            }
                            BoundingBoxCount++;
                        }
                    }
                }
            }
        }
    }

    /* Non-Maximum Supression filter */
    filter_boxes_nms(det_buff, det_buff.size(), TH_NMS);

    /* people count */
    sort(det_buff.begin(), det_buff.end(), PostProc::sort_detection_by_prob);
    det_buff.erase(remove_if(det_buff.begin(), det_buff.end(), PostProc::remove_prob_0), det_buff.end());
    if (det_buff.size() > NUM_MAX_PERSON)
    {
        det_buff.resize(NUM_MAX_PERSON);
    }
    *ppl_count = det_buff.size();
    
    /* Log Output */
    spdlog::info("YOLOX Result-------------------------------------");
    for (size_t i = 0; i < det_buff.size(); i++)
    {
        /* Skip the overlapped bounding boxes */
        spdlog::info(" Bounding Box Number : {}", i + 1);
        spdlog::info(" Bounding Box        : (X, Y, W, H) = ({}, {}, {}, {})", (int)det_buff[i].bbox.x, (int)det_buff[i].bbox.y, (int)det_buff[i].bbox.w, (int)det_buff[i].bbox.h);
        spdlog::info(" Detected Class      : {} (Class {})", label_file_map[det_buff[i].c].c_str(), det_buff[i].c);
        spdlog::info(" Probability         : {} %", (std::round((det_buff[i].prob * 100) * 10) / 10));
    }
    spdlog::info(" Bounding Box Count  : {}", BoundingBoxCount);
    spdlog::info(" Person Count        : {}", det_buff.size());

    mtx.lock();
    /* Clear the detected result list */
    det.clear();
    copy(det_buff.begin(), det_buff.end(), back_inserter(det));
    mtx.unlock();
    return;
}
#endif

/*****************************************
* Function Name : offset_hrnet
* Description   : Get the offset number to access the HRNet attributes
* Arguments     : b = Number to indicate which region [0~17]
*                 y = Number to indicate which region [0~64]
*                 x = Number to indicate which region [0~48]
* Return value  : offset to access the HRNet attributes.
*******************************************/
int32_t PostProc::offset_hrnet(int32_t b, int32_t y, int32_t x)
{
    return b * NUM_OUTPUT_W * NUM_OUTPUT_H + y * NUM_OUTPUT_W + x;
}

/*****************************************
* Function Name : sign
* Description   : Get the sign of the input value
* Arguments     : x = input value
* Return value  : returns the sign, 1 if positive -1 if not
*******************************************/
int8_t PostProc::sign(int32_t x)
{
    return x > 0 ? 1 : -1;
}

/*****************************************
* Function Name : R_Post_Proc_HRNet
* Description   : CPU post-processing for HRNet
*                 Microsoft COCO: Common Objects in Context' ECCV'2014
*                 More details can be found in the `paper
*                 <https://arxiv.org/abs/1405.0312>
*                 COCO Keypoint Indexes:
*                 0: 'nose',
*                 1: 'left_eye',
*                 2: 'right_eye',
*                 3: 'left_ear',
*                 4: 'right_ear',
*                 5: 'left_shoulder',
*                 6: 'right_shoulder',
*                 7: 'left_elbow',
*                 8: 'right_elbow',
*                 9: 'left_wrist',
*                 10: 'right_wrist',
*                 11: 'left_hip',
*                 12: 'right_hip',
*                 13: 'left_knee',
*                 14: 'right_knee',
*                 15: 'left_ankle',
*                 16: 'right_ankle'
* Arguments     : floatarr = drpai output address
*                 n_pers = number of the person detected
* Return value  : -
******************************************/
void PostProc::R_Post_Proc_HRNet(float* floatarr, int16_t crop_w, int16_t crop_h, float* lowest_kpt_score, float* preds)
{
    float preds_local[NUM_OUTPUT_C * 3];
    float score;
    int32_t b = 0;
    int32_t y = 0;
    int32_t x = 0;
    int32_t offs = 0;

    float center[] = {(float)(crop_w / 2 - 1), (float)(crop_h / 2 - 1)};
    int8_t ind_x = -1;
    int8_t ind_y = -1;
    float max_val = -1;
    float scale_x = 0;
    float scale_y = 0;
    float coords_x = 0;
    float coords_y = 0;
    float diff_x;
    float diff_y;
    int8_t i;
    
    for (b = 0; b < NUM_OUTPUT_C; b++)
    {
        float scale[] = {(float)(crop_w / 200.0), (float)(crop_h / 200.0)};
        ind_x = -1;
        ind_y = -1;
        max_val = -1;
        for (y = 0; y < NUM_OUTPUT_H; y++)
        {
            for (x = 0; x < NUM_OUTPUT_W; x++)
            {
                offs = offset_hrnet(b, y, x);
                if (max_val < floatarr[offs])
                {
                    /* Update the maximum value and indices */
                    max_val = floatarr[offs];
                    ind_x = x;
                    ind_y = y;
                }
            }
        }
        if (0 > max_val)
        {
            ind_x = -1;
            ind_y = -1;
            goto not_detect;
        }
        preds_local[b * 3 + 0] = float(ind_x);
        preds_local[b * 3 + 1] = float(ind_y);
        preds_local[b * 3 + 2] = max_val;
        offs = offset_hrnet(b, ind_y, ind_x);
        if ((ind_y > 1) && (ind_y < NUM_OUTPUT_H -1))
        {
            if ((ind_x > 1) && (ind_x < (NUM_OUTPUT_W -1)))
            {
                diff_x = floatarr[offs + 1] - floatarr[offs - 1];
                diff_y = floatarr[offs + NUM_OUTPUT_W] - floatarr[offs - NUM_OUTPUT_W];
                preds_local[b * 3 + 0] += sign(diff_x) * 0.25;
                preds_local[b * 3 + 1] += sign(diff_y) * 0.25;
            }
        }

        /* transform_preds */
        scale[0] *= 200;
        scale[1] *= 200;
        //udp (Unbiased Data Processing) = False
        scale_x = scale[0] / (NUM_OUTPUT_W);
        scale_y = scale[1] / (NUM_OUTPUT_H);
        coords_x = preds_local[b * 3 + 0];
        coords_y = preds_local[b * 3 + 1];
        preds_local[b * 3 + 0] = (coords_x * scale_x) + center[0] - (scale[0] * 0.5);
        preds_local[b * 3 + 1] = (coords_y * scale_y) + center[1] - (scale[1] * 0.5);
    }
    /* Clear the score in preparation for the update. */
    *lowest_kpt_score = 0;
    score = 1;
    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
        /* Adopt the lowest score. */
        if (preds_local[i * 3 + 2] < score)
        {
            score = preds_local[i * 3 + 2];
        }
    }
    /* Update the score for display thread. */
    *lowest_kpt_score = score;
    /* HRnet Logout. */
    spdlog::info("HRNet Result-------------------------------------");
    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
        spdlog::info("  ID {}: ({}, {}): {}%", i, (std::round((preds_local[i * 3 + 0]) * 100) / 100), (std::round((preds_local[i * 3 + 1]) * 100) / 100), (std::round((preds_local[i * 3 + 2] * 100) * 10) / 10));
    }
    goto end;

not_detect:
    *lowest_kpt_score = 0;
    goto end;

end:
    mtx.lock();
    copy(preds_local, preds_local + (NUM_OUTPUT_C * 3), preds);
    mtx.unlock();
    return;
}

/*****************************************
* Function Name : R_HRNet_Coord_Convert
* Description   : Convert the post processing result into drawable coordinates
* Arguments     : n_pers = number of the detected person
* Return value  : -
******************************************/
void PostProc::R_HRNet_Coord_Convert(int16_t crop_x, int16_t crop_y, uint8_t n_pers, uint16_t* id_x, uint16_t* id_y, float* preds)
{
    /* Render skeleton on image and print their details */
    int32_t posx;
    int32_t posy;
    int8_t i;
    float preds_local[NUM_OUTPUT_C * 3];
    mtx.lock();
    copy(preds, preds + (NUM_OUTPUT_C * 3), preds_local);
    mtx.unlock();

    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
#if (0) == INF_YOLOX_SKIP
        /* Conversion from input image coordinates to display image coordinates. */
        /* +0.5 is for rounding.*/
        posx = (int32_t)(preds_local[i * 3 + 0] + 0.5) + crop_x + OUTPUT_ADJ_X;
        posy = (int32_t)(preds_local[i * 3 + 1] + 0.5) + crop_y + OUTPUT_ADJ_Y;
        /* Make sure the coordinates are not off the screen. */
        posx = (posx < 0) ? 0 : posx;
        posx = (posx > IMREAD_IMAGE_WIDTH - KEY_POINT_SIZE -1 ) ? IMREAD_IMAGE_WIDTH -KEY_POINT_SIZE -1 : posx;
        posy = (posy < 0) ? 0 : posy;
        posy = (posy > IMREAD_IMAGE_HEIGHT -KEY_POINT_SIZE -1) ? IMREAD_IMAGE_HEIGHT -KEY_POINT_SIZE -1 : posy;
#else
        /* Conversion from input image coordinates to display image coordinates. */
        /* +0.5 is for rounding.                                                 */
        posx = (int32_t)(preds_local[i * 3 + 0] / CROPPED_IMAGE_WIDTH  * CROPPED_IMAGE_WIDTH  + 0.5) + OUTPUT_LEFT + OUTPUT_ADJ_X;
        posy = (int32_t)(preds_local[i * 3 + 1] / CROPPED_IMAGE_HEIGHT * CROPPED_IMAGE_HEIGHT + 0.5) + OUTPUT_TOP  + OUTPUT_ADJ_Y;
        /* Make sure the coordinates are not off the screen. */
        posx    = (posx < OUTPUT_LEFT) ? OUTPUT_LEFT : posx;
        posy    = (posy < OUTPUT_TOP)  ? OUTPUT_TOP  : posy;
        posx = (posx > OUTPUT_LEFT + CROPPED_IMAGE_WIDTH  - 1) ? (OUTPUT_LEFT + CROPPED_IMAGE_WIDTH   - 1) : posx;
        posy = (posy > OUTPUT_TOP  + CROPPED_IMAGE_HEIGHT - 1) ? (OUTPUT_TOP  + CROPPED_IMAGE_HEIGHT  - 1) : posy;
#endif
        id_x[i * NUM_MAX_PERSON + n_pers] = posx;
        id_y[i * NUM_MAX_PERSON + n_pers] = posy;
    }
    return;
}