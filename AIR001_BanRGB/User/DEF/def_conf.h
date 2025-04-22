/**
 * @brief һЩȫ�ֱ����Ķ����ļ�
 * @version v3.0
 * @date   2024/01/28
*/

#ifndef def_conf_H_
#define def_conf_H_

#include "air001xx_hal.h"
#include "air001_dev.h"
#include "air001xx_hal_gpio.h"

#define WS2812_NUMS 22// WS2812 numbers,��ҪΪż��������ĳЩЧ�����ܻ���Щ��ֵĵط������ǿ϶�����ʾ

/*HSVRGB*/
extern float HSV_H;
extern float HSV_S;
extern float HSV_V;

/*KEY*/
extern GPIO_PinState swh; // ����H״̬
extern GPIO_PinState sws; // ����S״̬
extern GPIO_PinState swv; // ����V״̬
extern GPIO_PinState swm; // ����M״̬
extern GPIO_PinState swr; // ����R״̬

extern int swh_nums;
extern int sws_nums;
extern int swv_nums;
extern int swm_nums; // RGB mode��ö���Ǵ�1��ʼ��
extern int swr_nums;

extern int Reverse_sign;    // R �����±�־
extern int ModeChange_sign; // M �����±�־
extern int EXTI0_1_sign;    // EXTI0 ��Ӧ��־
extern char Configured_M;   // ���ظ������ж�ʹ�ܱ�ʶ
extern char Configured_HSV; // ���ظ������ж�ʹ�ܱ�ʶ

/*ws2812b*/
/*0-17�ŵ����ö��*/
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

/*������ģʽ*/
typedef enum
{
    RGB_Mode1 = 1, // ȫ��ģʽ1
    RGB_Mode2,     // չʾģʽ2
    RGB_Mode3,     // ����չʾģʽ3
    RGB_Mode4,     // ����תģʽ4
    RGB_Mode5,     // ����ģʽ5
    RGB_Mode6,     // ����ģʽ6
    RGB_Mode7,     // չʾģʽ7
    RGB_Mode8      // չʾģʽ7

} RGBModeEnum;

extern RGBModeEnum RGB_Mode_END;
extern RGBModeEnum MODE_NOW; // ���ʼǣ�ö�ٵı���������static���Σ���.h�ļ�����extern��Ϊȫ�ֱ�������

void definition_init(void);
#endif
