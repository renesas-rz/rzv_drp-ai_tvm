/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : image.cpp
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for MiDaS with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "image_midas.h"
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
*                 mem_wayland = pointer to the memory for the display buffer
*                 mem_overlay = pointer to the memory for the overlay buffer
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
uint8_t Image::init(uint32_t w, uint32_t h, uint32_t c,
                    uint32_t ow, uint32_t oh, uint32_t oc, void *mem_wayland, void *mem_overlay)      
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
        img_buffer[i] =(unsigned char*)mem_wayland+(i*out_size);
        overlay_buffer[i] =(unsigned char*)mem_overlay+(i*out_size);
    }

    return 0;
}

/*****************************************
* Function Name : draw_point_yuyv
* Description   : Draw a single point on YUYV image
* Arguments     : x = X coordinate to draw a point
*                 y = Y coordinate to draw a point
*                 color = point color
* Return Value  : -
******************************************/
void Image::draw_point_yuyv(int32_t x, int32_t y, uint32_t color)
{
    uint32_t x0 = (uint32_t)x & ~0x1;

    uint32_t draw_u = (color >> 8) & 0xFF;
    uint32_t draw_v = (color >> 0) & 0xFF;

    uint32_t target_u = img_buffer[buf_id][(y * img_w + x0) * img_c + 1];
    uint32_t target_v = img_buffer[buf_id][(y * img_w + x0) * img_c + 3];

    img_buffer[buf_id][(y * img_w + x0) * img_c + 1] = (draw_u + target_u) / 2;
    img_buffer[buf_id][(y * img_w + x0) * img_c + 3] = (draw_v + target_v) / 2;

    if ((x & 1) == 0)
    {
        img_buffer[buf_id][(y * img_w + x0) * img_c]     = (color >> 16) & 0xFF;
    }
    else
    {
        img_buffer[buf_id][(y * img_w + x0) * img_c + 2] = (color >> 16) & 0xFF;
    }
    return;
}

/*****************************************
* Function Name : draw_line
* Description   : Draw a single line
* Arguments     : x0 = X coordinate of a starting point
*                 y0 = Y coordinate of a starting point
*                 x1 = X coordinate of a end point
*                 y1 = Y coordinate of a end point
*                 color = line color
* Return Value  : -
******************************************/
void Image::draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;
    int32_t sx = 1;
    int32_t sy = 1;
    float de;
    int32_t i;

    /* Change direction */
    if (0 > dx)
    {
        dx *= -1;
        sx *= -1;
    }

    if (0 > dy)
    {
        dy *= -1;
        sy *= -1;
    }

    draw_point_yuyv(x0, y0, color);

    if (dx > dy)
    {
        /* Horizontal Line */
        for (i = dx, de = i / 2; i; i--)
        {
            x0 += sx;
            de += dy;
            if(de > dx)
            {
                de -= dx;
                y0 += sy;
            }
            draw_point_yuyv(x0, y0, color);
        }
    }
    else
    {
        /* Vertical Line */
        for (i = dy, de = i / 2; i; i--)
        {
            y0 += sy;
            de += dx;
            if(de > dy)
            {
                de -= dy;
                x0 += sx;
            }
            draw_point_yuyv(x0, y0, color);
        }
    }
    return;
}


/*****************************************
* Function Name : draw_line2
* Description   : Draw a dowble line
* Arguments     : x0 = X coordinate of a starting point
*                 y0 = Y coordinate of a starting point
*                 x1 = X coordinate of a end point
*                 y1 = Y coordinate of a end point
*                 color = line color
* Return Value  : -
******************************************/
void Image::draw_line2(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
    int32_t x_min = x0;
    int32_t y_min = y0;
    int32_t x_max = x1;
    int32_t y_max = y1;
    
    draw_line(x_min, y_min, x_max, y_max, color);
    draw_line(x_min-1, y_min-1, x_max+1, y_max-1, color);
    return;
}


/*****************************************
* Function Name : draw_rect
* Description   : Draw a rectangle
* Arguments     : x = X coordinate at the top left of the rectangle
*                 y = Y coordinate at the top left of the rectangle
*                 w = width of the rectangle
*                 h = height of the rectangle
* Return Value  : -
******************************************/
void Image::draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
    int32_t x_min = x;
    int32_t y_min = y;
    int32_t x_max = x + w;
    int32_t y_max = y + h;
    /* Check the bounding box is in the image range */
    x_min = x_min < 1 ? 1 : x_min;
    x_max = (((int32_t)img_w - 2) < x_max) ? ((int32_t)img_w - 2) : x_max;
    y_min = y_min < 1 ? 1 : y_min;
    y_max = (((int32_t)img_h - 2) < y_max) ? ((int32_t)img_h - 2) : y_max;

    /* Draw the bounding box */
    draw_line(x_min, y_min, x_max, y_min, color);
    draw_line(x_max, y_min, x_max, y_max, color);
    draw_line(x_max, y_max, x_min, y_max, color);
    draw_line(x_min, y_max, x_min, y_min, color);
    draw_line(x_min-1, y_min-1, x_max+1, y_min-1, color);
    draw_line(x_max+1, y_min-1, x_max+1, y_max+1, color);
    draw_line(x_max+1, y_max+1, x_min-1, y_max+1, color);
    draw_line(x_min-1, y_max+1, x_min-1, y_min-1, color);

    return;
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

    cv::Mat yuyv_image(img_h, img_w, CV_8UC2, img_buffer[buf_id]);
    cv::Mat bgra_image;
    cv::Mat out_image(img_h, img_w, CV_8UC4, img_buffer[buf_id]);
    cv::cvtColor(yuyv_image, bgra_image, cv::COLOR_YUV2BGRA_YUYV);
    memcpy(out_image.data, bgra_image.data, img_w * img_h * out_c);

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
*                 in_w = width of current buffered image, which is mainly camera captured image.
*                 resize_w = width of resized image, which is mainly displayed on HDMI.
*                 in_h = height of current buffered image, which is mainly camera captured image.
*                 resize_h = height of resized image, which is mainly displayed on HDMI.
* Return value  : -
******************************************/
void Image::convert_size(int in_w, int resize_w, int in_h, int resize_h)
{
    // Return if resizing and padding is unnecessary
    if ( in_w == resize_w && in_h == resize_h )
    {
        return;
    }

#ifdef DEBUG_TIME_FLG
    using namespace std;
    chrono::system_clock::time_point start, end;
    start = chrono::system_clock::now();
#endif // DEBUG_TIME_FLG

    cv::Mat org_image(img_h, img_w, CV_8UC4, img_buffer[buf_id]);
    cv::Mat dst_image = org_image;  // shallow copy
    cv::Mat resize_image;
    cv::Mat padding_image;

    /* Resize */
    /* Use INTER_NEAREST which is faster than INTER_BILINEAR as long as OpenCVA is disabled. */
    cv::resize(dst_image, resize_image, cv::Size(resize_w, resize_h), 0, 0, cv::INTER_NEAREST);

    // Update reference (shallow copy)
    dst_image = resize_image;
    
    memcpy(img_buffer[buf_id], dst_image.data, resize_w * resize_h * out_c);	
    
#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Convert Size Time         : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG
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
    buf_id = (buf_id + 1) % WL_BUF_NUM;
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
void Image::reset_overlay_img(uint8_t id)
{
#ifdef DEBUG_TIME_FLG
    using namespace std;
    chrono::system_clock::time_point start, end;
    start = chrono::system_clock::now();
#endif // DEBUG_TIME_FLG

    cv::Mat src_image = cv::Mat::zeros(out_h, out_w, CV_8UC4);
    uint8_t* dst = overlay_buffer[id];
    uint8_t* src = src_image.data;
    memcpy(dst, src, out_w * out_h * out_c);

#ifdef DEBUG_TIME_FLG
    end = chrono::system_clock::now();
    double time = static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0);
    printf("Reset Overlay Buffer Time : %lf[ms]\n", time);
#endif // DEBUG_TIME_FLG
}

/*****************************************
* Function Name : write_string_rgb
* Description   : OpenCV putText() in RGB
* Arguments     : str = string to be drawn
*                 align_type : 1=TopLeft, 2=TopRight
*                 x = bottom left coordinate X of string to be drawn
*                 y = bottom left coordinate Y of string to be drawn
*                 scale = scale for letter size
*                 color = letter color must be in RGB, e.g. white = 0xFFFFFF
* Return Value  : -
******************************************/
void Image::write_string_rgb(std::string str, uint32_t align_type, uint32_t x, uint32_t y, float scale, uint32_t color)
{
    uint8_t thickness = CHAR_THICKNESS;
    /*Extract RGB information*/
    uint8_t r = (color >> 16) & 0x0000FF;
    uint8_t g = (color >> 8) & 0x0000FF;
    uint8_t b = (color >> 0) & 0x0000FF;
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
* Function Name : draw_depth_map
* Description   : apply a colormap to draw a depth map
* Arguments     : buffer = inference result
*                 capture = pointer to the memory for capture class
*                 depth_w = width of depth map (inference result)
*                 depth_h = height of depth map (inference result)
*                 resize_w = width of resized image, which is mainly displayed on HDMI
*                 resize_h = height of resized image, which is mainly displayed on HDMI
* Return value  : 0 if succeeded
*               not 0 otherwise
******************************************/
int8_t Image::draw_depth_map(uint8_t* buffer, Camera *capture, size_t depth_w, size_t depth_h, size_t resize_w, size_t resize_h)
{
    cv::Mat src(depth_h, depth_w, CV_8UC1, buffer);
    // Color Map Apply
    cv::Mat color_map;
    cv::applyColorMap(src, color_map, cv::COLORMAP_INFERNO);
    cv::Mat color_map_rgba;
    cv::cvtColor(color_map, color_map_rgba, cv::COLOR_BGR2BGRA);

    // Resize RGBA depth map ( depth_w x depth_h x RGBA ) to Camera displya image size ( resize_w x resize_h x 4 )
    cv::Mat depth_rgba_resized;
    cv::resize(color_map_rgba, depth_rgba_resized, cv::Size(resize_w, resize_h), 0, 0, cv::INTER_LINEAR);
    memcpy(this->overlay_buffer[buf_id], depth_rgba_resized.data, resize_w * resize_h * this->out_c);

    int ret = capture->video_buffer_flush_dmabuf(capture->overlay_buf->idx, capture->overlay_buf->size);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

/*****************************************
* Function Name : create_side_by_side
* Description   : place cam image and depth map side by side
* Arguments     : resized_w = width of resized image.
*                 resized_h = height of resized image.
*                 is_padding = whether padding or not between segmentation image resolution and HDMI resolution.
* Return value  : 0 if succeeded
*               not 0 otherwise
******************************************/
int8_t Image::create_side_by_side(int resized_w, int resized_h, bool is_padding)
{
    cv::Mat cam_image(resized_h, resized_w, CV_8UC4, img_buffer[buf_id]);
    cv::Mat depth_image(resized_h, resized_w, CV_8UC4, overlay_buffer[buf_id]);
    cv::Mat concat_image;
    cv::hconcat(cam_image, depth_image, concat_image);

    // Embed resized concat image ((resized_w x 2) x resized_h x 4 ) into Display buffer ( out_w * out_h * out_c )
    uint32_t concat_w = resized_w * 2;
    if ( is_padding )
    {
        uint32_t pad_left = (this->out_w - concat_w) / 2;
        uint32_t pad_right =  this->out_w - concat_w - pad_left;
        uint32_t pad_top = (this->out_h - resized_h) / 2;
        uint32_t pad_bottom = this->out_h - resized_h - pad_top;

        cv::Mat dst_image;
        copyMakeBorder(concat_image, dst_image, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0, 255));
        memcpy(this->img_buffer[buf_id], dst_image.data, this->out_w * this->out_h * this->out_c);
    }
    else
    {
        if(concat_w != this->out_w || resized_h != this->out_h)
        {
            cv::resize(concat_image, concat_image, cv::Size(this->out_w, this->out_h), 0, 0, cv::INTER_LINEAR);
        }
        // no padding
        memcpy(this->img_buffer[buf_id], concat_image.data, this->out_w * this->out_h * this->out_c);
    }
    return 0;
}