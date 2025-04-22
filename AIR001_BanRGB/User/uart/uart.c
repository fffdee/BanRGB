#include "uart.h"

UART_HandleTypeDef UartHandle;

void Uart_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1; // air001x_hal_gpio_ex.h,见数据手册，引脚复用
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1; // air001x_hal_gpio_ex.h,见数据手册，引脚复用
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    UartHandle.Instance = USART1;
    UartHandle.Init.BaudRate = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&UartHandle);
    __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE); // 开启接收中断

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0); /// 配置中断优先级，hal cortex.h  (air001_dev.h  位于pack管理里 Keil5\Packs\Keil\AIR001_DFP\1.1.0\Device\Include)
                                             // 仅第一个0有效，0-3，0最高
    HAL_NVIC_EnableIRQ(USART1_IRQn);         // 使能外部中断
}

/*****************  发送字符串 **********************/
void Usart_SendString(uint8_t *str)
{
    unsigned int k = 0;
    do
    {
        HAL_UART_Transmit(&UartHandle, (uint8_t *)(str + k), 1, 1000);
        k++;
    } while (*(str + k) != '\0');
}

// 重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口DEBUG_USART */
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);

    return (ch);
}

// 重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
    int ch;
    HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 1000);
    return (ch);
}