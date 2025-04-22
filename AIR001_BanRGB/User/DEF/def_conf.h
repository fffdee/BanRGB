/**
 * @brief 一些全局变量的定义文件
 * @version v3.0
 * @date   2024/01/28
*/

#ifndef def_conf_H_
#define def_conf_H_

#include "air001xx_hal.h"
#include "air001_dev.h"
#include "air001xx_hal_gpio.h"

#define WS2812_NUMS 22// WS2812 numbers,需要为偶数，否则，某些效果可能会有些奇怪的地方，但是肯定能显示

/*HSVRGB*/
extern float HSV_H;
extern float HSV_S;
extern float HSV_V;

/*KEY*/
extern GPIO_PinState swh; // 按键H状态
extern GPIO_PinState sws; // 按键S状态
extern GPIO_PinState swv; // 按键V状态
extern GPIO_PinState swm; // 按键M状态
extern GPIO_PinState swr; // 按键R状态

extern int swh_nums;
extern int sws_nums;
extern int swv_nums;
extern int swm_nums; // RGB mode的枚举是从1开始的
extern int swr_nums;

extern int Reverse_sign;    // R 键按下标志
extern int ModeChange_sign; // M 键按下标志
extern int EXTI0_1_sign;    // EXTI0 响应标志
extern char Configured_M;   // 防重复配置中断使能标识
extern char Configured_HSV; // 防重复配置中断使能标识

/*ws2812b*/
/*0-17号灯珠的枚举*/
typedef enum
{
    WS2812B_0,
    WS2812B_1,
    WS2812B_2,
    WS2812B_3,
    WS2812B_4,
    WS2812B_5,
    WS2812B_6,
    WS2812B_7,
    WS2812B_8,
    WS2812B_9,
    WS2812B_10,
    WS2812B_11,
    WS2812B_12,
    WS2812B_13,
    WS2812B_14,
    WS2812B_15,
    WS2812B_16,
    WS2812B_17,
		WS2812B_18,
		WS2812B_19,
		WS2812B_20,
		WS2812B_21,
	
} WS2812B_POS;

/*flash*/

extern unsigned char RGB_SHOW_NOW[WS2812_NUMS][3];
extern unsigned char RGB_SHOW_TEMP[WS2812_NUMS][3];
extern unsigned int RGB24BUT32[WS2812_NUMS];

/*可增加模式*/
typedef enum
{
    RGB_Mode1 = 1, // 全亮模式1
    RGB_Mode2,     // 展示模式2
    RGB_Mode3,     // 呼吸展示模式3
    RGB_Mode4,     // 呼吸转模式4
    RGB_Mode5,     // 流星模式5
    RGB_Mode6,     // 三叉模式6
    RGB_Mode7,     // 展示模式7
    RGB_Mode8      // 展示模式7

} RGBModeEnum;

extern RGBModeEnum RGB_Mode_END;
extern RGBModeEnum MODE_NOW; // 做笔记，枚举的变量不能用static修饰，在.h文件，用extern变为全局变量引用

void definition_init(void);
#endif
