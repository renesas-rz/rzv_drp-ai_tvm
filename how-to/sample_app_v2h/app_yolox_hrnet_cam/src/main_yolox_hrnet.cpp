/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : main.cpp
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for Megvii-Base Detection YOLOX and MMPose HRNet with MIPI/USB Camera
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
#include "define_color_yolox_hrnet.h"
/*Post process control*/
#include "post_proc_yolox_hrnet.h"
/*USB camera control*/
#include "camera.h"
/*Image control*/
#include "image_yolox_hrnet.h"
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
static atomic<uint8_t> hdmi_obj_ready  (0);

/*Global Variables*/
#if (0) == INF_YOLOX_SKIP
static float drpai_output_buf0[num_inf_out];
#endif
static float drpai_output_buf[INF_OUT_SIZE];
static uint8_t buf_id;
static Image img;
cv::Mat g_frame;

/*AI Inference for DRPAI*/
#if (0) == INF_YOLOX_SKIP
/* Pre-processing Runtime object */
PreRuntime preruntime;
/* DRP-AI TVM[*1] Runtime object */
MeraDrpRuntimeWrapper runtime;
#endif
MeraDrpRuntimeWrapper runtime_2;

#ifdef DISP_AI_FRAME_RATE
static double ai_fps = 0;
static double cap_fps = 0;
static double proc_time_capture = 0;
static uint32_t array_cap_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
#endif /* DISP_AI_FRAME_RATE */
static uint32_t disp_time = 0;
static uint32_t array_drp_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
static uint32_t array_disp_time[30] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
static int32_t drp_max_freq;
static int32_t drpai_freq;
static double array_hrnet_drpai_time[NUM_MAX_PERSON];
static double array_hrnet_proc_time[NUM_MAX_PERSON];
static float hrnet_preds[NUM_OUTPUT_C][3];
static uint16_t id_x[NUM_OUTPUT_C][NUM_MAX_PERSON];
static uint16_t id_y[NUM_OUTPUT_C][NUM_MAX_PERSON];

static int16_t cropx[NUM_MAX_PERSON];
static int16_t cropy[NUM_MAX_PERSON];
static int16_t croph[NUM_MAX_PERSON];
static int16_t cropw[NUM_MAX_PERSON];
static float lowest_kpt_score[NUM_MAX_PERSON];

/*YOLOX*/
static uint32_t ppl_count = 0;
static vector<detection> det_res;

#if END_DET_TYPE
static int8_t display_state=0;
#endif

static Wayland wayland;
static PostProc post_proc;
static vector<detection> det;
static Camera* capture = NULL;

#if (0) == INF_YOLOX_SKIP
static double yolox_pre_time = 0;
static double yolox_post_time = 0;
static double yolox_ai_time = 0;
#endif

static double pre_time = 0;
static double post_time = 0;
static double ai_time = 0;

bool delay_frame_inserted = false;

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
    if (0 == ret_err)
    {
        join_timeout.tv_sec += join_time;
        ret_err = pthread_timedjoin_np(*p_join_thread, NULL, &join_timeout);
    }
    return ret_err;
}

/*****************************************
* Function Name : get_result
* Description   : Get DRP-AI Output from memory via DRP-AI Driver
* Arguments     : out = array of output buffer
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t get_result(MeraDrpRuntimeWrapper& rt, float* out)
{
    int8_t ret = 0;
    int32_t i = 0;
    int32_t output_num = 0;
    std::tuple<InOutDataType, void*, int64_t> output_buffer;
    int64_t output_size;
    uint32_t size_count = 0;

    /* Get the number of output of the target model. */
    output_num = rt.GetNumOutput();

    size_count = 0;
    /*GetOutput loop*/
    for (i = 0; i<output_num; i++)
    {
        /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
        output_buffer = rt.GetOutput(i);

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
                out[j + size_count]=float16_to_float32(data_ptr[j]);
            }
        }
        else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
        {
            /*Output Data = std::get<1>(output_buffer)*/
            float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
            for (int j = 0; j<output_size; j++)
            {
                out[j + size_count]=data_ptr[j];
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
* Function Name : R_Pre_Proc_HRNet()
* Description   : Process CPU pre-processing for HRNet
* Arguments     : original_frame = input image (RGB)
*                 crop_x, crop_y, crop_w, crop_h = crop area
* Return value  : pre-processed image
******************************************/
static cv::Mat R_Pre_Proc_HRNet(cv::Mat original_frame, int16_t crop_x, int16_t crop_y, int16_t crop_w, int16_t crop_h)
{
    /* crop */
    cv::Rect roi(crop_x, crop_y, crop_w, crop_h);
    cv::Mat cropped_image = original_frame(roi);

    /* resize */
    cv::Mat resized_image;
    cv::resize(cropped_image, resized_image, cv::Size(NUM_INPUT_W, NUM_INPUT_H));

    /* normalising pixels */
    cv::Mat normalized_image;
    resized_image.convertTo(resized_image, CV_32F, 1.0 / 255.0);

    cv::Scalar mean(0.485, 0.456, 0.406);
    cv::Scalar stddev(0.229, 0.224, 0.225);
    cv::subtract(resized_image, mean, normalized_image);
    cv::divide(normalized_image, stddev, normalized_image);

    /* changing channel from hwc to chw */
    vector<cv::Mat> rgb_images;
    cv::split(normalized_image, rgb_images);
    cv::Mat m_flat_r = rgb_images[0].reshape(1, 1);
    cv::Mat m_flat_g = rgb_images[1].reshape(1, 1);
    cv::Mat m_flat_b = rgb_images[2].reshape(1, 1);
    cv::Mat matArray[] = {m_flat_r, m_flat_g, m_flat_b};
    cv::Mat frameCHW;
    cv::hconcat(matArray, 3, frameCHW);

    return frameCHW;
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
    float thre_kpt = TH_KPT;
    uint16_t id_x_local[NUM_OUTPUT_C][NUM_MAX_PERSON];
    uint16_t id_y_local[NUM_OUTPUT_C][NUM_MAX_PERSON];
#if (1) == INF_YOLOX_SKIP
    thre_kpt = TH_KPT_YOLOX_SKIP;
    int16_t crop_x;
    int16_t crop_y;
    int16_t crop_w;
    int16_t crop_h;
#endif

    mtx.lock();
    memcpy(id_x_local, id_x, sizeof(id_x));
    memcpy(id_y_local, id_y, sizeof(id_y));
    mtx.unlock();

#if (1) == INF_YOLOX_SKIP
    mtx.lock();
    crop_x = cropx[0];
    crop_y = cropy[0];
    crop_w = cropw[0];
    crop_h = croph[0];
    mtx.unlock();
    
    i = 0;
    Point cpos1 = img.convertPoint(crop_x, crop_y);
    Point cpos2 = img.convertPoint(crop_x + crop_w, crop_y + crop_h);
    cv::Point point1(cpos1.x, cpos1.y+2); 
    cv::Point point2(cpos2.x, cpos2.y-3);
    img.draw_rect_hrnet(point1, point2, cv::Scalar(0, 255, 255, 255), 4);
#endif

    for (i = 0; i < ppl_count; i++)
    {    
        /* Check If All Key Points Were Detected: If Over Threshold, It will Draw Complete Skeleton */
        if (lowest_kpt_score[i] > thre_kpt)
        {
            /* Draw limb */
            for (sk_id = 0; sk_id < NUM_LIMB; sk_id++)
            {
                uint8_t sk[] = {skeleton[sk_id][0], skeleton[sk_id][1]};
                int pos1[] = {id_x_local[sk[0]][i], id_y_local[sk[0]][i]};
                int pos2[] = {id_x_local[sk[1]][i], id_y_local[sk[1]][i]};
                
                if ((0 < pos1[0]) && (MIPI_WIDTH > pos1[0])
                    && (0 < pos1[1]) && (MIPI_WIDTH > pos1[1]))
                {
                    if ((0 < pos2[0]) && (MIPI_WIDTH > pos2[0])
                        && (0 < pos2[1]) && (MIPI_WIDTH > pos2[1]))
                    {
                        Point cpos1 = img.convertPoint(pos1[0], pos1[1]);
                        Point cpos2 = img.convertPoint(pos2[0], pos2[1]);
                        cv::Point point1(cpos1.x, cpos1.y);  // Example coordinates
                        cv::Point point2(cpos2.x, cpos2.y);
                        img.draw_line_hrnet(point1, point2, cv::Scalar(0, 255, 255, 255), 4);
                    }
                }
            }
    
            /* Draw Rectangle As Key Points */
            for (v = 0; v < NUM_OUTPUT_C; v++)
            {
                /* Draw Rectangles On Each Skeleton Key Points */
                Point cdraw_p = img.convertPoint(id_x_local[v][i], id_y_local[v][i]);
                cv::Point point_node_bl(cdraw_p.x - 4, cdraw_p.y - 4);
                cv::Point point_node_tr(cdraw_p.x + 4, cdraw_p.y + 4);
                img.draw_rect_hrnet(point_node_bl, point_node_tr, cv::Scalar(0, 0, 255, 255), cv::FILLED);
            }
        }
    }
    return;
}

/*****************************************
* Function Name : draw_bounding_box
* Description   : Draw bounding box on image.
* Arguments     : -
* Return value  : -
******************************************/
void draw_bounding_box(void)
{
    vector<detection> det_buff;
    stringstream stream;
    string result_str;
    uint32_t color = 0;
 
    mtx.lock();
    copy(det_res.begin(), det_res.end(), back_inserter(det_buff));
    mtx.unlock();

    /* Draw bounding box on RGB image. */
    for (uint32_t i = 0; i < det_buff.size(); i++)
    {
        /* Skip the overlapped bounding boxes */
        if (det_buff[i].prob == 0) continue;
        
        color = box_color[det_buff[i].c];
        /* Clear string stream for bounding box labels */
        stream.str("");
        /* Draw the bounding box on the image */
        stream << fixed << setprecision(2) << det_buff[i].prob;
        result_str = label_file_map[det_buff[i].c]+ " "+ stream.str();
#ifdef CAM_INPUT_VGA
        float scale_x = (float)CAM_RESIZED_WIDTH / (float)CAM_IMAGE_WIDTH;
        float scale_y = (float)CAM_RESIZED_HEIGHT / (float)CAM_IMAGE_HEIGHT;
        int padding_w = (IMAGE_OUTPUT_WIDTH - CAM_RESIZED_WIDTH) / 2;
        int padding_h = (IMAGE_OUTPUT_HEIGHT - CAM_RESIZED_HEIGHT) / 2;
        img.draw_rect_box((int)det_buff[i].bbox.x * scale_x + padding_w, (int)det_buff[i].bbox.y * scale_y + padding_h, (int)det_buff[i].bbox.w * scale_x, (int)det_buff[i].bbox.h * scale_y, result_str.c_str(),color);
#else
        img.draw_rect_box((int)det_buff[i].bbox.x, (int)det_buff[i].bbox.y, (int)det_buff[i].bbox.w, (int)det_buff[i].bbox.h, result_str.c_str(),color);
#endif
    }
    return;
}

/*****************************************
* Function Name : print_time
* Description   : print the time result
* Arguments     : label = string to display
*                 time = processsing time to display
*                 color = text color
*                 idx = index to determine text height
* Return value  : -
******************************************/
void print_time(Image* img, const string label, double time, uint32_t color, int32_t idx)
{
    stringstream stream;
    string str = "";

    stream.str("");
    stream << label << std::setw(4) << std::fixed << std::setprecision(1) << std::round(time * 10) / 10 << "msec";
    str = stream.str();
    img->write_string_rgb(str, 2, TEXT_WIDTH_OFFSET_R,  LINE_HEIGHT_OFFSET + (LINE_HEIGHT * idx), CHAR_SCALE_LARGE, color);
}

/*****************************************
* Function Name : print_result
* Description   : print the result on display.
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
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

    double total_time = ai_time + pre_time + post_time;
    string model_hrnet = "HRNet";
    string str_model_total_hrnet = model_hrnet + " x " + to_string((uint32_t)ppl_count) + "  " + STR_TOTAL_TIME;
    vector<pair<string, double>> result_hrnet = {
        {str_model_total_hrnet, total_time},
        {STR_INFERENCE, ai_time},
        {STR_PRE_PROCESS, pre_time},
        {STR_POST_PROCESS, post_time}
    };
    
#if (0) == INF_YOLOX_SKIP
    double yolox_total_time = yolox_ai_time + yolox_pre_time + yolox_post_time;
    string model_yolox = "YOLOX";
    string str_model_total_yolox = model_yolox + "  " + STR_TOTAL_TIME;
    vector<pair<string, double>> result_yolox = {
        {str_model_total_yolox, yolox_total_time},
        {STR_INFERENCE, yolox_ai_time},
        {STR_PRE_PROCESS, yolox_pre_time},
        {STR_POST_PROCESS, yolox_post_time}
    };

    /* Draw YOLOX Time on RGB image.*/
    for (int i = 0; i < result_yolox.size(); i++)
    {
        index++;
        print_time(img, result_yolox[i].first, result_yolox[i].second, 0x00FF00u, index);
    }
#endif
    
    /* Draw HRNet Time on RGB image.*/
    for (int i = 0; i < result_hrnet.size(); i++)
    {
        index++;
        print_time(img, result_hrnet[i].first, result_hrnet[i].second, 0xFFF000u, index);
    }

#ifdef DISP_AI_FRAME_RATE
    /* Draw AI/Camera Frame Rate on RGB image.*/
    stream.str("");
    stream << "AI/Camera Frame Rate: " << std::setw(3) << (uint32_t)ai_fps << "/" << (uint32_t)cap_fps << "fps";
    str = stream.str();
    img->write_string_rgb(str, 1, TEXT_WIDTH_OFFSET_L, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 1), CHAR_SCALE_LARGE, WHITE_DATA);
#endif /* DISP_AI_FRAME_RATE */
#if (1) == INF_YOLOX_SKIP && defined(CAM_INPUT_VGA)
    if (TH_KPT_YOLOX_SKIP < lowest_kpt_score[0])
    {
#ifdef DISP_AI_FRAME_RATE
        int num = 2;
#else
        int num = 1;
#endif
        for (int i = 0; i < NUM_OUTPUT_C; i++)
        {
            /* Clear string stream */
            stream.str("");
            /* Create DRP-AI result  */
            stream << "  ID " << i << ": (" << static_cast<int>(std::round(hrnet_preds[i][0])) << ", " << static_cast<int>(std::round(hrnet_preds[i][1])) << "): " <<  static_cast<int>(std::round(hrnet_preds[i][2] * 100)) << "%";
            str = stream.str();
            img->write_string_rgb(str, 1, TEXT_WIDTH_OFFSET*2, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * num), CHAR_SCALE_SMALL, WHITE_DATA);
            num++;
        }
    }
#endif // YOLOX SKIP & CAM_INPUT_VGA
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
    /* Semaphore Variable */
    int32_t inf_sem_check = 0;
    int32_t inf_cnt = -1;
    /* Variable for getting Inference output data */
    void* output_ptr;
    uint32_t out_size;
    /* Variable for Pre-processing parameter configuration */
    s_preproc_param_t in_param;
    /* Variable for checking return value */
    int8_t ret = 0;
    /* Variable for Performance Measurement */
    static double hrnet_pre_time = 0;
    static double hrnet_post_time = 0;
    static double hrnet_ai_time = 0;
#if (0) == INF_YOLOX_SKIP
    static struct timespec yolox_pre_start_time;
    static struct timespec yolox_pre_end_time;
    static struct timespec yolox_inf_start_time;
    static struct timespec yolox_inf_end_time;
    static struct timespec yolox_post_start_time;
    static struct timespec yolox_post_end_time;
#endif
    static struct timespec pre_start_time;
    static struct timespec pre_end_time;
    static struct timespec inf_start_time;
    static struct timespec inf_end_time;
    static struct timespec post_start_time;
    static struct timespec post_end_time;
    static struct timespec drp_prev_time = { .tv_sec = 0, .tv_nsec = 0, };

    static uint32_t ppl_count_local = 0;
    static uint16_t id_x_local[NUM_OUTPUT_C][NUM_MAX_PERSON];
    static uint16_t id_y_local[NUM_OUTPUT_C][NUM_MAX_PERSON];
    static float lowest_kpt_score_local[NUM_MAX_PERSON];

    printf("Inference Thread Starting\n");
    printf("Inference Loop Starting\n");
    /* Inference Loop Start */
    while (1)
    {
        inf_cnt++;
        spdlog::info("[START] Start DRP-AI Inference...");
        spdlog::info("Inference ----------- No. {}", (inf_cnt + 1));
        while (1)
        {
            /* Gets the Termination request semaphore value. If different then 1 Termination was requested */
            /* Checks if sem_getvalue is executed wihtout issue */
            errno = 0;
            ret = sem_getvalue(&terminate_req_sem, &inf_sem_check);
            if (0 != ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Semaphore Value: errno=%d\n", errno);
                goto err;
            }
            /* Checks the semaphore value */
            if (1 != inf_sem_check)
            {
                goto ai_inf_end;
            }
            /* Checks if image frame from Capture Thread is ready. */
            if (inference_start.load())
            {
                break;
            }
            usleep(WAIT_TIME);
        }
        errno = 0; 
        memset(array_hrnet_drpai_time, 0, sizeof(array_hrnet_drpai_time));
        memset(array_hrnet_proc_time, 0, sizeof(array_hrnet_proc_time));
        
        /* RGB image */
        mtx.lock();
        cv::Mat frame = g_frame.clone();
        mtx.unlock();

#if (0) == INF_YOLOX_SKIP
        /* YOLOX */
        in_param.pre_in_addr = (uintptr_t)frame.data;

        /* Gets Pre-process starting time */
        ret = timespec_get(&yolox_pre_start_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Pre-process Start Time\n");
            goto err;
        }
        ret = preruntime.Pre(&in_param, &output_ptr, &out_size);
        if (0 < ret)
        {
            fprintf(stderr, "[ERROR] Failed to run Pre-processing Runtime Pre()\n");
            goto err;
        }

        /*Set Pre-processing output to be inference input. */
        runtime.SetInput(0, (float*)output_ptr);

        /* Gets Pre-process End time */
        ret = timespec_get(&yolox_pre_end_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Pre-process Start Time\n");
            goto err;
        }

        /* Pre-process Time Result */
        yolox_pre_time = (timedifference_msec(yolox_pre_start_time, yolox_pre_end_time) * TIME_COEF);

        /* Gets AI Inference starting time */
        ret = timespec_get(&yolox_inf_start_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get Pre-process Start Time\n");
            goto err;
        }

        runtime.Run(drpai_freq);
#else
        ret = 1;  /* YOLOX Skip*/
#endif

#if (0) == INF_YOLOX_SKIP
        /* Gets AI Inference End Time */
        ret = timespec_get(&yolox_inf_end_time, TIME_UTC);
        if ( 0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to Get Inference End Time\n");
            goto err;
        }

        /*Inference Time Result*/
        yolox_ai_time = (timedifference_msec(yolox_inf_start_time, yolox_inf_end_time) * TIME_COEF);
#endif

#if (0) == INF_YOLOX_SKIP
        /* YOLOX R_Post_Proc time start */
        ret = timespec_get(&yolox_post_start_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get R_Post_Proc Start Time\n");
            goto err;
        }

        /* Process to read the DRPAI output data. */
        ret = get_result(runtime, drpai_output_buf0);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to get result from memory.\n");
            goto err;
        }

        /* Preparation For Post-Processing */
        det_res.clear();

        /*CPU Post-Processing For YOLOX*/
        post_proc.R_Post_Proc(&drpai_output_buf0[0], det_res, &ppl_count_local);

        /* YOLOX R_Post_Proc time end*/
        ret = timespec_get(&yolox_post_end_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to get R_Post_Proc end Time\n");
            goto err;
        }
        yolox_post_time = (timedifference_msec(yolox_post_start_time, yolox_post_end_time) * TIME_COEF);
#else
        ppl_count_local = 1;  /* YOLOX Skip*/
#endif
        for (uint32_t i = 0; i < ppl_count_local; i++)
        {
#if (0) == INF_YOLOX_SKIP
            croph[i] = det_res[i].bbox.h + CROP_ADJ_X;
            cropw[i] = det_res[i].bbox.w + CROP_ADJ_Y;
#else
            /* YOLOX Skip*/
            croph[i] = CROPPED_IMAGE_HEIGHT;
            cropw[i] = CROPPED_IMAGE_WIDTH;
#endif
            /* Checks that cropping height and width does not exceeds image dimension */
            if (croph[i] < 1)
            {
                croph[i] = 1;
            }
            else if (croph[i] > IMREAD_IMAGE_HEIGHT)
            {
                croph[i] = IMREAD_IMAGE_HEIGHT;
            }
            else
            {
                /*Do Nothing*/
            }
            if (cropw[i] < 1)
            {
                cropw[i] = 1;
            }
            else if (cropw[i] > IMREAD_IMAGE_WIDTH)
            {
                cropw[i] = IMREAD_IMAGE_WIDTH;
            }
            else
            {
                /*Do Nothing*/
            }
#if (0) == INF_YOLOX_SKIP
            /* Compute Cropping Y Position based on Detection Result */
            /* If Negative Cropping Position */
            if (det_res[i].bbox.y < (croph[i]/2))
            {
                cropy[i] = 0;
            }
            else if (det_res[i].bbox.y > (IMREAD_IMAGE_HEIGHT-croph[i]/2)) /* If Exceeds Image Area */
            {
                cropy[i] = IMREAD_IMAGE_HEIGHT-croph[i];
            }
            else
            {
                cropy[i] = (int16_t)det_res[i].bbox.y - croph[i]/2;
            }
            /* Compute Cropping X Position based on Detection Result */
            /* If Negative Cropping Position */
            if (det_res[i].bbox.x < (cropw[i]/2))
            {
                cropx[i] = 0;
            }
            else if (det_res[i].bbox.x > (IMREAD_IMAGE_WIDTH-cropw[i]/2)) /* If Exceeds Image Area */
            {
                cropx[i] = IMREAD_IMAGE_WIDTH-cropw[i];
            }
            else
            {
                cropx[i] = (int16_t)det_res[i].bbox.x - cropw[i]/2;
            }
#else
            cropx[i] = OUTPUT_LEFT;
            cropy[i] = 0;
#endif
            /* Checks that combined cropping position with width and height does not exceed the image dimension */
            if (cropx[i] + cropw[i] > IMREAD_IMAGE_WIDTH)
            {
                cropw[i] = IMREAD_IMAGE_WIDTH - cropx[i];
            }
            if (cropy[i] + croph[i] > IMREAD_IMAGE_HEIGHT)
            {
                croph[i] = IMREAD_IMAGE_HEIGHT - cropy[i];
            }   
            
            /* HRNet */
            /* Gets Pre-process starting time */
            ret = timespec_get(&pre_start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Pre-process Start Time\n");
                goto err;
            }

            /* CPU Pre Processing For HRNet */    
            cv::Mat frame_hrnet = R_Pre_Proc_HRNet(frame, cropx[i], cropy[i], cropw[i], croph[i]);

            /* Gets AI Pre-process End Time */
            ret = timespec_get(&pre_end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to Get Pre-process End Time\n");
                goto err;
            }

            /* Set Pre-processing output to be inference input. */
            runtime_2.SetInput(0, frame_hrnet.ptr<float>());

            /* Pre-process Time Result */
            hrnet_pre_time += (timedifference_msec(pre_start_time, pre_end_time) * TIME_COEF);

            /* Gets inference starting time */
            ret = timespec_get(&inf_start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Inference Start Time\n");
                goto err;
            }

            runtime_2.Run();
            
            /* Gets AI Inference End Time */
            ret = timespec_get(&inf_end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Inference Start Time\n");
                goto err;
            }

            /* Inference Time Result */
            hrnet_ai_time += (timedifference_msec(inf_start_time, inf_end_time) * TIME_COEF);

            /* Gets Post-process starting time */
            ret = timespec_get(&post_start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Post-process Start Time\n");
                goto err;
            }

            /* Process to read the DRPAI output data. */
            ret = get_result(runtime_2, drpai_output_buf);
            if (0 != ret)
            {
                fprintf(stderr, "[ERROR] Failed to get result from memory.\n");
                goto err;
            }

            /* CPU Post Processing For HRNet & Display the Results */
            post_proc.R_Post_Proc_HRNet(&drpai_output_buf[0], cropw[i], croph[i], &lowest_kpt_score_local[i], (float*)hrnet_preds);

            if (lowest_kpt_score_local[i] > 0)
            {
                post_proc.R_HRNet_Coord_Convert(cropx[i], cropy[i], i, (uint16_t*)id_x_local, (uint16_t*)id_y_local, (float*)hrnet_preds);
            }

            /* R_Post_Proc time end */
            ret = timespec_get(&post_end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to Get R_Post_Proc End Time\n");
                goto err;
            }
            hrnet_post_time += (timedifference_msec(post_start_time, post_end_time)*TIME_COEF);
        }

        /* Copy data for Display Thread */
        ppl_count = 0;
        memcpy(lowest_kpt_score, lowest_kpt_score_local, sizeof(lowest_kpt_score_local));
        memcpy(id_x, id_x_local, sizeof(id_x_local));
        memcpy(id_y, id_y_local, sizeof(id_y_local));
        ppl_count = ppl_count_local;
        pre_time = hrnet_pre_time;
        post_time = hrnet_post_time;
        ai_time = hrnet_ai_time;
        hrnet_pre_time = 0;
        hrnet_post_time = 0;
        hrnet_ai_time = 0;

#if (0) == INF_YOLOX_SKIP
        /* Display Processing YOLOX Time On Log File */
        double yolox_total_time = yolox_ai_time + yolox_pre_time + yolox_post_time;
        spdlog::info("YOLOX");
        spdlog::info(" YOLOX Total AI Time: {} [ms]", std::round(yolox_total_time * 10) / 10);
        spdlog::info(" YOLOX Inference: {} [ms]", std::round(yolox_ai_time * 10) / 10);
        spdlog::info(" YOLOX PreProcess: {} [ms]", std::round(yolox_pre_time * 10) / 10);
        spdlog::info(" YOLOX PostProcess: {} [ms]", std::round(yolox_post_time * 10) / 10);
#endif

        /* Display Processing Time On Log File */
        int idx = inf_cnt % SIZE_OF_ARRAY(array_drp_time);
        double total_time = ai_time + pre_time + post_time;
        array_drp_time[idx] = ai_time;
        drp_prev_time = inf_end_time;
        spdlog::info("HRNet");
        spdlog::info(" HRNet Total AI Time: {} [ms]", std::round(total_time * 10) / 10);
        spdlog::info(" HRNet Inference: {} [ms]", std::round(ai_time * 10) / 10);
        spdlog::info(" HRNet PreProcess: {} [ms]", std::round(pre_time * 10) / 10);
        spdlog::info(" HRNet PostProcess: {} [ms]", std::round(post_time * 10) / 10);
             
#ifdef DISP_AI_FRAME_RATE
        int arraySum = std::accumulate(array_drp_time, array_drp_time + SIZE_OF_ARRAY(array_drp_time), 0);
        double arrayAvg = 1.0 * arraySum / SIZE_OF_ARRAY(array_drp_time);
        ai_fps = 1.0 / arrayAvg * 1000.0 + 0.5;
        spdlog::info("AI Frame Rate {} [fps]", (int32_t)ai_fps);
#endif /* DISP_AI_FRAME_RATE */

        inference_start.store(0);
    }
    /* End of Inference Loop */

/* Error Processing */
err:
    /*Set Termination Request Semaphore to 0*/
    sem_trywait(&terminate_req_sem);
    goto ai_inf_end;

/* AI Thread Termination */
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
    uint8_t* img_buffer;
    uint8_t* img_buffer0;
    uint8_t capture_stabe_cnt = 8;  // Counter to wait for the camera to stabilize

    deque<uint8_t*> frame_buffer;
    const size_t frame_size = CAM_IMAGE_HEIGHT * CAM_IMAGE_WIDTH * CAM_IMAGE_CHANNEL_YUY2;
    uint8_t* black_frame = new uint8_t[frame_size];
    for (size_t i = 0; i < frame_size; i += 4) {
        black_frame[i] = 16;       // Y0
        black_frame[i + 1] = 128;  // U0
        black_frame[i + 2] = 16;   // Y1
        black_frame[i + 3] = 128;  // V0
    }
    
#ifdef DISP_AI_FRAME_RATE
    int32_t cap_cnt = -1;
    static struct timespec capture_time;
    static struct timespec capture_time_prev = { .tv_sec = 0, .tv_nsec = 0, };
#endif /* DISP_AI_FRAME_RATE */

    printf("Capture Thread Starting\n");

    img_buffer0 = (uint8_t*)capture->drpai_buf->mem;
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

                    g_frame = img.convert_format_Mat_rgb(img_buffer0);

                    /* Flush capture image area cache */
                    ret = capture->video_buffer_flush_dmabuf(capture->drpai_buf->idx, capture->drpai_buf->size);
                    if (0 != ret)
                    {
                        goto err;
                    }
                    inference_start.store(1); /* Flag for AI Inference Thread. */
                }

                if (!delay_frame_inserted)
                {
                    for (int i = 0; i < DELAY_FRAMES; i++)
                    {
                        uint8_t* frame = new uint8_t[frame_size];
                        if (black_frame == nullptr) 
                        {
                            fprintf(stderr, "[ERROR] black_frame is null\n");
                            break;
                        }
                        memcpy(frame, black_frame, frame_size);
                        frame_buffer.push_back(frame);
                    }
                    delay_frame_inserted = true;
                }

                if (!img_obj_ready.load())
                {
                    uint8_t* frame = new uint8_t[frame_size];
                    memcpy(frame, img_buffer, frame_size);
                    frame_buffer.push_back(frame);

                    if (!frame_buffer.empty())
                    {
                        uint8_t* front_frame = frame_buffer.front();
                        img.camera_to_image(front_frame, frame_size);
                        delete[] front_frame;
                        frame_buffer.pop_front();

                    }
                    
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
    bool padding = false;
#ifdef CAM_INPUT_VGA
    padding = true;
#endif // CAM_INPUT_VGA
    timespec inf_start_time;
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
            ret = timespec_get(&inf_start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Display Start Time\n");
                goto err;
            }
            
            /* Convert YUYV image to BGRA format. */
            img.convert_format();

            /* Convert output image size. */
            img.convert_size(CAM_IMAGE_WIDTH, CAM_RESIZED_WIDTH, CAM_IMAGE_HEIGHT, CAM_RESIZED_HEIGHT, padding);
            
            /* Draw Complete Skeleton. */
            draw_skeleton();

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
            
#ifdef DEBUG_TIME_FLG
            double img_proc_time = (timedifference_msec(inf_start_time, end_time) * TIME_COEF);
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
    int32_t disp_cnt = 0;

    timespec inf_start_time;
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
            ret = timespec_get(&inf_start_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to get Display Start Time\n");
                goto err;
            }
            /*Update Wayland*/
            wayland.commit(img.get_img(buf_id), NULL);

#if END_DET_TYPE // To display the app_pointer_det in front of this application.
            if (display_state == 0) 
            {
                display_state = 1;
            }
#endif

            hdmi_obj_ready.store(0);
            ret = timespec_get(&end_time, TIME_UTC);
            if (0 == ret)
            {
                fprintf(stderr, "[ERROR] Failed to Get Display End Time\n");
                goto err;
            }
            disp_time = (uint32_t)((timedifference_msec(disp_prev_time, end_time) * TIME_COEF));
            int idx = disp_cnt++ % SIZE_OF_ARRAY(array_disp_time);
            array_disp_time[idx] = disp_time;
            disp_prev_time = end_time;
#ifdef DEBUG_TIME_FLG
            /* Draw Disp Frame Rate on RGB image.*/
            int arraySum = std::accumulate(array_disp_time, array_disp_time + SIZE_OF_ARRAY(array_disp_time), 0);
            double arrayAvg = 1.0 * arraySum / SIZE_OF_ARRAY(array_disp_time);
            double disp_fps = 1.0 / arrayAvg * 1000.0;
            double disp_proc_time = (timedifference_msec(inf_start_time, end_time) * TIME_COEF);

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

#if END_DET_TYPE 
        // 1. Receive the end command via named pipe /tmp/appdetect from app_pointer_det.
        // 2. Send the end command via named pipe /tmp/gui to app_rzv2h_demo
        int fd;
        char str[BUF_SIZE];
        char str_end[BUF_SIZE] = "end";
        ssize_t size;
        mkfifo("/tmp/appdetect", 0666);
        fd = open("/tmp/appdetect", O_RDWR);
        size = read(fd, str, BUF_SIZE);
        if (size > 0)
        {
            /* When mouse clicked. */
            printf("mouse clicked. : %s\n", str);
            str[size] = '\n';

            if (strcmp(str, str_end) == 0)
            {
                if (system("echo \"end\" > /tmp/gui") == -1)
                {
                    printf("[ERROR] Failed to send command\n");
                }
                goto err;
            }
        }
        close(fd);
#else

        c = getchar();
        if (EOF != c)
        {
            /* When key is pressed. */
            printf("key Detected.\n");
            goto err;
        }
#endif // END_DET_TYPE

        /* When nothing is detected. */
        usleep(WAIT_TIME);
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
        
#if END_DET_TYPE // To launch app_pointer_det.
        if (display_state == 1)
        {
            if (system("./../app_pointer_det & ") == -1)
            {
                printf("Command Error\n");
                goto main_proc_end;
            }
            display_state = 2;
        }
#endif
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

int32_t main(int32_t argc, char* argv[])
{
    /* Log File Setting */
    auto now = std::chrono::system_clock::now();
    auto tm_time = spdlog::details::os::localtime(std::chrono::system_clock::to_time_t(now));
    char date_buf[64];
    char time_buf[128];
    memset(time_buf, 0, sizeof(time_buf));
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d_%H-%M-%S", &tm_time);
    sprintf(time_buf,"logs/%s_app_yolox_hrnet_cam.log",date_buf);
    auto logger = spdlog::basic_logger_mt("logger", time_buf);
    spdlog::set_default_logger(logger);

    /*
    The following three lines disable the OpenCV accelerator feature. 
    This code is intended to prepare for possible DRP-AI resource contention 
    in the RZ/V2N. 
    Note: If using OpenCV Accelerator v1.10 or later, the OpenCV functionality
          used in this Deeplabv3 sample application does not use DRP-AI 
          resources, so this workaround can be removed in the RZ/V2N. 
          Furthermore, this workaround is not necessary for the RZ/V2H, 
          as it runs on a different functional unit(DRP) than DRP-AI.
          For more information,see 
           https://github.com/renesas-rz/rzv2h_opencv_accelerator .
    */
#ifdef V2N
    unsigned long OCA_list[16];
    for (int i=0; i < 16; i++) OCA_list[i] = 0;
    OCA_Activate( &OCA_list[0] );
#endif

    /* DRP-AI Frequency Setting */
    if (2 <= argc)
    {
        drp_max_freq = atoi(argv[1]);
    }
    else
    {
        drp_max_freq = 2;
    }
    if (3 <= argc)
    {
        drpai_freq = atoi(argv[2]);
    }
    else
    {
        drpai_freq = 2;
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
#if (0) == INF_YOLOX_SKIP
    printf("Model : Megvii-Base Detection YOLOX | %s  \n", model_dir_yolox.c_str());
#endif
    printf("Model : MMPose HRNet | %s  \n", model_dir_hrnet.c_str());
    printf("Input : %s\n", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    spdlog::info("  RZ/V2H DRP-AI Sample Application");
#if (0) == INF_YOLOX_SKIP
    spdlog::info("  Model : Megvii-Base Detection YOLOX | {} ", model_dir_yolox.c_str());
#endif
    spdlog::info("  Model : MMPose HRNet | {} ", model_dir_hrnet.c_str());
    spdlog::info("  Input : {}", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    printf("Argument : <DRP0_max_freq_factor> = %d\n", drp_max_freq);
    printf("Argument : <AI-MAC_freq_factor> = %d\n", drpai_freq);
    
    uint64_t drpaimem_addr_start = 0;
    errno = 0;

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
    /*Load pre_dir object to DRP-AI */
    ret = preruntime.Load(pre_dir_yolox);
    if (0 < ret)
    {
        fprintf(stderr, "[ERROR] Failed to run Pre-processing Runtime Load().\n");
        goto end_close_drpai;
    }

    /* YOLOX */
    runtime_status = runtime.LoadModel(model_dir_yolox, drpaimem_addr_start + DRPAI_MEM_OFFSET_YOLOX);
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
#endif

    /* HRNet */
#if (0) == INF_YOLOX_SKIP
    drpaimem_addr_start = drpaimem_addr_start + DRPAI_MEM_OFFSET_HRNET;
#endif
    runtime_status = runtime_2.LoadModel(model_dir_hrnet, drpaimem_addr_start);
    if(!runtime_status)
    {
        fprintf(stderr, "[ERROR] Failed to load model.\n");
        goto end_close_drpai;
    }
    /*Get input data */
    input_data_type = runtime_2.GetInputDataType(0);
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
