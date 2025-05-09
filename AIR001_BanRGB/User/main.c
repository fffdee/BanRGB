#include "air001xx_hal.h"
#include "air001xx.h"
#include "uart.h"
#include "string.h"
#include "main.h"
#include "rgb.h"
#include "stdint.h"
#include "stdlib.h"
#define TRAN_LIMIT 200
#define TRAN_COUNT 15
#define RESET_COUNT 40
#define HRESET_COUNT 100


uint8_t rx_buffer[200] = {0}; 

void Error_Handler(void);
static void APP_SystemClockConfig(void); 
uint16_t i;
uint8_t RxFlag;
uint8_t err = 0;

uint8_t resetFlag = 0;
uint8_t rgbflag = 0;
uint8_t rgbdata[4] = {0};
uint8_t rgb_count = 0;
uint8_t red[3] = {0};
uint16_t rgb_range[3];
uint16_t rgb_range_old[3];
uint8_t tran_count[3];
uint16_t tick = 0;

extern volatile uint8_t ch;

 
IWDG_HandleTypeDef   IwdgHandle;

RGB rgb_t = {
	
		.total = 64,
		.rgb_map = {
			
			30,29,28,25,22,19,16,13,10,9,
			32,31,26,23,20,17,14,12,7,8,
			34,33,27,24,21,18,15,11,5,6,
			36,35,42,45,48,51,54,57,3,4,
			38,37,41,44,47,50,53,56,2,1,
			39,40,43,46,49,52,55,58,59,0,
			
		},
		.RGB_SHOW_NOW = {0},
	
};


void Color_Browse(void)
{			
				for(i=0;i<240;i++){
				
				memset(rgb_t.RGB_SHOW_NOW,0xFF,WS2812_MAX_NUMS*3);
				rgb_t.RGB_SHOW_NOW[i%WS2812_MAX_NUMS][1]=i;
				WS2812B_SendRGB((unsigned char(*)[3])rgb_t.RGB_SHOW_NOW);
        HAL_Delay(1);

				memset(rgb_t.RGB_SHOW_NOW,0x10,WS2812_MAX_NUMS*3);
				WS2812B_SendRGB((unsigned char(*)[3])rgb_t.RGB_SHOW_NOW);
				HAL_Delay(1);
					
				}
}


void screen_mode_loop()
{
		
		for(i=0;i<WS2812_MAX_NUMS*5;i++){
				
				if(rx_buffer[i]==0xEA){
								
							uint8_t index  = rgb_t.rgb_map[rx_buffer[i+1]];
							uint8_t count=0;
							for(;count<3;count++){
					
										rgb_t.RGB_SHOW_NOW[index][count]= rx_buffer[i+2+count];
										if(index==WS2812_MAX_NUMS-2)
												rgb_t.RGB_SHOW_NOW[0][count]= rx_buffer[i+2+count];
									}
							
							HAL_UART_IRQHandler(&UartHandle);
						
				}
						
				
				if(rx_buffer[i]==0xEC){		
					
				
				}
			}
			

			
			for(i=0;i<WS2812_MAX_NUMS;i++){
			
			
				rgb_range[0]+=rgb_t.RGB_SHOW_NOW[i][0];
				rgb_range[1]+=rgb_t.RGB_SHOW_NOW[i][1];
				rgb_range[2]+=rgb_t.RGB_SHOW_NOW[i][2];

		}
			
		for(i=0;i<3;i++){
			rgb_range[i] = rgb_range[i]/WS2812_MAX_NUMS;
			tran_count[i] = abs(rgb_range[i]-rgb_range_old[i]); 
		}

			uint8_t total_rgb = (rgb_range[0]+rgb_range[1]+rgb_range[2])/3;
			if(total_rgb<TRAN_LIMIT){

				WS2812B_SendRGB((unsigned char(*)[3])rgb_t.RGB_SHOW_NOW);

			}else{
					if(tran_count[0]>TRAN_COUNT ||tran_count[1]>TRAN_COUNT ||tran_count[2]>TRAN_COUNT ){
						printf("tran_Count:%d ,%d, %d",tran_count[0],tran_count[1],tran_count[2]);
						printf("rgb_range:%d ,%d, %d",rgb_range[0],rgb_range[1],rgb_range[2]);
								WS2812B_SendRGB((unsigned char(*)[3])rgb_t.RGB_SHOW_NOW);
							for(i=0;i<3;i++)
							rgb_range_old[i] = rgb_range[i];
							
					}
				}
	
	
}

int main(void)
{

	HAL_Init();
	APP_SystemClockConfig();
	Uart_Init();
	SPI_CONFIG();
	printf("RESET!");
	__HAL_RCC_LSI_ENABLE();
  IwdgHandle.Instance = IWDG;                     /* 选择IWDG */
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;  /* 配置32分频 */
  IwdgHandle.Init.Reload = (1000);                /* IWDG计数器重装载值为1000，1s */

  if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)       
  {
    Error_Handler();
  }
	while (1)
	{	

		screen_mode_loop();
		
		if (HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK)
    {
			rgbflag=1;
			printf("reset!");
			for(i=0;i<WS2812_MAX_NUMS;i++)
				SetColor(i,red);
			NVIC_SystemReset(); 
      Error_Handler();
    }
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
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz; /* 设置HSI输出时钟为16MHz,库会设置校准值 */
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

void Error_Handler(void)
{

	while (1)
	{
	}
}
