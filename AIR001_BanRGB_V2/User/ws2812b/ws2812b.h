/**
 * @brief 修改WS2812B两个数组的值，RGB_SHOW_NOW和RGB_SHOW_TEMP
 * @version v3.0
 * @date   2024/01/28
*/

#ifndef ws2812b_H_
#define ws2812b_H_

#include "def_conf.h"
#include "air001xx_hal.h"
#include "flash.h"
#include "spi.h"


void WS2812B_x_ON(unsigned char WS2812B_x, unsigned char temp_x);
void WS2812B_x_OFF(unsigned char WS2812B_x);
void WS2812B_x_SHOW_ALL(void);
void WS2812B_x_CLEAR_ALL(void);
void RGB_NOW_TEMP_ALLCLEAR(void);

void WS2812_RESET(void);
void WS2812B_x_ON_With_Value(unsigned char WS2812B_x, unsigned char temp_x, float Value);

void RGB_Now_Transfer(unsigned char num1, unsigned char num2);
void RGB_Temp_Transfer(unsigned char num1, unsigned char num2) ;
void Color_Browse(void);
#endif
