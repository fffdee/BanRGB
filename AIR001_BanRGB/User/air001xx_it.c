#include "air001xx_it.h"
#include "uart.h"
#include "main.h"
#include "string.h"
void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

volatile uint8_t ch = 0;
extern uint8_t rx_buffer[320]; // ���ջ�����������������256�ֽ�

uint16_t count = 0;
void USART1_IRQHandler(void) // ����1�ж�
{
 

  if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
  {

    ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
		rx_buffer[count++] = ch;
		if(count>319) count=0;
    RxFlag = 1;
  }
	HAL_UART_IRQHandler(&UartHandle);
			
 
}

/*

swm = HAL_GPIO_ReadPin(M_PORT, M_PIN);
  if (swm == 0)
  {
    ModeChange_sign = 1; // ģʽ�ı��־λ��1�������¶�ȡRGB_SHOW_TEMP[][]

    RGB_Storage(MODE_NOW, RGB_SHOW_NOW); // ÿ�θ���modeǰ���洢��ǰmode��RGBЧ��

    if (MODE_NOW < RGB_Mode_END) // RGB_Mode_END��flash.c�ж���,�м���Ч���Ͷ��嵽�Ƕ�ȥ�����Ϊ6
    {
      MODE_NOW += 1;
    }
    else if (MODE_NOW == RGB_Mode_END)
    {
      MODE_NOW = RGB_Mode1;
    }
  }
  else
  {
    MODE_NOW = MODE_NOW;
  }
*/