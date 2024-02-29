/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : camera.cpp
* Version      : 0.90
* Description  : RZ/V2H DRP-AI Sample Application for Megvii-Base Detection YOLOX with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "camera.h"
#include <errno.h>

Camera::Camera()
{
    camera_width    = CAM_IMAGE_WIDTH;
    camera_height   = CAM_IMAGE_HEIGHT;
    camera_color    = CAM_IMAGE_CHANNEL_YUY2;
}

Camera::~Camera()
{
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
* Function Name : start_camera
* Description   : Function to initialize USB camera capture
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/

int8_t Camera::start_camera()
{
    int8_t ret = 0;
    int32_t i = 0;
    int32_t n = 0;
//    uint8_t* word_ptr;
    
#if INPUT_CAM_TYPE == 1
    std::string sw_cmd1 = format("media-ctl -d /dev/media0 -V \"\'rzg2l_csi2 16000400.csi20\':1 [fmt:UYVY8_2X8/%s field:none]\"", MIPI_CAM_RES);
    std::string sw_cmd2 = format("media-ctl -d /dev/media0 -V \"\'imx462 0-001f\':0 [fmt:UYVY8_2X8/%s field:none]\"", MIPI_CAM_RES);
    const char* commands[6] =
    {
        "v4l2-ctl -d 0 -c framerate=30",
        "v4l2-ctl -d 0 -c white_balance_auto_preset=0",
        "media-ctl -d /dev/media0 -r",
        "media-ctl -d /dev/media0 -l \"\'rzg2l_csi2 16000400.csi20\':1 -> \'CRU output\':0 [1]\"",
        &sw_cmd1[0],
        &sw_cmd2[0],
    };
    /* media-ctl command */
    for (i = 0; i < 6; i++)
    {
        printf("%s\n", commands[i]);
        ret = system(commands[i]);
        printf("system ret = %d\n", ret);
        if (ret < 0)
        {
            printf("%s: failed media-ctl commands. index = %d\n", __func__, i);
            return -1;
        }
    }

#endif  /* INPUT_CAM_TYPE */

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

    wayland_buf = (camera_dma_buffer*)malloc(sizeof(wayland_buf));
    ret = video_buffer_alloc_dmabuf(wayland_buf,WAYLANDBUF);
    
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] Failed to Allocate DMA buffer for the wayland_buf\n");
        return -1;
    }

    drpai_buf = (camera_dma_buffer*)malloc(sizeof(camera_dma_buffer));
    ret = video_buffer_alloc_dmabuf(drpai_buf,DRPAIBUF);

    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] Failed to Allocate DMA buffer for the drpai_buf\n");
        return -1;
    }
    
    for (n =0; n < CAP_BUF_NUM; n++)
    {
        dma_buf[n] = (camera_dma_buffer*)malloc(sizeof(camera_dma_buffer[n]));
        ret = video_buffer_alloc_dmabuf(dma_buf[n],CAPTUREBUF);
        if (-1 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to Allocate DMA buffer for the dma_buf\n");
            return ret;
        }
        memset(&buf_capture, 0, sizeof(buf_capture));
        buf_capture.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf_capture.memory = V4L2_MEMORY_DMABUF;
        buf_capture.index = n;
        buf_capture.m.fd = (unsigned long) dma_buf[n]->dbuf_fd;
        buf_capture.length = dma_buf[n]->size;
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


/*****************************************
* Function Name : close_capture
* Description   : Close camera and free buffer
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::close_camera()
{
    int8_t ret = 0;
    int32_t i = 0;

    ret = stop_capture();
    if (0 != ret) return ret;

    video_buffer_free_dmabuf(wayland_buf);
    free(wayland_buf);
    wayland_buf = NULL;

    video_buffer_free_dmabuf(drpai_buf);
    free(drpai_buf);
    drpai_buf = NULL;

    for (i = 0;i<CAP_BUF_NUM;i++)
    {
        video_buffer_free_dmabuf(dma_buf[i]);
        free(dma_buf[i]);
        dma_buf[i] = NULL;
    }

    close(m_fd);
    return 0;
}


/*****************************************
* Function Name : xioctl
* Description   : ioctl calling
* Arguments     : fd = V4L2 file descriptor
*                 request = V4L2 control ID defined in videodev2.h
*                 arg = set value
* Return value  : int = output parameter
******************************************/
int8_t Camera::xioctl(int8_t fd, int32_t request, void * arg)
{
    int8_t r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

/*****************************************
* Function Name : start_capture
* Description   : Set STREAMON
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
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

/*****************************************
* Function Name : capture_qbuf
* Description   : Function to enqueue the buffer.
*                 (Call this function after capture_image() to restart filling image data into buffer)
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
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


/*****************************************
* Function Name : capture_image
* Description   : Function to capture image and return the physical memory address where the captured image stored.
*                 Must call capture_qbuf after calling this function.
* Arguments     : -
* Return value  : the physical memory address where the captured image stored.
******************************************/
uint64_t Camera::capture_image()
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
            if (EINTR == errno) continue;
            return 0;
        }
        break;
    }

    /* Get buffer where camera stored data */
    ret = xioctl(m_fd, VIDIOC_DQBUF, &buf_capture);
    if (-1 == ret)
    {
        return 0;
    }

    ret = video_buffer_flush_dmabuf(dma_buf[buf_capture.index]->idx, dma_buf[buf_capture.index]->size);
    if (0 != ret)
    {
        return 0;
    }
    return  dma_buf[buf_capture.index]->phy_addr;
}

/*****************************************
* Function Name : stop_capture
* Description   : Set STREAMOFF
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::stop_capture()
{
    int8_t ret = 0;
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_DMABUF;

    ret = xioctl(m_fd, VIDIOC_STREAMOFF, &buf.type);
    if (-1 == ret)
    {
        return -1;
    }
    return 0;
}

/*****************************************
* Function Name : open_camera_device
* Description   : Function to open camera *called by start_camera
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::open_camera_device()
{
    char dev_name[4096] = {0};
    int32_t i = 0;
    int8_t ret = 0;
    struct v4l2_capability fmt;

    for (i = 0; i < 15; i++)
    {
        snprintf(dev_name, sizeof(dev_name), "/dev/video%d", i);
        m_fd = open(dev_name, O_RDWR);
        if (-1 == m_fd)
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

#if INPUT_CAM_TYPE == 1
        ret = strcmp((const char*)fmt.driver, "rzg2l_cru");
        if (0 == ret)
        {
            printf("[INFO] CSI2 Camera: %s\n", dev_name);
            break;
        }
#else /* INPUT_CAM_TYPE */
        /* Search USB camera */
        ret = strcmp((const char*)fmt.driver, "uvcvideo");
        if (0 == ret)
        {
            printf("[INFO] USB Camera: %s\n", dev_name);
            break;
        }
#endif /* INPUT_CAM_TYPE */
        close(m_fd);
    }

    if (15 <= i)
    {
        return -1;
    }
    return 0;
}

/*****************************************
* Function Name : init_camera_fmt
* Description   : Function to request format *called by start_camera
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::init_camera_fmt()
{
    int8_t ret = 0;
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
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

/*****************************************
* Function Name : init_buffer
* Description   : Initialize camera buffer *called by start_camera
* Arguments     : -
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::init_buffer()
{
    int8_t ret = 0;
    int32_t i = 0;
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = CAP_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_DMABUF;

    /*Request a buffer that will be kept in the device*/
    ret = xioctl(m_fd, VIDIOC_REQBUFS, &req);
    if (-1 == ret)
    {
        printf("[ERROR] VIDIOC_REQBUFS Failed: %d\n", ret);
        return -1;
    }

    struct v4l2_buffer buf;
    for (i =0; i < CAP_BUF_NUM; i++)
    {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;

        /* Extract buffer information */
        ret = xioctl(m_fd, VIDIOC_QUERYBUF, &buf);
        if (-1 == ret)
        {
            printf("[ERROR] VIDIOC_QUERYBUF Failed: %d\n", ret);
            return -1;
        }

    }

    return 0;
}


/*****************************************
* Function Name : save_bin
* Description   : Get the capture image from buffer and save it into binary file
* Arguments     : filename = binary file name to be saved
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::save_bin(std::string filename)
{
    int8_t ret = 0;
    FILE * fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        return -1;
    }

    /* Get data from buffer and write to binary file */
    ret = fwrite((uint8_t *)dma_buf[buf_capture.index]->mem, sizeof(uint8_t), dma_buf[buf_capture.index]->size, fp);

    if (!ret)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*****************************************
* Function Name : video_buffer_alloc_dmabuf
* Description   : Allocate a DMA buffer for the camera
* Arguments     : buffer = pointer to the camera_dma_buffer struct
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t Camera::video_buffer_alloc_dmabuf(struct camera_dma_buffer *buffer,int buf_size)
{
    MMNGR_ID id;
    uint32_t phard_addr;
    void *puser_virt_addr;
    int m_dma_fd;

    buffer->size = buf_size;
    mmngr_alloc_in_user_ext(&id, buffer->size, &phard_addr, &puser_virt_addr, MMNGR_VA_SUPPORT_CACHED, NULL);
    memset((void*)puser_virt_addr, 0, buffer->size);
    buffer->idx = id;
    buffer->mem = (void *)puser_virt_addr;
    buffer->phy_addr = phard_addr;
    if (!buffer->mem)
    {
        return -1;
    }
    mmngr_export_start_in_user_ext(&id, buffer->size, phard_addr, &m_dma_fd, NULL);
    buffer->dbuf_fd = m_dma_fd;
    return 0;
}

/*****************************************
* Function Name : video_buffer_free_dmabuf
* Description   : free a DMA buffer for the camera
* Arguments     : buffer = pointer to the camera_dma_buffer struct
* Return value  : -
******************************************/
void Camera::video_buffer_free_dmabuf(struct camera_dma_buffer *buffer)
{
    mmngr_free_in_user_ext(buffer->idx);
    return;
}


/*****************************************
* Function Name : video_buffer_flush_dmabuf
* Description   : flush a DMA buffer for the camera
* Arguments     : buffer = pointer to the camera_dma_buffer struct
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int Camera::video_buffer_flush_dmabuf(uint32_t idx, uint32_t size)
{
    int mm_ret = 0;
    
    /* Flush capture image area cache */
    mm_ret = mmngr_flush(idx, 0, size);
    
    return mm_ret;
}

/*****************************************
* Function Name : get_img
* Description   : Function to return the camera buffer
* Arguments     : -
* Return value  : camera buffer
******************************************/
uint8_t * Camera::get_img()
{
    return (uint8_t *)dma_buf[buf_capture.index]->mem;

}

/*****************************************
* Function Name : get_size
* Description   : Function to return the camera buffer size (W x H x C)
* Arguments     : -
* Return value  : camera buffer size (W x H x C )
******************************************/
int32_t Camera::get_size()
{
    return dma_buf[buf_capture.index]->size;
}

/*****************************************
* Function Name : get_w
* Description   : Get camera_width. This function is currently NOT USED.
* Arguments     : -
* Return value  : camera_width = width of camera capture image.
******************************************/
int32_t Camera::get_w()
{
    return camera_width;
}

/*****************************************
* Function Name : set_w
* Description   : Set camera_width. This function is currently NOT USED.
* Arguments     : w = new camera capture image width
* Return value  : -
******************************************/
void Camera::set_w(int32_t w)
{
    camera_width= w;
    return;
}

/*****************************************
* Function Name : get_h
* Description   : Get camera_height. This function is currently NOT USED.
* Arguments     : -
* Return value  : camera_height = height of camera capture image.
******************************************/
int32_t Camera::get_h()
{
    return camera_height;
}

/*****************************************
* Function Name : set_h
* Description   : Set camera_height. This function is currently NOT USED.
* Arguments     : w = new camera capture image height
* Return value  : -
******************************************/
void Camera::set_h(int32_t h)
{
    camera_height = h;
    return;
}

/*****************************************
* Function Name : get_c
* Description   : Get camera_color. This function is currently NOT USED.
* Arguments     : -
* Return value  : camera_color = color channel of camera capture image.
******************************************/
int32_t Camera::get_c()
{
    return camera_color;
}

/*****************************************
* Function Name : set_c
* Description   : Set camera_color. This function is currently NOT USED.
* Arguments     : c = new camera capture image color channel
* Return value  : -
******************************************/
void Camera::set_c(int32_t c)
{
    camera_color= c;
    return;
}
