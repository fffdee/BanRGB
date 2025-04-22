#include "air001xx_it.h"
#include "uart.h"

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

extern uint8_t Rxflag;

void USART1_IRQHandler(void) // 串口1中断
{
  uint8_t ch = 0;

  if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
  {
    ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
    WRITE_REG(UartHandle.Instance->DR, ch);
  }

  HAL_UART_IRQHandler(&UartHandle);
}

/*

swm = HAL_GPIO_ReadPin(M_PORT, M_PIN);
  if (swm == 0)
  {
    ModeChange_sign = 1; // 模式改变标志位置1，需重新读取RGB_SHOW_TEMP[][]

    RGB_Storage(MODE_NOW, RGB_SHOW_NOW); // 每次更改mode前，存储当前mode的RGB效果

    if (MODE_NOW < RGB_Mode_END) // RGB_Mode_END在flash.c中定义,有几种效果就定义到那儿去，最大为6
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