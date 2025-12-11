# BanRGB V2.0 主动握手机制说明

## 概述

BanRGB V2.0 上位机现已升级为**主动握手**连接机制，不再被动等待下位机发送数据，而是主动向所有可用串口发起握手，只有收到正确回复的设备才会建立连接。

---

## 主动握手流程

### 1. 扫描串口
上位机启动后自动扫描系统所有可用串口（COM1, COM2, COM3...）

### 2. 并行握手
向所有串口并行发送握手命令：
```
BANRGB?\r\n
```

### 3. 等待响应
每个串口等待 **1秒** 的握手响应时间

### 4. 识别目标设备
只有收到包含 **'E'** 字符的响应才认为是目标设备：
- ✓ 响应包含 'E' → 握手成功，建立连接
- ✗ 无响应或响应不匹配 → 关闭该串口，继续扫描其他端口

### 5. 建立连接
- 关闭所有未匹配的串口线程
- 与目标设备建立唯一连接
- 发送初始化命令（如果配置了）
- 启动心跳检测（如果配置了）

---

## 配置说明

### config.ini 配置文件

```ini
[Settings]
sac_debug = 1  # 调试模式：1=开启，0=关闭

[ConnCommand]
command1 =     # 连接后的初始化命令1（可选）
command2 =     # 连接后的初始化命令2（可选）
baudrate = 115200  # 波特率

[Handshake]
# 主动握手命令，上位机向所有串口发送此命令
command = BANRGB?\r\n
# 握手超时时间（秒）
timeout = 1.0

[MatchCommand]
# 握手响应匹配标识，只有收到包含此字符的响应才建立连接
command = E

[Heartbeat]
command =      # 心跳命令（可选，留空则不启用心跳）
interval = 5000  # 心跳间隔（毫秒）
```

---

## 下位机适配要求

### 方案1：响应握手命令（推荐）

下位机需要监听串口数据，当收到 `BANRGB?\r\n` 时，立即回复：

```c
// 伪代码示例
void UART_RxCallback() {
    if (strstr(uart_buffer, "BANRGB?")) {
        // 回复握手响应
        HAL_UART_Transmit(&huart1, (uint8_t*)"E", 1, 100);
        // 或者更详细的响应
        HAL_UART_Transmit(&huart1, (uint8_t*)"BANRGB_OK\r\n", 11, 100);
    }
}
```

### 方案2：上电主动发送'E'（兼容现有代码）

如果下位机不响应握手命令，也可以在上电时主动发送 'E'：

```c
// main函数中
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    // 上电后发送标识
    HAL_UART_Transmit(&huart1, (uint8_t*)"E", 1, 100);
    
    while(1) {
        // 主循环
    }
}
```

**注意**：方案2需要修改配置，将握手命令留空：
```ini
[Handshake]
command =  # 留空，等待设备主动发送
timeout = 3.0  # 增加超时时间
```

---

## 优势对比

### 旧版（被动等待）
- ❌ 需要下位机先上电，再开上位机
- ❌ 操作顺序要求严格
- ❌ 可能误连接其他设备
- ❌ 连接速度慢

### 新版（主动握手）
- ✅ 无需关心启动顺序
- ✅ 自动识别目标设备
- ✅ 避免误连接
- ✅ 并行扫描，速度快
- ✅ 支持热插拔重连

---

## 调试工具

### 使用测试工具验证握手

运行测试脚本：
```bash
python test_handshake.py
```

测试工具会显示：
- 扫描到的所有串口
- 每个串口的握手尝试
- 握手响应内容
- 连接成功/失败状态

---

## 故障排查

### 问题1：扫描不到串口
**原因**：
- 设备未连接
- 驱动未安装
- 端口被其他程序占用

**解决**：
- 检查设备管理器
- 安装CH340/CP2102驱动
- 关闭占用端口的程序（如Arduino IDE、串口助手）

### 问题2：握手超时
**原因**：
- 下位机未响应握手命令
- 波特率不匹配
- 串口缓冲区未清空

**解决**：
- 确认下位机已实现握手响应
- 检查config.ini中的波特率设置
- 重启上位机和下位机

### 问题3：连接后立即断开
**原因**：
- 心跳命令配置错误
- 下位机未响应心跳

**解决**：
- 暂时关闭心跳（command留空）
- 检查下位机心跳响应逻辑

### 问题4：误连接其他设备
**原因**：
- 其他设备也回复了包含'E'的数据

**解决**：
- 修改握手命令为更独特的字符串（如 "BANRGB_V2?"）
- 修改匹配标识为更长的字符串（如 "BANRGB_OK"）

---

## 技术细节

### 线程管理
- 每个串口一个独立线程
- 握手失败自动关闭线程
- 连接成功后关闭其他线程
- 避免资源泄漏

### 缓冲区设计
- 环形缓冲区（1024字节）
- 自动覆盖最旧数据
- 线程安全

### 错误处理
- 端口不存在（ERROR_PORT_NOT_FOUND）
- 端口被占用（ERROR_PORT_BUSY）
- 权限不足（ERROR_PERMISSION）
- 握手超时（ERROR_HANDSHAKE_TIMEOUT）
- 握手失败（ERROR_HANDSHAKE_FAILED）

---

## 性能参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 握手超时 | 1.0s | 可配置 |
| 重连间隔 | 5.0s | 可配置 |
| 心跳间隔 | 5.0s | 可配置 |
| 波特率 | 115200 | 可配置 |
| 缓冲区大小 | 1024字节 | 固定 |

---

## 示例代码

### Python 上位机调用

```python
from SerialAutoConn import SerialPortAutoConnect

# 创建自动连接实例
serial = SerialPortAutoConnect('config.ini')

# 连接信号
serial.connected.connect(lambda port: print(f"已连接: {port}"))
serial.disconnected.connect(lambda port: print(f"已断开: {port}"))
serial.data_available.connect(lambda data: print(f"收到数据: {data}"))

# 启动自动连接
serial.auto_connect(retry_mode=True, retry_interval=5000)

# 发送数据
if serial.is_connected():
    serial.send_data("F00E01")  # 示例：LED控制命令
```

### C 下位机响应

```c
// 串口接收回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 检查握手命令
        if (strncmp((char*)rx_buffer, "BANRGB?", 7) == 0) {
            // 回复握手响应
            uint8_t response[] = "E";
            HAL_UART_Transmit(&huart1, response, 1, 100);
        }
        
        // 继续接收
        HAL_UART_Receive_IT(&huart1, rx_buffer, RX_BUFFER_SIZE);
    }
}
```

---

## 更新日志

### V2.0.1 (2024-01-XX)
- ✅ 实现主动握手机制
- ✅ 支持并行串口扫描
- ✅ 添加握手超时检测
- ✅ 优化线程管理
- ✅ 添加调试测试工具
- ✅ 更新配置文件格式
- ✅ 完善错误处理

---

## 联系方式

如有问题或建议，请在项目 Issue 中反馈。

---

**BanRGB Team**  
*让RGB更简单*
