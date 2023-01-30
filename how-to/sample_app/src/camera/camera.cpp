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
* File Name    : camera.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "camera.h"
#include <errno.h>
#include <cmath>
#include <iostream>

#include "../util/measure_time.h"

Camera::Camera()
{
    camera_width = 0;
    camera_height = 0;
    camera_color = 0;
}

Camera::~Camera()
{
}

/**
 * @brief ceil3
 * @details ceil num specifiy digit
 * @param num number
 * @param base ceil digit
 * @return int32_t result
 */
static int32_t ceil3(int32_t num, int32_t base)
{
    double x = (double)(num) / (double)(base);
    double y = ceil(x) * (double)(base);
    return (int32_t)(y);
}

/**
 * @brief calc_udmabuf_addr
 * @details calclate u-dma-buf address
 * @return uint64_t u-dma-buf address
 */
static uint64_t calc_udmabuf_addr()
{
    uint64_t ret_address = 0;

    /* Obtain udmabuf memory area starting address */
    int8_t fd = 0;
    char addr[1024];
    int32_t read_ret = 0;
    errno = 0;
    fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY);
    if (0 > fd)
    {
        fprintf(stderr, "[ERROR] Failed to open udmabuf0/phys_addr : errno=%d\n", errno);
        return -1;
    }
    read_ret = read(fd, addr, 1024);
    if (0 > read_ret)
    {
        fprintf(stderr, "[ERROR] Failed to read udmabuf0/phys_addr : errno=%d\n", errno);
        close(fd);
        return -1;
    }
    sscanf(addr, "%lx", &ret_address);
    close(fd);
    /* Filter the bit higher than 32 bit */
    ret_address &= 0xFFFFFFFF;

    return ret_address;
}

/**
 * @brief start_camera
 * @details  Function to initialize USB camera capture
 * @return int8_t  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::start_camera()
{
    int8_t ret = 0;
    int32_t i = 0;
    int32_t n = 0;

    printf("Camera width = %d\n", camera_width);
    printf("Camera height = %d\n", camera_height);
    printf("Camera channel = %d\n", camera_color);

    ret = open_camera_device();
    if (0 != ret) return ret;

    ret = init_camera_fmt();
    if (0 != ret) return ret;

    ret = init_buffer();
    if (0 != ret) return ret;

    udmabuf_address = calc_udmabuf_addr();

    udmabuf_file = open("/dev/udmabuf0", O_RDWR);
    if (0 > udmabuf_file)
    {
        printf("[ERROR] /dev/udmabuf0 open Failed...\n");
        return -1;
    }
    /* page size alignment.*/
    int32_t offset = ceil3(imageLength, sysconf(_SC_PAGE_SIZE));
    _offset = offset;
    for (n = 0; n < CAP_BUF_NUM; n++)
    {
        /* fit to page size.*/
        buffer[n] = (uint8_t*)mmap(NULL, imageLength, PROT_READ | PROT_WRITE, MAP_SHARED, udmabuf_file, n * offset);

        if (MAP_FAILED == buffer[n])
        {
            printf("print error string by strerror: %s\n", strerror(errno));
            return -1;
        }

        /* Write once to allocate physical memory to u-dma-buf virtual space.
        * Note: Do not use memset() for this.
        *       Because it does not work as expected. */
        {
            uint8_t* word_ptr = buffer[n];
            for (i = 0; i < _offset; i++)
            {
                word_ptr[i] = 0;
            }
        }

        memset(&buf_capture, 0, sizeof(buf_capture));
        buf_capture.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf_capture.memory = V4L2_MEMORY_USERPTR;
        buf_capture.index = n;
        /* buffer[i] must be casted to unsigned long type in order to assign it to V4L2 buffer */
        buf_capture.m.userptr = reinterpret_cast<unsigned long>(buffer[n]);
        buf_capture.length = imageLength;
        ret = xioctl(m_fd, VIDIOC_QBUF, &buf_capture);
        if (-1 == ret)
        {
            return -1;
        }
    }

    ret = start_capture();
    if (0 != ret) return ret;

    return 0;
}


/**
 * @brief close_capture
 * @details Close camera and free buffer
 * @return int8_t  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::close_camera()
{
    int8_t ret = 0;
    int32_t i = 0;

    ret = stop_capture();
    if (0 != ret) return ret;

    for (i = 0; i < CAP_BUF_NUM; i++)
    {
        munmap(buffer[i], _offset);
    }
    close(udmabuf_file);
    close(m_fd);
    return 0;
}

/**
 * @brief xioctl
 * @details  ioctl calling
 * @param fd V4L2 file descriptor
 * @param request V4L2 control ID defined in videodev2.h
 * @param arg set value
 * @return int8_t output parameter
 */
int8_t Camera::xioctl(int8_t fd, int32_t request, void* arg)
{
    int8_t r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

/**
 * @brief start_capture
 * @details Set STREAMON
 * @return int8_t 0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::start_capture()
{
    int8_t ret = 0;
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = xioctl(m_fd, VIDIOC_STREAMON, &buf.type);
    if (-1 == ret)
    {
        return -1;
    }
    return 0;
}


/**
 * @brief capture_qbuf
 * @details Function to enqueue the buffer.
 *                 (Call this function after capture_image() to restart filling image data into buffer)
 * @return int8_t  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::capture_qbuf()
{
    int8_t ret = 0;

    ret = xioctl(m_fd, VIDIOC_QBUF, &buf_capture);
    if (-1 == ret)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief capture_image
 * @details  Function to capture image and return the physical memory address where the captured image stored.
 *                 Must call capture_qbuf after calling this function.
 * @return uint32_t the physical memory address where the captured image stored.
 */
uint32_t Camera::capture_image()
{
    int8_t ret = 0;
    fd_set fds;
    /*Delete all file descriptor from fds*/
    FD_ZERO(&fds);
    /*Add m_fd to file descriptor set fds*/
    FD_SET(m_fd, &fds);


    /* Check when a new frame is available */
    while (1)
    {
        ret = select(m_fd + 1, &fds, NULL, NULL, NULL);
        if (0 > ret)
        {
            if (EINTR == errno)
            {
                cout << "capture select error!" << endl;
                continue;
            }
            return 0;
        }
        break;
    }

    /* Get buffer where camera stored data */

    ret = xioctl(m_fd, VIDIOC_DQBUF, &buf_capture);
    if (-1 == ret)
    {
        cout << "capture select error!" << endl;
        return 0;
    }
    return udmabuf_address + buf_capture.index * _offset;
}

/**
 * @brief stop_capture
 * @details Set STREAMOFF
 * @return int8_t 0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::stop_capture()
{
    //printf("stop_capture");
    int8_t ret = 0;
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;

    ret = xioctl(m_fd, VIDIOC_STREAMOFF, &buf.type);
    if (-1 == ret)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief open_camera_device
 * @details Function to open camera *called by start_camera
 * @return int8_t 0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::open_camera_device()
{
    char dev_name[4096] = { 0 };
    int32_t i = 0;
    int8_t ret = 0;
    struct v4l2_capability fmt;

    for (i = 0; i < 15; i++)
    {
        snprintf(dev_name, sizeof(dev_name), "/dev/video%d", i);
        m_fd = open(dev_name, O_RDWR);
        if (m_fd == -1)
        {
            continue;
        }

        /* Check device is valid (Query Device information) */
        memset(&fmt, 0, sizeof(fmt));
        ret = xioctl(m_fd, VIDIOC_QUERYCAP, &fmt);
        if (-1 == ret)
        {
            return -1;
        }

        /* Search USB camera */
        ret = strcmp((const char*)fmt.driver, "uvcvideo");
        if (0 == ret)
        {
            printf("[INFO] USB Camera: %s\n", dev_name);
            break;
        }
        close(m_fd);
    }

    if (i >= 15)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief init_camera_fmt
 * @details Function to request format *called by start_camera
 * @return int8_t  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::init_camera_fmt()
{
    int8_t ret = 0;
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = camera_width;
    fmt.fmt.pix.height = camera_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;


    ret = xioctl(m_fd, VIDIOC_S_FMT, &fmt);
    if (-1 == ret)
    {
        printf("[ERROR] VIDIOC_S_FMT Failed: %d\n", ret);
        return -1;
    }

    struct v4l2_streamparm* setfps;
    setfps = (struct v4l2_streamparm*)calloc(1, sizeof(struct v4l2_streamparm));
    memset(setfps, 0, sizeof(struct v4l2_streamparm));
    setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps->parm.capture.timeperframe.numerator = 1;
    setfps->parm.capture.timeperframe.denominator = 30;
    if (ioctl(m_fd, VIDIOC_S_PARM, setfps) < 0)
    {
        perror("VIDIOC_S_PARM");
    }

    return 0;
}

/**
 * @brief init_buffer
 * @details  Initialize camera buffer *called by start_camera
 * @return  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::init_buffer()
{
    int8_t ret = 0;
    int32_t i = 0;
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = CAP_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    /*Request a buffer that will be kept in the device*/
    ret = xioctl(m_fd, VIDIOC_REQBUFS, &req);
    if (-1 == ret)
    {
        printf("[ERROR] VIDIOC_REQBUFS Failed: %d\n", ret);
        return -1;
    }

    struct v4l2_buffer buf;
    for (i = 0; i < CAP_BUF_NUM; i++)
    {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;

        /* Extract buffer information */
        ret = xioctl(m_fd, VIDIOC_QUERYBUF, &buf);
        if (-1 == ret)
        {
            printf("[ERROR] VIDIOC_QUERYBUF Failed: %d\n", ret);
            return -1;
        }

    }
    imageLength = buf.length;

    return 0;
}

/**
 * @brief save_bin
 * @details  Get the capture image from buffer and save it into binary file
 * @param filename binary file name to be saved
 * @return int8_t  0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::save_bin(std::string filename)
{
    int8_t ret = 0;
    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        return -1;
    }

    /* Get data from buffer and write to binary file */
    ret = fwrite(buffer[buf_capture.index], sizeof(uint8_t), imageLength, fp);
    if (!ret)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}


/**
 * @brief get_buf_capture_index
 * @details Function to return the camera buffer index
 * @return int8_t  camera buffer index
 */
int8_t Camera::get_buf_capture_index()
{
    return buf_capture.index;
}

/**
 * @brief get_inference_buf_capture_index
 * @details Function to return the inference camera buffer index
 * @return int8_t inference camera buffer index
 */
int8_t Camera::get_inference_buf_capture_index()
{
    return inference_buf_capture.index;
}


/**
 * @brief sync_inference_buf_capture
 * @details Function to sync the camera buffer and the inference camera buffer
 */
void Camera::sync_inference_buf_capture()
{
    _using_inf = true;
    inference_buf_capture = buf_capture;
    return;
}

/**
 * @brief inference_capture_qbuf
 * @details  Function to enqueue the inference buffer.
 *                 (Call this function at the end of the inference thread to
 *                  restart filling image data into buffer)
 * @return int8_t  0 if succeeded
*                 not 0 otherwise
 */
int8_t Camera::inference_capture_qbuf()
{
    int8_t ret = 0;

    ret = xioctl(m_fd, VIDIOC_QBUF, &inference_buf_capture);
    if (-1 == ret)
    {
        return -1;
    }
    _using_inf = false;

    return 0;
}


/**
 * @brief get_img
 * @details Function to return the camera buffer
 * @return uint8_t* camera buffer
 */
uint8_t* Camera::get_img()
{
    return buffer[buf_capture.index];
}


/**
 * @brief get_size
 * @details Function to return the camera buffer size (W x H x C)
 * @return int32_t camera buffer size (W x H x C )
 */
int32_t Camera::get_size()
{
    return imageLength;
}


/**
 * @brief get_w
 * @details Get camera_width. This function is currently NOT USED.
 * @return int32_t width of camera capture image.
 */
int32_t Camera::get_w()
{
    return camera_width;
}


/**
 * @brief set_w
 * @details Set camera_width. This function is currently NOT USED.
 * @param w new camera capture image width
 */
void Camera::set_w(int32_t w)
{
    camera_width = w;
    return;
}


/**
 * @brief get_h
 * @details Get camera_height. This function is currently NOT USED.
 * @return int32_t height of camera capture image.
 */
int32_t Camera::get_h()
{
    return camera_height;
}


/**
 * @brief set_h
 * @details Set camera_height. This function is currently NOT USED.
 * @param h new camera capture image height
 */
void Camera::set_h(int32_t h)
{
    camera_height = h;
    return;
}


/**
 * @brief get_c
 * @details Get camera_color. This function is currently NOT USED.
 * @return int32_t color channel of camera capture image.
 */
int32_t Camera::get_c()
{
    return camera_color;
}


/**
 * @brief set_c
 * @details Set camera_color. This function is currently NOT USED.
 * @param c new camera capture image color channel
 */
void Camera::set_c(int32_t c)
{
    camera_color = c;
    return;
}
