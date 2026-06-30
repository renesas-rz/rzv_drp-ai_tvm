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
* Version      : 1.2.0
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "camera.h"
#include <errno.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include "../util/measure_time.h"

Camera::Camera()
{
    camera_width = 0;
    camera_height = 0;
    camera_color = 0;
    m_fd = -1;
    _using_inf = false;

    for (int i = 0; i < CAP_BUF_NUM; i++)
    {
        dma_buf[i] = nullptr;
    }
}

Camera::~Camera()
{
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
    int32_t n = 0;

    printf("Camera width = %d\n", camera_width);
    printf("Camera height = %d\n", camera_height);
    printf("Camera channel = %d\n", camera_color);

    ret = open_camera_device();
    if (0 != ret)
    {
        printf("failed to open_camera_device\n");
        return ret;
    }

    ret = init_camera_fmt();
    if (0 != ret)
    {
        printf("failed to init_camera_fmt\n");
        return ret;
    }

    ret = init_buffer();
    if (0 != ret)
    {
        printf("failed to init_buffer\n");
        return ret;
    }

    for (n = 0; n < CAP_BUF_NUM; n++)
    {
        dma_buf[n] = (camera_dma_buffer*)malloc(sizeof(camera_dma_buffer));
        if (nullptr == dma_buf[n])
        {
            fprintf(stderr, "[ERROR] Failed to allocate camera_dma_buffer struct\n");
            return -1;
        }

        const uint32_t yuyv_buf_size =
            static_cast<uint32_t>(camera_width) *
            static_cast<uint32_t>(camera_height) * 2U;

        ret = video_buffer_alloc_dmabuf(dma_buf[n], yuyv_buf_size);
        if (-1 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to Allocate DMA buffer for dma_buf[%d]\n", n);
            return ret;
        }

        memset(&buf_capture, 0, sizeof(buf_capture));
        buf_capture.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf_capture.memory = V4L2_MEMORY_DMABUF;
        buf_capture.index = n;
        buf_capture.m.fd = (unsigned long)dma_buf[n]->dbuf_fd;
        buf_capture.length = dma_buf[n]->size;

        ret = xioctl(m_fd, VIDIOC_QBUF, &buf_capture);
        if (-1 == ret)
        {
            fprintf(stderr, "[ERROR] VIDIOC_QBUF failed at start_camera buffer[%d]: errno=%d (%s)\n", n, errno, strerror(errno));
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
        if (nullptr != dma_buf[i])
        {
            video_buffer_free_dmabuf(dma_buf[i]);
            free(dma_buf[i]);
            dma_buf[i] = nullptr;
        }
    }

    if (0 <= m_fd)
    {
        close(m_fd);
        m_fd = -1;
    }
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
int8_t Camera::xioctl(int fd, int32_t request, void* arg)
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
        fprintf(stderr, "[ERROR] VIDIOC_STREAMON failed: errno=%d (%s)\n", errno, strerror(errno));
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
        fprintf(stderr, "[ERROR] capture_qbuf VIDIOC_QBUF failed: errno=%d (%s)\n", errno, strerror(errno));
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
        fprintf(stderr, "[ERROR] VIDIOC_DQBUF failed: errno=%d (%s)\n", errno, strerror(errno));
        return 0;
    }

    ret = video_buffer_flush_dmabuf(dma_buf[buf_capture.index]->idx, dma_buf[buf_capture.index]->size);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] mmngr_flush failed: ret=%d\n", ret);
        return 0;
    }

    return dma_buf[buf_capture.index]->phy_addr;
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
    buf.memory = V4L2_MEMORY_DMABUF;

    ret = xioctl(m_fd, VIDIOC_STREAMOFF, &buf.type);
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] VIDIOC_STREAMOFF failed: errno=%d (%s)\n", errno, strerror(errno));
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
            close(m_fd);
            m_fd = -1;
            continue;
        }

        /* Search USB camera */
        ret = strcmp((const char*)fmt.driver, "uvcvideo");
        if (0 == ret)
        {
            printf("[INFO] USB Camera: %s\n", dev_name);
            break;
        }
        close(m_fd);
        m_fd = -1;
    }

    if (i >= 15)
    {
        fprintf(stderr, "[ERROR] No USB camera device found\n");
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

    printf("[INFO] Requesting camera format: %dx%d YUYV\n", camera_width, camera_height);

    ret = xioctl(m_fd, VIDIOC_S_FMT, &fmt);
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] VIDIOC_S_FMT Failed: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    printf("[INFO] Camera format set: %dx%d fourcc=0x%08x\n",
           fmt.fmt.pix.width,
           fmt.fmt.pix.height,
           fmt.fmt.pix.pixelformat);

    struct v4l2_streamparm setfps;
    memset(&setfps, 0, sizeof(setfps));
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.numerator = 1;
    setfps.parm.capture.timeperframe.denominator = 30;
    if (ioctl(m_fd, VIDIOC_S_PARM, &setfps) < 0)
    {
        perror("VIDIOC_S_PARM");
    }
    else
    {
        printf("[INFO] Camera FPS request set to %d/%d\n",
               setfps.parm.capture.timeperframe.denominator,
               setfps.parm.capture.timeperframe.numerator);
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
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = CAP_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_DMABUF;

    printf("[INFO] Requesting %d DMABUF buffers\n", CAP_BUF_NUM);

    ret = xioctl(m_fd, VIDIOC_REQBUFS, &req);
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] VIDIOC_REQBUFS Failed: errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    printf("[INFO] VIDIOC_REQBUFS accepted count=%d\n", req.count);

    for (int i = 0; i < CAP_BUF_NUM; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;

        ret = xioctl(m_fd, VIDIOC_QUERYBUF, &buf);
        if (-1 == ret)
        {
            fprintf(stderr, "[ERROR] VIDIOC_QUERYBUF Failed at index %d: errno=%d (%s)\n", i, errno, strerror(errno));
            return -1;
        }
    }

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

    ret = fwrite((uint8_t*)dma_buf[buf_capture.index]->mem, sizeof(uint8_t), dma_buf[buf_capture.index]->size, fp);
    if (!ret)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/**
 * @brief video_buffer_alloc_dmabuf
 * @details Allocate a DMA buffer for the camera
 * @param buffer pointer to the camera_dma_buffer struct
 * @param buf_size requested buffer size
 * @return int8_t 0 if succeeded
 *                 not 0 otherwise
 */
int8_t Camera::video_buffer_alloc_dmabuf(struct camera_dma_buffer* buffer, int buf_size)
{
    MMNGR_ID id;
    /* The physical-address argument type of mmngr_alloc_in_user_ext() differs
     * between environments. Select the type the local mmngr header expects:
     *   - RZ/V2M (dunfell)  : unsigned long  (64-bit physical address)
     *   - RZ/V2L (scarthgap): unsigned int   (32-bit physical address)
     * MMNGR_PHYS_ADDR_64BIT is defined by CMake when PRODUCT=V2M. */
#ifdef MMNGR_PHYS_ADDR_64BIT
    unsigned long phard_addr = 0;
#else
    unsigned int  phard_addr = 0;
#endif
    void* puser_virt_addr;
    int m_dma_fd;
    int mm_ret = 0;

    buffer->size = buf_size;
    mm_ret = mmngr_alloc_in_user_ext(&id, buffer->size, &phard_addr, &puser_virt_addr, MMNGR_VA_SUPPORT_CACHED, NULL);
    if (0 != mm_ret)
    {
        fprintf(stderr, "[ERROR] mmngr_alloc_in_user_ext failed: ret=%d\n", mm_ret);
        return -1;
    }

    memset((void*)puser_virt_addr, 0, buffer->size);
    buffer->idx = id;
    buffer->mem = (void*)puser_virt_addr;
    /* On RZ/V2M (dunfell) the mmngr reserved region is placed above 4GB, so
     * mmngr_alloc_in_user_ext() may return a 33-bit physical address. DRP-AI
     * accesses the buffer using the lower 32 bits of the physical address, so
     * mask off the upper bits here. On RZ/V2L (scarthgap) the address is already
     * within the 32-bit range and this mask is a no-op. */
    buffer->phy_addr = static_cast<uint32_t>(phard_addr & 0xFFFFFFFFUL);
    if (!buffer->mem)
    {
        fprintf(stderr, "[ERROR] mmngr_alloc_in_user_ext returned null virtual address\n");
        return -1;
    }

    /* Pass the full (unmasked) physical address to the DMABUF export; only the
     * value handed to DRP-AI (buffer->phy_addr) is masked to the lower 32 bits. */
    mm_ret = mmngr_export_start_in_user_ext(&id, buffer->size, phard_addr, &m_dma_fd, NULL);
    if (0 != mm_ret)
    {
        fprintf(stderr, "[ERROR] mmngr_export_start_in_user_ext failed: ret=%d\n", mm_ret);
        mmngr_free_in_user_ext(buffer->idx);
        return -1;
    }

    buffer->dbuf_fd = m_dma_fd;
    return 0;
}

/**
 * @brief video_buffer_free_dmabuf
 * @details free a DMA buffer for the camera
 * @param buffer pointer to the camera_dma_buffer struct
 */
void Camera::video_buffer_free_dmabuf(struct camera_dma_buffer* buffer)
{
    if (nullptr == buffer)
    {
        return;
    }
    mmngr_free_in_user_ext(buffer->idx);
}

/**
 * @brief video_buffer_flush_dmabuf
 * @details flush a DMA buffer for the camera
 * @param idx mmngr buffer index
 * @param size buffer size
 * @return int 0 if succeeded
 *             not 0 otherwise
 */
int Camera::video_buffer_flush_dmabuf(uint32_t idx, uint32_t size)
{
    return mmngr_flush(idx, 0, size);
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
        fprintf(stderr, "[ERROR] inference_capture_qbuf VIDIOC_QBUF failed: errno=%d (%s)\n", errno, strerror(errno));
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
    return (uint8_t*)dma_buf[buf_capture.index]->mem;
}

/**
 * @brief get_inference_img
 * @details Function to return the inference camera buffer
 * @return uint8_t* inference camera buffer
 */
uint8_t* Camera::get_inference_img()
{
    return (uint8_t*)dma_buf[inference_buf_capture.index]->mem;
}

/**
 * @brief get_size
 * @details Function to return the camera buffer size (W x H x C)
 * @return int32_t camera buffer size (W x H x C )
 */
int32_t Camera::get_size()
{
    return dma_buf[buf_capture.index]->size;
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
