/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : main.cpp
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for MMPose HRNet with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
/*DRP-AI TVM[*1] Runtime*/
#include "MeraDrpRuntimeWrapper.h"
/*Pre-processing Runtime Header*/
#include "PreRuntime.h"
/*DRPAI Driver Header*/
#include <linux/drpai.h>
/*Definition of Macros & other variables*/
#include "define.h"
#include "define_color_hrnet.h"
/*USB camera control*/
#include "camera.h"
/*Image control*/
#include "image_hrnet.h"
/*Wayland control*/
#include "wayland.h"
/*YOLOX Post-Processing*/
#include "box.h"
/*Mutual exclusion*/
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace std;
/*****************************************
* Global Variables
******************************************/
/*Multithreading*/
static sem_t terminate_req_sem;
static pthread_t ai_inf_thread;
static pthread_t kbhit_thread;
static pthread_t capture_thread;
static pthread_t img_thread;
static pthread_t hdmi_thread;
static mutex mtx;

/*Flags*/
static atomic<uint8_t> inference_start (0);
static atomic<uint8_t> img_obj_ready   (0);
static atomic<uint8_t> hdmi_obj_ready   (0);

/*Global Variables*/
static float drpai_output_buf0[num_inf_out];
static float drpai_output_buf[INF_OUT_SIZE];
static uint64_t capture_address;
static uint8_t buf_id;
static Image img;

/*AI Inference for DRPAI*/
/* DRP-AI TVM[*1] Runtime object */
MeraDrpRuntimeWrapper runtime;
/* Pre-processing Runtime object */
PreRuntime preruntime;


static int drpai_fd0 = -1;
static int drpai_fd1 = -1;
static drpai_handle_t *drpai_hdl0 = NULL;
static drpai_data_t drpai_data0;
static drpai_handle_t *drpai_hdl1 = NULL;
static drpai_data_t drpai_data1;

static double yolox_drpai_time = 0;
static double hrnet_drpai_time = 0;
#ifdef DISP_AI_FRAME_RATE
static double ai_fps = 0;
static double cap_fps = 0;
static double proc_time_capture = 0;
static uint32_t array_cap_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
#endif /* DISP_AI_FRAME_RATE */
static double yolox_proc_time = 0;
static double hrnet_proc_time = 0;
static uint32_t disp_time = 0;
static uint32_t array_drp_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
static uint32_t array_disp_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
static int32_t drp_max_freq;
static int32_t drpai_freq;
//static uint32_t ai_time = 0;
static double array_hrnet_drpai_time[NUM_MAX_PERSON];
static double array_hrnet_proc_time[NUM_MAX_PERSON];
static float hrnet_preds[NUM_OUTPUT_C][3];
static uint16_t id_x[NUM_OUTPUT_C][NUM_MAX_PERSON];
static uint16_t id_y[NUM_OUTPUT_C][NUM_MAX_PERSON];
static uint16_t id_x_local[NUM_OUTPUT_C][NUM_MAX_PERSON]; /*To be used only in Inference Threads*/
static uint16_t id_y_local[NUM_OUTPUT_C][NUM_MAX_PERSON]; /*To be used only in Inference Threads*/

static int16_t cropx[NUM_MAX_PERSON];
static int16_t cropy[NUM_MAX_PERSON];
static int16_t croph[NUM_MAX_PERSON];
static int16_t cropw[NUM_MAX_PERSON];
static float lowest_kpt_score[NUM_MAX_PERSON];
static float lowest_kpt_score_local[NUM_MAX_PERSON]; /*To be used only in Inference Threads*/

/*YOLOX*/
static uint32_t bcount = 0;
static uint32_t ppl_count_local = 0; /*To be used only in Inference Threads*/
static uint32_t ppl_count = 0;
static vector<detection> det_res;
static vector<detection> det_ppl;

static Wayland wayland;
static vector<detection> det;
static Camera* capture = NULL;

static double pre_time = 0;
static double post_time = 0;
static double ai_time = 0;

/*****************************************
* Function Name     : float16_to_float32
* Description       : Function by Edgecortex. Cast uint16_t a into float value.
* Arguments         : a = uint16_t number
* Return value      : float = float32 number
******************************************/
float float16_to_float32(uint16_t a)
{
    return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

/*****************************************
* Function Name : timedifference_msec
* Description   : compute the time differences in ms between two moments
* Arguments     : t0 = start time
*                 t1 = stop time
* Return value  : the time difference in ms
******************************************/
static double timedifference_msec(struct timespec t0, struct timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1000000.0;
}

/*****************************************
* Function Name : wait_join
* Description   : waits for a fixed amount of time for the thread to exit
* Arguments     : p_join_thread = thread that the function waits for to Exit
*                 join_time = the timeout time for the thread for exiting
* Return value  : 0 if successful
*                 not 0 otherwise
******************************************/
static int8_t wait_join(pthread_t *p_join_thread, uint32_t join_time)
{
    int8_t ret_err;
    struct timespec join_timeout;
    ret_err = clock_gettime(CLOCK_REALTIME, &join_timeout);
    if ( 0 == ret_err )
    {
        join_timeout.tv_sec += join_time;
        ret_err = pthread_timedjoin_np(*p_join_thread, NULL, &join_timeout);
    }
    return ret_err;
}

/*****************************************
* Function Name : get_result
* Description   : Get DRP-AI Output from memory via DRP-AI Driver
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t get_result()
{
    int8_t ret = 0;
    int32_t i = 0;
    int32_t output_num = 0;
    std::tuple<InOutDataType, void*, int64_t> output_buffer;
    int64_t output_size;
    uint32_t size_count = 0;

    /* Get the number of output of the target model. */
    output_num = runtime.GetNumOutput();
    size_count = 0;
    /*GetOutput loop*/
    for (i = 0;i<output_num;i++)
    {
        /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
        output_buffer = runtime.GetOutput(i);
        /*Output Data Size = std::get<2>(output_buffer). */
        output_size = std::get<2>(output_buffer);
        /*Output Data Type = std::get<0>(output_buffer)*/
        if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
        {
            /*Output Data = std::get<1>(output_buffer)*/
            uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
            for (int j = 0; j<output_size; j++)
            {
                /*FP16 to FP32 conversion*/
                drpai_output_buf[j + size_count]=float16_to_float32(data_ptr[j]);
            }
        }
        else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
        {
            /*Output Data = std::get<1>(output_buffer)*/
            float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
            for (int j = 0; j<output_size; j++)
            {
                drpai_output_buf[j + size_count]=data_ptr[j];
            }
        }
        else
        {
            fprintf(stderr, "[ERROR] Output data type : not floating point.\n");
            ret = -1;
            break;
        }
        size_count += output_size;
    }
    return ret;
}

/*****************************************
* Function Name : sigmoid
* Description   : Helper function for YOLO Post Processing
* Arguments     : x = input argument for the calculation
* Return value  : sigmoid result of input x
******************************************/
static double sigmoid(double x)
{
    return 1.0/(1.0 + exp(-x));
}

/*****************************************
* Function Name : softmax
* Description   : Helper function for YOLO Post Processing
* Arguments     : val[] = array to be computed softmax
* Return value  : -
******************************************/
static void softmax(float val[NUM_CLASS])
{
    float max_num = -FLT_MAX;
    float sum = 0;
    int32_t i;
    for ( i = 0 ; i<NUM_CLASS ; i++ )
    {
        max_num = max(max_num, val[i]);
    }

    for ( i = 0 ; i<NUM_CLASS ; i++ )
    {
        val[i]= (float) exp(val[i] - max_num);
        sum+= val[i];
    }

    for ( i = 0 ; i<NUM_CLASS ; i++ )
    {
        val[i]= val[i]/sum;
    }
    return;
}

/*****************************************
* Function Name : index
* Description   : Get the index of the bounding box attributes based on the input offset.
* Arguments     : n = output layer number.
*                 offs = offset to access the bounding box attributesd.
*                 channel = channel to access each bounding box attribute.
* Return value  : index to access the bounding box attribute.
******************************************/
int32_t index(uint8_t n, int32_t offs, int32_t channel)
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
int32_t offset(uint8_t n, int32_t b, int32_t y, int32_t x)
{
    uint8_t num = num_grids[n];
    uint32_t prev_layer_num = 0;
    int32_t i = 0;

    for (i = 0 ; i < n; i++)
    {
        prev_layer_num += NUM_BB *(NUM_CLASS + 5)* num_grids[i] * num_grids[i];
    }
    return prev_layer_num + b *(NUM_CLASS + 5)* num * num + y * num + x;
}

/*****************************************
* Function Name : R_Post_Proc
* Description   : Process CPU post-processing for YoloX
* Arguments     : floatarr = drpai output address
*                 det = detected boxes details
*                 box_count = total number of boxes
* Return value  : -
******************************************/
static void R_Post_Proc(float* floatarr, vector<detection>& det, uint32_t* box_count)
{
    uint32_t count = 0;
    uint32_t BoundingBoxCount = 0;
    /*Memory Access*/
    /* Following variables are required for correct_region_boxes in Darknet implementation*/
    /* Note: This implementation refers to the "darknet detector test" */
    vector<detection> det_buff;
    float new_w, new_h;
    float correct_w = 1.;
    float correct_h = 1.;
    if ((float) (MODEL_IN_W / correct_w) < (float) (MODEL_IN_H/correct_h) )
    {
        new_w = (float) MODEL_IN_W;
        new_h = correct_h * MODEL_IN_W / correct_w;
    }
    else
    {
        new_w = correct_w * MODEL_IN_H / correct_h;
        new_h = MODEL_IN_H;
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
    float classes[NUM_CLASS];
    float max_pred = 0;
    int32_t pred_class = -1;
    float probability = 0;
    detection d;
    /* Clear the detected result list */
    det.clear();

    //YOLOX
    int stride = 0;
    vector<int> strides = {8, 16, 32};

    for (n = 0; n<NUM_INF_OUT_LAYER; n++)
    {
        num_grid = num_grids[n];
        anchor_offset = 2 * NUM_BB * (NUM_INF_OUT_LAYER - (n + 1));

        for (b = 0;b<NUM_BB;b++)
        {
           stride = strides[n];
            for (y = 0;y<num_grid;y++)
            {
                for (x = 0;x<num_grid;x++)
                {
                    offs = offset(n, b, y, x);
                    tc = floatarr[index(n, offs, 4)];

                    objectness = tc;

                    if (objectness > TH_PROB)
                    {
                        /* Get the class prediction */
                        for (i = 0;i < NUM_CLASS;i++)
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
                                /*get_yolo_box/get_region_box in paper implementation*/
                                center_x = (tx+ float(x))* stride;
                                center_y = (ty+ float(y))* stride;
                                center_x = center_x  / (float) MODEL_IN_W;
                                center_y = center_y  / (float) MODEL_IN_H;
                                box_w = exp(tw) * stride;
                                box_h = exp(th) * stride;
                                box_w = box_w / (float) MODEL_IN_W;
                                box_h = box_h / (float) MODEL_IN_H;
                                
                                /* Adjustment for size */
                                /* correct_yolo/region_boxes */
                                center_x = (center_x - (MODEL_IN_W - new_w) / 2. / MODEL_IN_W) / ((float) new_w / MODEL_IN_W);
                                center_y = (center_y - (MODEL_IN_H - new_h) / 2. / MODEL_IN_H) / ((float) new_h / MODEL_IN_H);
                                box_w *= (float) (MODEL_IN_W / new_w);
                                box_h *= (float) (MODEL_IN_H / new_h);

                                center_x = round(center_x * DRPAI_IN_WIDTH);
                                center_y = round(center_y * DRPAI_IN_HEIGHT);
                                box_w = round(box_w * DRPAI_IN_WIDTH);
                                box_h = round(box_h * DRPAI_IN_HEIGHT);
                                
                                Box bb = {center_x, center_y, box_w, box_h};
                                d = {bb, pred_class, probability};
                                det_buff.push_back(d);
                                count++;
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
    *box_count = count;
    /* Log Output */
    spdlog::info("YOLOX Result-------------------------------------");
    int iBoxCount=0;
    for(i = 0; i < det_buff.size(); i++)
    {
        /* Skip the overlapped bounding boxes */
        if (det_buff[i].prob == 0) continue;
        spdlog::info(" Bounding Box Number : {}",i+1);
        spdlog::info(" Bounding Box        : (X, Y, W, H) = ({}, {}, {}, {})", (int)det_buff[i].bbox.x, (int)det_buff[i].bbox.y, (int)det_buff[i].bbox.w, (int)det_buff[i].bbox.h);
        spdlog::info(" Detected Class      : {} (Class {})", label_file_map[det_buff[i].c].c_str(), det_buff[i].c);
        spdlog::info(" Probability         : {} %", (std::round((det_buff[i].prob*100) * 10) / 10));
        iBoxCount++;
    }
    spdlog::info(" Bounding Box Count  : {}", BoundingBoxCount);
    spdlog::info(" Person Count        : {}", iBoxCount);

    mtx.lock();
    /* Clear the detected result list */
    det.clear();
    copy(det_buff.begin(), det_buff.end(), back_inserter(det));
    mtx.unlock();
    return ;
}

/*****************************************
* Function Name : people_counter
* Description   : Function to count the real number of people detected and does not exceeds the maximum number
* Arguments     : det = detected boxes details
*                 ppl = detected people details
*                 box_count = total number of boxes
*                 ppl_count = actual number of people
* Return value  : -
******************************************/
static void people_counter(vector<detection>& det, vector<detection>& ppl, uint32_t box_count, uint32_t* ppl_count)
{
    mtx.lock();
    uint32_t count = 0;
    ppl.clear();
    for(uint32_t i = 0; i<box_count; i++)
    {
        if(0 == det[i].prob)
        {
            continue;
        }
        else
        {
            ppl.push_back(det[i]);
            count++;
            if(count > NUM_MAX_PERSON-1)
            {
                break;
            }
        }
    }
    *ppl_count = count;
    mtx.unlock();
}

/*****************************************
* Function Name : offset_hrnet
* Description   : Get the offset number to access the HRNet attributes
* Arguments     : b = Number to indicate which region [0~17]
*                 y = Number to indicate which region [0~64]
*                 x = Number to indicate which region [0~48]
* Return value  : offset to access the HRNet attributes.
*******************************************/
static int32_t offset_hrnet(int32_t b, int32_t y, int32_t x)
{
    return b * NUM_OUTPUT_W * NUM_OUTPUT_H + y * NUM_OUTPUT_W + x;
}

/*****************************************
* Function Name : sign
* Description   : Get the sign of the input value
* Arguments     : x = input value
* Return value  : returns the sign, 1 if positive -1 if not
*******************************************/
static int8_t sign(int32_t x)
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
static void R_Post_Proc_HRNet(float* floatarr, uint8_t n_pers)
{
    mtx.lock();
    float score;
    int32_t b = 0;
    int32_t y = 0;
    int32_t x = 0;
    int32_t offs = 0;

    float center[] = {(float)(cropw[n_pers] / 2 -1), (float)(croph[n_pers] / 2 - 1)};
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
    
    for(b = 0; b < NUM_OUTPUT_C; b++)
    {
        float scale[] = {(float)(cropw[n_pers] / 200.0), (float)(croph[n_pers] / 200.0)};
        ind_x = -1;
        ind_y = -1;
        max_val = -1;
        for(y = 0; y < NUM_OUTPUT_H; y++)
        {
            for(x = 0; x < NUM_OUTPUT_W; x++)
            {
                offs = offset_hrnet(b, y, x);
                if (max_val < floatarr[offs])
                {
                    /*Update the maximum value and indices*/
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
        hrnet_preds[b][0] = float(ind_x);
        hrnet_preds[b][1] = float(ind_y);
        hrnet_preds[b][2] = max_val;
        offs = offset_hrnet(b, ind_y, ind_x);
        if ((ind_y > 1) && (ind_y < NUM_OUTPUT_H -1))
        {
            if ((ind_x > 1) && (ind_x < (NUM_OUTPUT_W -1)))
            {
                diff_x = floatarr[offs + 1] - floatarr[offs - 1];
                diff_y = floatarr[offs + NUM_OUTPUT_W] - floatarr[offs - NUM_OUTPUT_W];
                hrnet_preds[b][0] += sign(diff_x) * 0.25;
                hrnet_preds[b][1] += sign(diff_y) * 0.25;
            }
        }

        /*transform_preds*/
        scale[0] *= 200;
        scale[1] *= 200;
        //udp (Unbiased Data Processing) = False
        scale_x = scale[0] / (NUM_OUTPUT_W);
        scale_y = scale[1] / (NUM_OUTPUT_H);
        coords_x = hrnet_preds[b][0];
        coords_y = hrnet_preds[b][1];
        hrnet_preds[b][0] = (coords_x * scale_x) + center[0] - (scale[0] * 0.5);
        hrnet_preds[b][1] = (coords_y * scale_y) + center[1] - (scale[1] * 0.5);
    }
    /* Clear the score in preparation for the update. */
    lowest_kpt_score_local[n_pers] = 0;
    score = 1;
    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
        /* Adopt the lowest score. */
        if (hrnet_preds[i][2] < score)
        {
            score = hrnet_preds[i][2];
        }
    }
    /* Update the score for display thread. */
    lowest_kpt_score_local[n_pers] = score;
    /* HRnet Logout. */
    spdlog::info("HRNet Result-------------------------------------");
    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
        spdlog::info("  ID {}: ({}, {}): {}%", i, (std::round((hrnet_preds[i][0]) * 100) / 100), (std::round((hrnet_preds[i][1]) * 100) / 100), (std::round((hrnet_preds[i][2]*100) * 10) / 10));
    }
    goto end;

not_detect:
    lowest_kpt_score_local[n_pers] = 0;
    goto end;

end:
    mtx.unlock();
    return;
}

/*****************************************
* Function Name : R_HRNet_Coord_Convert
* Description   : Convert the post processing result into drawable coordinates
* Arguments     : n_pers = number of the detected person
* Return value  : -
******************************************/
static void R_HRNet_Coord_Convert(uint8_t n_pers)
{
    /* Render skeleton on image and print their details */
    int32_t posx;
    int32_t posy;
    int8_t i;
    mtx.lock();

    for (i = 0; i < NUM_OUTPUT_C; i++)
    {
#if (0) == INF_YOLOX_SKIP
        /* Conversion from input image coordinates to display image coordinates. */
        /* +0.5 is for rounding.*/
        posx = (int32_t)(hrnet_preds[i][0] + 0.5) + cropx[n_pers] + OUTPUT_ADJ_X;
        posy = (int32_t)(hrnet_preds[i][1] + 0.5) + cropy[n_pers] + OUTPUT_ADJ_Y;
        /* Make sure the coordinates are not off the screen. */
        posx = (posx < 0) ? 0 : posx;
        posx = (posx > IMREAD_IMAGE_WIDTH - KEY_POINT_SIZE -1 ) ? IMREAD_IMAGE_WIDTH -KEY_POINT_SIZE -1 : posx;
        posy = (posy < 0) ? 0 : posy;
        posy = (posy > IMREAD_IMAGE_HEIGHT -KEY_POINT_SIZE -1) ? IMREAD_IMAGE_HEIGHT -KEY_POINT_SIZE -1 : posy;
#else
        /* Conversion from input image coordinates to display image coordinates. */
        /* +0.5 is for rounding.                                                 */
        posx = (int32_t)(hrnet_preds[i][0] / CROPPED_IMAGE_WIDTH  * CROPPED_IMAGE_WIDTH  + 0.5) + OUTPUT_LEFT + OUTPUT_ADJ_X;
        posy = (int32_t)(hrnet_preds[i][1] / CROPPED_IMAGE_HEIGHT * CROPPED_IMAGE_HEIGHT + 0.5) + OUTPUT_TOP  + OUTPUT_ADJ_Y;
        /* Make sure the coordinates are not off the screen. */
        posx    = (posx < OUTPUT_LEFT) ? OUTPUT_LEFT : posx;
        posy    = (posy < OUTPUT_TOP)  ? OUTPUT_TOP  : posy;
        posx = (posx > OUTPUT_LEFT + CROPPED_IMAGE_WIDTH  - 1) ? (OUTPUT_LEFT + CROPPED_IMAGE_WIDTH   - 1) : posx;
        posy = (posy > OUTPUT_TOP  + CROPPED_IMAGE_HEIGHT - 1) ? (OUTPUT_TOP  + CROPPED_IMAGE_HEIGHT  - 1) : posy;
#endif
        id_x_local[i][n_pers] = posx;
        id_y_local[i][n_pers] = posy;
    }
    mtx.unlock();
    return;
}

/*****************************************
* Function Name : draw_skeleton
* Description   : Draw Complete Skeleton on image.
* Arguments     : -
* Return value  : -
******************************************/
static void draw_skeleton(void)
{
    int32_t sk_id;
    uint8_t v;
    uint8_t i;
    float   thre_kpt = TH_KPT;
#if (1) == INF_YOLOX_SKIP
    thre_kpt = TH_KPT_YOLOX_SKIP;
#endif

    mtx.lock();

#if (1) == INF_YOLOX_SKIP
    i=0;
    img.draw_rect(cropx[i], cropy[i], cropw[i], croph[i]-1, YELLOW_DATA);
    img.draw_rect(cropx[i]+1, cropy[i]+1, cropw[i]-2, croph[i]-3, YELLOW_DATA);
#endif

    if(ppl_count > 0)
    {
        for(i=0; i < ppl_count; i++)
        {    
            /*Check If All Key Points Were Detected: If Over Threshold, It will Draw Complete Skeleton*/
            if (lowest_kpt_score[i] > thre_kpt)
            {
                /* Draw limb */
                for (sk_id = 0; sk_id < NUM_LIMB; sk_id++)
                {
                    uint8_t sk[] = {skeleton[sk_id][0], skeleton[sk_id][1]};
                    int pos1[] = {id_x[sk[0]][i], id_y[sk[0]][i]};
                    int pos2[] = {id_x[sk[1]][i], id_y[sk[1]][i]};
                    
                    if ((0 < pos1[0]) && (MIPI_WIDTH > pos1[0])
                        && (0 < pos1[1]) && (MIPI_WIDTH > pos1[1]))
                    {
                        if ((0 < pos2[0]) && (MIPI_WIDTH > pos2[0])
                            && (0 < pos2[1]) && (MIPI_WIDTH > pos2[1]))
                        {
                            img.draw_line2(pos1[0], pos1[1], pos2[0],pos2[1], YELLOW_DATA);
                        }
                    }
                }
        
                /*Draw Rectangle As Key Points*/
                for(v = 0; v < NUM_OUTPUT_C; v++)
                {
                    /*Draw Rectangles On Each Skeleton Key Points*/
                    img.draw_rect(id_x[v][i], id_y[v][i], KEY_POINT_SIZE, KEY_POINT_SIZE, RED_DATA);
                    img.draw_rect(id_x[v][i], id_y[v][i], KEY_POINT_SIZE+1, KEY_POINT_SIZE+1, RED_DATA);
                }
            }
        }
    }
    mtx.unlock();
    return;
}

/*****************************************
* Function Name : draw_bounding_box
* Description   : Draw bounding box on image.
* Arguments     : -
* Return value  : 0 if succeeded
*               not 0 otherwise
******************************************/
void draw_bounding_box(void)
{
    vector<detection> det_buff;
    stringstream stream;
    string result_str;
    int32_t i = 0;
    uint32_t color=0;
 
    mtx.lock();
    copy(det_res.begin(), det_res.end(), back_inserter(det_buff));
    mtx.unlock();

    /* Draw bounding box on RGB image. */
    for (i = 0; i < det_buff.size(); i++)
    {
        /* Skip the overlapped bounding boxes */
        if (det_buff[i].prob == 0) continue;
        
        color = box_color[det_buff[i].c];
        /* Clear string stream for bounding box labels */
        stream.str("");
        /* Draw the bounding box on the image */
        stream << fixed << setprecision(2) << det_buff[i].prob;
        result_str = label_file_map[det_buff[i].c]+ " "+ stream.str();
        img.draw_rect_box((int)det_buff[i].bbox.x, (int)det_buff[i].bbox.y, (int)det_buff[i].bbox.w, (int)det_buff[i].bbox.h, result_str.c_str(),color);
    }
    return;
}

/*****************************************
* Function Name : print_result
* Description   : print the result on display.
* Arguments     : -
* Return value  : 0 if succeeded
*               not 0 otherwise
******************************************/
int8_t print_result(Image* img)
{
#ifdef DEBUG_TIME_FLG
    using namespace std;
    chrono::system_clock::time_point start, end;
    start = chrono::system_clock::now();
#endif // DEBUG_TIME_FLG

    int32_t index = 0;
    stringstream stream;
    string str = "";
    string DispStr = "";
    uint32_t total_time = ai_time + pre_time + post_time;
    
#if (0) == INF_YOLOX_SKIP
    /* Draw Inference YOLOX Time Result on RGB image.*/
    stream.str("");
    if (yolox_drpai_time< 10){
       DispStr = "YOLOX           Pre-Proc + Inference Time (DRP-AI) :  ";
    }else{
       DispStr = "YOLOX           Pre-Proc + Inference Time (DRP-AI) : ";
    }
    stream << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(yolox_drpai_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R,  LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0x00FF00u);
 
    /* Draw Post-Proc YOLOX Time on RGB image.*/
    stream.str("");
    if (yolox_proc_time< 10){
       DispStr = "Post-Proc Time (CPU) :  ";
    }else{
       DispStr = "Post-Proc Time (CPU) : ";
    }
    stream << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(yolox_proc_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0x00FF00u);
#endif
    
    /* Draw total_time on RGB image.*/
    stream.str("");
    if (total_time< 10){
       DispStr = "  Total AI Time :  ";
    }else{
       DispStr = "  Total AI Time : ";
    }
    stream << "HRNet x " << (uint32_t)ppl_count << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(total_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R,  LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0xFFF000u);
 
    /* Draw ai_time HRNet Time on RGB image.*/
    stream.str("");
    if (ai_time< 10){
       DispStr = "Inference :  ";
    }else{
       DispStr = "Inference : ";
    }
    stream << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(ai_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0xFFF000u);

    /* Draw PreProcess Time HRNet Time on RGB image.*/
    stream.str("");
    if (pre_time< 10){
       DispStr = "PreProcess :  ";
    }else{
       DispStr = "PreProcess : ";
    }
    stream << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(pre_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0xFFF000u);

    /* Draw PostProcess Time HRNet Time on RGB image.*/
    stream.str("");
    if (post_time< 10){
       DispStr = "PostProcess :  ";
    }else{
       DispStr = "PostProcess : ";
    }
    stream << DispStr << std::setw(3) << std::fixed << std::setprecision(1) << std::round(post_time * 10) / 10 << "msec";
    str = stream.str();
    index++;
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * index), CHAR_SCALE_LARGE, 0xFFF000u);

#ifdef DISP_AI_FRAME_RATE
    /* Draw AI/Camera Frame Rate on RGB image.*/
    stream.str("");
    stream << "AI/Camera Frame Rate: " << std::setw(3) << (uint32_t)ai_fps << "/" << (uint32_t)cap_fps << "fps";
    str = stream.str();
    img->write_string_rgb(str, 1, TEXT_WIDTH_OFFSET_L, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 1), CHAR_SCALE_LARGE, WHITE_DATA);
#endif /* DISP_AI_FRAME_RATE */

#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Draw Text Time            : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG

    return 0;
}

/*****************************************
* Function Name : R_Inf_Thread
* Description   : Executes the DRP-AI inference thread
* Arguments     : threadid = thread identification
* Return value  : -
******************************************/
void *R_Inf_Thread(void *threadid)
{
    /*Semaphore Variable*/
    int32_t inf_sem_check = 0;
    int32_t inf_cnt = -1;
    /*Variable for getting Inference output data*/
    void* output_ptr;
    uint32_t out_size;
    /*Variable for Pre-processing parameter configuration*/
    s_preproc_param_t in_param;
    /*Variable for checking return value*/
    int8_t ret = 0;
    /*Variable for Performance Measurement*/
    double total_hrnet_drpai_time = 0;
    double total_hrnet_proc_time = 0;
    timespec yolox_sta_time;
    timespec yolox_end_time;
    static struct timespec yolox_drp_start_time;
    static struct timespec yolox_drp_end_time;
    timespec hrnet_sta_time;
    timespec hrnet_end_time;
	static struct timespec hrnet_drp_start_time;
    static struct timespec hrnet_drp_end_time;
     static struct timespec start_time;
	static struct timespec inf_start_time;
    static struct timespec inf_end_time;
    //static struct timespec inf_end_time;
    static struct timespec pre_start_time;
    static struct timespec pre_end_time;
    static struct timespec post_start_time;
    static struct timespec post_end_time;
    static struct timespec drp_prev_time = { .tv_sec = 0, .tv_nsec = 0, };
    int i = 0; 
    /*HRNet Modify Parameters*/

    printf("Inference Thread Starting\n");

    in_param.pre_in_shape_w = CAM_IMAGE_WIDTH;
    in_param.pre_in_shape_h = CAM_IMAGE_HEIGHT;

    printf("Inference Loop Starting\n");
    /*Inference Loop Start*/
    while(1)
    {
        inf_cnt++;
        spdlog::info("[START] Start DRP-AI Inference...");
        spdlog::info("Inference ----------- No. {}", (inf_cnt + 1));
        while(1)
        {
            /*Gets the Termination request semaphore value. If different then 1 Termination was requested*/
            /*Checks if sem_getvalue is executed wihtout issue*/
            errno = 0;
            ret = sem_getvalue(&terminate_req_sem, &inf_sem_check);
            if (0 != ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
                goto err;
            }
            /*Checks the semaphore value*/
            if (1 != inf_sem_check)
            {
                goto ai_inf_end;
            }
            /*Checks if image frame from Capture Thread is ready.*/
            if (inference_start.load())
            {
                break;
            }
            usleep(WAIT_TIME);
        }
        errno = 0; 
        memset(array_hrnet_drpai_time,0,sizeof(array_hrnet_drpai_time));
        memset(array_hrnet_proc_time,0,sizeof(array_hrnet_proc_time));   


#if (0) == INF_YOLOX_SKIP
        /*Gets inference starting time*/
        ret = timespec_get(&yolox_drp_start_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Inference Start Time\n");
            goto err;
        }
        /*Start DRP-AI Driver*/
        errno = 0;
        ret = start_drpai(drpai_hdl0, (uintptr_t) capture_address, drp_max_freq, drpai_freq);
        if (0 > ret)
        {
            fprintf(stderr, "[ERROR] Failed to run DRPAI_START: errno=%d\n", errno);
            goto err;
        }
#else
        ret = 1;  /* YOLOX Skip*/
#endif

#if (0) == INF_YOLOX_SKIP
            ret = sem_getvalue(&terminate_req_sem, &inf_sem_check);
            if (0 != ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
                goto err;
            }
            /*Checks the semaphore value*/
            if (1 != inf_sem_check)
            {
                goto ai_inf_end;
            }
            
            /*Setup pselect settings*/
            FD_ZERO(&rfds0);
            FD_SET(drpai_fd0, &rfds0);
            tv.tv_sec = DRPAI_TIMEOUT;
            tv.tv_nsec = 0;

            /*Wait Till The DRP-AI Ends*/
            ret = pselect(drpai_fd0+1, &rfds0, NULL, NULL, &tv, NULL);
#endif

#if (0) == INF_YOLOX_SKIP
                /*Gets AI Inference End Time*/
                ret = timespec_get(&yolox_drp_end_time, TIME_UTC);
                if ( 0 == ret)
                {
                    fprintf(stderr, "[ERROR] Failed to Get Inference End Time\n");
                    goto err;
                }

                /*Checks if DRPAI Inference ended without issue*/
                inf_status = ioctl(drpai_fd0, DRPAI_GET_STATUS, &drpai_status0);
#endif

#if (0) == INF_YOLOX_SKIP
                    /*Process to read the DRPAI output data.*/
                    ret = get_result(drpai_fd0, drpai_data0.address, num_inf_out * sizeof(drpai_output_buf0[0]));
                    if (0 != ret)
                    {
                        fprintf(stderr, "[ERROR] Failed to get result from memory.\n");
                        goto err;
                    }

                    /* YOLOX R_Post_Proc time start*/
                    ret = timespec_get(&yolox_sta_time, TIME_UTC);
                    if (0 == ret)
                    {
                        fprintf(stderr, "[ERROR] Failed to get R_Post_Proc Start Time\n");
                        goto err;
                    }
                    /*Preparation For Post-Processing*/
                    bcount = 0;
                    det_res.clear();
                    /*CPU Post-Processing For YOLOX*/
                    R_Post_Proc(&drpai_output_buf0[0], det_res, &bcount);
                    /*Count the Number of People Detected*/
                    ppl_count_local = 0;
                    people_counter(det_res, det_ppl, bcount, &ppl_count_local);
                    /* YOLOX R_Post_Proc time end*/
                    ret = timespec_get(&yolox_end_time, TIME_UTC);
                    if (0 == ret)
                    {
                        fprintf(stderr, "[ERROR] Failed to get R_Post_Proc end Time\n");
                        goto err;
                    }
                    yolox_proc_time  = (timedifference_msec(yolox_sta_time, yolox_end_time) * TIME_COEF);
#else
                    ppl_count_local = 1;  /* YOLOX Skip*/
#endif
                    if(ppl_count_local > 0)
                    {
                        for(i = 0; i < ppl_count_local; i++)
                        {
#if (0) == INF_YOLOX_SKIP
                            croph[i] = det_ppl[i].bbox.h + CROP_ADJ_X;
                            cropw[i] = det_ppl[i].bbox.w + CROP_ADJ_Y;
#else
                            /* YOLOX Skip*/
                            croph[i] = CROPPED_IMAGE_HEIGHT;
                            cropw[i] = CROPPED_IMAGE_WIDTH;
#endif
                            /*Checks that cropping height and width does not exceeds image dimension*/
                            if(croph[i] < 1)
                            {
                                croph[i] = 1;
                            }
                            else if(croph[i] > IMREAD_IMAGE_HEIGHT)
                            {
                                croph[i] = IMREAD_IMAGE_HEIGHT;
                            }
                            else
                            {
                                /*Do Nothing*/
                            }
                            if(cropw[i] < 1)
                            {
                                cropw[i] = 1;
                            }
                            else if(cropw[i] > IMREAD_IMAGE_WIDTH)
                            {
                                cropw[i] = IMREAD_IMAGE_WIDTH;
                            }
                            else
                            {
                                /*Do Nothing*/
                            }
#if (0) == INF_YOLOX_SKIP
                            /*Compute Cropping Y Position based on Detection Result*/
                            /*If Negative Cropping Position*/
                            if(det_ppl[i].bbox.y < (croph[i]/2))
                            {
                                cropy[i] = 0;
                            }
                            else if(det_ppl[i].bbox.y > (IMREAD_IMAGE_HEIGHT-croph[i]/2)) /*If Exceeds Image Area*/
                            {
                                cropy[i] = IMREAD_IMAGE_HEIGHT-croph[i];
                            }
                            else
                            {
                                cropy[i] = (int16_t)det_ppl[i].bbox.y - croph[i]/2;
                            }
                            /*Compute Cropping X Position based on Detection Result*/
                            /*If Negative Cropping Position*/
                            if(det_ppl[i].bbox.x < (cropw[i]/2))
                            {
                                cropx[i] = 0;
                            }
                            else if(det_ppl[i].bbox.x > (IMREAD_IMAGE_WIDTH-cropw[i]/2)) /*If Exceeds Image Area*/
                            {
                                cropx[i] = IMREAD_IMAGE_WIDTH-cropw[i];
                            }
                            else
                            {
                                cropx[i] = (int16_t)det_ppl[i].bbox.x - cropw[i]/2;
                            }
#else
                            cropx[i] = OUTPUT_LEFT;
                            cropy[i] = 0;
#endif
                            /*Checks that combined cropping position with width and height does not exceed the image dimension*/
                            if(cropx[i] + cropw[i] > IMREAD_IMAGE_WIDTH)
                            {
                                cropw[i] = IMREAD_IMAGE_WIDTH - cropx[i];
                            }
                            if(cropy[i] + croph[i] > IMREAD_IMAGE_HEIGHT)
                            {
                                croph[i] = IMREAD_IMAGE_HEIGHT - cropy[i];
                            }       
                           
                            in_param.pre_in_addr    = (uintptr_t) capture_address;
                            /*Gets Pre-process starting time*/
                            ret = timespec_get(&pre_start_time, TIME_UTC);
                            if ( 0 == ret)
                            {
                                fprintf(stderr, "[ERROR] Failed to get Pre-process Start Time\n");
                                goto err;
                            }
                                   ret = preruntime.Pre(&in_param, &output_ptr, &out_size);
                            /*Gets AI Pre-process End Time*/
                            ret = timespec_get(&pre_end_time, TIME_UTC);
                            if ( 0 == ret)
                            {
                                fprintf(stderr, "[ERROR] Failed to Get Pre-process End Time\n");
                                goto err;
                            }
                            /*Set Pre-processing output to be inference input. */
                            runtime.SetInput(0, (float*)output_ptr);
                            /*Pre-process Time Result*/
                            pre_time = (timedifference_msec(pre_start_time, pre_end_time) * TIME_COEF);

                            /*Gets inference starting time*/
                            ret = timespec_get(&start_time, TIME_UTC);
                            if (0 == ret)
                            {
                                fprintf(stderr, "[ERROR] Failed to get Inference Start Time\n");
                                goto err;
                            }
                            runtime.Run();
                            /*Gets AI Inference End Time*/
			                ret = timespec_get(&inf_end_time, TIME_UTC);
                            /*Inference Time Result*/
                            ai_time = (timedifference_msec(start_time, inf_end_time) * TIME_COEF);

                            /*Gets Post-process starting time*/
                            ret = timespec_get(&post_start_time, TIME_UTC);
                            if (0 == ret)
                            {
                                fprintf(stderr, "[ERROR] Failed to get Post-process Start Time\n");
                                goto err;
                            }

                            /*Process to read the DRPAI output data.*/
                            ret = get_result();
                            if (0 != ret)
                            {
                                fprintf(stderr, "[ERROR] Failed to get result from memory.\n");
                                goto err;
                            }
                            /*Preparation for Post-Processing*/
                            /*CPU Post Processing For HRNet & Display the Results*/
                            R_Post_Proc_HRNet(&drpai_output_buf[0],i);

                            if(lowest_kpt_score_local[i] > 0)
                            {
                            R_HRNet_Coord_Convert(i);
                            }
                            /* R_Post_Proc time end*/
                            ret = timespec_get(&post_end_time, TIME_UTC);
                            if (0 == ret)
                            {
                            fprintf(stderr, "[ERROR] Failed to Get R_Post_Proc End Time\n");
                            goto err;
                            }
                            post_time = (timedifference_msec(post_start_time, post_end_time)*TIME_COEF);
                        }
                    }    
                    /*Copy data for Display Thread*/
	                ppl_count=0;
                    memcpy(lowest_kpt_score,lowest_kpt_score_local,sizeof(lowest_kpt_score_local));
                    memcpy(id_x, id_x_local, sizeof(id_x_local));
                    memcpy(id_y, id_y_local,sizeof(id_y_local));
                    ppl_count = ppl_count_local;


#if (0) == INF_YOLOX_SKIP
        /*Display Processing YOLOX Time On Log File*/
        yolox_drpai_time = (timedifference_msec(yolox_drp_start_time, yolox_drp_end_time) * TIME_COEF);
        spdlog::info("YOLOX");
        spdlog::info(" Pre-Proc + Inference Time (DRP-AI): {} [ms]", std::round(yolox_drpai_time * 10) / 10);
        spdlog::info(" Post-Proc Time (CPU): {} [ms]", std::round(yolox_proc_time * 10) / 10);
#endif

        /*Display Processing Time On Log File*/
        //drpai_time = timedifference_msec(start_time, inf_end_time) * TIME_COEF;
        int idx = inf_cnt % SIZE_OF_ARRAY(array_drp_time);
       // ai_time = (uint32_t)((timedifference_msec(start_time, inf_end_time) * TIME_COEF));
        uint32_t total_time = ai_time + pre_time + post_time;
        array_drp_time[idx] = ai_time;
        drp_prev_time = inf_end_time;
        spdlog::info("Total AI Time: {} [ms]", std::round(total_time * 10) / 10);
        spdlog::info("Inference: {} [ms]", std::round(ai_time * 10) / 10);
        spdlog::info("PreProcess: {} [ms]", std::round(pre_time * 10) / 10);
        spdlog::info("PostProcess: {} [ms]", std::round(post_time * 10) / 10);
        
        /*Display Processing Frame Rate On Log File*/
        /*ai_time = (uint32_t)((timedifference_msec(drp_prev_time, inf_end_time) * TIME_COEF));
        int idx = inf_cnt % SIZE_OF_ARRAY(array_drp_time);
        array_drp_time[idx] = ai_time;
        drp_prev_time = inf_end_time; */        
#ifdef DISP_AI_FRAME_RATE
        int arraySum = std::accumulate(array_drp_time, array_drp_time + SIZE_OF_ARRAY(array_drp_time), 0);
        double arrayAvg = 1.0 * arraySum / SIZE_OF_ARRAY(array_drp_time);
        ai_fps = 1.0 / arrayAvg * 1000.0 + 0.5;
        spdlog::info("AI Frame Rate {} [fps]", (int32_t)ai_fps);
#endif /* DISP_AI_FRAME_RATE */

        inference_start.store(0);
    }
    /*End of Inference Loop*/

/*Error Processing*/
err:
    /*Set Termination Request Semaphore to 0*/
    sem_trywait(&terminate_req_sem);
    goto ai_inf_end;
/*AI Thread Termination*/
ai_inf_end:
    /*To terminate the loop in Capture Thread.*/
    printf("AI Inference Thread Terminated\n");
    pthread_exit(NULL);
}

/*****************************************
* Function Name : R_Capture_Thread
* Description   : Executes the V4L2 capture with Capture thread.
* Arguments     : threadid = thread identification
* Return value  : -
******************************************/
void *R_Capture_Thread(void *threadid)
{
    Camera* capture = (Camera*) threadid;
    /*Semaphore Variable*/
    int32_t capture_sem_check = 0;
    /*First Loop Flag*/
    uint64_t capture_addr = 0;
    int8_t ret = 0;
    uint8_t * img_buffer;
    uint8_t * img_buffer0;
    uint8_t capture_stabe_cnt = 8;  // Counter to wait for the camera to stabilize
    int32_t cap_cnt = -1;
#ifdef DISP_AI_FRAME_RATE
    static struct timespec capture_time;
    static struct timespec capture_time_prev = { .tv_sec = 0, .tv_nsec = 0, };
#endif /* DISP_AI_FRAME_RATE */

#if (0) == INPUT_CAM_TYPE
    double elapsed_time_last_disp = 0;
    double target_disp_fps = 15.0;
#endif

    printf("Capture Thread Starting\n");

    img_buffer0 = (uint8_t *)capture->drpai_buf->mem;
    if (MAP_FAILED == img_buffer0)
    {
        fprintf(stderr, "[ERROR] Failed to mmap\n");
        goto err;
    }
#if (1) == DRPAI_INPUT_PADDING
    /** Fill buffer with the brightness 114. */
    for( uint32_t i = 0; i < CAM_IMAGE_WIDTH * CAM_IMAGE_WIDTH * CAM_IMAGE_CHANNEL_YUY2; i += 4 )
    {
        /// Y =  0.299R + 0.587G + 0.114B
        img_buffer0[i]   = 114;    
        img_buffer0[i+2] = 114;
        /// U = -0.169R - 0.331G + 0.500B + 128
        img_buffer0[i+1] = 128;
        /// V =  0.500R - 0.419G - 0.081B + 128
        img_buffer0[i+3] = 128;
    }
#endif  /* (1) == DRPAI_INPUT_PADDING */
    capture_address = capture->drpai_buf->phy_addr;

    while(1)
    {
        /*Gets the Termination request semaphore value. If different then 1 Termination was requested*/
        /*Checks if sem_getvalue is executed wihtout issue*/
        errno = 0;
        ret = sem_getvalue(&terminate_req_sem, &capture_sem_check);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
            goto err;
        }
        /*Checks the semaphore value*/
        if (1 != capture_sem_check)
        {
            goto capture_end;
        }

        /* Capture USB camera image and stop updating the capture buffer */
        capture_addr = (uint32_t)capture->capture_image();

#ifdef DISP_AI_FRAME_RATE
        cap_cnt++;
        ret = timespec_get(&capture_time, TIME_UTC);
        proc_time_capture = (timedifference_msec(capture_time_prev, capture_time) * TIME_COEF);
        capture_time_prev = capture_time;

        int idx = cap_cnt % SIZE_OF_ARRAY(array_cap_time);
        array_cap_time[idx] = (uint32_t)proc_time_capture;
        int arraySum = std::accumulate(array_cap_time, array_cap_time + SIZE_OF_ARRAY(array_cap_time), 0);
        double arrayAvg = 1.0 * arraySum / SIZE_OF_ARRAY(array_cap_time);
        cap_fps = 1.0 / arrayAvg * 1000.0 + 0.5;
#endif /* DISP_AI_FRAME_RATE */

        if (capture_addr == 0)
        {
            fprintf(stderr, "[ERROR] Failed to capture image from camera.\n");
            goto err;
        }
        else
        {
            /* Do not process until the camera stabilizes, because the image is unreliable until the camera stabilizes. */
            if( capture_stabe_cnt > 0 )
            {
                capture_stabe_cnt--;
            }
            else
            {
                img_buffer = capture->get_img();
                if (!inference_start.load())
                {
                    /* Copy captured image to Image object. This will be used in Display Thread. */
                    memcpy(img_buffer0, img_buffer, capture->get_size());
                    /* Flush capture image area cache */
                    ret = capture->video_buffer_flush_dmabuf(capture->drpai_buf->idx, capture->drpai_buf->size);
                    if (0 != ret)
                    {
                        goto err;
                    }
                    inference_start.store(1); /* Flag for AI Inference Thread. */
                }

                if (!img_obj_ready.load())
                {
                    img.camera_to_image(img_buffer, capture->get_size());
                    ret = capture->video_buffer_flush_dmabuf(capture->wayland_buf->idx, capture->wayland_buf->size);
                    if (0 != ret)
                    {
                        goto err;
                    }
                    img_obj_ready.store(1); /* Flag for Display Thread. */
                }
            }
        }

        /* IMPORTANT: Place back the image buffer to the capture queue */
        ret = capture->capture_qbuf();
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to enqueue capture buffer.\n");
            goto err;
        }
    } /*End of Loop*/

/*Error Processing*/
err:
    sem_trywait(&terminate_req_sem);
    goto capture_end;

capture_end:
    /*To terminate the loop in AI Inference Thread.*/
    inference_start.store(1);

    printf("Capture Thread Terminated\n");
    pthread_exit(NULL);
}

/*****************************************
* Function Name : R_Img_Thread
* Description   : Executes img proc with img thread
* Arguments     : threadid = thread identification
* Return value  : -
******************************************/
void *R_Img_Thread(void *threadid)
{
    /*Semaphore Variable*/
    int32_t hdmi_sem_check = 0;
    /*Variable for checking return value*/
    int8_t ret = 0;
    double img_proc_time = 0;
    bool padding = false;
#ifdef CAM_INPUT_VGA
    padding = true;
#endif // CAM_INPUT_VGA
    timespec start_time;
    timespec end_time;

    printf("Image Thread Starting\n");
    while(1)
    {
        /*Gets The Termination Request Semaphore Value, If Different Then 1 Termination Is Requested*/
        /*Checks If sem_getvalue Is Executed Without Issue*/
        errno = 0;
        ret = sem_getvalue(&terminate_req_sem, &hdmi_sem_check);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
            goto err;
        }
        /*Checks the semaphore value*/
        if (1 != hdmi_sem_check)
        {
            goto hdmi_end;
        }
        /* Check img_obj_ready flag which is set in Capture Thread. */
        if (img_obj_ready.load())
        {
            ret = timespec_get(&start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Display Start Time\n");
                goto err;
            }
            
            /* Draw Complete Skeleton. */
            draw_skeleton();

            /* Convert YUYV image to BGRA format. */
            img.convert_format();

            /* Convert output image size. */
            img.convert_size(CAM_IMAGE_WIDTH, DRPAI_OUT_WIDTH, padding);

#if (0) == INF_YOLOX_SKIP
            /* Draw bounding box on image. */
            draw_bounding_box();
#endif            
            /*displays AI Inference Results on display.*/
            print_result(&img);

            buf_id = img.get_buf_id();
            img_obj_ready.store(0);

            if (!hdmi_obj_ready.load())
            {
                hdmi_obj_ready.store(1); /* Flag for AI Inference Thread. */
            }
            
            ret = timespec_get(&end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to Get Display End Time\n");
                goto err;
            }
            img_proc_time = (timedifference_msec(start_time, end_time) * TIME_COEF);
            
#ifdef DEBUG_TIME_FLG
            printf("Img Proc Time             : %lf[ms]\n", img_proc_time);
#endif
        }
        usleep(WAIT_TIME); //wait 1 tick time
    } /*End Of Loop*/

/*Error Processing*/
err:
    /*Set Termination Request Semaphore To 0*/
    sem_trywait(&terminate_req_sem);
    goto hdmi_end;

hdmi_end:
    /*To terminate the loop in Capture Thread.*/
    img_obj_ready.store(0);
    printf("Img Thread Terminated\n");
    pthread_exit(NULL);
}
/*****************************************
* Function Name : R_Display_Thread
* Description   : Executes the HDMI Display with Display thread
* Arguments     : threadid = thread identification
* Return value  : -
******************************************/
void *R_Display_Thread(void *threadid)
{
    /*Semaphore Variable*/
    int32_t hdmi_sem_check = 0;
    /*Variable for checking return value*/
    int8_t ret = 0;
    double disp_proc_time = 0;
    int32_t disp_cnt = 0;

    timespec start_time;
    timespec end_time;
    static struct timespec disp_prev_time = { .tv_sec = 0, .tv_nsec = 0, };

    /* Initialize waylad */
    ret = wayland.init(capture->wayland_buf->idx, IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, IMAGE_CHANNEL_BGRA);
    if(0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to initialize Image for Wayland\n");
        goto err;
    }

    printf("Display Thread Starting\n");
    while(1)
    {
        /*Gets The Termination Request Semaphore Value, If Different Then 1 Termination Is Requested*/
        /*Checks If sem_getvalue Is Executed Without Issue*/
        errno = 0;
        ret = sem_getvalue(&terminate_req_sem, &hdmi_sem_check);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
            goto err;
        }
        /*Checks the semaphore value*/
        if (1 != hdmi_sem_check)
        {
            goto hdmi_end;
        }
        /* Check hdmi_obj_ready flag which is set in Capture Thread. */
        if (hdmi_obj_ready.load())
        {
            ret = timespec_get(&start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Display Start Time\n");
                goto err;
            }
            /*Update Wayland*/
            wayland.commit(img.get_img(buf_id), NULL);

            hdmi_obj_ready.store(0);
            ret = timespec_get(&end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to Get Display End Time\n");
                goto err;
            }
            disp_proc_time = (timedifference_msec(start_time, end_time) * TIME_COEF);
            disp_time = (uint32_t)((timedifference_msec(disp_prev_time, end_time) * TIME_COEF));
            int idx = disp_cnt++ % SIZE_OF_ARRAY(array_disp_time);
            array_disp_time[idx] = disp_time;
            disp_prev_time = end_time;
#ifdef DEBUG_TIME_FLG
            /* Draw Disp Frame Rate on RGB image.*/
            int arraySum = std::accumulate(array_disp_time, array_disp_time + SIZE_OF_ARRAY(array_disp_time), 0);
            double arrayAvg = 1.0 * arraySum / SIZE_OF_ARRAY(array_disp_time);
            double disp_fps = 1.0 / arrayAvg * 1000.0;

            printf("Disp Proc Time            : %lf[ms]\n", disp_proc_time);
            printf("Disp Frame Rate           : %lf[fps]\n", disp_fps);
            printf("Dipslay ------------------------------ No. %d\n", disp_cnt);
#endif
        }
        usleep(WAIT_TIME); //wait 1 tick time
    } /*End Of Loop*/

/*Error Processing*/
err:
    /*Set Termination Request Semaphore To 0*/
    sem_trywait(&terminate_req_sem);
    goto hdmi_end;

hdmi_end:
    /*To terminate the loop in Capture Thread.*/
    hdmi_obj_ready.store(0);
    printf("Display Thread Terminated\n");
    pthread_exit(NULL);
}

/*****************************************
* Function Name : R_Kbhit_Thread
* Description   : Executes the Keyboard hit thread (checks if enter key is hit)
* Arguments     : threadid = thread identification
* Return value  : -
******************************************/
void *R_Kbhit_Thread(void *threadid)
{
    /*Semaphore Variable*/
    int32_t kh_sem_check = 0;
    /*Variable to store the getchar() value*/
    int32_t c = 0;
    /*Variable for checking return value*/
    int8_t ret = 0;

    printf("Key Hit Thread Starting\n");

    printf("************************************************\n");
    printf("* Press ENTER key to quit. *\n");
    printf("************************************************\n");

    /*Set Standard Input to Non Blocking*/
    errno = 0;
    ret = fcntl(0, F_SETFL, O_NONBLOCK);
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] Failed to run fctnl(): errno=%d\n", errno);
        goto err;
    }

    while(1)
    {
        /*Gets the Termination request semaphore value. If different then 1 Termination was requested*/
        /*Checks if sem_getvalue is executed wihtout issue*/
        errno = 0;
        ret = sem_getvalue(&terminate_req_sem, &kh_sem_check);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
            goto err;
        }
        /*Checks the semaphore value*/
        if (1 != kh_sem_check)
        {
            goto key_hit_end;
        }

        c = getchar();
        if (EOF != c)
        {
            /* When key is pressed. */
            printf("key Detected.\n");
            goto err;
        }
        else
        {
            /* When nothing is pressed. */
            usleep(WAIT_TIME);
        }
    }

/*Error Processing*/
err:
    /*Set Termination Request Semaphore to 0*/
    sem_trywait(&terminate_req_sem);
    goto key_hit_end;

key_hit_end:
    printf("Key Hit Thread Terminated\n");
    pthread_exit(NULL);
}

/*****************************************
* Function Name : R_Main_Process
* Description   : Runs the main process loop
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t R_Main_Process()
{
    /*Main Process Variables*/
    int8_t main_ret = 0;
    /*Semaphore Related*/
    int32_t sem_check = 0;
    /*Variable for checking return value*/
    int8_t ret = 0;

    printf("Main Loop Starts\n");
    while(1)
    {
        /*Gets the Termination request semaphore value. If different then 1 Termination was requested*/
        errno = 0;
        ret = sem_getvalue(&terminate_req_sem, &sem_check);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
            goto err;
        }
        /*Checks the semaphore value*/
        if (1 != sem_check)
        {
            goto main_proc_end;
        }
        /*Wait for 1 TICK.*/
        usleep(WAIT_TIME);
    }

/*Error Processing*/
err:
    sem_trywait(&terminate_req_sem);
    main_ret = 1;
    goto main_proc_end;
/*Main Processing Termination*/
main_proc_end:
    printf("Main Process Terminated\n");
    return main_ret;
}
/*****************************************
* Function Name : get_drpai_start_addr
* Description   : Function to get the start address of DRPAImem.
* Arguments     : drpai_fd: DRP-AI file descriptor
* Return value  : If non-zero, DRP-AI memory start address.
*                 0 is failure.
******************************************/
#ifdef V2H
uint64_t get_drpai_start_addr(int drpai_fd)
#else
uint32_t get_drpai_start_addr(int drpai_fd)
#endif
{
    int ret = 0;
    drpai_data_t drpai_data;

    errno = 0;

    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(drpai_fd , DRPAI_GET_DRPAI_AREA, &drpai_data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to get DRP-AI Memory Area : errno=" << errno << std::endl;
        return 0;
    }

    return drpai_data.address;
}

/*****************************************
* Function Name : set_drpai_freq
* Description   : Function to set the DRP and DRP-AI frequency.
* Arguments     : drpai_fd: DRP-AI file descriptor
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int set_drpai_freq(int drpai_fd)
{
    int ret = 0;
    uint32_t data;

    errno = 0;
    data = drp_max_freq;
    ret = ioctl(drpai_fd , DRPAI_SET_DRP_MAX_FREQ, &data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to set DRP Max Frequency : errno=" << errno << std::endl;
        return -1;
    }

    errno = 0;
    data = drpai_freq;
    ret = ioctl(drpai_fd , DRPAI_SET_DRPAI_FREQ, &data);
    if (-1 == ret)
    {
        std::cerr << "[ERROR] Failed to set DRP-AI Frequency : errno=" << errno << std::endl;
        return -1;
    }
    return 0;
}

/*****************************************
* Function Name : init_drpai
* Description   : Function to initialize DRP-AI.
* Arguments     : drpai_fd: DRP-AI file descriptor
* Return value  : If non-zero, DRP-AI memory start address.
*                 0 is failure.
******************************************/
#ifdef V2H
uint64_t init_drpai(int drpai_fd)
#else
uint32_t init_drpai(int drpai_fd)
#endif
{
    int ret = 0;
#ifdef V2H
    uint64_t drpai_addr = 0;
#else
    uint32_t drpai_addr = 0;
#endif

    /*Get DRP-AI memory start address*/
    drpai_addr = get_drpai_start_addr(drpai_fd);
    if (drpai_addr == 0)
    {
        return 0;
    }

    /*Set DRP-AI frequency*/
    ret = set_drpai_freq(drpai_fd);
    if (ret != 0)
    {
        return 0;
    }

    return drpai_addr;
}

int32_t main(int32_t argc, char * argv[])
{
    /* Log File Setting */
    auto now = std::chrono::system_clock::now();
    auto tm_time = spdlog::details::os::localtime(std::chrono::system_clock::to_time_t(now));
    char date_buf[64];
    char time_buf[128];
    memset(time_buf,0,sizeof(time_buf));
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d_%H-%M-%S", &tm_time);
    sprintf(time_buf,"logs/%s_app_hrnet_yolox_cam.log",date_buf);
    auto logger = spdlog::basic_logger_mt("logger", time_buf);
    spdlog::set_default_logger(logger);

    /* DRP-AI Frequency Setting */
    if (2 <= argc)
    {
        drp_max_freq = atoi(argv[1]);
    }
    else
    {
        drp_max_freq = DRP_MAX_FREQ;
    }
    if (3 <= argc)
    {
        drpai_freq = atoi(argv[2]);
    }
    else
    {
        drpai_freq = DRPAI_FREQ;
    }

    int8_t main_proc = 0;
    int8_t ret = 0;
    int8_t ret_main = 0;
    /*Multithreading Variables*/
    int32_t create_thread_ai = -1;
    int32_t create_thread_key = -1;
    int32_t create_thread_capture = -1;
    int32_t create_thread_img = -1;
    int32_t create_thread_hdmi = -1;
    int32_t sem_create = -1;
    InOutDataType input_data_type;
    bool runtime_status = false;

    printf("RZ/V2H DRP-AI Sample Application\n");
    printf("Model : MMPose HRNet | %s  \n", model_dir.c_str());
#if (0) == INF_YOLOX_SKIP
    printf("Model : Megvii-Base Detection YOLOX | %s\n", AI0_DESC_NAME);
#endif
    printf("Input : %s\n", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    spdlog::info("  RZ/V2H DRP-AI Sample Application");
    spdlog::info("  Model : MMPose HRNet | {} ", model_dir.c_str());
    spdlog::info("  Input : {}", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    printf("Argument : <DRP0_max_freq_factor> = %d\n", drp_max_freq);
    printf("Argument : <AI-MAC_freq_factor> = %d\n", drpai_freq);
    
    uint64_t drpaimem_addr_start = 0;
    
    errno = 0;
#if (0) == INF_YOLOX_SKIP
    drpai_fd0 = open("/dev/drpai0", O_RDWR);
    if (0 > drpai_fd0)
    {
        fprintf(stderr, "[ERROR] Failed to open DRP-AI Driver: errno=%d\n", errno);
        return -1;
    }
#endif
    int drpai_fd = open("/dev/drpai0", O_RDWR);
    if (0 > drpai_fd)
    {
        fprintf(stderr, "[ERROR] Failed to open DRP-AI Driver : errno=%d\n", errno);
        goto end_main;
    }
    
    /*Initialzie DRP-AI (Get DRP-AI memory address and set DRP-AI frequency)*/
    drpaimem_addr_start = init_drpai(drpai_fd);
    if (drpaimem_addr_start == 0)
    {
        goto end_close_drpai;
    }
#if (0) == INF_YOLOX_SKIP
    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(drpai_fd0, DRPAI_GET_DRPAI_AREA, &drpai_data0);	
    if (ret == -1)
    {
        fprintf(stderr, "[ERROR] Failed to get DRP-AI Memory Area: errno=%d\n", errno);
        ret_main = -1;
        goto end_close_drpai;
    }

    /* Load DRP-AI Data from Filesystem to Memory via DRP-AI Driver */
    /* YOLOX */
    drpai_hdl0 = load_drpai_obj_dynamic(drpai_fd0, AI0_DESC_NAME, drpai_data0.address);
    if (NULL == drpai_hdl0)
    {
        fprintf(stderr, "[ERROR] Failed to load DRP-AI Data\n");
        ret_main = -1;
        goto end_close_drpai;
    }
#endif

#if (0) == INF_YOLOX_SKIP
    drpai_data1.address = (drpai_hdl0->data_inout.start_address + drpai_hdl0->data_inout.object_files_size + 0x1000000) & 0xFFFFFFFFFF000000;
#endif
    /*Load pre_dir object to DRP-AI */
    ret = preruntime.Load(pre_dir);
    if (0 < ret)
    {
        fprintf(stderr, "[ERROR] Failed to run Pre-processing Runtime Load().\n");
        goto end_close_drpai;
    }

    runtime_status = runtime.LoadModel(model_dir, drpaimem_addr_start);

    if(!runtime_status)
    {
        fprintf(stderr, "[ERROR] Failed to load model.\n");
        goto end_close_drpai;
    }
    /*Get input data */
    input_data_type = runtime.GetInputDataType(0);
    if (InOutDataType::FLOAT32 == input_data_type)
    {
        /*Do nothing*/
    }
    else if (InOutDataType::FLOAT16 == input_data_type)
    {
        fprintf(stderr, "[ERROR] Input data type : FP16.\n");
        /*If your model input data type is FP16, use std::vector<uint16_t> for reading input data. */
        goto end_close_drpai;
    }
    else
    {
        fprintf(stderr, "[ERROR] Input data type : neither FP32 nor FP16.\n");
        goto end_close_drpai;
    }


    /* Create Camera Instance */
    capture = new Camera();

    /* Init and Start Camera */
    ret = capture->start_camera();
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to initialize Camera.\n");
        delete capture;
        ret_main = ret;
        goto end_main;
    }

    /*Initialize Image object.*/
    ret = img.init(CAM_IMAGE_WIDTH, CAM_IMAGE_HEIGHT, CAM_IMAGE_CHANNEL_YUY2, IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, IMAGE_CHANNEL_BGRA, capture->wayland_buf->mem);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to initialize Image object.\n");
        ret_main = ret;
        goto end_close_camera;
    }
    
    /*Termination Request Semaphore Initialization*/
    /*Initialized value at 1.*/
    sem_create = sem_init(&terminate_req_sem, 0, 1);
    if (0 != sem_create)
    {
        fprintf(stderr, "[ERROR] Failed to Initialize Termination Request Semaphore.\n");
        ret_main = -1;
        goto end_threads;
    }

    /*Create Key Hit Thread*/
    create_thread_key = pthread_create(&kbhit_thread, NULL, R_Kbhit_Thread, NULL);
    if (0 != create_thread_key)
    {
        fprintf(stderr, "[ERROR] Failed to create Key Hit Thread.\n");
        ret_main = -1;
        goto end_threads;
    }

    /*Create Inference Thread*/
    create_thread_ai = pthread_create(&ai_inf_thread, NULL, R_Inf_Thread, NULL);
    if (0 != create_thread_ai)
    {
        sem_trywait(&terminate_req_sem);
        fprintf(stderr, "[ERROR] Failed to create AI Inference Thread.\n");
        ret_main = -1;
        goto end_threads;
    }

    /*Create Capture Thread*/
    create_thread_capture = pthread_create(&capture_thread, NULL, R_Capture_Thread, (void *) capture);
    if (0 != create_thread_capture)
    {
        sem_trywait(&terminate_req_sem);
        fprintf(stderr, "[ERROR] Failed to create Capture Thread.\n");
        ret_main = -1;
        goto end_threads;
    }

    /*Create Image Thread*/
    create_thread_img = pthread_create(&img_thread, NULL, R_Img_Thread, NULL);
    if(0 != create_thread_img)
    {
        sem_trywait(&terminate_req_sem);
        fprintf(stderr, "[ERROR] Failed to create Image Thread.\n");
        ret_main = -1;
        goto end_threads;
    }

	/*Create Display Thread*/
    create_thread_hdmi = pthread_create(&hdmi_thread, NULL, R_Display_Thread, NULL);
    if(0 != create_thread_hdmi)
    {
        sem_trywait(&terminate_req_sem);
        fprintf(stderr, "[ERROR] Failed to create Display Thread.\n");
        ret_main = -1;
        goto end_threads;
    }

    /*Main Processing*/
    main_proc = R_Main_Process();
    if (0 != main_proc)
    {
        fprintf(stderr, "[ERROR] Error during Main Process\n");
        ret_main = -1;
    }
    goto end_threads;

end_threads:
    if(0 == create_thread_hdmi)
    {
        ret = wait_join(&hdmi_thread, DISPLAY_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit Display Thread on time.\n");
            ret_main = -1;
        }
    }
    if(0 == create_thread_img)
    {
        ret = wait_join(&img_thread, DISPLAY_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit Image Thread on time.\n");
            ret_main = -1;
        }
    }
    if (0 == create_thread_capture)
    {
        ret = wait_join(&capture_thread, CAPTURE_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit Capture Thread on time.\n");
            ret_main = -1;
        }
    }
    if (0 == create_thread_ai)
    {
        ret = wait_join(&ai_inf_thread, AI_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit AI Inference Thread on time.\n");
            ret_main = -1;
        }
    }
    if (0 == create_thread_key)
    {
        ret = wait_join(&kbhit_thread, KEY_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit Key Hit Thread on time.\n");
            ret_main = -1;
        }
    }

    /*Delete Terminate Request Semaphore.*/
    if (0 == sem_create)
    {
        sem_destroy(&terminate_req_sem);
    }

    /* Exit waylad */
    wayland.exit();
    goto end_close_camera;

end_close_camera:
    /*Close USB Camera.*/
    ret = capture->close_camera();
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to close Camera.\n");
        ret_main = -1;
    }
    delete capture;
    goto end_close_drpai;

end_close_drpai:
    if (0 < drpai_fd)
    {
        errno = 0;
        ret = close(drpai_fd);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to close DRP-AI Driver: errno=%d\n", errno);
            ret_main = -1;
        }
    }
    goto end_main;

end_main:
    printf("Application End\n");
    return ret_main;
}
