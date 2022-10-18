/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2022
 *　
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
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
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : recognize_base.cpp
* Version      : 1.0.1
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "recognize_base.h"
#include "../image_converter.h"
#include "../command/camera_image.h"
#include "../command/cpu_usage.h"
#include "../include/lwsock.hpp"
#include "common/recognize_define.h"
#include "opencv2/opencv.hpp"
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>

/**
 * @brief RecognizeBase
 * @details  Construct a new Recognize Base:: Recognize Base object
 * @param server reference to websocker server
 */
RecognizeBase::RecognizeBase(shared_ptr<WebsocketServer> server)
{
    _server = server;

    _pthread_ai_inf = 0;
    _pthread_capture = 0;
    _pthread_framerate = 0;
}

/**
 * @brief print_measure_log
 * @details Print measurement log to console
 * @param item measurement item
 * @param time elapsed time
 * @param unit unit
 */
void print_measure_log(string item, float time, string unit)
{
    printf("[MeasLog],%s, %.1f, [%s]\n", item.c_str(), time, unit.c_str());
}

/**
 * @brief print_measure_log
 * @details Print measurement log to console
 * @param item measurement item
 * @param log log
 */
void print_measure_log(string item, string log)
{
    printf("[MeasLog],%s,%s\n", item.c_str(), log.c_str());
}

/**
 * @brief float16_to_float32
 * @details Function by Edgecortex. Convert uin16_t number into float value.
 * @param a float16 number
 * @return float float32 number
 */
float float16_to_float32(uint16_t a)
{
    return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

/**
 * @brief initialize
 * @details Initialization for recognize process.
 * @param model Model to be run
 * @return int32_t success:0 error: != 0
 */
int32_t RecognizeBase::initialize(IRecognizeModel* model)
{
    std::cout << "############ INIT ############" << std::endl;
    _model = shared_ptr<IRecognizeModel>(move(model));

    std::cout << "DRP PREFIX:" << _model->model_prefix << std::endl;
    std::cout << "outbuff :" << _model->outBuffSize << std::endl;

    _outBuffSize = _model->outBuffSize;
    dir = _model->model_dir + "/";

    cap_w = _model->_capture_w;
    cap_h = _model->_capture_h;
    cap_c = _model->_capture_c;
    model_w = _model->_model_w;
    model_h = _model->_model_h;
    model_c = _model->_model_c;
    mode = _model->_id;

    return 0;
}

/**
 * @brief recognize_start
 * @details Load drp-ai data / start threads
 * @return int32_t success:0 error: != 0
 */
int32_t RecognizeBase::recognize_start()
{
    printf("RZ/V2MA DRP-AI TVM Demo\n");
    printf("Model :  %s\n", _model->model_prefix.c_str());
    printf("Input : USB Camera\n");

    /* Create Camera Instance */
    _capture = make_shared<Camera>();
    /* Init and Start Camera */
    _capture->set_w(_model->_capture_w);
    _capture->set_h(_model->_capture_h);
    _capture->set_c(_model->_capture_c);
    int8_t ret = _capture->start_camera();
    int32_t create_thread_ai;
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to initialize USB Camera.\n");
        return -1;
    }

    wake_ = false;
    capture_enabled.store(true);

    /* capture thread */
    _capture_running = true;
    int32_t create_thread_cap = pthread_create(&_pthread_capture, NULL, capture_thread, this);
    if (0 != create_thread_cap)
    {
        fprintf(stderr, "[ERROR] Failed to create AI Inference Thread.\n");
        return -1;
    }

#ifdef INFERENE_ON

    /* inference thread */
    _inf_running = true;
    if((_model->_id == MODE_TVM_DEEPPOSE_DRPAI) || (_model->_id == MODE_TVM_DEEPPOSE_CPU))
    {
        create_thread_ai = pthread_create(&_pthread_ai_inf, NULL, tvm_inference_thread, this);
    }

    if (0 != create_thread_ai)
    {
        fprintf(stderr, "[ERROR] Failed to create AI Inference Thread.\n");
        return -1;
    }

#endif // INFERENE_ON


    /* framerate thread */
    _fps_runnning = true;
    int32_t framerate_thread_cap = pthread_create(&_pthread_framerate, NULL, framerate_thread, this);
    if (0 != framerate_thread_cap)
    {
        fprintf(stderr, "[ERROR] Failed to create Framerate Thread.\n");
        return -1;
    }

    return 0;
}
/**
 * @brief recognize_end
 * @details Recognize end proc
 */
void RecognizeBase::recognize_end()
{
    end_all_threads();
    close_camera();
    std::cout << "********************** END *********************" << std::endl;
}

/**
 * @brief capture_thread
 * @details usb camera capture and store udma buffer address for DRP-AI
 * @param arg pointer to itself
 * @return void*
 */
void* RecognizeBase::capture_thread(void* arg)
{
    RecognizeBase* me = (RecognizeBase*)arg;
    shared_ptr<Camera> capture = me->_capture;

    /*First Loop Flag*/
    uint32_t capture_addr = 0;
    int8_t ret = 0;


    CameraImage notify;
    vector<uint8_t> output;

    printf("_capture Thread Starting\n");
    while (me->_capture_running)
    {
        errno = 0;
#ifdef SEQUENCTCIAL
        std::cout << "[cam_thread]waiting for capture enable..." << std::endl;
        while (!me->capture_enabled.load())
        {
            usleep(1);
        }

        {
            Measuretime m("capture time");
            capture_addr = capture->capture_image();
        }
        me->capture_address = capture_addr;

        {
            Measuretime m("copy time");

            me->input_data = capture->get_img();

            me->capture_enabled.store(false);
            /* sync with inference thread*/
            unique_lock<mutex> lock(me->mtx_);
            me->wake_ = true;
            me->cv_.notify_all();
            std::cout << "[cam_thread]signaled for capture enable !!" << std::endl;

        }
#else
        /* _capture USB camera image and stop updating the _capture buffer */
        {
            Measuretime mmm("capture_time");
            capture_addr = capture->capture_image();
        }

#ifdef INFERENE_ON
        if (me->capture_enabled.load())
        {
            Measuretime m("capture enable proc time");
            me->capture_enabled.store(false);
            capture->sync_inference_buf_capture();
            me->capture_address = capture_addr;
            me->input_data = capture->get_img();
            /* sync with inference thread*/
            unique_lock<mutex> lock(me->mtx_);
            me->wake_ = true;
            me->cv_.notify_all();
        }
#endif
#endif

        me->_camera_frame_count.store(me->_camera_frame_count.load() + 1);


        if (0 == capture_addr)
        {
            fprintf(stderr, "[ERROR] Failed to _capture image from camera.\n");
            break;
        }
        else
        {
#ifdef SEND_CAMERA_ON
            uint8_t* send_image = capture->get_img();

            notify.img = me->get_send_image(send_image);
            {
                Measuretime m("Command create and send time[camera]");
                /* Send ImageNotify*/
                me->_server->send_command(notify.CreateRequest());
            }
#endif
        }
        /* IMPORTANT: Place back the image buffer to the _capture queue */
#ifdef SEQUENCTCIAL

        while (!me->capture_enabled.load())
        {
            usleep(1);
        }
        ret = capture->capture_qbuf();

#endif

#ifdef INFERENE_ON
#ifndef SEQUENCTCIAL

        /* IMPORTANT: Place back the image buffer to the _capture queue */
        if (capture->get_buf_capture_index() != capture->get_inference_buf_capture_index())
        {
            {
                Measuretime m("Deque capture buf time");
                ret = capture->capture_qbuf();
            }
        }
#endif
#else
        {
            Measuretime m("Deque capture buf time");
            ret = capture->capture_qbuf();
        }
#endif
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to enqueue _capture buffer.\n");
            break;
        }
    } /*End of Loop*/

    /*To terminate the loop in AI Inference Thread.*/
    unique_lock<mutex> lock(me->mtx_);
    me->wake_ = true;
    me->cv_.notify_all();

    cout << "<<<<<<<<<<<<<<<<<<<<< Capture Thread Terminated >>>>>>>>>>>>>>>>>>" << endl;

    pthread_exit(NULL);
    me->_pthread_capture = 0;

    return NULL;
}

/**
 * @brief inference thread
 * @details inference and send results
 * @param arg pointer to itself
 * @return void*
 */
void* RecognizeBase::tvm_inference_thread(void* arg)
{
    RecognizeBase* me = (RecognizeBase*)arg;

    shared_ptr<Camera> capture = me->_capture;

    int8_t ret = 0;
    /* Variables required for Transpose&Normalize in pre-processing */
    std::vector<float> chw;
    float val = 0;
    /* Time Measure variables */
    timespec start_time, end_time;
    float wait_time, load_time, ai_time, get_time, preproc_time;
    /* DRP-AI TVM[*1] Runtime object */
    MeraDrpRuntimeWrapper runtime;
    /*Pre-processing output buffer pointer (DRP-AI TVM[*1] input data)*/
    float* pre_output_ptr;
    /*Pre-processing Runtime Variables*/
    const std::string pre_dir = "preprocess_tvm_v2ma";
    PreRuntime preruntime;
    s_preproc_param_t in_param;
    uint32_t out_size;
    /*CPU Pre-processing cv::Mat*/
    cv::Mat image, image_rgb, image_resize, image_float;
    /*Inference Variables*/
    int8_t inf_status = 0;
    int32_t inf_cnt = -1;
    drpai_status_t drpai_status;
    shared_ptr<float> drpai_output_buf;
    recognizeData_t data;
    float mean[] = { 0.485, 0.456, 0.406 };
    float stdev[] = { 0.229, 0.224, 0.225 };
    /*Mode: DRP-AI vs CPU*/
    uint8_t drpai_mode = 0;
    std::cout<<"[INFO] Model : " << me->dir << std::endl;
    printf("Inference Thread Starting\n");

    /*Check current mode*/
    if (me->mode == MODE_TVM_DEEPPOSE_DRPAI) drpai_mode = 1;

    /*Pre-processing preparation*/
    if (drpai_mode)
    {
        preruntime.Load(pre_dir);
        /*Define pre-processing parameter*/
        in_param.pre_in_shape_w = me->cap_w;
        in_param.pre_in_shape_h = me->cap_h;
        in_param.pre_in_format = INPUT_YUYV;
        in_param.resize_w = me->model_w;
        in_param.resize_h = me->model_h;
        in_param.resize_alg = ALG_BILINEAR;
        /*Compute normalize coefficient, cof_add/cof_mul for DRP-AI from mean/std */
        in_param.cof_add[0]= -255*mean[0];//-123.675;
        in_param.cof_add[1]= -255*mean[1];//-116.28;
        in_param.cof_add[2]= -255*mean[2];//-103.53;
        in_param.cof_mul[0]= 1/(stdev[0]*255);//0.01712475;
        in_param.cof_mul[1]= 1/(stdev[1]*255);//0.017507;
        in_param.cof_mul[2]= 1/(stdev[2]*255);//0.01742919;
    }
    else
    {
        /*Initial input data opencv container*/
        image.create(me->cap_h, me->cap_w, CV_8UC2);
        image_rgb.create(me->cap_h, me->cap_w, CV_8UC3);
        image_resize.create(me->model_h, me->model_w, CV_8UC3);
        image_float.create(me->model_h, me->model_w,  CV_32FC3);
    }
    /*DRP-AI TVM[*1]::Load model_dir structure and its weight to runtime object */
    runtime.LoadModel(me->dir);

    /*DRP-AI TVM[*1]::Get input data type*/
    auto input_data_type = runtime.GetInputDataType(0);

    /*DRP-AI TVM[*1]::Inference Loop Start*/
    while (me->_inf_running)
    {
        std::cout << "[inf_thread]waiting for inference start..." << std::endl;
        /*Checks if image frame from _capture Thread is ready.*/
        {
            Measuretime m("Inference start wait time");

            unique_lock<mutex> lock(me->mtx_);
            me->cv_.wait(lock, [me] {return me->wake_; });
            me->wake_ = false;
        }
        /*DRP-AI TVM[*1]:: Preprocess*/
        me->get_time(start_time);
        if (drpai_mode)
        {
            /*Define parameter to be changed in PreRuntime*/
            in_param.pre_in_addr = (uintptr_t)me->capture_address;
            /*Run pre-processing*/
            preruntime.Pre(&in_param, &pre_output_ptr, &out_size);
        }
        else
        {
            /*Load input image to opencv container*/
            memcpy(image.data, me->input_data, (me->cap_c * me->cap_w * me->cap_h));
            /*Pre-processing*/
            cv::cvtColor(image, image_rgb, cv::COLOR_YUV2RGB_YUYV);
            cv::resize(image_rgb, image_resize, cv::Size(me->model_w, me->model_h), 0, 0, cv::INTER_LINEAR);
            image_resize.convertTo(image_float, CV_32FC3, 1.0/255.0);
            for (int c = 0; c < (me->model_c) ; c++)
            {
                for (int y = 0; y < (me->model_h) ; y++)
                {
                    for (int x = 0; x < (me->model_w); x++)
                    {
                        val = (image_float.at<cv::Vec3f>(y, x)[c]  - mean[c])/stdev[c];
                        chw.push_back(val);
                    }
                }
            }
            pre_output_ptr = chw.data();
        }
        me->get_time(end_time);
        preproc_time = (float)((me->timedifference_msec(start_time, end_time)));
        print_measure_log("AI preprocess Time", preproc_time, "ms");

        /*Input data type can be either FLOAT32 or FLOAT16, which depends on the model */
        /*For DeepPose, Input=FP32*/
        if (InOutDataType::FLOAT32 == input_data_type)
        {
            std::cout<<"[INFO] Input FP32."<<std::endl;
            /*DRP-AI TVM[*1]::Set input data to DRP-AI TVM[*1]*/
            runtime.SetInput(0, pre_output_ptr);
        }
        else
        {
            std::cerr << "[ERROR] Input data type : not FP32." << std::endl;
            return 0;
        }

        /**DRP-AI TVM[*1]::Start Inference*/
        errno = 0;
        inf_cnt++;
        printf("[START] Start AI inference...\n");
        printf("Inference ----------- No. %d\n", (inf_cnt + 1));
        /*Gets inference starting time*/
        me->get_time(start_time);
        /*DRP-AI TVM[*1]::Run inference*/
        runtime.Run();
        /*Gets AI Inference End Time*/
        me->get_time(end_time);
        /*Inference End Time */
        ai_time = (float)((me->timedifference_msec(start_time, end_time)));
        print_measure_log("AI Inference Time", ai_time, "ms");
        /*Process to read the DRP-AI output data.*/
        /* DRP-AI TVM[*1]::Get the number of output of the target model. For DeepPose, 1 output. */
        auto output_num = runtime.GetNumOutput();
        for (int i = 0;i<output_num;i++)
        {
            /* DRP-AI TVM[*1]::Get the output including meta-data, i.e. data-type */
            auto output_buffer = runtime.GetOutput(i);
            /*Output Data Size = std::get<2>(output_buffer). For DeepPose, out_size = 98*2 = 196*/
            int64_t out_size = std::get<2>(output_buffer);
            uint32_t keypoint_num = out_size / 2;
            drpai_output_buf.reset(new float[me->_outBuffSize], std::default_delete<float[]>());
            /*Output Data Type = std::get<0>(output_buffer)*/
            if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
            {
                /*If DRP-AI*/
                /*Output Data = std::get<1>(output_buffer)*/
                uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
                for (int j = 0;j<keypoint_num;j++)
                {
                    /*FP16 to FP32 conversion*/
                    drpai_output_buf.get()[2*j]=float16_to_float32(data_ptr[2*j]);
                    drpai_output_buf.get()[2*j+1]=float16_to_float32(data_ptr[2*j+1]);
                }
            }
            else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
            {
                /*If CPU*/
                /*Output Data = std::get<1>(output_buffer)*/
                float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
                for (int j = 0;j<keypoint_num;j++)
                {
                    drpai_output_buf.get()[2*j]=data_ptr[2*j];
                    drpai_output_buf.get()[2*j+1]=data_ptr[2*j+1];
                }
            }
            else
            {
                std::cerr << "[ERROR] Output data type : not floating point type." << std::endl;
                break;
            }
        }

        /*Fill AI Inference result structure*/
        data.predict_image = me->input_data;
        data.predict_result = move(drpai_output_buf);
        data.drp_time_ms = ai_time;
        data.preproc_time_ms = preproc_time;
        /*Post-process start (AI inference result postprocess + image compress + JSON data sending)*/
        me->inference_postprocess(arg, data);

        Measuretime m("Deque inference_capture_qbuf buf time");

        chw.clear();
        ret = capture->inference_capture_qbuf();
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to enqueue _capture buffer.\n");
            break;
        }
        me->_ai_frame_count.store(me->_ai_frame_count.load() + 1);
        me->capture_enabled.store(true); /* Flag for _capture Thread. */
    }
    /*End of Inference Loop*/

    /*To terminate the loop in _capture Thread.*/
    me->capture_enabled.store(true);

    cout << "<<<<<<<<<<<<<<<<<<<<< AI Inference Thread Terminated >>>>>>>>>>>>>>>>>>" << endl;
    pthread_exit(NULL);
    me->_pthread_ai_inf = 0;
    return NULL;
}

/**
 * @brief framerate_thread
 * @details Framerate and cpu usage thread
 * @param arg pointer to itself
 * @return void*
 */
void* RecognizeBase::framerate_thread(void* arg)
{
    RecognizeBase* me = (RecognizeBase*)arg;
    CPUUsage notify;
    while (me->_fps_runnning)
    {
        sleep(1);

        /* AI FPS */
        int32_t count = me->_ai_frame_count.load();
        me->_ai_frame_count.store(0);
        print_measure_log("------------------------------>AI FPS", count, "fps");
        /*Camerar FPS*/
        int32_t cam_count = me->_camera_frame_count.load();
        me->_camera_frame_count.store(0);
        print_measure_log("------------------------------>Camera FPS", cam_count, "fps");

        /* CPU usage*/
        string cpuUsage = me->_analyzer.get_cpu_usage(2);
        print_measure_log("CPU Usage", cpuUsage);

        /* Memory usage*/
        float memoryUsage = me->_analyzer.get_memory_usage();
        print_measure_log("Memory Usage", memoryUsage, "%");

        /* send CPU usage and Memory usage*/
        notify.cpu_usage = cpuUsage;
        notify.mem_usage = memoryUsage;

        me->_server->send_command(notify.CreateRequest());
    }
    cout << "<<<<<<<<<<<<<<<<<<<<< FPS Thread Terminated >>>>>>>>>>>>>>>>>>" << endl;
    pthread_exit(NULL);
    me->_pthread_framerate = 0;
    return NULL;
}

/**
 * @brief inference_postprocess
 * @details Postprocess and send command
 * @param arg pointer to itself
 * @param data inference result data
 */
void RecognizeBase::inference_postprocess(void* arg, recognizeData_t& data)
{
    timespec start_time;
    timespec end_time;

    RecognizeBase* me = (RecognizeBase*)arg;

    me->get_time(start_time);
    {
        Measuretime m("Post process time");
        _model->inf_post_process(data.predict_result.get());
    }
    me->get_time(end_time);
    float post_time = (float)((me->timedifference_msec(start_time, end_time)));

    string b64;
    shared_ptr<PredictNotifyBase> notify;

#ifdef SEND_INFERENCE_RESULT_ON
    /* create send image (encoded to base64) */
    b64 = get_send_image(data.predict_image);
#endif

#ifdef COUT_INFERENCE_RESULT_ON
    _model->print_result();
#endif
    {
        Measuretime m("Create predict result time");
        notify = _model->get_command();
    }

    {
        Measuretime m("Command create and send time[predict]");
        /* Create websocket command*/
        notify->img = b64;
        notify->img_org_w = _model->_capture_w;
        notify->img_org_h = _model->_capture_h;
        notify->drp_time = data.drp_time_ms;
        notify->pre_time = data.preproc_time_ms;
        notify->post_time = post_time;

        /*  Send websocket coomand*/
        me->_server->send_command(notify->CreateRequest());

    }

    data.predict_result.reset();
}

/**
 * @brief end_all_threads
 * @details terminate all threads.
 * @return int32_t success :0
 */
int32_t RecognizeBase::end_all_threads()
{
    int32_t ret;
    int32_t ret_main;
    capture_enabled.store(true);

    _capture_running = false;
    _inf_running = false;
    _fps_runnning = false;

    if (0 != _pthread_capture)
    {
        ret = wait_join(&_pthread_capture, CAPTURE_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit _capture Thread on time.[%d]\n", ret);
            ret_main = -1;
        }
    }

    if (0 != _pthread_ai_inf)
    {
        ret = wait_join(&_pthread_ai_inf, AI_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit AI Inference Thread on time.[%d]\n", ret);
            ret_main = -1;
        }
    }

    if (0 != _pthread_framerate)
    {
        ret = wait_join(&_pthread_framerate, FRAMERATE_THREAD_TIMEOUT);
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to exit Framerate Thread on time.\n");
            ret_main = -1;
        }
    }


    std::cout << "********************** ALL THREAD END *********************" << std::endl;

    return ret_main;
}

/**
 * @brief wait_join
 * @details Wait for thread
 * @param p_join_thread target thread.
 * @param join_time timeout time[sec]
 * @return int8_t success:0
 */
int8_t RecognizeBase::wait_join(pthread_t* p_join_thread, uint32_t join_time)
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
/**
 * @brief timedifference_msec
 * @details Calculate time diffrence between t0 and t1
 * @param t0 time (start)
 * @param t1 time (end)
 * @return double diff time[ms]
 */
double RecognizeBase::timedifference_msec(timespec t0, timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1000000.0;
}

/**
 * @brief get current time
 * @details Run timespec_get to obtain the current time
 * @param[out] time_t reference to store current time
 * @return int32_t success:0
 */
int32_t RecognizeBase::get_time(timespec& time_t)
{
    int32_t ret = timespec_get(&time_t, TIME_UTC);
    if (0 == ret)
    {
        fprintf(stderr, "[ERROR] Failed to get Inference Start Time\n");
    }
    return ret;
}

/**
 * @brief close_camera
 * @details Close USB Camea
 */
void RecognizeBase::close_camera()
{
    std::cout << "close_camera" << std::endl;
    /*Close USB Camera.*/
    if (NULL != _capture && 0 != _pthread_capture)
    {
        int8_t ret = _capture->close_camera();
        if (0 != ret)
        {
            fprintf(stderr, "[ERROR] Failed to close USB Camera.\n");
        }
        cout << "Camera Closed." << endl;
    }
}

/**
 * @brief get_send_image
 * @details Get base64 jpeg image
 * @param image input yuyv image
 * @return string base64 jpeg image
 */
string RecognizeBase::get_send_image(uint8_t* image)
{
    vector<uint8_t> output;
    vector<uint8_t> input;
    {
        Measuretime mm("Jpeg compress Time");
        ImageConverter::compress_jpeg_turbo(image, _model->_capture_w, _model->_capture_h, output, JPEG_QUALUTY);
    }

    ////////////////////
    // jpeg->Base64
    string b64;
    {
        Measuretime m("Base64 encode time");
        b64 = lwsock::b64encode(output.data(), output.size());
    }
    return b64;
}
