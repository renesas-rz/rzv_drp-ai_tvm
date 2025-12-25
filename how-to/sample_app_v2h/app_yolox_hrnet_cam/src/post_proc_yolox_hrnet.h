/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : post_proc.h
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for Megvii-Base Detection YOLOX and MMPose HRNet with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef POST_PROC_H
#define POST_PROC_H

#include "box.h"
#include "define.h"
#include "define_color_yolox_hrnet.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace std;

class PostProc
{
    public:
        PostProc();
        ~PostProc();
        void R_Post_Proc(float* floatarr, vector<detection>& det, uint32_t* ppl_count);
        void R_Post_Proc_HRNet(float* floatarr, int16_t crop_w, int16_t crop_h, float* lowest_kpt_score, float* preds);
        void R_HRNet_Coord_Convert(int16_t crop_x, int16_t crop_y, uint8_t n_pers, uint16_t* id_x, uint16_t* id_y, float* preds);

    private:
        double sigmoid(double x);
        void softmax(float val[NUM_CLASS]);
        int32_t index(uint8_t n, int32_t offs, int32_t channel);
        int32_t offset(uint8_t n, int32_t b, int32_t y, int32_t x);
        static bool sort_detection_by_prob(const detection& a, const detection& b);
        static bool remove_prob_0(const detection& a);
        int32_t offset_hrnet(int32_t b, int32_t y, int32_t x);
        int8_t sign(int32_t x);
};

#endif