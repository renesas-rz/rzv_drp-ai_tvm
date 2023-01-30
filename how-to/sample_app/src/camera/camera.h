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
* File Name    : camera.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <linux/videodev2.h>
#include "define.h"

class Camera
{
    public:
        Camera();
        ~Camera();

        int8_t start_camera();
        int8_t capture_qbuf();
        uint32_t capture_image();
        int8_t close_camera();
        int8_t save_bin(std::string filename);

        int8_t get_buf_capture_index();
        int8_t get_inference_buf_capture_index();
        void sync_inference_buf_capture();
        int8_t inference_capture_qbuf();

        uint8_t * get_img();
        int32_t get_size();
        int32_t get_w();
        void set_w(int32_t w);
        int32_t get_h();
        void set_h(int32_t h);
        int32_t get_c();
        void set_c(int32_t c);
        bool _using_inf;

    private:
        std::string device;
        int32_t camera_width;
        int32_t camera_height;
        int32_t camera_color;
        int8_t m_fd;
        int32_t imageLength;
        uint8_t *buffer[CAP_BUF_NUM];
        int8_t udmabuf_file;
        int32_t _offset;
        uint64_t udmabuf_address;
        struct v4l2_buffer buf_capture;
        struct v4l2_buffer inference_buf_capture;

        int8_t xioctl(int8_t fd, int32_t request, void *arg);
        int8_t start_capture();
        int8_t stop_capture();
        int8_t open_camera_device();
        int8_t init_camera_fmt();
        int8_t init_buffer();
};

#endif
