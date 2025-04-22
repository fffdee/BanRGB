#include "def_conf.h"

/*HSVRGB*/
float HSV_H = 180;
float HSV_S = 0.5;
float HSV_V = 0.5;

/*KEY*/
GPIO_PinState swh = 1; // ����H״̬
GPIO_PinState sws = 1; // ����S״̬
GPIO_PinState swv = 1; // ����V״̬
GPIO_PinState swm = 1; // ����M״̬
GPIO_PinState swr = 1; // ����R״̬

int swh_nums = 0;
int sws_nums = 0;
int swv_nums = 0;
int swm_nums = 1; // RGB mode��ö���Ǵ�1��ʼ��
int swr_nums = 0;

/**Interrupt*/
int Reverse_sign = 1;    // R �����±�־
int ModeChange_sign = 0; // M  �����±�־,���������ж� 0�жϿ�����1�жϹرգ��˴�����Ϊ1��Ϊ�˵�һ��Update����
int EXTI0_1_sign = 1;    // EXTI0 ��Ӧ��־
char Configured_M = 0;   // ���ظ������ж�ʹ�ܱ�ʶ
char Configured_HSV = 0; // ���ظ������ж�ʹ�ܱ�ʶ

/*flash*/
unsigned char RGB_SHOW_NOW[WS2812_NUMS][3] = {0};
unsigned int RGB24BUT32[WS2812_NUMS] = {0};
unsigned char RGB_SHOW_TEMP[WS2812_NUMS][3];

/*�м���Ч���Ͷ��嵽�Ƕ�ȥ�����ΪRGB_Mode6*/
RGBModeEnum RGB_Mode_END = RGB_Mode6;
RGBModeEnum MODE_NOW = RGB_Mode1;

/*��0*/

void definition_init(void)
{
    int led_pixels = 0;
    int color_channel = 0;

    /*KEY*/
    swh = 1; // ����H״̬
    sws = 1; // ����S״̬
    swv = 1; // ����V״̬
    swm = 1; // ����M״̬
    swr = 1; // ����R״̬

    swh_nums = 0;
    sws_nums = 0;
    swv_nums = 0;
    swm_nums = 1; // RGB mode��ö���Ǵ�1��ʼ��
    swr_nums = 0;

    Reverse_sign = 1;    // R �����±�־
    ModeChange_sign = 1; // M  �����±�־,���������ж� 0�жϿ�����1�жϹرգ��˴�����Ϊ1��Ϊ�˵�һ��Update����
    EXTI0_1_sign = 1;    // EXTI0 ��Ӧ��־
    Configured_M = 0;    // ���ظ������ж�ʹ�ܱ�ʶ
    Configured_HSV = 0;  // ���ظ������ж�ʹ�ܱ�ʶ

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

    /*�м���Ч���Ͷ��嵽�Ƕ�ȥ�����ΪRGB_Mode7*/
    MODE_NOW = RGB_Mode1;
}
