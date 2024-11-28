/***********************************************************************************************************************
* Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : main.cpp
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for TopFormer with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
/*DRP-AI TVM[*1] Runtime*/
#include "MeraDrpRuntimeWrapper.h"
/*Pre-processing Runtime Header*/
#include "PreRuntime.h"
/*Definition of Macros & other variables*/
#include "define.h"
/*MIPI camera control*/
#include "camera.h"
/*Image control*/
#include "image_topformer.h"
/*Wayland control*/
#include "wayland.h"
/*Mutual exclusion*/
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <opencv2/opencv.hpp>

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
static float  drpai_output_buf[TVM_MODEL_IN_W * TVM_MODEL_IN_H * NUM_CLASS];
static uint8_t postproc_data[TVM_MODEL_IN_W * TVM_MODEL_IN_H];
static uint8_t postproc_data_class_exist[NUM_CLASS];
static uint64_t capture_address;
static uint8_t buf_id;
static Image img;

/*AI Inference for DRPAI*/
/* DRP-AI TVM[*1] Runtime object */
MeraDrpRuntimeWrapper runtime;
/* Pre-processing Runtime object */
PreRuntime preruntime;
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
#if END_DET_TYPE
static int8_t display_state=0;
#endif
static Wayland wayland;
static Camera* capture = NULL;

static double pre_time = 0;
static double post_time = 0;
static double ai_time = 0;
static double total_time = 0;

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
    int32_t output_num = 0;
    std::tuple<InOutDataType, void*, int64_t> output_buffer;
    int64_t output_size;

    /* Get the number of output of the target model. */
    output_num = runtime.GetNumOutput();
    if(output_num != TVM_MODEL_OUT_NUM){
        return -1;
    }

    /* output_buffer below is tuple, which is { data type, address of output data, number of elements } */
    output_buffer = runtime.GetOutput(0);
    /*Output Data Size = std::get<2>(output_buffer). */
    output_size = std::get<2>(output_buffer);
    if(output_size != TVM_MODEL_OUT_W * TVM_MODEL_OUT_H * NUM_CLASS){
        return -1;
    }

    /*Output Data Type = std::get<0>(output_buffer)*/
    if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
    {
        /*Output Data = std::get<1>(output_buffer)*/
        uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
        for (int j = 0; j<output_size; j++)
        {
            /*FP16 to FP32 conversion*/
            drpai_output_buf[j]=float16_to_float32(data_ptr[j]);
        }
    }
    else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
    {
        /*Output Data = std::get<1>(output_buffer)*/
        float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
        for (int j = 0; j<output_size; j++)
        {
            drpai_output_buf[j]=data_ptr[j];
        }
    }
    else
    {
        fprintf(stderr, "[ERROR] Output data type : not floating point.\n");
        ret = -1;
    }
    return ret;
}

/*****************************************
* Function Name : sign
* Description   : Get the sign of the input value
* Arguments     : x = input value
* Return value  : returns the sign, 1 if positive -1 if not
*******************************************/
int8_t sign(int32_t x)
{
    return x > 0 ? 1 : -1;
}

/*****************************************
* Function Name : R_Post_Proc_TopFormer
* Description   : CPU post-processing for TopFormer
*
* Arguments     : floatarr = drpai output address
* Return value  : -
******************************************/

void R_Post_Proc_TopFormer(float* floatarr)
{
    mtx.lock();    

    /* Clear class flgas indicating whether each class exist or not */
    for (int i = 0; i < NUM_CLASS; i++)
    {
        postproc_data_class_exist[i] = 0;
    }

    /* Transpose TVM output CHW -> HWC to raise cache utilization efficiency 
       when scanning the probabilities along "C" axis to determine the class index for each pixels. */
    float tmp_output_buf[TVM_MODEL_OUT_W * TVM_MODEL_OUT_H][NUM_CLASS];
    for (int ci = 0; ci < NUM_CLASS; ci++) 
    {
        for (int i = 0; i < TVM_MODEL_OUT_W * TVM_MODEL_OUT_H; i++) 
        {
            float current_value = floatarr[ci * TVM_MODEL_OUT_W * TVM_MODEL_OUT_H + i];
            tmp_output_buf[i][ci] = current_value;
        }
    }

    /* Convert output probabilities to label indice array */
    for (int i = 0; i < TVM_MODEL_OUT_W * TVM_MODEL_OUT_H; i++)
    {
        /* Get maximum probability index */
        uint8_t tmp_max_idx = 0;
        float tmp_max_value = tmp_output_buf[i][0];

        for (int ci = 1; ci < NUM_CLASS; ci++) {
            float current_value = tmp_output_buf[i][ci];
            if (tmp_max_value < current_value) {
                tmp_max_value = current_value;
                tmp_max_idx = ci;
            }
        }

        /* Write the maximum probability index.*/
        postproc_data[i] = tmp_max_idx;

        /* Record the class flags indicating whether each class exist or not */
        postproc_data_class_exist[tmp_max_idx] = 1; // true
    }

    /* Write into log */
    char strColor[32];
    for( int i = 0; i < NUM_CLASS ; i++)
    {
        if( postproc_data_class_exist[i] > 0 )
        {
            memset(strColor,0,sizeof(strColor));
            sprintf(strColor,"(RGB:0x%2x%2x%2x)",g_ade_palette[i][0], g_ade_palette[i][1], g_ade_palette[i][2]);
            spdlog::info(" Detected Class {} : {} {}", (i), g_ade_classes[i], strColor);
        }
    }

    mtx.unlock();
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

    mtx.lock();
    std::stringstream stream;
    std::string str = "";
    
    /* Draw Segmentation on RGB image.*/
    int ret = img->draw_sem_seg(postproc_data, capture, TVM_MODEL_OUT_W, TVM_MODEL_OUT_H, CAM_RESIZED_WIDTH, CAM_RESIZED_HEIGHT, CAM_RESIZED_PADDING);
    if (ret != 0)
    {
        return -1;
    }
    
    /* Draw detected class information on RGB images*/
    for (int i = 0, num = 1; i < NUM_CLASS; i++)
    {
        if (postproc_data_class_exist[i] == 1)
        {
            stream.str("");
            stream << "Class" << std::setw(3) << i + 1 <<  " : " << g_ade_classes[i];
            img->write_string_overlay(
                stream.str(), 
                1, 
                TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * num), 
                CHAR_SCALE_LARGE, 
                (g_ade_palette[i][0] << 16) | (g_ade_palette[i][1] << 8) | g_ade_palette[i][2]
            );
            num++;
        }
    }

    /* Draw Total Time Result on RGB image.*/
    stream.str("");
    stream << "Total AI Time : " << std::setw(3) << std::fixed << std::setprecision(1) << std::round(total_time * 10) / 10 << "msec";
    str = stream.str();
    img->write_string_overlay(str, 2, TEXT_WIDTH_OFFSET,  LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 1), CHAR_SCALE_LARGE, 0xFFF000u);
    
    /* Draw Inference Time on RGB image.*/
    stream.str("");
    stream << "  Inference   : " << std::setw(3) << std::fixed << std::setprecision(1) << std::round(ai_time * 10) / 10 << "msec";
    str = stream.str();
    img->write_string_overlay(str, 2, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 2), CHAR_SCALE_LARGE, 0xFFF000u);

    /* Draw PreProcess Time on RGB image.*/
    stream.str("");
    stream << "  PreProcess  : " << std::setw(3) << std::fixed << std::setprecision(1) << std::round(pre_time * 10) / 10 << "msec";
    str = stream.str();
    img->write_string_overlay(str, 2, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 3), CHAR_SCALE_LARGE, 0xFFF000u);

    /* Draw PostProcess Time on RGB image.*/
    stream.str("");
    stream << "  PostProcess : " << std::setw(3) << std::fixed << std::setprecision(1) << std::round(post_time * 10) / 10 << "msec";
    str = stream.str();
    img->write_string_overlay(str, 2, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 4), CHAR_SCALE_LARGE, 0xFFF000u);

#ifdef DISP_AI_FRAME_RATE
    /* Draw AI/Camera Frame Rate on RGB image.*/
    stream.str("");
    stream << "AI/Camera Frame Rate: " << std::setw(3) << (uint32_t)ai_fps << "/" << (uint32_t)cap_fps << "fps";
    str = stream.str();
    img->write_string_overlay(str, 2, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 5), CHAR_SCALE_LARGE, 0xFFF000u);
#endif /* DISP_AI_FRAME_RATE */

#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Draw Overlay Image Time   : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG
    mtx.unlock();

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
    static struct timespec start_time;
    static struct timespec inf_end_time;
    static struct timespec pre_start_time;
    static struct timespec pre_end_time;
    static struct timespec post_start_time;
    static struct timespec post_end_time;
    static struct timespec drp_prev_time = { .tv_sec = 0, .tv_nsec = 0, };

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

        in_param.pre_in_addr    = (uintptr_t) capture_address;
        /*Gets Pre-process starting time*/
        ret = timespec_get(&pre_start_time, TIME_UTC);
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
        if ( 0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to Get Inference End Time\n");
            goto err;
        }
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
        /*CPU Post-Processing For TopFormer*/
        R_Post_Proc_TopFormer(drpai_output_buf);

        /* R_Post_Proc time end*/
        ret = timespec_get(&post_end_time, TIME_UTC);
        if (0 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to Get R_Post_Proc End Time\n");
            goto err;
        }
        post_time = (timedifference_msec(post_start_time, post_end_time)*TIME_COEF);

        /*Display Processing Time On Log File*/
        int idx = inf_cnt % SIZE_OF_ARRAY(array_drp_time);
        ai_time = timedifference_msec(start_time, inf_end_time) * TIME_COEF;
        total_time = ai_time + pre_time + post_time;
        array_drp_time[idx] = ai_time;
        drp_prev_time = inf_end_time;
        spdlog::info("Total AI Time: {} [ms]", std::round(total_time * 10) / 10);
        spdlog::info("Inference: {} [ms]", std::round(ai_time * 10) / 10);
        spdlog::info("PreProcess: {} [ms]", std::round(pre_time * 10) / 10);
        spdlog::info("PostProcess: {} [ms]", std::round(post_time * 10) / 10);

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
    const uint8_t img_skip_num = 3;
    uint8_t img_skip_cnt = 0;
#ifdef DISP_AI_FRAME_RATE
    int32_t cap_cnt = -1;
    static struct timespec capture_time;
    static struct timespec capture_time_prev = { .tv_sec = 0, .tv_nsec = 0, };
#endif /* DISP_AI_FRAME_RATE */
    
    printf("Capture Thread Starting\n");

    img_buffer0 = (uint8_t *)capture->drpai_buf->mem;
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
                
                img_skip_cnt++;
                if (!img_obj_ready.load() && img_skip_cnt >= img_skip_num)
                {
                    img_skip_cnt = 0;
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
    timespec start_time;
    timespec end_time;

    // Adjust class color table
    class_ade_initialize();

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
                        
            /* Convert YUYV image to BGRA format. */
            img.convert_format();

            /* Convert output image size. */
            img.convert_size(CAM_IMAGE_WIDTH, CAM_RESIZED_WIDTH, CAM_IMAGE_HEIGHT, CAM_RESIZED_HEIGHT, CAM_RESIZED_PADDING);

            /*displays AI Inference Results on display.*/
            ret = print_result(&img);
            if (0 != ret)
            {
                fprintf(stderr, "[ERROR] Failed to displays AI Inference Results on display.\n");
                goto err;
            }
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
            double img_proc_time = (timedifference_msec(start_time, end_time) * TIME_COEF);
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

    timespec start_time;
    timespec end_time;
    static struct timespec disp_prev_time = { .tv_sec = 0, .tv_nsec = 0, };

    /* Initialize waylad (draw overlay)*/
    ret = wayland.init(capture->wayland_buf->idx, IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, IMAGE_CHANNEL_BGRA,true);
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
            wayland.commit(img.get_img(buf_id), img.get_overlay_img(buf_id));

            /*Reset Overlay image*/
            img.reset_overlay_img(buf_id);

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
            double disp_proc_time = (timedifference_msec(start_time, end_time) * TIME_COEF);
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

/*****************************************
* Function Name : format
* Description   : Format specification.
* Arguments     : fmt = an object that represents the format string.
*                 args = arguments to be formatted
* Return value  : A string object holding the formatted result.
******************************************/
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args)
{
    size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ...);
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
    return std::string(&buf[0], &buf[0] + len);
}

/*****************************************
* Function Name : set_tvm_num_threads
* Description   : Function to set the TVM_NUM_THREADS environment variable
* Arguments     : tvm_num_threads: TVM_NUM_THREADS
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int set_tvm_num_threads(uint8_t tvm_num_threads)
{
    std::string _tvm_num_threads = format( "%d", tvm_num_threads);
    uint8_t overwrite = 1;
    int8_t ret = 0;
    errno = 0;
    ret = setenv("TVM_NUM_THREADS", &_tvm_num_threads[0], overwrite);
    if ( ret < 0 )
    {
        printf("failed to export TVM_NUM_THREADS : errno=%d\n", errno);
        return -1;
    }
    printf("TVM_NUM_THREADS = %s\n", getenv("TVM_NUM_THREADS"));
    return 0;
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
    sprintf(time_buf,"logs/%s_app_topformer_cam.log",date_buf);
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
    int drpai_fd;

    printf("RZ/V2H DRP-AI Sample Application\n");
    printf("Model : TopFormer | %s\n", model_dir.c_str());
    printf("Input : %s\n", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    spdlog::info("  RZ/V2H DRP-AI Sample Application");
    spdlog::info("  Model : TopFormer | {}", model_dir.c_str());
    spdlog::info("  Input : {}", INPUT_CAM_NAME);
    spdlog::info("************************************************");
    printf("Argument : <DRP0_max_freq_factor> = %d\n", drp_max_freq);
    printf("Argument : <AI-MAC_freq_factor> = %d\n", drpai_freq);

    uint64_t drpaimem_addr_start = 0;

    /* Check if TVM_NUM_THREADS environment variable is set or not */
    char * tvm_num_threads = getenv("TVM_NUM_THREADS");
    if( NULL == tvm_num_threads )
    {
        // If not, set the TVM_NUM_THREADS to TVM_NUM_THREADS_DEFAULT 
        ret = set_tvm_num_threads(TVM_NUM_THREADS_DEFAULT);
        if (0 > ret)
        {
            fprintf(stderr, "[ERROR] Failed to the number of threads for TVM.\n" );
            goto end_main;   
        }
    }
    
    errno = 0;
    drpai_fd = open("/dev/drpai0", O_RDWR);
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
    ret = img.init(CAM_IMAGE_WIDTH, CAM_IMAGE_HEIGHT, CAM_IMAGE_CHANNEL_YUY2, IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, IMAGE_CHANNEL_BGRA, capture->wayland_buf->mem, capture->overlay_buf->mem);
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
    /*Close MIPI Camera.*/
    ret = capture->close_camera();
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to close Camera.\n");
        ret_main = -1;
    }
    delete capture;
    goto end_close_drpai;

end_close_drpai:

    /*Close DRP-AI Driver.*/
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

