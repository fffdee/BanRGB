#include "ws2812b.h"



/**
 * @brief  ������Ӧ�ĵ�����ʾ���ױ���Ϊ�����ĵƣ��α���Ϊȡ��TEMP����ĵڼ�������.
 * @param  WS2812B_x Specifies the WS2812B  nums  0-17
 * @param  temp_x  specifies the number of lights in the RGB_SHOW_TEMP array ��where x can be (0..17)
 * @retval None
 */
void WS2812B_x_ON(unsigned char WS2812B_x, unsigned char temp_x)
{
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        RGB_SHOW_NOW[WS2812B_x][i] = RGB_SHOW_TEMP[temp_x][i];
    }
}

/**
 * @brief  ������Ӧ�ĵ�����ʾ���ױ���Ϊ�����ĵƣ��α���Ϊȡ��TEMP����ĵڼ�������.
 * @param  WS2812B_x Specifies the WS2812B  nums  0-17
 * @param  temp_x  specifies the number of lights in the RGB_SHOW_TEMP array ��where x can be (0..17)
 * @param  Value  ��������ֵ��0-1
 * @retval None
 */
void WS2812B_x_ON_With_Value(unsigned char WS2812B_x, unsigned char temp_x, float Value)
{
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        RGB_SHOW_NOW[WS2812B_x][i] = RGB_SHOW_TEMP[temp_x][i] * Value;
    }
}

/**
 * @brief  �رն�Ӧ�ĵ�����ʾ.
 * @param  WS2812B_x Specifies the WS2812B  This parameter can be any combination of WS2812B_x where x can be (0..17)
 * @retval None
 */
void WS2812B_x_OFF(unsigned char WS2812B_x)
{
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        RGB_SHOW_NOW[WS2812B_x][i] = 0;
    }
}

/**
 * @brief  ���л�ȡ������ɫȫ������ʾ.
 * @retval None
 */
void WS2812B_x_SHOW_ALL(void)
{
    int i = 0;
    for (i = 0; i < WS2812_NUMS; i++)
    {
        RGB_SHOW_NOW[i][0] = RGB_SHOW_TEMP[i][0];
        RGB_SHOW_NOW[i][1] = RGB_SHOW_TEMP[i][1];
        RGB_SHOW_NOW[i][2] = RGB_SHOW_TEMP[i][2];
    }
}

/**
 * @brief  �����ɫ.
 * @retval None
 */
void WS2812B_x_CLEAR_ALL(void)
{
    int i = 0;
    for (i = 0; i < WS2812_NUMS; i++)
    {
        RGB_SHOW_NOW[i][0] = 0;
        RGB_SHOW_NOW[i][1] = 0;
        RGB_SHOW_NOW[i][2] = 0;
    }
}

/**
 * @brief ���RGB_SHOW_NOW��RGB_SHOW_TEMP���������������Ϣ����Mode_Scan�����е��ã���������飬�����룬��ҪһЩ�Ʋ���ʱ��Ҫ�õ�
 * @param none
 * @retval ��
*/
void RGB_NOW_TEMP_ALLCLEAR(void)
{
    int i = 0;
    for (i = 0; i < WS2812_NUMS; i++)
    {
        RGB_SHOW_NOW[i][0] = 0;
        RGB_SHOW_NOW[i][1] = 0;
        RGB_SHOW_NOW[i][2] = 0;
        RGB_SHOW_TEMP[i][0] = 0;
        RGB_SHOW_TEMP[i][1] = 0;
        RGB_SHOW_TEMP[i][2] = 0;
    }
}

/**
 * @brief ����Ws2812B����������ֲ����ģ�����һ��ֹͣ�źţ���һ��ʱ���ڽ��յ�����ȫΪ�ߵ�ƽ����ʾ�˴ν������ݽ���
 *        ��һ�ν��յ������������һ����;�����ĵ��еط�Ƶ�����Ǿ�����reset����ĳ���
 * @param 
 * @retval none
*/
void WS2812_RESET(void)
{
    
    unsigned char reset[700] = {0} ;     //������ջ���Σ��
	
    HAL_SPI_Transmit(&Spi1Handle, reset, 700, 0xFFFF);
}

/**
 * @brief ��ת��/����ʱ��RGB_SHOW_NOW����Ĵ������,��num2�Ƶ����ݴ���num1
 * @param num1 0-18
 * @param num2 0-18
 * @rtval �޷���ֵ
*/
void RGB_Now_Transfer(unsigned char num1, unsigned char num2)
{
    RGB_SHOW_NOW[num1][0] = RGB_SHOW_NOW[num2][0];
    RGB_SHOW_NOW[num1][1] = RGB_SHOW_NOW[num2][1];
    RGB_SHOW_NOW[num1][2] = RGB_SHOW_NOW[num2][2];
}

/**
 * @brief ��ת��/����ʱ��RGB_SHOW_TEMP���鴦�����,��num2�Ƶ����ݴ���num1
 * @param num1 0-18
 * @param num2 0-18
 * @rtval �޷���ֵ
*/
void RGB_Temp_Transfer(unsigned char num1, unsigned char num2)
{
    RGB_SHOW_TEMP[num1][0] = RGB_SHOW_TEMP[num2][0];
    RGB_SHOW_TEMP[num1][1] = RGB_SHOW_TEMP[num2][1];
    RGB_SHOW_TEMP[num1][2] = RGB_SHOW_TEMP[num2][2];
}

/**
 * @brief  �����ã���WS2812b��������ɫ��ӦHSV
 * @retval
 */
void Color_Browse(void)
{
    int i = 0;
    RGB_NOW_TEMP_ALLCLEAR();
    for (i = 0; i <= 36; i++)
    {
        HSV_H = i * 10;
        HSV_S = 1;
        HSV_V = 0.02;
        HSV_Update_NOW();
        WS2812_RESET();
        WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
        HAL_Delay(1000);
        WS2812B_x_CLEAR_ALL();
        WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
        HAL_Delay(1000);
				printf("Hello World!\r\n");
    }
}
