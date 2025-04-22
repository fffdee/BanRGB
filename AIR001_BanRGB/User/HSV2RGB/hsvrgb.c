#include "hsvrgb.h"
#include <stdio.h>
#include <math.h>


/**
 * @brief  ��RGB����ת��ΪHSV���ݣ���flash�ж�������RGB���ݣ���Ҫ����HSV��������Ҫ��ת��ΪHSV����
 * @param  RGB[WS2812_NUMS][3]  ��������ָ����� RGB_SHOW_NOW ���� RGB_SHOW_TEMP 
 * @param  lampIndex    0-WS2812_NUMS ������lampIndex���Ƶ�RGBתΪHSV
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

    // ���� HSV_H ֵ
    if (delta == 0)
    {
        HSV_H = 0; // ��ɫ
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
    } // ������max = r ,g<b

    // ���� HSV_S ֵ
    if (max == 0)
    {
        HSV_S = 0;
    }
    else
    {
        HSV_S = delta / max;
    }

    // ���� HSV_V ֵ
    HSV_V = max;
}


/**
 * @brief  ��HSV����ת��ΪRGB���ݣ���HSV�����޸ĺ��ֵתΪRGBֵ����������RGB[WS2812_NUMS][3]�����RGB[lampIndex][0] [1] [2]λ��
 * @param  RGB[WS2812_NUMS][3]  ��������ָ����� RGB_SHOW_NOW ���� RGB_SHOW_TEMP 
 * @param  lampIndex    0-WS2812_NUMS ����HSV����תΪ��lampIndex���Ƶ�RGB����
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
 * @brief  ����ǰHSVֵת��RGB_SHOW_NOW����ĵ�һ�У���RGB_SHOW_NOW[][]���������ݶ��滻ΪRGB_SHOW_NOW��һ�е�Ԫ��
 * @param  none
 * @retval �޷���ֵ
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
 * @brief  ����ǰHSVֵת��RGB_SHOW_TEMP����ĵ�һ�У���RGB_SHOW_TEMP[][]���������ݶ��滻ΪRGB_SHOW_TEMP��һ�е�Ԫ��
 * @param  none
 * @retval �޷���ֵ
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
