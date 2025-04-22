#ifndef __UART_H__
#define __UART_H__

#include "air001_dev.h"
#include "air001xx_hal.h"
#include "air001xx_hal_uart.h"
#include "air001xx_hal_gpio.h"
#include "air001xx_hal_rcc.h"
#include "stdio.h"

// 串口引脚
#define UART_GPIO_PORT GPIOA
#define UART_TX_PIN GPIO_PIN_2
#define UART_RX_PIN GPIO_PIN_3
// 串口信息
#define UART_DEBUG_BAUDRATE 115200
#define UART_DEBUG USART1
// 串口中断
#define UART_DEBUG_IRQn USART1_IRQn              // stm32f10x.h   中断号
#define UART_DEBUG_IRQHandller USART1_IRQHandler // startup.stm32f10x_hd.s  中断服务函数

extern UART_HandleTypeDef UartHandle;

void Uart_Init(void);
void Usart_SendString(uint8_t *str);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);

#endif /* __UART_H__ */
