/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : concat.h
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for mmpose Detection YOLOX Pose with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef POST_PROC_H
#define POST_PROC_H

#include "define.h"

class PostProc
{
    public:
        PostProc();
        ~PostProc();
        float Mul_8400[num_grid_points];
        float Add_8400x2[num_grid_points * 2];
        void init_param();
        void PrePost_Proc(float* out_key34_80, float* out_key34_40, float* out_key34_20, float* out_box_80, float* out_box_40, float* out_box_20, 
            float* out_key17_80, float* out_key17_40, float* out_key17_20, float* out_conf, float* output_buf);

    private:
        double sigmoid(double x);
        float* get_add_param(uint32_t size);
        float* Mul_Add_part(float* out_key1, float* out_key2);
        float* Post_box(float* out_box);
        void concat_transpose(float* out_80x80, float* out_40x40, float* out_20x20, uint32_t c, float* final_out);
        void mul_add_process(float* key1, float* key2, float* out);
        void post_box_process(float* box, float* out);
};

#endif
