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

#include "def_conf.h"
#include "flash.h"
#include "air001xx_hal_spi.h"



extern  SPI_HandleTypeDef Spi1Handle;

void SPI_CONFIG(void);
void WS2812B_SendRGB(unsigned char (*RGB)[3]) ;




#endif /*spi_H_*/