#include "air001xx_hal.h"
#include "air001xx.h"
#include "uart.h"
#include "string.h"
#include "main.h"
#include "stdint.h"
#include "stdlib.h"
#define TRAN_LIMIT 200
#define TRAN_COUNT 15
#define RESET_COUNT 40
#define HRESET_COUNT 100
uint8_t rx_buffer[WS2812_NUMS*5] = {0}; // ���ջ�����������������256�ֽ�

void Error_Handler(void);
static void APP_SystemClockConfig(void); // RCC ʱ������
uint16_t i;
uint8_t RxFlag;
uint8_t err = 0;
extern volatile uint8_t ch;
 IWDG_HandleTypeDef   IwdgHandle;
uint8_t led_map[WS2812_NUMS] = {

  30,29,28,25,22,19,16,13,10,9,
  32,31,26,23,20,17,14,12,7,8,
  34,33,27,24,21,18,15,11,5,6,
  36,35,42,45,48,51,54,57,3,4,
  38,37,41,44,47,50,53,56,2,1,
  39,40,43,46,49,52,55,58,59,0,
 

};

void smooth_color_transition(uint8_t *prev, uint8_t *curr) {
  uint8_t i = 0;  
	for (; i < 3; i++) {
        if (curr[i] > prev[i] + 70) {
            curr[i] = prev[i] + 70;
        } else if (curr[i] < prev[i] - 70) {
            curr[i] = prev[i] - 70;
        }
    }
}

void Color_Browse(void)
{			
				for(i=0;i<240;i++){
				
				memset(RGB_SHOW_NOW,0xFF,WS2812_NUMS*3);
				RGB_SHOW_NOW[i%WS2812_NUMS][1]=i;
				WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
        HAL_Delay(1);
//				printf("dark!\r\n");
				memset(RGB_SHOW_NOW,0x10,WS2812_NUMS*3);
				WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
				HAL_Delay(1);
					
				}
}
uint8_t resetFlag = 0;
uint8_t rgbflag = 0;
uint8_t rgbdata[4] = {0};
uint8_t rgb_count = 0;
uint8_t red[3] = {0};
uint16_t rgb_range[3];
uint16_t rgb_range_old[3];
uint8_t tran_count[3];
uint16_t tick = 0;
int main(void)
{

	HAL_Init();
	APP_SystemClockConfig();
	Uart_Init();
	SPI_CONFIG();
	printf("RESET!");
	__HAL_RCC_LSI_ENABLE();
  IwdgHandle.Instance = IWDG;                     /* ѡ��IWDG */
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;  /* ����32��Ƶ */
  IwdgHandle.Init.Reload = (1000);                /* IWDG��������װ��ֵΪ1000��1s */
  /* ��ʼ��IWDG */
  if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)       
  {
    Error_Handler();
  }
	while (1)
	{	
			for(i=0;i<WS2812_NUMS*5;i++){
				
				if(rx_buffer[i]==0xEA){
								
							uint8_t index  = led_map[rx_buffer[i+1]];
							uint8_t count=0;
							for(;count<3;count++){
					
										RGB_SHOW_NOW[index][count]= rx_buffer[i+2+count];
										if(index==WS2812_NUMS-2)
												RGB_SHOW_NOW[0][count]= rx_buffer[i+2+count];
									}
							
							HAL_UART_IRQHandler(&UartHandle);
						
				}
						
				
				if(rx_buffer[i]==0xEB){		
					
					if(rx_buffer[i+1]==0xFF&&rx_buffer[i+2]==0xBE){		
						resetFlag = 1;
						
					}
				}
			}
			
//		memset(rgb_range,0,6);
//		for(i=0;i<WS2812_NUMS;i++){
//			
//			if(RGB_SHOW_NOW[i][0]>TRAN_LIMIT)
//				rgb_range[0]++;
//			if(RGB_SHOW_NOW[i][1]>TRAN_LIMIT)
//				rgb_range[1]++;
//			if(RGB_SHOW_NOW[i][2]>TRAN_LIMIT)
//				rgb_range[2]++;	
//		}
//		
//		if(rgb_range[0]<TRAN_COUNT &&rgb_range[1]<TRAN_COUNT &&rgb_range[2]<TRAN_COUNT )
//		WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
			
			for(i=0;i<WS2812_NUMS;i++){
			
			
				rgb_range[0]+=RGB_SHOW_NOW[i][0];
				rgb_range[1]+=RGB_SHOW_NOW[i][1];
				rgb_range[2]+=RGB_SHOW_NOW[i][2];

		}
		for(i=0;i<3;i++){
			rgb_range[i] = rgb_range[i]/WS2812_NUMS;
			tran_count[i] = abs(rgb_range[i]-rgb_range_old[i]); 
		}
//		if((HAL_GetTick()-tick)>300){
//			if(rgb_range[0]<TRAN_LIMIT &&rgb_range[1]<TRAN_LIMIT &&rgb_range[2]<TRAN_LIMIT )
//			WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
//			tick = HAL_GetTick();
//		}
			uint8_t total_rgb = (rgb_range[0]+rgb_range[1]+rgb_range[2])/3;
			if(total_rgb<TRAN_LIMIT){
//			if(rgb_range[0]<TRAN_LIMIT &&rgb_range[1]<TRAN_LIMIT &&rgb_range[2]<TRAN_LIMIT ){
				WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
//				printf("tran_Count:%d ,%d, %d",tran_count[0],tran_count[1],tran_count[2]);
//				printf("rgb_range:%d ,%d, %d",rgb_range[0],rgb_range[1],rgb_range[2]);
//				if(tran_count[0]>RESET_COUNT ||tran_count[1]>RESET_COUNT ||tran_count[2]>RESET_COUNT)
//						if(resetFlag==1)
//								NVIC_SystemReset(); 
//				if(tran_count[0]>HRESET_COUNT ||tran_count[1]>HRESET_COUNT ||tran_count[2]>HRESET_COUNT)
//					if(resetFlag==0)
//						NVIC_SystemReset(); 
			}else{
					if(tran_count[0]>TRAN_COUNT ||tran_count[1]>TRAN_COUNT ||tran_count[2]>TRAN_COUNT ){
						printf("tran_Count:%d ,%d, %d",tran_count[0],tran_count[1],tran_count[2]);
						printf("rgb_range:%d ,%d, %d",rgb_range[0],rgb_range[1],rgb_range[2]);
//								if(resetFlag==1)
//								NVIC_SystemReset(); 
								WS2812B_SendRGB((unsigned char(*)[3])RGB_SHOW_NOW);
							for(i=0;i<3;i++)
							rgb_range_old[i] = rgb_range[i];
							
					}
				}
//			for(i=0;i<3;i++)
//						rgb_range_old[i] = rgb_range[i];

		if (HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK)
    {
			rgbflag=1;
			printf("reset!");
			for(i=0;i<WS2812_NUMS;i++)
				SetColor(i,red);
			NVIC_SystemReset(); 
      Error_Handler();
    }
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
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz; /* ����HSI���ʱ��Ϊ16MHz,�������У׼ֵ */
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
	
//	NVIC_SystemReset(); //���������λ
	while (1)
	{
	}
}
