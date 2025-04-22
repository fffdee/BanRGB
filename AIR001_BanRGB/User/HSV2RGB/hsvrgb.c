#include "hsvrgb.h"
#include <stdio.h>
#include <math.h>


/**
 * @brief  将RGB数据转存为HSV数据，从flash中读出的是RGB数据，想要启用HSV按键，需要先转换为HSV数据
 * @param  RGB[WS2812_NUMS][3]  传入数组指针就行 RGB_SHOW_NOW 或者 RGB_SHOW_TEMP 
 * @param  lampIndex    0-WS2812_NUMS ，将第lampIndex个灯的RGB转为HSV
 * @retval none
*/
void RGB2HSV(unsigned char RGB[WS2812_NUMS][3], int lampIndex)
{
    unsigned char R = RGB[lampIndex][0];
    unsigned char G = RGB[lampIndex][1];
    unsigned char B = RGB[lampIndex][2];

    double r = R / 255.0;
    double g = G / 255.0;
    double b = B / 255.0;

    double max = fmax(r, fmax(g, b));
    double min = fmin(r, fmin(g, b));
    double delta = max - min;

    // 计算 HSV_H 值
    if (delta == 0)
    {
        HSV_H = 0; // 无色
    }
    else if (max == r)
    {
        HSV_H = 60 * ((g - b) / delta);
    }
    else if (max == g)
    {
        HSV_H = 60 * (((b - r) / delta) + 2);
    }
    else
    {
        HSV_H = 60 * (((r - g) / delta) + 4);
    }

    if (HSV_H < 0)
    {
        HSV_H += 360;
    } // 补充了max = r ,g<b

    // 计算 HSV_S 值
    if (max == 0)
    {
        HSV_S = 0;
    }
    else
    {
        HSV_S = delta / max;
    }

    // 计算 HSV_V 值
    HSV_V = max;
}


/**
 * @brief  将HSV数据转存为RGB数据，将HSV按键修改后的值转为RGB值，并保存在RGB[WS2812_NUMS][3]数组的RGB[lampIndex][0] [1] [2]位置
 * @param  RGB[WS2812_NUMS][3]  传入数组指针就行 RGB_SHOW_NOW 或者 RGB_SHOW_TEMP 
 * @param  lampIndex    0-WS2812_NUMS ，将HSV数据转为第lampIndex个灯的RGB数据
 * @retval none
*/
void HSV2RGB(unsigned char RGB[WS2812_NUMS][3], int lampIndex)
{

    double C = HSV_V * HSV_S;
    double X = C * (1 - fabs(fmod(HSV_H / 60, 2) - 1));
    double m = HSV_V - C;

    double r, g, b;

    if (HSV_H >= 0 && HSV_H < 60)
    {
        r = C;
        g = X;
        b = 0;
    }
    else if (HSV_H >= 60 && HSV_H < 120)
    {
        r = X;
        g = C;
        b = 0;
    }
    else if (HSV_H >= 120 && HSV_H < 180)
    {
        r = 0;
        g = C;
        b = X;
    }
    else if (HSV_H >= 180 && HSV_H < 240)
    {
        r = 0;
        g = X;
        b = C;
    }
    else if (HSV_H >= 240 && HSV_H < 300)
    {
        r = X;
        g = 0;
        b = C;
    }
    else
    {
        r = C;
        g = 0;
        b = X;
    }

    RGB[lampIndex][0] = (r + m) * 255;
    RGB[lampIndex][1] = (g + m) * 255;
    RGB[lampIndex][2] = (b + m) * 255;
}

/**
 * @brief  将当前HSV值转到RGB_SHOW_NOW数组的第一行，将RGB_SHOW_NOW[][]的所有数据都替换为RGB_SHOW_NOW第一行的元素
 * @param  none
 * @retval 无返回值
*/
void HSV_Update_NOW(void)
{
    int led_bit_index;
    HSV2RGB(RGB_SHOW_NOW, 0);
    for (led_bit_index = 1; led_bit_index < WS2812_NUMS; led_bit_index++)
    {
        RGB_SHOW_NOW[led_bit_index][0] = RGB_SHOW_NOW[0][0];
        RGB_SHOW_NOW[led_bit_index][1] = RGB_SHOW_NOW[0][1];
        RGB_SHOW_NOW[led_bit_index][2] = RGB_SHOW_NOW[0][2];
    }
}

/**
 * @brief  将当前HSV值转到RGB_SHOW_TEMP数组的第一行，将RGB_SHOW_TEMP[][]的所有数据都替换为RGB_SHOW_TEMP第一行的元素
 * @param  none
 * @retval 无返回值
*/
void HSV_Update_TEMP(void)
{
    int led_bit_index;
    HSV2RGB(RGB_SHOW_TEMP, 0);
    for (led_bit_index = 1; led_bit_index < WS2812_NUMS; led_bit_index++)
    {
        RGB_SHOW_TEMP[led_bit_index][0] = RGB_SHOW_TEMP[0][0];
        RGB_SHOW_TEMP[led_bit_index][1] = RGB_SHOW_TEMP[0][1];
        RGB_SHOW_TEMP[led_bit_index][2] = RGB_SHOW_TEMP[0][2];
    }
}
