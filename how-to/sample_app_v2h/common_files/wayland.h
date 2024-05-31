/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : wayland.h
* Version      : 1.00
* Description  : for RZ/V2H DRP-AI Sample Application with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef WAYLAND_H
#define WAYLAND_H


#include "define.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <wayland-client.h>
#include <wayland-egl.h>

class Wayland
{
    /* structure of Shader settings */
    typedef struct _SShader {
        GLuint unProgram;
        GLint nAttrPos;
        GLint nAttrColor;
    } SShader;

    public:
        Wayland();
        ~Wayland();

        uint8_t init(uint32_t idx, uint32_t w, uint32_t h, uint32_t c, bool overlay = false);
        uint8_t exit();
        uint8_t commit(uint8_t* cam_buffer, uint8_t* ol_buffer);

        struct wl_compositor *compositor = NULL;
        struct wl_shm *shm = NULL;
        struct wl_shell *shell = NULL;

    private:
        uint32_t img_h;
        uint32_t img_w;
        uint32_t img_c;
        bool     img_overlay;

        struct wl_display *display = NULL;
        struct wl_surface *surface;
        struct wl_shell_surface *shell_surface;
        struct wl_registry *registry = NULL;
        EGLDisplay eglDisplay;
        EGLSurface eglSurface;
        SShader sShader;
        GLuint textures[2];

        GLuint LoadShader(GLenum type, const char* shaderSrc);
        GLuint initProgramObject(SShader* pShader);
        uint8_t render(SShader* pShader, GLuint texID);
        uint8_t setupTexture(GLuint texID, uint8_t* src_pixels);
};

#endif
