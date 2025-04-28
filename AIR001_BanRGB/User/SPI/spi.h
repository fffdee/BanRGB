/**
 * @brief 使用SPI模拟WS2812B时序，将24bit数据按位依次发送出去，
          PA5-SCK  (AF0)
          PA6-MISO(AF0)
          PA7-MOSI(AF0)  数据从PA7口发出
          PA4-NSS(AF0)
        
 * @version v3.0
 * @date   2024/01/28
*/


#ifndef spi_H_
#define spi_H_

#include "air001xx_hal_spi.h"
#include "main.h"


extern  SPI_HandleTypeDef Spi1Handle;

void SPI_CONFIG(void);
void WS2812B_SendRGB(unsigned char (*RGB)[3]) ;
void WS2812_RESET(void);
void SetColor(uint8_t index, uint8_t* color);
 extern uint8_t RGB_SHOW_NOW[WS2812_NUMS][3];

void ReadRGBDataFromFlash(uint8_t Destination_Array[WS2812_NUMS][3]);
void RGB_Storage(uint8_t Source_Array[WS2812_NUMS][3]);

#endif /*spi_H_*/