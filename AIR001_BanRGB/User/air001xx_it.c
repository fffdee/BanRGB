#include "air001xx_it.h"
#include "uart.h"
#include "main.h"
#include "string.h"
#include "rgb.h"
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
extern uint8_t rx_buffer[200]; // 接收缓冲区，假设最大接收256字节

uint16_t count = 0;
void USART1_IRQHandler(void) // 串口1中断
{
 

  if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
  {

    ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
		rx_buffer[count++] = ch;
		if(count>199) count=0;
  }
	HAL_UART_IRQHandler(&UartHandle);
			
 
}

