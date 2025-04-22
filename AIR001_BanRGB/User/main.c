#include "air001xx_hal.h"
#include "air001xx.h"
#include "uart.h"

uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

void Error_Handler(void);
static void APP_SystemClockConfig(void); // RCC ʱ������

int main(void)
{
	int led;

	HAL_Init();
	APP_SystemClockConfig();
	Uart_Init();
	SPI_CONFIG();
	while (1)
	{
 	  Color_Browse();
		printf("Hello World!\r\n");

		HAL_Delay(1000);
	}
}
/*����HSI16MHZ��SPI����APB1,8MHZ*/
static void APP_SystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/* ����HSI,HSE,LSE,LSI,PLL����ʱ�� */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI ;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;						  /* ʹ��HSI */
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;						  /* HSIԤ��Ƶ */
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* ����HSI���ʱ��Ϊ16MHz,�������У׼ֵ */
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;						  /* ��ֹHSE */
	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;					  /* ����HSEƵ�ʷ�Χ,û�ÿ��Բ����� */
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;						  /* ��ֹLSI */
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;						  /* ��ֹLSE */
	RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;				  /*  LSEĬ����������  */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;					  /* �������໷��ƵPLL */
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;			  /* PLL��ʱ��Դ��PLLʱ��Դֻ��24Mʱ, ��ʹ��PLL���˴��Ķ���ûɶ��*/

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) /* ����ʱ�� */
	{
		Error_Handler();
	}

	/* ��ʼ��AHB,APB����ʱ�� */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; /* ����AHBʱ��Դ */
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	   /* ����AHBԤ��Ƶ */
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;	   /* ����APB1Ԥ��Ƶ,APB1����Ƶ����SPI������2��Ƶ */

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) /* �������� */
	{
		Error_Handler();
	}

	__HAL_RCC_FLASH_CLK_ENABLE(); /*flashʹ��*/
	__HAL_RCC_GPIOB_CLK_ENABLE(); /* ʹ��GPIOBʱ�� */
	__HAL_RCC_GPIOA_CLK_ENABLE(); /* ʹ��GPIOAʱ�� */
	__HAL_RCC_SPI1_CLK_ENABLE();  /*ʹ��ʱ�� SPI1*/
	__HAL_RCC_PWR_CLK_ENABLE();
}

///*����HSI16MHZ��SPI����APB1,8MHZ*/
//static void APP_SystemClockConfig(void)
//{
//	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

//	/* ����HSI,HSE,LSE,LSI,PLL����ʱ�� */
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
//	RCC_OscInitStruct.HSIState = RCC_HSI_ON;						  /* ʹ��HSI */
//	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;						  /* HSIԤ��Ƶ */
//	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* ����HSI���ʱ��Ϊ24MHz,�������У׼ֵ */
//	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;						  /* ��ֹHSE */
//	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;					  /* ����HSEƵ�ʷ�Χ,û�ÿ��Բ����� */
//	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;						  /* ��ֹLSI */
//	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;						  /* ��ֹLSE */
//	RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;				  /*  LSEĬ����������  */
//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;					  /* �������໷��ƵPLL */
//	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;			  /* PLL��ʱ��Դ��PLLʱ��Դֻ��24Mʱ, ��ʹ��PLL���˴��Ķ���ûɶ��*/

//	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) /* ����ʱ�� */
//	{
//		Error_Handler();
//	}

//	/* ��ʼ��AHB,APB����ʱ�� */
//	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
//	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* ����AHBʱ��Դ */
//	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;		  /* ����AHBԤ��Ƶ */
//	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;		  /* ����APB1Ԥ��Ƶ,APB1����Ƶ����SPI������2��Ƶ */

//	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) /* �������� */
//	{
//		Error_Handler();
//	}

//	__HAL_RCC_FLASH_CLK_ENABLE(); /*flashʹ��*/
//	__HAL_RCC_GPIOB_CLK_ENABLE(); /* ʹ��GPIOBʱ�� */
//	__HAL_RCC_GPIOA_CLK_ENABLE(); /* ʹ��GPIOAʱ�� */
//	__HAL_RCC_SPI1_CLK_ENABLE();  /*ʹ��ʱ�� SPI1*/
//	__HAL_RCC_PWR_CLK_ENABLE();
//}

void Error_Handler(void)
{
	while (1)
	{
	}
}
