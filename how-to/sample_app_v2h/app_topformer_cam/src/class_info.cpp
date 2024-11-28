/***********************************************************************************************************************
* Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : class_info.cpp
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for TopFormer with MIPI/USB Camera
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "class_info.h"

/*******************************************************************************
* Static functions
*******************************************************************************/
/* Include the class name and color palette from mmsegmentation/class_names.cpp */
#include "mmsegmentation/class_names.cpp"

/*******************************************************************************
* The following functions are defined to change colors of the `mmsegmentation/class_names.cpp`.
* It is hard to see the default palette color when these colors superposition over the camera image 
* because the palette is defined so that it can describe the typical color of class object.
* Therefore, the following functions raise the visibility of segmentation color by controlling the color palette.
*******************************************************************************/
/*****************************************
* Function Name     : color_rgb_to_hsv
* Description       : Convert the RGB color to HSV (Hue, Saturation, Value) 
* Arguments         : r, g, b is input RGB color
*                     h, s, v is output HSV color
* Return Value      : -
******************************************/
static void color_rgb_to_hsv( float_t r, float_t g, float_t b, float_t* h, float_t* s , float_t* v )
{
    float_t max, min;
    float_t x, y, bias;

    if ( r == g && g == b )
    {
        *h = 0;
        *s = 0;
        *v = r; // r = g = b
    }
    else
    {
        if ( r >= g && r >= b )
        { 
            max = r;
            min = g <= b ? g : b;
            x = g;
            y = b;
            bias = 0;
        }
        else if ( g >= r && g >= b )
        {
            max = g;
            min = r <= b ? r : b;
            x = b;
            y = r;
            bias = 120;
        }
        else // if ( b >= r && b >= g )
        {
            max = b;
            min = r <= g ? r : g;
            x = r;
            y = g;
            bias = 240;
        }

        *h = 60 * ( (x-y) / (max - min) ) + bias;
        *h = fmodf32(*h, 360.0);
        if ( *h < 0 ) *h = *h + 360;
        *s = (max-min) / max * 255.0;
        *v = max;
    }
}

/*****************************************
* Function Name     : color_hsv_to_rgb
* Description       : Convert the HSV color to RGB
* Arguments         : h, s, v is input HSV color
*                     r, g, b is output RGB color
* Return Value      : -
******************************************/
static void color_hsv_to_rgb( float_t* r, float_t* g, float_t* b, float_t h, float_t s , float_t v )
{
    float_t max, min;
    max = v;
    min = max - ( s/255.0 * max );
    h = fmodf32(h, 360.0);

    if ( h >= 0 && h < 60 )
    {
        *r = max;
        *g = (h / 60.0) * (max-min) + min ;
        *b = min;
    }
    else if ( h >= 60 && h < 120 )
    {
        *r = ( (120.0-h) / 60.0 ) * (max-min) + min;
        *g = max;
        *b = min;
    }
    else if ( h >= 120 && h < 180 )
    {
        *r = min;
        *g = max;
        *b = ( (h-120.0) / 60.0 ) * (max-min) + min;
    }
    else if ( h >= 180 && h < 240 )
    {
        *r = min;
        *g = ( (240.0-h) / 60.0 ) * (max-min) + min;
        *b = max;
    }
    else if ( h >= 240 && h < 300 )
    {
        *r = ( (h-240.0) / 60.0 ) * (max-min) + min;
        *g = min;
        *b = max;
    }
    else // ( h >= 300 && h < 360 )
    {
        *r = max;
        *g = min;
        *b = ( (360.0-h) / 60.0 ) * (max-min) + min;
    }
}


/*****************************************
* Function Name     : class_ade_color_configuration
* Description       : Configure the color palette in HSV space.
* Arguments         : -
* Return Value      : -
******************************************/
static void class_ade_color_configuration()
{
    for(int i = 0; i < NUM_CLASS; i++ )
    {
        float_t r = g_ade_palette[i][0];
        float_t g = g_ade_palette[i][1];
        float_t b = g_ade_palette[i][2];
        float_t h, s, v;
        color_rgb_to_hsv(r, g, b, &h, &s, &v);

        // Gray scale colors
        if ( s == 0 )   
        {
            // Here, raise saturation as a red color
            s = 128;
        }
        // Other colors
        else            
        {
            // Here, rotate Hue 60 degrees.
            h = h - 60;
            h = fmodf32(h, 360.0);
            if ( h < 0 ) h = h + 360;
        }        

        color_hsv_to_rgb(&r, &g, &b, h, s, v);
        g_ade_palette[i][0] = (uint8_t) r;
        g_ade_palette[i][1] = (uint8_t) g;
        g_ade_palette[i][2] = (uint8_t) b;
    }    
}

/*****************************************
* Function Name     : class_ade_color_set_wall_color
* Description       : Set the wall color
* Arguments         : r, g, b is input RGB color to be set to "wall" class color
* Return Value      : -
******************************************/
static void class_ade_color_set_wall_color(uint8_t r, uint8_t g, uint8_t b)
{
    g_ade_palette[0][0] = r;
    g_ade_palette[0][1] = g;
    g_ade_palette[0][2] = b;
}

/*****************************************
* Function Name     : class_ade_color_reset_wall_color
* Description       : Reset the wall color to the default color.
* Arguments         : -
* Return Value      : -
******************************************/
static void class_ade_color_reset_wall_color()
{
    class_ade_color_set_wall_color(120, 120, 120);
}

/*******************************************************************************
* Public Functions
*******************************************************************************/
/*****************************************
* Function Name     : class_ade_initialize
* Description       : Initialize the color palette by the above functions from the default color palette.
* Arguments         : -
* Return Value      : -
******************************************/
void class_ade_initialize()
{
    /* Changing color color palette in HSV space. In detail of the change, please refer to the function. */
    class_ade_color_configuration();
    
    /* Reset the only wall color to the default gray scale color which is low light as segmentation
       because the "wall" often occupy the overall image and dyes the screen the same color. */
    class_ade_color_reset_wall_color();
}
