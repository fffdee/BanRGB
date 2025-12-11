/**
 * @file    uart_handshake_example.c
 * @brief   BanRGB V2.0 下位机握手响应示例代码
 * @date    2024
 * @note    适用于 AIR001 MCU，可移植到其他平台
 */

#include "main.h"
#include "string.h"

/* 串口接收缓冲区 */
#define UART_RX_BUFFER_SIZE 128
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
uint8_t uart_rx_index = 0;

/* 握手标识 */
#define HANDSHAKE_CMD "BANRGB?"
#define HANDSHAKE_RESPONSE "E"

/**
 * @brief  串口初始化（已由CubeMX生成，此处仅供参考）
 */
void UART_Init(void)
{
    // CubeMX已生成初始化代码
    // 启动中断接收
    HAL_UART_Receive_IT(&huart1, uart_rx_buffer, 1);
}

/**
 * @brief  串口接收中断回调函数
 * @param  huart: 串口句柄
 * @note   在 stm32xxxx_it.c 中被调用
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint8_t received_byte = uart_rx_buffer[0];
        
        /* 方案1：简单字符匹配（适用于单字符握手） */
        if (received_byte == '?') {
            // 检测到握手命令末尾的 '?'
            // 立即回复握手响应
            HAL_UART_Transmit(&huart1, (uint8_t*)HANDSHAKE_RESPONSE, 
                              strlen(HANDSHAKE_RESPONSE), 100);
        }
        
        /* 方案2：完整字符串匹配（更严格，推荐） */
        // 将接收到的字节添加到缓冲区
        static uint8_t handshake_buffer[16] = {0};
        static uint8_t handshake_index = 0;
        
        handshake_buffer[handshake_index++] = received_byte;
        if (handshake_index >= sizeof(handshake_buffer)) {
            handshake_index = 0;  // 防止溢出
        }
        
        // 检查是否收到完整握手命令
        if (strstr((char*)handshake_buffer, HANDSHAKE_CMD) != NULL) {
            // 收到握手命令，发送响应
            HAL_UART_Transmit(&huart1, (uint8_t*)HANDSHAKE_RESPONSE, 
                              strlen(HANDSHAKE_RESPONSE), 100);
            
            // 清空握手缓冲区
            memset(handshake_buffer, 0, sizeof(handshake_buffer));
            handshake_index = 0;
        }
        
        /* 继续接收下一个字节 */
        HAL_UART_Receive_IT(&huart1, uart_rx_buffer, 1);
    }
}

/**
 * @brief  方案3：DMA接收模式（高性能）
 * @note   需要在CubeMX中配置DMA
 */
#ifdef USE_DMA_UART

#define DMA_RX_BUFFER_SIZE 256
uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];
uint32_t last_dma_pos = 0;

void UART_DMA_Init(void)
{
    // 启动DMA循环接收
    HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, DMA_RX_BUFFER_SIZE);
}

void UART_DMA_Process(void)
{
    // 获取当前DMA位置
    uint32_t current_pos = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    
    if (current_pos != last_dma_pos)
    {
        // 计算接收的数据长度
        uint32_t length;
        if (current_pos > last_dma_pos) {
            length = current_pos - last_dma_pos;
        } else {
            length = DMA_RX_BUFFER_SIZE - last_dma_pos + current_pos;
        }
        
        // 检查是否包含握手命令
        for (uint32_t i = 0; i < length; i++)
        {
            uint32_t pos = (last_dma_pos + i) % DMA_RX_BUFFER_SIZE;
            
            // 简单检查（可以改为更复杂的状态机）
            if (dma_rx_buffer[pos] == '?')
            {
                // 发送握手响应
                HAL_UART_Transmit(&huart1, (uint8_t*)HANDSHAKE_RESPONSE, 
                                  strlen(HANDSHAKE_RESPONSE), 100);
                break;
            }
        }
        
        last_dma_pos = current_pos;
    }
}

#endif /* USE_DMA_UART */

/**
 * @brief  方案4：上电主动发送（最简单，但不推荐）
 * @note   仅当无法实现握手响应时使用
 */
void UART_SendBootMessage(void)
{
    // 延迟确保串口稳定
    HAL_Delay(100);
    
    // 发送上电标识
    HAL_UART_Transmit(&huart1, (uint8_t*)"E", 1, 100);
    
    // 可选：发送更详细的设备信息
    // HAL_UART_Transmit(&huart1, (uint8_t*)"BANRGB_V2.0_READY\r\n", 19, 100);
}

/**
 * @brief  主函数中调用示例
 */
void main(void)
{
    /* HAL初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 外设初始化 */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_SPI1_Init();
    
    /* 启动串口接收 */
    HAL_UART_Receive_IT(&huart1, uart_rx_buffer, 1);
    
    /* 可选：上电发送标识（方案4） */
    // UART_SendBootMessage();
    
    /* 主循环 */
    while (1)
    {
        /* DMA模式需要轮询处理 */
        #ifdef USE_DMA_UART
        UART_DMA_Process();
        #endif
        
        /* 其他业务逻辑 */
        // ...
    }
}

/**
 * @brief  调试输出函数（可选）
 * @param  format: 格式化字符串
 * @note   用于串口调试输出
 */
void Debug_Printf(const char *format, ...)
{
    #ifdef DEBUG_ENABLE
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    #endif
}

/**
 * @brief  完整示例：结合RGB控制
 * @note   展示握手与业务逻辑的结合
 */
typedef enum {
    STATE_INIT,
    STATE_WAIT_HANDSHAKE,
    STATE_CONNECTED,
    STATE_ERROR
} SystemState_t;

SystemState_t system_state = STATE_INIT;

void System_StateMachine(void)
{
    switch (system_state)
    {
        case STATE_INIT:
            // 初始化完成，等待握手
            system_state = STATE_WAIT_HANDSHAKE;
            break;
            
        case STATE_WAIT_HANDSHAKE:
            // 此状态下会在串口中断中自动响应握手
            // 收到正确的RGB控制命令后转入连接状态
            // （在实际的串口处理函数中检测）
            break;
            
        case STATE_CONNECTED:
            // 已连接，处理RGB控制命令
            // 定期发送心跳响应（如果上位机有心跳检测）
            break;
            
        case STATE_ERROR:
            // 错误状态，重新初始化
            HAL_Delay(1000);
            system_state = STATE_INIT;
            break;
    }
}

/**
 * @file    移植说明
 * @brief   如何移植到其他平台
 * 
 * STM32系列：
 * - 直接使用，修改USART1为实际使用的串口
 * 
 * ESP32/ESP8266：
 * - 将 HAL_UART_Transmit 改为 Serial.write()
 * - 将 HAL_UART_Receive_IT 改为 Serial.available() 轮询
 * 
 * Arduino：
 * - Serial.begin(115200);
 * - if (Serial.available()) { char c = Serial.read(); }
 * - if (c == '?') { Serial.write('E'); }
 * 
 * 其他平台：
 * - 参考平台的串口API进行修改
 */
