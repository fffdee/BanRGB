# AIR001 BanRGB 通信协议适配文档

## 📡 通信协议说明

### 下位机（AIR001）协议

#### 1. 上电初始化
```
下位机上电 → 发送 'E' → 上位机识别设备
```

#### 2. LED 颜色控制命令
**格式**: `0xEA [index] [R] [G] [B]`

| 字节 | 说明 | 值 |
|------|------|-----|
| Byte 0 | 命令头 | 0xEA |
| Byte 1 | LED 索引 | 0-63 (根据配置) |
| Byte 2 | 红色分量 | 0-255 |
| Byte 3 | 绿色分量 | 0-255 |
| Byte 4 | 蓝色分量 | 0-255 |

**示例**:
```python
# 设置第0个LED为红色
data = bytes([0xEA, 0, 255, 0, 0])
```

#### 3. LED 总数配置命令
**格式**: `0xEC [count] [checksum] 0xCE`

| 字节 | 说明 | 值 |
|------|------|-----|
| Byte 0 | 命令头 | 0xEC |
| Byte 1 | LED 总数 | 1-100 |
| Byte 2 | 校验值 | 127 + count |
| Byte 3 | 命令尾 | 0xCE |

**示例**:
```python
# 配置61个LED
led_count = 61
checksum = 127 + led_count  # 188
data = bytes([0xEC, led_count, checksum, 0xCE])
```

### 上位机（PyQt6）适配

#### 配置文件更新
**config.ini**:
```ini
[ConnCommand]
command1 =           # 无需初始化命令
command2 = 
baudrate = 115200

[MatchCommand]
command = E          # 上电响应字符

[Heartbeat]
command =            # 无心跳命令
interval = 5000
```

## 🔧 关键代码适配

### 1. LED 颜色发送函数
```python
def set_led_color(led_index, color, auto_connect):
    """发送LED颜色控制命令"""
    red = color[0]
    green = color[1]
    blue = color[2]
    # 协议: 0xEA [index] [R] [G] [B]
    data = bytes([0xEA, led_index, red, green, blue])
    
    if auto_connect.is_connected():
        thread = auto_connect.threads.get(auto_connect.connected_port)
        if thread and thread.serial_port and thread.serial_port.is_open:
            thread.serial_port.write(data)
```

### 2. LED 映射配置发送
```python
def send_led_map_data(self):
    """发送LED映射配置数据"""
    if self.auto_connect.is_connected():
        thread = self.auto_connect.threads.get(self.auto_connect.connected_port)
        if thread and thread.serial_port and thread.serial_port.is_open:
            # 协议: 0xEC [count] [127+count] 0xCE
            send_data = bytes([0xEC, self.led_count, 127 + self.led_count, 0xCE])
            thread.serial_port.write(send_data)
            print(f"Sent LED_MAP data: {send_data}")
```

## 📊 工作模式

### 下位机模式
```c
typedef enum{
    ONLINE = 0,    // 联机模式 - 接收上位机控制
    NORMAL,        // 普通模式 - 固定颜色
    PWM,           // PWM模式 - 呼吸灯
    TRANSFORM,     // 变换模式
} MODE;
```

### 上位机对应
```python
# 仅使用 ONLINE 模式（联机模式）
# 上位机通过屏幕取色或自定义颜色控制LED
```

## 🔄 数据流程

### 屏幕取色模式
```
1. 上位机捕获屏幕
   ↓
2. 缩放到LED网格尺寸
   ↓
3. 为每个LED发送颜色命令
   数据: 0xEA [index] [R] [G] [B]
   ↓
4. 下位机接收并显示
   ↓
5. 循环（50ms刷新）
```

### 自定义颜色模式
```
1. 用户选择颜色
   ↓
2. 发送到所有LED
   for i in range(led_count):
       send: 0xEA [i] [R] [G] [B]
   ↓
3. 下位机接收并显示
```

## ⚙️ 硬件配置

### 下位机 (AIR001)
- **MCU**: AIR001
- **串口**: USART1
- **波特率**: 115200
- **LED驱动**: WS2812B (SPI)
- **最大LED数**: 100

### 引脚定义
```c
// UART
PA2 - USART1_TX
PA3 - USART1_RX

// WS2812B (SPI)
SPI1 - WS2812B 数据线

// 按键
PB6 - 模式切换按键
```

## 🎮 按键功能（下位机本地）

| 按键类型 | 时长 | 功能 |
|---------|------|------|
| 短按 | 20-1000ms | 随机颜色(非联机模式) |
| 长按 | 1000-3000ms | 切换模式 |
| 长长按 | >3000ms | 特殊功能 |

## 🔍 调试信息

### 上位机日志
```
[debug] 尝试连接串口 COM35 (第 1 次)
[debug] 串口 COM35 已打开
[debug] 从 COM35 接收到数据: E
[debug] 在 COM35 上检测到配对成功命令: E
[debug] 已连接到串口: COM35
Sent LED_MAP data: b'\xec=\xbb\xce'
```

### 下位机响应
```c
// 上电发送
printf("E");

// 接收LED颜色命令
if(rx_buffer[i] == 0xEA) {
    uint8_t index = rx_buffer[i+1];
    rgb_t.RGB_SHOW_NOW[index][0] = rx_buffer[i+2]; // R
    rgb_t.RGB_SHOW_NOW[index][1] = rx_buffer[i+3]; // G
    rgb_t.RGB_SHOW_NOW[index][2] = rx_buffer[i+4]; // B
}

// 接收LED总数配置
if(rx_buffer[i] == 0xEC) {
    if(rx_buffer[i+2] == (rx_buffer[i+1]+127) && rx_buffer[i+3] == 0xCE) {
        p_rgb->total = rx_buffer[i+1];
    }
}
```

## 📝 重要注意事项

### 1. LED 索引范围
- 配置的 LED 数量必须匹配实际硬件
- 默认: 61 个 LED
- 最大: 100 个 LED

### 2. 颜色格式
- 顺序: R, G, B (GRB 在 WS2812B 驱动层转换)
- 范围: 0-255

### 3. 特殊处理
```c
// 下位机会自动将倒数第二个LED颜色复制到最后一个LED
if(index == rgb_t.total-2)
    p_rgb->RGB_SHOW_NOW[rgb_t.total-1][count] = 
        p_rgb->RGB_SHOW_NOW[0][count];
```

### 4. 看门狗
- 下位机使用 IWDG 看门狗
- 必须定期刷新，否则系统复位

## 🚀 快速开始

### 1. 硬件连接
```
AIR001 <--> USB转串口 <--> 电脑
```

### 2. 启动流程
```
1. 给AIR001上电
2. 下位机发送 'E'
3. 上位机检测到 'E' 并连接
4. 上位机发送LED配置
5. 开始发送颜色数据
```

### 3. 验证连接
- 观察上位机界面状态变为"已连接"
- LED 灯带开始响应屏幕颜色

## 🔧 故障排除

### 问题1: 未检测到设备
**原因**: 未收到 'E' 字符
**解决**: 
1. 检查串口连接
2. 检查波特率 (115200)
3. 重启下位机

### 问题2: LED 不亮
**原因**: LED 总数未配置
**解决**: 
1. 确保发送了 LED 配置命令
2. 检查 LED 数量设置

### 问题3: 颜色错误
**原因**: 
- LED 映射不匹配
- 颜色格式错误
**解决**:
1. 检查 rgb_config.ini 中的 LED_MAP
2. 确认颜色顺序为 R, G, B

## 📚 相关文件

### 下位机 (AIR001_BanRGB/)
- `User/main.c` - 主程序
- `User/uart/uart.c` - 串口驱动
- `User/ws2812b/ws2812b.c` - WS2812B 驱动
- `User/rgb.h` - RGB 结构定义

### 上位机 (BanRGB_V2.0/)
- `main.py` - 主程序
- `SerialAutoConn.py` - 自动连接
- `config.ini` - 串口配置
- `rgb_config.ini` - RGB 配置

---

**协议版本**: AIR001 BanRGB V2.0  
**最后更新**: 2025-12-11  
**兼容性**: 完全兼容
