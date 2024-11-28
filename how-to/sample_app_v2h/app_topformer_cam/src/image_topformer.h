/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : image.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for TopFormer with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include "define.h"
#include "class_info.h"
#include "camera.h"

class Image
{
    public:
        Image();
        ~Image();

        uint8_t* img_buffer[WL_BUF_NUM];
        uint8_t* overlay_buffer[WL_BUF_NUM];
        uint8_t get_buf_id();
        void write_string_rgb(std::string str, uint32_t x, uint32_t y, float size, uint32_t color);
        void write_string_overlay(std::string str, uint32_t align_type, uint32_t x, uint32_t y, float scale, uint32_t color);
        int8_t draw_sem_seg(uint8_t* buffer, Camera *capture, size_t class_w, size_t class_h, size_t seg_w, size_t seg_h, bool is_padding);

        uint32_t get_H();
        uint32_t get_W();
        uint32_t get_C();
        uint8_t* get_img(uint8_t id);
        uint8_t* get_overlay_img(uint8_t id);
        uint8_t at(int32_t a);
        void set(int32_t a, uint8_t val);

        uint8_t init(uint32_t w, uint32_t h, uint32_t c, uint32_t ow, uint32_t oh, uint32_t oc, void *mem_wayland, void *mem_overlay);
        
        void reset_overlay_img(uint8_t id);
        void draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
        void convert_format();
        void convert_size(int in_w, int resize_w, int in_h, int resize_h, bool is_padding);
        void camera_to_image(const uint8_t* buffer, int32_t size);
        void draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
        void draw_line2(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);

    private:
        uint8_t buf_id = 0;

        /* Input Image (YUYV from V4L2) Information */
        uint32_t img_h;
        uint32_t img_w;
        uint32_t img_c;
        /* Output Image (BGRA for Wayland) Information */
        uint32_t out_h;
        uint32_t out_w;
        uint32_t out_c;

        void draw_point_yuyv(int32_t x, int32_t y, uint32_t color);
        uint8_t Clip(int value);
};

#endif
