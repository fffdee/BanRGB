#include "def_conf.h"

/*HSVRGB*/
float HSV_H = 180;
float HSV_S = 0.5;
float HSV_V = 0.5;

/*KEY*/
GPIO_PinState swh = 1; // 按键H状态
GPIO_PinState sws = 1; // 按键S状态
GPIO_PinState swv = 1; // 按键V状态
GPIO_PinState swm = 1; // 按键M状态
GPIO_PinState swr = 1; // 按键R状态

int swh_nums = 0;
int sws_nums = 0;
int swv_nums = 0;
int swm_nums = 1; // RGB mode的枚举是从1开始的
int swr_nums = 0;

/**Interrupt*/
int Reverse_sign = 1;    // R 键按下标志
int ModeChange_sign = 0; // M  键按下标志,用于屏蔽中断 0中断开启，1中断关闭，此处定义为1是为了第一次Update进行
int EXTI0_1_sign = 1;    // EXTI0 响应标志
char Configured_M = 0;   // 防重复配置中断使能标识
char Configured_HSV = 0; // 防重复配置中断使能标识

/*flash*/
unsigned char RGB_SHOW_NOW[WS2812_NUMS][3] = {0};
unsigned int RGB24BUT32[WS2812_NUMS] = {0};
unsigned char RGB_SHOW_TEMP[WS2812_NUMS][3];

/*有几种效果就定义到那儿去，最大为RGB_Mode6*/
RGBModeEnum RGB_Mode_END = RGB_Mode6;
RGBModeEnum MODE_NOW = RGB_Mode1;

/*归0*/

void definition_init(void)
{
    int led_pixels = 0;
    int color_channel = 0;

    /*KEY*/
    swh = 1; // 按键H状态
    sws = 1; // 按键S状态
    swv = 1; // 按键V状态
    swm = 1; // 按键M状态
    swr = 1; // 按键R状态

    swh_nums = 0;
    sws_nums = 0;
    swv_nums = 0;
    swm_nums = 1; // RGB mode的枚举是从1开始的
    swr_nums = 0;

    Reverse_sign = 1;    // R 键按下标志
    ModeChange_sign = 1; // M  键按下标志,用于屏蔽中断 0中断开启，1中断关闭，此处定义为1是为了第一次Update进行
    EXTI0_1_sign = 1;    // EXTI0 响应标志
    Configured_M = 0;    // 防重复配置中断使能标识
    Configured_HSV = 0;  // 防重复配置中断使能标识

    /*flash*/
    for (led_pixels = 0; led_pixels < WS2812_NUMS; led_pixels++)
    {
        RGB24BUT32[led_pixels] = 0;
        for (color_channel = 0; color_channel < 3; color_channel++)
        {
            RGB_SHOW_NOW[led_pixels][color_channel] = 0;
            RGB_SHOW_TEMP[led_pixels][color_channel] = 0;
        }
    }

    /*有几种效果就定义到那儿去，最大为RGB_Mode7*/
    MODE_NOW = RGB_Mode1;
}
