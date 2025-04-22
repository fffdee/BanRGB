#include "air001xx_hal.h"
#include "air001xx.h"
#include "uart.h"

uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

void Error_Handler(void);
static void APP_SystemClockConfig(void); // RCC 时钟配置

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
/*配置HSI16MHZ，SPI所在APB1,8MHZ*/
static void APP_SystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/* 配置HSI,HSE,LSE,LSI,PLL所有时钟 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI ;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;						  /* 使能HSI */
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;						  /* HSI预分频 */
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* 设置HSI输出时钟为16MHz,库会设置校准值 */
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;						  /* 禁止HSE */
	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;					  /* 设置HSE频率范围,没用可以不设置 */
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;						  /* 禁止LSI */
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;						  /* 禁止LSE */
	RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;				  /*  LSE默认驱动能力  */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;					  /* 不用锁相环倍频PLL */
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;			  /* PLL的时钟源，PLL时钟源只有24M时, 再使能PLL，此处的定义没啥用*/

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) /* 配置时钟 */
	{
		Error_Handler();
	}

	/* 初始化AHB,APB总线时钟 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; /* 配置AHB时钟源 */
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	   /* 设置AHB预分频 */
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;	   /* 设置APB1预分频,APB1不分频，在SPI配置中2分频 */

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) /* 配置总线 */
	{
		Error_Handler();
	}

	__HAL_RCC_FLASH_CLK_ENABLE(); /*flash使能*/
	__HAL_RCC_GPIOB_CLK_ENABLE(); /* 使能GPIOB时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE(); /* 使能GPIOA时钟 */
	__HAL_RCC_SPI1_CLK_ENABLE();  /*使能时钟 SPI1*/
	__HAL_RCC_PWR_CLK_ENABLE();
}

///*配置HSI16MHZ，SPI所在APB1,8MHZ*/
//static void APP_SystemClockConfig(void)
//{
//	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

//	/* 配置HSI,HSE,LSE,LSI,PLL所有时钟 */
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
//	RCC_OscInitStruct.HSIState = RCC_HSI_ON;						  /* 使能HSI */
//	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;						  /* HSI预分频 */
//	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* 设置HSI输出时钟为24MHz,库会设置校准值 */
//	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;						  /* 禁止HSE */
//	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;					  /* 设置HSE频率范围,没用可以不设置 */
//	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;						  /* 禁止LSI */
//	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;						  /* 禁止LSE */
//	RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;				  /*  LSE默认驱动能力  */
//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;					  /* 不用锁相环倍频PLL */
//	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;			  /* PLL的时钟源，PLL时钟源只有24M时, 再使能PLL，此处的定义没啥用*/

//	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) /* 配置时钟 */
//	{
//		Error_Handler();
//	}

//	/* 初始化AHB,APB总线时钟 */
//	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
//	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* 配置AHB时钟源 */
//	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;		  /* 设置AHB预分频 */
//	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;		  /* 设置APB1预分频,APB1不分频，在SPI配置中2分频 */

//	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) /* 配置总线 */
//	{
//		Error_Handler();
//	}

//	__HAL_RCC_FLASH_CLK_ENABLE(); /*flash使能*/
//	__HAL_RCC_GPIOB_CLK_ENABLE(); /* 使能GPIOB时钟 */
//	__HAL_RCC_GPIOA_CLK_ENABLE(); /* 使能GPIOA时钟 */
//	__HAL_RCC_SPI1_CLK_ENABLE();  /*使能时钟 SPI1*/
//	__HAL_RCC_PWR_CLK_ENABLE();
//}

void Error_Handler(void)
{
	while (1)
	{
	}
}
