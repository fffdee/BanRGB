#include "spi.h"
#include "main.h"
#include "rgb.h"
#include "air001xx_hal.h"
#include "air001xx.h"
SPI_HandleTypeDef Spi1Handle;
unsigned char data[WS2812_MAX_NUMS * 24];

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

/**
 * @brief 初始化SPI的MCU Support Package，在SPI_CONFIG()中被HAL_SPI_Init调用，来自例程
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    
    GPIO_InitTypeDef GPIO_InitStruct;
    /* SPI1 初始化 */
    if (hspi->Instance == SPI1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();  /* GPIOB时钟使能 */
        __HAL_RCC_GPIOA_CLK_ENABLE();  /* GPIOA时钟使能 */
        __HAL_RCC_SYSCFG_CLK_ENABLE(); /* 使能SYSCFG时钟 */
        __HAL_RCC_SPI1_CLK_ENABLE();   /* SPI1时钟使能 */
        __HAL_RCC_DMA_CLK_ENABLE();    /* DMA时钟使能 */

        /*
          PA5-SCK  (AF0)
          PA6-MISO(AF0)
          PA7-MOSI(AF0)
          PA4-NSS(AF0)
        */

        /*GPIO配置为SPI：SCK/MISO/MOSI*/
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/*SPI 配置函数,本函数嵌套了其他HAL_MspInit()、 HAL_SPI_MspInit();*/
void SPI_CONFIG(void)
{
   
    Spi1Handle.Instance = SPI1;                                  /* SPI1 */
    Spi1Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* 2分频 */
    Spi1Handle.Init.Direction = SPI_DIRECTION_2LINES;            /* 全双工 */
    Spi1Handle.Init.CLKPolarity = SPI_POLARITY_LOW;              /* 时钟极性低 ，空闲时低电平*/
    Spi1Handle.Init.CLKPhase = SPI_PHASE_1EDGE;                  /* 数据采样从第一个时钟边沿开始 */
    Spi1Handle.Init.DataSize = SPI_DATASIZE_8BIT;                /* SPI数据长度为8bit */
    Spi1Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                 /* 先发送MSB，Most significant bit */
    Spi1Handle.Init.NSS = SPI_NSS_SOFT;                   /* NSS软件模式(硬件模式) */
    Spi1Handle.Init.Mode = SPI_MODE_MASTER;                      /* 配置为主机 */
    if (HAL_SPI_DeInit(&Spi1Handle) != HAL_OK)
    {
        Error_Handler();
    }

    /*SPI初始化*/
    if (HAL_SPI_Init(&Spi1Handle) != HAL_OK) // 这个函数的中间调用了HAL_SPI_MspInit（&SpiHandle）
    {
        Error_Handler();
    }

    HAL_MspInit();
}


void WS2812_RESET(void)
{
    
  unsigned char reset[700] = {0xff} ;     //可能有栈溢出危险
	
   HAL_SPI_Transmit(&Spi1Handle, reset, 700, 0xFFFF);
}
/**
 * @brief   将二维数组指针的RGB数据通过SPI发出去，WS2812b的数据，为GRB格式，接受的数据依次按位保存在G、R、B，每一个都是8位0-255
 * @param   (*RGB)[3]  RGB_SHOW_TEMP或者rgb_t.RGB_SHOW_NOW数组；
 * @retval  无
*/
void WS2812B_SendRGB(unsigned char (*RGB)[3])    //rgb_t.RGB_SHOW_NOW[18][3]  就一直发GRBGRB就行；5
{

  unsigned char *ptr = data;

int led= 0;
	int i = 0;
for(led=0;led < WS2812_MAX_NUMS; led++)
{
    // 将G数据转换为WS2812B数据格式
  for (i= 0; i < 8; i++)
  {
		if (RGB[led][1]& (1 << (7 - i)))   //g
    {
      *ptr++ = 0xF8; // 1的时序   或者FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0的时序 
    }
    
  }
	/*转换R*/
	for (i= 0; i < 8; i++)
  {
		if (RGB[led][0]& (1 << (7 - i)))   //r
    {
      *ptr++ = 0xF8; // 1的时序   或者FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0的时序 
    }
    
  }

  /*B */
	for (i= 0; i < 8; i++)
  {
		if (RGB[led][2]& (1 << (7 - i)))   //b
    {
      *ptr++ = 0xF8; // 1的时序   或者FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0的时序 
    }

  }
	HAL_SPI_Transmit(&Spi1Handle, &data[0], WS2812_MAX_NUMS*24,0xFFFF);
//		if(WS2812_MAX_NUMS<36){
//				HAL_SPI_Transmit(&Spi1Handle, data, WS2812_MAX_NUMS*24,0xFFFF);
//		}else{
//				HAL_SPI_Transmit(&Spi1Handle, data, 36*24,0xFFFF);
//				HAL_SPI_Transmit(&Spi1Handle, &data[36*24], (WS2812_MAX_NUMS-36)*24,0xFFFF);
//		}

}
  


  
}


void SetColor(uint8_t index, uint8_t* color){

		rgb_t.RGB_SHOW_NOW[index][0]=color[0];
		rgb_t.RGB_SHOW_NOW[index][1]=color[1];
		rgb_t.RGB_SHOW_NOW[index][2]=color[2];
		
}


