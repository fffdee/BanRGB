# AIR001_BanRGB_V2 固件更新指南

## 更新说明

本次固件更新为 AIR001_BanRGB_V2 添加了**主动握手响应功能**，使上位机能够自动识别和连接设备。

## 更新内容

### 1. 新增握手响应函数

在 `air001xx_it.c` 中添加：

```c
// 握手响应函数
void Handle_Handshake(void)
{
    const char* response = "BANRGB_OK\n";
    HAL_UART_Transmit(&UartHandle, (uint8_t*)response, strlen(response), 1000);
}
```

### 2. 修改串口中断处理

修改 `USART1_IRQHandler()` 函数：

```c
void USART1_IRQHandler(void) // 串口1中断
{
    if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
    {
        ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
        
        // 处理握手命令 'E'
        if(ch == 'E' || ch == 0x45) {
            Handle_Handshake();
            count = 0; // 重置计数器
        }
        else {
            rx_buffer[count++] = ch;
            if(count > (rgb_t.total*5)-1) count = 0;
        }
    }
    HAL_UART_IRQHandler(&UartHandle);
}
```

### 3. 移除开机自动发送

在 `main.c` 的 `main()` 函数中，移除以下行：

```c
// 移除这一行
printf("E");
```

修改后的 `main()` 函数开头部分：

```c
int main(void)
{
    HAL_Init();
    HAL_InitTick(3);
    APP_SystemClockConfig(); // 系统时钟配置
    Uart_Init(); // 串口初始化
    SPI_CONFIG(); // SPI配置
    KEY_CONFIG(); // 按键配置
    // printf("E"); <-- 删除这一行
    __HAL_RCC_LSI_ENABLE();
    // ...后续代码...
}
```

## 修改的文件清单

```
AIR001_BanRGB_V2/
├── User/
│   ├── air001xx_it.c        ✏️ 修改（新增握手响应）
│   └── main.c               ✏️ 修改（移除自动发送）
```

## 工作原理

### 旧版本（V1.0）
```
下位机上电 → 自动发送 'E' → 上位机被动接收 → 建立连接
```

**问题**：
- 上位机无法主动发现设备
- 需要等待下位机发送信号
- 可能连接到错误的设备

### 新版本（V2.0）
```
上位机扫描串口 → 向每个串口发送 'E' → 下位机检测并响应 'BANRGB_OK' → 上位机验证响应 → 建立连接
```

**优势**：
- ✅ 上位机主动控制连接过程
- ✅ 自动识别正确的设备
- ✅ 支持多个串口并发测试
- ✅ 握手机制更安全可靠

## 烧录步骤

### 使用 Keil MDK

1. **打开工程**
   - 启动 Keil MDK
   - 打开 `AIR001_BanRGB_V2/air001_template.uvprojx`

2. **编译工程**
   - 点击 `Project` -> `Build Target` (F7)
   - 确认编译成功，无错误

3. **连接调试器**
   - 使用 ST-Link 或其他支持的调试器连接 AIR001 开发板
   - 确认连接正常

4. **烧录固件**
   - 点击 `Flash` -> `Download` (F8)
   - 等待烧录完成

5. **复位设备**
   - 按下复位按钮或断电重启

### 使用命令行工具

如果使用 OpenOCD 或其他工具：

```bash
# 编译
make clean
make

# 烧录
openocd -f interface/stlink.cfg -f target/air001.cfg -c "program build/air001template.hex verify reset exit"
```

## 验证更新

### 1. 硬件测试

连接设备后，使用串口调试工具：

1. **打开串口**：115200 波特率，8N1
2. **发送握手命令**：发送字符 `'E'`（单字符，无换行）
3. **检查响应**：应该收到 `"BANRGB_OK\n"`

### 2. 上位机测试

1. **启动 BanRGB V2.0**
2. **观察调试输出**（启用 `sac_debug = 1`）：
   ```
   [debug] 找到以下串口: ['COM3']
   [debug] 向 COM3 发送握手命令: 'E'
   [debug] COM3 握手响应: 'BANRGB_OK\n'
   [debug] COM3 握手成功！
   ```
3. **检查连接状态**：状态栏应显示 "已连接: COMX"

### 3. 功能测试

- ✅ 屏幕取色功能正常
- ✅ 自定义颜色功能正常
- ✅ LED数量配置正常
- ✅ 断开重连功能正常

## 常见问题

### Q1: 烧录后无法连接

**可能原因**：
- 串口波特率不匹配
- USB驱动未安装
- 固件烧录不完整

**解决方法**：
1. 检查 `uart.c` 中波特率是否为 115200
2. 重新安装 CH340/CP2102 驱动
3. 重新烧录固件，确认烧录成功

### Q2: 握手超时

**可能原因**：
- 下位机未正确响应握手命令
- 串口线连接不良
- 握手响应函数有误

**解决方法**：
1. 使用串口调试工具手动测试握手
2. 检查 `Handle_Handshake()` 函数实现
3. 确认 `USART1_IRQHandler()` 正确处理 'E' 字符

### Q3: 收到错误响应

**可能原因**：
- 代码修改不完整
- 编译优化导致问题
- 内存溢出

**解决方法**：
1. 对照本文档重新检查所有修改
2. 尝试降低编译优化等级
3. 检查 `rx_buffer` 大小是否足够

## 回退到旧版本

如果需要回退到旧版本，恢复以下修改：

### 1. 恢复 main.c
```c
int main(void)
{
    HAL_Init();
    HAL_InitTick(3);
    APP_SystemClockConfig();
    Uart_Init();
    SPI_CONFIG();
    KEY_CONFIG();
    printf("E");  // 恢复这一行
    __HAL_RCC_LSI_ENABLE();
    // ...
}
```

### 2. 恢复 air001xx_it.c
```c
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
    {
        ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
        rx_buffer[count++] = ch;
        if(count > (rgb_t.total*5)-1) count = 0;
    }
    HAL_UART_IRQHandler(&UartHandle);
}
```

## 技术支持

如有问题，请参考：
- 握手协议详细说明：`HANDSHAKE_PROTOCOL.md`
- AIR001协议文档：`PROTOCOL_AIR001.md`
- 上位机使用说明：`README_V2.md`

---

**固件版本**: AIR001_BanRGB_V2.0  
**更新日期**: 2024-01  
**作者**: BanGO
