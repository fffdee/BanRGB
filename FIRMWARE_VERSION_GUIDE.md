# AIR001 固件版本说明

## 📦 固件版本对比

项目中包含两个 AIR001 固件版本：

### AIR001_BanRGB（V1.0 - 旧版）
- 📁 位置：`AIR001_BanRGB/`
- 📅 版本：V1.0（早期版本）
- ⚠️ 状态：**不推荐使用**

### AIR001_BanRGB_V2（V2.0 - 新版）
- 📁 位置：`AIR001_BanRGB_V2/`
- 📅 版本：V2.0（当前版本）
- ✅ 状态：**推荐使用**

## 🔄 主要区别

### V1.0（旧版）

**特性**：
- 开机自动发送 'E' 字符
- 被动等待上位机连接
- 无握手响应机制
- 基础RGB控制功能

**问题**：
- ❌ 上位机无法主动发现设备
- ❌ 可能误连其他串口设备
- ❌ 连接不够可靠

### V2.0（新版）

**特性**：
- ✅ **主动握手响应**：检测到 'E' 命令后回复 "BANRGB_OK"
- ✅ **智能设备识别**：只有正确握手才建立连接
- ✅ **移除自动发送**：不再开机自动发送，改为响应式
- ✅ **完整RGB控制**：支持所有LED控制功能

**优势**：
- ✅ 上位机主动控制连接过程
- ✅ 精确识别BanRGB设备
- ✅ 连接更可靠、更安全
- ✅ 支持自动重连

## 📝 V2.0 更新内容

### 1. 握手响应函数

**文件**：`air001xx_it.c`

```c
// 新增握手响应函数
void Handle_Handshake(void)
{
    const char* response = "BANRGB_OK\n";
    HAL_UART_Transmit(&UartHandle, (uint8_t*)response, strlen(response), 1000);
}
```

### 2. 中断处理优化

**文件**：`air001xx_it.c`

```c
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET)
    {
        ch = (uint16_t)READ_REG(UartHandle.Instance->DR);
        
        // 新增：检测握手命令
        if(ch == 'E' || ch == 0x45) {
            Handle_Handshake();
            count = 0;
        }
        else {
            rx_buffer[count++] = ch;
            if(count > (rgb_t.total*5)-1) count = 0;
        }
    }
    HAL_UART_IRQHandler(&UartHandle);
}
```

### 3. 移除自动发送

**文件**：`main.c`

```c
int main(void)
{
    HAL_Init();
    HAL_InitTick(3);
    APP_SystemClockConfig();
    Uart_Init();
    SPI_CONFIG();
    KEY_CONFIG();
    // printf("E"); <-- V2.0 中已移除
    __HAL_RCC_LSI_ENABLE();
    // ...
}
```

## 🎯 推荐使用方案

### ✅ 推荐：AIR001_BanRGB_V2

**适用场景**：
- 需要自动设备识别
- 多个串口设备环境
- 需要可靠的自动连接
- 新项目开发

**优势**：
- 最新功能和bug修复
- 主动握手机制
- 更好的用户体验
- 完整文档支持

### ⚠️ 不推荐：AIR001_BanRGB (V1.0)

**可能适用场景**：
- 维护旧项目
- 需要兼容旧上位机

**限制**：
- 无主动握手支持
- 连接可靠性较低
- 可能误连其他设备

## 📖 迁移指南

### 从 V1.0 迁移到 V2.0

#### 1. 固件更新

直接使用 V2.0 固件：

```bash
# 打开工程
AIR001_BanRGB_V2/air001_template.uvprojx

# 编译并烧录
Build Target (F7) -> Download (F8)
```

#### 2. 上位机配置

V2.0 上位机自动支持主动握手，无需额外配置。

确保 `config.ini` 配置正确：

```ini
[Handshake]
command = E
timeout = 1.0

[MatchCommand]
command = BANRGB_OK
```

#### 3. 验证更新

1. 启用调试模式：`sac_debug = 1`
2. 启动上位机程序
3. 观察日志输出：
   ```
   [debug] 向 COM3 发送握手命令: 'E'
   [debug] COM3 握手响应: 'BANRGB_OK\n'
   [debug] COM3 握手成功！
   ```

## 🗂️ 目录结构

```
BanRGB/
├── AIR001_BanRGB/          ⚠️ V1.0 旧版（不推荐）
│   ├── User/
│   │   ├── main.c          (开机自动发送 'E')
│   │   └── air001xx_it.c   (无握手响应)
│   └── air001_template.uvprojx
│
└── AIR001_BanRGB_V2/       ✅ V2.0 新版（推荐）
    ├── User/
    │   ├── main.c          (移除自动发送)
    │   └── air001xx_it.c   (支持握手响应)
    ├── air001_template.uvprojx
    └── FIRMWARE_UPDATE_GUIDE.md
```

## ❓ 常见问题

### Q1: 可以同时使用两个版本吗？

**答**：不建议。V1.0 和 V2.0 的握手机制不同，建议统一使用 V2.0。

### Q2: V1.0 固件能连接 V2.0 上位机吗？

**答**：不能。V2.0 上位机使用主动握手机制，需要设备响应 "BANRGB_OK"，而 V1.0 固件不支持此功能。

### Q3: 如何判断当前使用的是哪个版本？

**答**：
- 方法1：检查 `air001xx_it.c` 是否有 `Handle_Handshake()` 函数
- 方法2：使用串口调试工具发送 'E'，如果收到 "BANRGB_OK" 则是 V2.0

### Q4: V1.0 固件还会更新吗？

**答**：不会。所有新功能和bug修复都在 V2.0 中进行。

### Q5: 升级到 V2.0 需要改硬件吗？

**答**：不需要。V2.0 与 V1.0 硬件完全兼容，只需更新固件即可。

## 📚 相关文档

- [V2.0 固件更新指南](AIR001_BanRGB_V2/FIRMWARE_UPDATE_GUIDE.md)
- [主动握手协议文档](out/BanRGB_V2.0/HANDSHAKE_PROTOCOL.md)
- [项目总览](PROJECT_README.md)
- [实现总结](IMPLEMENTATION_SUMMARY.md)

## 📊 版本对比表

| 特性 | V1.0 (旧版) | V2.0 (新版) |
|------|------------|------------|
| 握手机制 | ❌ 无 | ✅ 主动响应 |
| 设备识别 | ❌ 被动等待 | ✅ 智能识别 |
| 自动重连 | ⚠️ 有限支持 | ✅ 完整支持 |
| 多串口支持 | ❌ 无 | ✅ 支持 |
| 连接可靠性 | ⚠️ 中等 | ✅ 高 |
| 文档完整性 | ⚠️ 基础 | ✅ 完善 |
| 推荐使用 | ❌ 不推荐 | ✅ 推荐 |

## 🎉 总结

**强烈推荐使用 AIR001_BanRGB_V2！**

V2.0 版本不仅包含所有 V1.0 的功能，还新增了主动握手机制，大大提升了连接的可靠性和用户体验。如果您还在使用 V1.0，建议尽快升级到 V2.0。

---

**文档版本**: 1.0  
**更新日期**: 2024-01  
**作者**: BanGO
