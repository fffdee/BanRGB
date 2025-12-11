#include "ws2812b.h"



/**
 * @brief  开启对应的灯珠显示，首变量为开启的灯，次变量为取得TEMP数组的第几个数据.
 * @param  WS2812B_x Specifies the WS2812B  nums  0-17
 * @param  temp_x  specifies the number of lights in the RGB_SHOW_TEMP array ，where x can be (0..17)
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
 * @brief  开启对应的灯珠显示，首变量为开启的灯，次变量为取得TEMP数组的第几个数据.
 * @param  WS2812B_x Specifies the WS2812B  nums  0-17
 * @param  temp_x  specifies the number of lights in the RGB_SHOW_TEMP array ，where x can be (0..17)
 * @param  Value  亮度修正值，0-1
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
 * @brief  关闭对应的灯珠显示.
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
 * @brief  所有获取灯珠颜色全正常显示.
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
 * @brief  清除颜色.
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
 * @brief 清楚RGB_SHOW_NOW和RGB_SHOW_TEMP两个数组的所有信息，在Mode_Scan函数中调用，先清空数组，再填入，需要一些灯不亮时需要用到
 * @param none
 * @retval 无
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
 * @brief 根据Ws2812B数组的数据手册来的，这是一个停止信号，在一定时间内接收的数据全为高电平，表示此次接收数据结束
 *        下一次接收到的数据填入第一个灯;如果你的灯有地方频闪，那就增大reset数组的长度
 * @param 
 * @retval none
*/
void WS2812_RESET(void)
{
    
    unsigned char reset[700] = {0} ;     //可能有栈溢出危险
	
    HAL_SPI_Transmit(&Spi1Handle, reset, 700, 0xFFFF);
}

/**
 * @brief 灯转动/流动时，RGB_SHOW_NOW数组的处理代码,将num2灯的数据传给num1
 * @param num1 0-18
 * @param num2 0-18
 * @rtval 无返回值
*/
void RGB_Now_Transfer(unsigned char num1, unsigned char num2)
{
    RGB_SHOW_NOW[num1][0] = RGB_SHOW_NOW[num2][0];
    RGB_SHOW_NOW[num1][1] = RGB_SHOW_NOW[num2][1];
    RGB_SHOW_NOW[num1][2] = RGB_SHOW_NOW[num2][2];
}

/**
 * @brief 灯转动/流动时，RGB_SHOW_TEMP数组处理代码,将num2灯的数据传给num1
 * @param num1 0-18
 * @param num2 0-18
 * @rtval 无返回值
*/
void RGB_Temp_Transfer(unsigned char num1, unsigned char num2)
{
    RGB_SHOW_TEMP[num1][0] = RGB_SHOW_TEMP[num2][0];
    RGB_SHOW_TEMP[num1][1] = RGB_SHOW_TEMP[num2][1];
    RGB_SHOW_TEMP[num1][2] = RGB_SHOW_TEMP[num2][2];
}

/**
 * @brief  测试用，看WS2812b的所有颜色对应HSV
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
