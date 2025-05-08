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
 * @brief ��ʼ��SPI��MCU Support Package����SPI_CONFIG()�б�HAL_SPI_Init���ã���������
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    
    GPIO_InitTypeDef GPIO_InitStruct;
    /* SPI1 ��ʼ�� */
    if (hspi->Instance == SPI1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();  /* GPIOBʱ��ʹ�� */
        __HAL_RCC_GPIOA_CLK_ENABLE();  /* GPIOAʱ��ʹ�� */
        __HAL_RCC_SYSCFG_CLK_ENABLE(); /* ʹ��SYSCFGʱ�� */
        __HAL_RCC_SPI1_CLK_ENABLE();   /* SPI1ʱ��ʹ�� */
        __HAL_RCC_DMA_CLK_ENABLE();    /* DMAʱ��ʹ�� */

        /*
          PA5-SCK  (AF0)
          PA6-MISO(AF0)
          PA7-MOSI(AF0)
          PA4-NSS(AF0)
        */

        /*GPIO����ΪSPI��SCK/MISO/MOSI*/
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/*SPI ���ú���,������Ƕ��������HAL_MspInit()�� HAL_SPI_MspInit();*/
void SPI_CONFIG(void)
{
   
    Spi1Handle.Instance = SPI1;                                  /* SPI1 */
    Spi1Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* 2��Ƶ */
    Spi1Handle.Init.Direction = SPI_DIRECTION_2LINES;            /* ȫ˫�� */
    Spi1Handle.Init.CLKPolarity = SPI_POLARITY_LOW;              /* ʱ�Ӽ��Ե� ������ʱ�͵�ƽ*/
    Spi1Handle.Init.CLKPhase = SPI_PHASE_1EDGE;                  /* ���ݲ����ӵ�һ��ʱ�ӱ��ؿ�ʼ */
    Spi1Handle.Init.DataSize = SPI_DATASIZE_8BIT;                /* SPI���ݳ���Ϊ8bit */
    Spi1Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                 /* �ȷ���MSB��Most significant bit */
    Spi1Handle.Init.NSS = SPI_NSS_SOFT;                   /* NSS���ģʽ(Ӳ��ģʽ) */
    Spi1Handle.Init.Mode = SPI_MODE_MASTER;                      /* ����Ϊ���� */
    if (HAL_SPI_DeInit(&Spi1Handle) != HAL_OK)
    {
        Error_Handler();
    }

    /*SPI��ʼ��*/
    if (HAL_SPI_Init(&Spi1Handle) != HAL_OK) // ����������м������HAL_SPI_MspInit��&SpiHandle��
    {
        Error_Handler();
    }

    HAL_MspInit();
}


void WS2812_RESET(void)
{
    
  unsigned char reset[700] = {0xff} ;     //������ջ���Σ��
	
   HAL_SPI_Transmit(&Spi1Handle, reset, 700, 0xFFFF);
}
/**
 * @brief   ����ά����ָ���RGB����ͨ��SPI����ȥ��WS2812b�����ݣ�ΪGRB��ʽ�����ܵ��������ΰ�λ������G��R��B��ÿһ������8λ0-255
 * @param   (*RGB)[3]  RGB_SHOW_TEMP����rgb_t.RGB_SHOW_NOW���飻
 * @retval  ��
*/
void WS2812B_SendRGB(unsigned char (*RGB)[3])    //rgb_t.RGB_SHOW_NOW[18][3]  ��һֱ��GRBGRB���У�5
{

  unsigned char *ptr = data;

int led= 0;
	int i = 0;
for(led=0;led < WS2812_MAX_NUMS; led++)
{
    // ��G����ת��ΪWS2812B���ݸ�ʽ
  for (i= 0; i < 8; i++)
  {
		if (RGB[led][1]& (1 << (7 - i)))   //g
    {
      *ptr++ = 0xF8; // 1��ʱ��   ����FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0��ʱ�� 
    }
    
  }
	/*ת��R*/
	for (i= 0; i < 8; i++)
  {
		if (RGB[led][0]& (1 << (7 - i)))   //r
    {
      *ptr++ = 0xF8; // 1��ʱ��   ����FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0��ʱ�� 
    }
    
  }

  /*B */
	for (i= 0; i < 8; i++)
  {
		if (RGB[led][2]& (1 << (7 - i)))   //b
    {
      *ptr++ = 0xF8; // 1��ʱ��   ����FC   F8
    }
    else
    {
      *ptr++ = 0xC0; // 0��ʱ�� 
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


