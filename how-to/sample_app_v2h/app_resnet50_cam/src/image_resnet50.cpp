/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : image.cpp
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for PyTorch ResNet with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "image_resnet50.h"
#include <opencv2/opencv.hpp>

Image::Image()
{

}


Image::~Image()
{

}


/*****************************************
* Function Name : get_H
* Description   : Function to get the image height
*                 This function is NOT used currently.
* Arguments     : -
* Return value  : img_h = current image height
******************************************/
uint32_t Image::get_H()
{
    return img_h;
}


/*****************************************
* Function Name : get_W
* Description   : Function to get the image width
*                 This function is NOT used currently.
* Arguments     : -
* Return value  : img_w = current image width
******************************************/
uint32_t Image::get_W()
{
    return img_w;
}


/*****************************************
* Function Name : get_C
* Description   : Function to set the number of image channel
*                 This function is NOT used currently.
* Arguments     : c = new number of image channel to be set
* Return value  : -
******************************************/
uint32_t Image::get_C()
{
    return img_c;
}

/*****************************************
* Function Name : get_img
* Description   : Function to return the camera buffer
* Arguments     : -
* Return value  : camera buffer
******************************************/
uint8_t* Image::get_img(uint8_t id)
{
    return img_buffer[id];
}

uint8_t* Image::get_overlay_img(uint8_t id)
{
    return overlay_buffer[id];
}


/*****************************************
* Function Name : init
* Description   : Function to initialize img_buffer in Image class
*                 This application uses udmabuf in order to
*                 continuous memory area for DRP-AI input data
* Arguments     : w = input image width in YUYV
*                 h = input image height in YUYV
*                 c = input image channel in YUYV
*                 ow = output image width in BGRA to be displayed via Wayland
*                 oh = output image height in BGRA to be displayed via Wayland
*                 oc = output image channel in BGRA to be displayed via Wayland
*                 mem = pointer to the memory for the display buffer
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t Image::init(uint32_t w, uint32_t h, uint32_t c,
                    uint32_t ow, uint32_t oh, uint32_t oc, void *mem)
{
    int32_t i;
    
    /*Initialize input image information */
    img_w = w;
    img_h = h;
    img_c = c;
    /*Initialize output image information*/
    out_w = ow;
    out_h = oh;
    out_c = oc;

    uint32_t out_size = out_w * out_h * out_c;

    for (i = 0; i < WL_BUF_NUM; i++)
    {
        img_buffer[i] =(unsigned char*)mem+(i*out_size);
    }
    
    return 0;
}


/*****************************************
* Function Name : write_string_rgb
* Description   : OpenCV putText() in RGB
* Arguments     : str = string to be drawn
*                 x = bottom left coordinate X of string to be drawn
*                 y = bottom left coordinate Y of string to be drawn
*                 scale = scale for letter size
*                 color = letter color must be in RGB, e.g. white = 0xFFFFFF
* Return Value  : -
******************************************/
void Image::write_string_rgb(std::string str, uint32_t align_type,  uint32_t x, uint32_t y, float scale, uint32_t color)
{
    uint8_t thickness = CHAR_THICKNESS;
    /*Extract RGB information*/
    uint8_t r = (color >> 16) & 0x0000FF;
    uint8_t g = (color >>  8) & 0x0000FF;
    uint8_t b = color & 0x0000FF;
    int ptx = 0;
    int pty = 0;
    /*OpenCV image data is in BGRA */
    cv::Mat bgra_image(out_h, out_w, CV_8UC4, img_buffer[buf_id]);

    int baseline = 0;
    cv::Size size = cv::getTextSize(str.c_str(), cv::FONT_HERSHEY_SIMPLEX, scale, thickness + 2, &baseline);
    if (align_type == 1)
    {
        ptx = x;
        pty = y;
    }
    else if (align_type == 2)
    {
        ptx = out_w - (size.width + x);
        pty = y;
    }
    /*Color must be in BGR order*/
    cv::putText(bgra_image, str.c_str(), cv::Point(ptx, pty), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(0x00, 0x00, 0x00, 0xFF), thickness + 2);
    cv::putText(bgra_image, str.c_str(), cv::Point(ptx, pty), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(b, g, r, 0xFF), thickness);
}


/*****************************************
* Function Name : convert_format
* Description   : Convert YUYV image to BGRA format
* Arguments     : -
* Return value  : -
******************************************/
void Image::convert_format()
{
#ifdef DEBUG_TIME_FLG
    using namespace std;
    chrono::system_clock::time_point start, end;
    start = chrono::system_clock::now();
#endif // DEBUG_TIME_FLG

    uint8_t* pd = img_buffer[buf_id];
    uint8_t buffer[img_w * img_h * out_c];
    int pix_count = 0;
    for (int i = 0; i < img_h * img_w / 2; i++)
    {
        int y0 = (int)pd[0] - 16;
        int u0 = (int)pd[1] - 128;
        int y1 = (int)pd[2] - 16;
        int v0 = (int)pd[3] - 128;

        pd += 4;
        buffer[pix_count++] = Clip((298 * y0 + 516 * u0 + 128) >> 8); // blue
        buffer[pix_count++] = Clip((298 * y0 - 100 * u0 - 208 * v0 + 128) >> 8); // green
        buffer[pix_count++] = Clip((298 * y0 + 409 * v0 + 128) >> 8); // red
        buffer[pix_count++] = 255;

        buffer[pix_count++] = Clip((298 * y1 + 516 * u0 + 128) >> 8); // blue
        buffer[pix_count++] = Clip((298 * y1 - 100 * u0 - 208 * v0 + 128) >> 8); // green
        buffer[pix_count++] = Clip((298 * y1 + 409 * v0 + 128) >> 8); // red
        buffer[pix_count++] = 255;
    }
    memcpy(img_buffer[buf_id], &buffer, img_w * img_h * out_c);

#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Convert YUYV To BGRA Time : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG
}

uint8_t Image::Clip(int value)
{
    //unsigned char ret = (uint8_t)std::round(value);
    if (value > 255)
    {
        value = 255;
    }
    if (value < 0)
    {
        value = 0;
    }
    return value;
}


/*****************************************
* Function Name : convert_size
* Description   : Scale down the input data (1920x1080) to the output data (1280x720) using OpenCV.
* Arguments     : -
* Return value  : -
******************************************/
void Image::convert_size(int in_w, int resize_w, bool is_padding)
{
    if (in_w == resize_w)
    {
        return;
    }

    cv::Mat org_image(img_h, img_w, CV_8UC4, img_buffer[buf_id]);
    cv::Mat resize_image;
    /* Resize */
    cv::resize(org_image, resize_image, cv::Size(), 1.0 * resize_w / in_w, 1.0 * resize_w / in_w);
	
    if (is_padding)
    {
        cv::Mat dst_image;
        copyMakeBorder(resize_image, dst_image, 0, 0, (out_w - resize_w) / 2, (out_w - resize_w) / 2, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0, 255));
        memcpy(img_buffer[buf_id], dst_image.data, out_w * out_h * out_c);
    }
    else
    {
        memcpy(img_buffer[buf_id], resize_image.data, out_w * out_h * out_c);
    }
}

/*****************************************
* Function Name : camera_to_image
* Description   : Function to copy the external image buffer data to img_buffer
*                 This is only place where the buf_id is updated.
* Arguments     : buffer = buffer to copy the image data
*                 size = size of buffer
* Return value  : none
******************************************/
void Image::camera_to_image(const uint8_t* buffer, int32_t size)
{
    /* Update buffer id */
    buf_id = ++buf_id % WL_BUF_NUM;
    memcpy(img_buffer[buf_id], buffer, sizeof(uint8_t)*size);
}


/*****************************************
* Function Name : at
* Description   : Get the value of img_buffer at index a.
*                 This function is NOT used currently.
* Arguments     : a = index of img_buffer
* Return Value  : value of img_buffer at index a
******************************************/
uint8_t Image::at(int32_t a)
{
    return img_buffer[buf_id][a];
}

/*****************************************
* Function Name : set
* Description   : Set the value of img_buffer at index a.
*                 This function is NOT used currently.
* Arguments     : a = index of img_buffer
*                 val = new value to be set
* Return Value  : -
******************************************/
void Image::set(int32_t a, uint8_t val)
{
    img_buffer[buf_id][a] = val;
    return;
}
/*****************************************
* Function Name : get_buf_id
* Description   : Get the value of the buf_id.
* Arguments     : -
* Return Value  : value of buf_id-
******************************************/
uint8_t Image::get_buf_id(void)
{
    return buf_id;
}


/*****************************************
* Function Name : reset_overlay_img
* Description   : -
* Arguments     : -
* Return Value  : -
******************************************/
void Image::reset_overlay_img()
{
#ifdef DEBUG_TIME_FLG
    using namespace std;
    chrono::system_clock::time_point start, end;
    start = chrono::system_clock::now();
#endif // DEBUG_TIME_FLG

    cv::Mat src_image = cv::Mat::zeros(out_h, out_w, CV_8UC4);
    uint8_t* dst = overlay_buffer[buf_id];
    uint8_t* src = src_image.data;
    memcpy(dst, src, out_w * out_h * out_c);

#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Reset Overlay Buffer Time : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG
}
