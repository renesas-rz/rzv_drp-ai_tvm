/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : concat_proc.cpp
* Version      : 2.6.0
* Description  : RZ/V2H DRP-AI Sample Application for mmpose Detection YOLOX Pose with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "post_proc_yolox_pose.h"
#include <opencv2/opencv.hpp>
#include <thread>

using namespace std;

PostProc::PostProc()
{

}

PostProc::~PostProc()
{

}

/*****************************************
* Function Name : sigmoid
* Description   : Helper function for YOLO Post Processing
* Arguments     : x = input argument for the calculation
* Return value  : sigmoid result of input x
******************************************/
double PostProc::sigmoid(double x)
{
    return 1.0/(1.0 + exp(-x));
}

/*****************************************
* Function Name : get_add_param
* Description   : Helper function for YOLO Post Processing
* Arguments     : size = 80 / 40 / 20
* Return value  : size x size x 2 array
******************************************/
float* PostProc::get_add_param(uint32_t size)
{
    uint32_t coeff = 8 * (80 / size);
    float* out = new float[size * size * 2];
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int idx = j * size + i;
            out[idx * 2 + 0] = i * coeff;
            out[idx * 2 + 1] = j * coeff;
        }
    }
    return out;
}

/*****************************************
* Function Name : init_param
* Description   : init Mul_8400 & Add_8400x2
* Arguments     : -
* Return value  : -
******************************************/
void PostProc::init_param()
{
    /* Mul_8400 */
    for (int i = 0; i < num_grid_0; i++)
    {
        Mul_8400[i] = 8;
    }
    for (int i = 0; i < num_grid_1; i++)
    {
        Mul_8400[num_grid_0 + i] = 16;
    }
    for (int i = 0; i < num_grid_2; i++)
    {
        Mul_8400[num_grid_0 + num_grid_1 + i] = 32;
    }

    /* Add_8400x2 */
    float* Add_80x80 = get_add_param(80);
    float* Add_40x40 = get_add_param(40);
    float* Add_20x20 = get_add_param(20);

    int add_idx = 0;
    for (int i = 0; i < num_grid_0 * 2; i++)
    {
        Add_8400x2[add_idx + i] = Add_80x80[i];
    }
    add_idx += num_grid_0 * 2;
    for ( int i = 0; i < num_grid_1 * 2; i++)
    {
        Add_8400x2[add_idx + i] = Add_40x40[i];
    }
    add_idx += num_grid_1 * 2;
    for (int i = 0; i < num_grid_2 * 2; i++)
    {
        Add_8400x2[add_idx + i] = Add_20x20[i];
    }
    delete[] Add_80x80;
    delete[] Add_40x40;
    delete[] Add_20x20;
}

/*****************************************
* Function Name : Mul_Add_part
* Description   : Helper function for YOLO Post Processing
* Arguments     : out_key1 = key34 array (8400, 34)
                  out_key2 = key17 array (8400, 17)
* Return value  : result of input array (8400, 51)
******************************************/
float* PostProc::Mul_Add_part(float* out_key1, float* out_key2)
{
    float* out = new float[num_grid_points * (NUM_KPTS * 2 + NUM_KPTS)];
    for (int i = 0; i < num_grid_points; i++)
    {
        float mul = Mul_8400[i];
        float add1 = Add_8400x2[i * 2 + 0];
        float add2 = Add_8400x2[i * 2 + 1];
        
        for (int j = 0; j < NUM_KPTS; j++)
        {
            int idx = i * (NUM_KPTS * 2 + NUM_KPTS) + j * 3;
            /* Mul & Add */
            out[idx + 0] = out_key1[i * NUM_KPTS * 2 + j * 2 + 0] * mul + add1;
            out[idx + 1] = out_key1[i * NUM_KPTS * 2 + j * 2 + 1] * mul + add2;
            /* Sigmoid */
            out[idx + 2] = sigmoid(out_key2[i * NUM_KPTS + j]);
        }
    }
    return out;
}

/*****************************************
* Function Name : Post_box
* Description   : Helper function for YOLO Post Processing
* Arguments     : out_box = box array (8400, 4)
* Return value  : result of input array (8400, 4)
******************************************/
float* PostProc::Post_box(float* out_box)
{
    float* out = new float[num_grid_points * BOX_SIZE];
    for (int i = 0; i < num_grid_points; i++)
    {
        float out_box1 = out_box[i * BOX_SIZE + 0];
        float out_box2 = out_box[i * BOX_SIZE + 1];
        float out_box3 = out_box[i * BOX_SIZE + 2];
        float out_box4 = out_box[i * BOX_SIZE + 3];
        float mul = Mul_8400[i];
        float add1 = Add_8400x2[i * 2 + 0];
        float add2 = Add_8400x2[i * 2 + 1];

        /* Mul & Add */
        float n1_n1_gather = out_box1 * mul + add1;
        float n1_n2_gather = out_box2 * mul + add2;

        /* Exp & Mul & Div */
        float v0 = min(max(out_box3, 0.0f), 6.0f);
        float v1 = min(max(out_box4, 0.0f), 6.0f);
        float n2_n1_g_div = exp(v0) * mul / 2.0f;
        float n2_n2_g_div = exp(v1) * mul / 2.0f;

        /* Sub & Add */
        out[i * BOX_SIZE + 0] = n1_n1_gather - n2_n1_g_div;
        out[i * BOX_SIZE + 1] = n1_n2_gather - n2_n2_g_div;
        out[i * BOX_SIZE + 2] = n1_n1_gather + n2_n1_g_div;
        out[i * BOX_SIZE + 3] = n1_n2_gather + n2_n2_g_div;
    }
    return out;
}

/*****************************************
* Function Name : concatenate
* Description   : Concatenate array
* Arguments     : out_80x80, out_40x40, out_20x20 = input array
                  c = channel of output array
                  final_out = output array
* Return value  : -
******************************************/
void PostProc::concat_transpose(float* out_80x80, float* out_40x40, float* out_20x20, uint32_t c, float* final_out)
{
    int out_row = 0;

    /* 80x80 */
    for (int h = 0; h < num_grids[0]; h++)
    {
        for (int w = 0; w < num_grids[0]; w++)
        {
            for (int c_idx = 0; c_idx < c; c_idx++)
            {
                final_out[out_row * c + c_idx] = out_80x80[c_idx * num_grids[0] * num_grids[0] + h * num_grids[0] + w];
            }
            out_row++;
        }
    }

    /* 40x40 */
    for (int h = 0; h < num_grids[1]; h++)
    {
        for (int w = 0; w < num_grids[1]; w++)
        {
            for (int c_idx = 0; c_idx < c; c_idx++)
            {
                final_out[out_row * c + c_idx] = out_40x40[c_idx * num_grids[1] * num_grids[1] + h * num_grids[1] + w];
            }
            out_row++;
        }
    }

    /* 20x20 */
    for (int h = 0; h < num_grids[2]; h++)
    {
        for (int w = 0; w < num_grids[2]; w++)
        {
            for (int c_idx = 0; c_idx < c; c_idx++)
            {
                final_out[out_row * c + c_idx] = out_20x20[c_idx * num_grids[2] * num_grids[2] + h * num_grids[2] + w];
            }
            out_row++;
        }
    }
}

/*****************************************
* Function Name : mul_add_process
* Description   : process for thread
* Arguments     : -
* Return value  : -
******************************************/
void PostProc::mul_add_process(float* key1, float* key2, float* out)
{
    float* tmp = Mul_Add_part(key1, key2);
    copy(tmp, tmp + (num_grid_points * (NUM_KPTS * 2 + NUM_KPTS)), out);
    delete[] tmp;
}

/*****************************************
* Function Name : post_box_process
* Description   : process for thread
* Arguments     : -
* Return value  : -
******************************************/
void PostProc::post_box_process(float* box, float* out)
{
    float* tmp = Post_box(box);
    copy(tmp, tmp + (num_grid_points * BOX_SIZE), out);
    delete[] tmp;
}

/*****************************************
* Function Name : PrePost_Proc
* Description   : prepost process for YOLOX Pose
* Arguments     : out_key34 = keypoints(34) array
                  out_box = box array
                  out_key17 = keypoints(17) array
                  out_conf = confidence array
                  output_buf = output array
* Return value  : -
******************************************/
void PostProc::PrePost_Proc(float* out_key34_80, float* out_key34_40, float* out_key34_20, float* out_box_80, float* out_box_40, float* out_box_20, 
    float* out_key17_80, float* out_key17_40, float* out_key17_20, float* out_conf, float* output_buf)
{
    /* Transpose */
    float* out_key34_t = new float[num_grid_points * NUM_KPTS * 2];
    float* out_box_t = new float[num_grid_points * BOX_SIZE];
    float* out_key17_t = new float[num_grid_points * NUM_KPTS];

#if (1) == CPU_CONCAT_MULTI_THREAD
    thread thread_t_key34(&PostProc::concat_transpose, this, out_key34_80, out_key34_40, out_key34_20, NUM_KPTS * 2, out_key34_t);
    thread thread_t_box(&PostProc::concat_transpose, this, out_box_80, out_box_40, out_box_20, BOX_SIZE, out_box_t);
    thread thread_t_key17(&PostProc::concat_transpose, this, out_key17_80, out_key17_40, out_key17_20, NUM_KPTS, out_key17_t);
    thread_t_key34.join();
    thread_t_box.join();
    thread_t_key17.join();
#else
    concat_transpose(out_key34_80, out_key34_40, out_key34_20, NUM_KPTS * 2, out_key34_t);
    concat_transpose(out_box_80, out_box_40, out_box_20, BOX_SIZE, out_box_t);
    concat_transpose(out_key17_80, out_key17_40, out_key17_20, NUM_KPTS, out_key17_t);
#endif

    /* Mul_Add_part & Post_box */
    // init_param();
    float* post_out1 = new float[num_grid_points * (NUM_KPTS * 2 + NUM_KPTS)];
    float* post_out2 = new float[num_grid_points * BOX_SIZE];

#if (1) == CPU_CONCAT_MULTI_THREAD
    thread thread_mul_add(&PostProc::mul_add_process, this, out_key34_t, out_key17_t, post_out1);
    thread thread_post_box(&PostProc::post_box_process, this, out_box_t, post_out2);
    thread_mul_add.join();
    thread_post_box.join();
#else
    post_out1 = Mul_Add_part(out_key34_t, out_key17_t);
    post_out2 = Post_box(out_box_t);
#endif
    delete[] out_key34_t;
    delete[] out_key17_t;
    delete[] out_box_t;

    /* Concat */
    for (int i = 0; i < num_grid_points; i++)
    {
        for (int j = 0; j < BOX_SIZE; j++)
        {
            output_buf[i * num_channels + j] = post_out2[i * BOX_SIZE + j];
        }
        for (int j = 0; j < 1; j++)
        {
            output_buf[i * num_channels + BOX_SIZE + j] = out_conf[i * 1 + j];
        }
        for (int j = 0; j < NUM_KPTS * 2 + NUM_KPTS; j++)
        {
            output_buf[i * num_channels + BOX_SIZE + 1 + j] = post_out1[i * (NUM_KPTS * 2 + NUM_KPTS) + j];
        }
    }
    delete[] post_out1;
    delete[] post_out2;
}