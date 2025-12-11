# BanRGB - 智能RGB灯带控制系统 V2.0

![BanRGB](image/BanGO.png)

## 📋 项目简介

BanRGB 是一个完整的 RGB LED 灯带控制系统，包含上位机控制软件（Python + PyQt6）和下位机固件（AIR001微控制器）。V2.0 版本采用机甲赛博朋克风格UI设计，支持屏幕取色、自定义颜色、智能串口识别等功能。

### ✨ 核心特性

- 🎨 **屏幕取色模式**：实时采集屏幕颜色并同步到LED灯带
- 🖌️ **自定义颜色**：RGB色彩选择器，自由调配颜色
- 🔌 **智能串口识别**：主动握手机制，自动识别和连接设备
- 🎮 **机甲风格UI**：赛博朋克设计，紧凑布局，炫酷视觉效果
- 🔄 **自动重连**：连接断开后自动重新扫描和连接
- 📊 **实时状态显示**：LED数量、连接状态、调试信息

## 🗂️ 项目结构

```
BanRGB/
├── out/
│   └── BanRGB_V2.0/           # 上位机程序（V2.0）
│       ├── main.py            # 主程序入口
│       ├── SerialAutoConn.py  # 串口自动连接模块
│       ├── config.ini         # 配置文件
│       ├── rgb_config.ini     # RGB配置
│       ├── README_V2.md       # 使用说明
│       ├── HANDSHAKE_PROTOCOL.md  # 握手协议文档
│       ├── PROTOCOL_AIR001.md     # AIR001协议文档
│       ├── UI_DESIGN.md       # UI设计文档
│       ├── QUICKSTART.md      # 快速开始指南
│       └── CHANGELOG.md       # 更新日志
│
├── AIR001_BanRGB_V2/          # 下位机固件（V2）
│   ├── User/
│   │   ├── main.c             # 主程序
│   │   ├── uart/              # 串口驱动
│   │   ├── ws2812b/           # WS2812B驱动
│   │   └── air001xx_it.c      # 中断处理（含握手响应）
│   ├── air001_template.uvprojx  # Keil工程文件
│   └── FIRMWARE_UPDATE_GUIDE.md # 固件更新指南
│
├── arduino-BanRGB/            # Arduino版本（早期版本）
├── config/                    # 配置文件模板
├── image/                     # 图片资源
├── model/                     # 3D模型文件
└── tool/                      # 工具脚本
```

## 🚀 快速开始

### 上位机使用

#### 1. 环境准备

**系统要求**：
- Windows 7/8/10/11
- Python 3.8+

**安装依赖**：
```bash
pip install PyQt6 pyserial pillow mss numpy
```

#### 2. 运行程序

```bash
cd out/BanRGB_V2.0
python main.py
```

#### 3. 配置说明

编辑 `config.ini`：

```ini
[Settings]
sac_debug = 1              # 启用调试模式

[Handshake]
command = E                # 握手命令
timeout = 1.0              # 握手超时（秒）

[MatchCommand]
command = BANRGB_OK        # 期望的握手响应

[Heartbeat]
command =                  # 心跳命令（可选）
interval = 5000            # 心跳间隔（毫秒）
```

编辑 `rgb_config.ini`：

```ini
[LED]
led_count = 64             # LED灯珠数量
```

### 下位机固件

#### 1. 硬件准备

- **微控制器**：AIR001 开发板
- **LED灯带**：WS2812B 或兼容型号
- **调试器**：ST-Link 或兼容调试器
- **开发环境**：Keil MDK-ARM V5

#### 2. 烧录固件

1. 打开 Keil 工程：`AIR001_BanRGB_V2/air001_template.uvprojx`
2. 编译工程：`Project` -> `Build Target` (F7)
3. 连接调试器
4. 烧录固件：`Flash` -> `Download` (F8)

详细步骤请参考：[固件更新指南](AIR001_BanRGB_V2/FIRMWARE_UPDATE_GUIDE.md)

#### 3. 硬件连接

```
AIR001 开发板
├── PA2 (UART TX) -> USB转TTL RX
├── PA3 (UART RX) -> USB转TTL TX
├── SPI1 (MOSI)   -> WS2812B DIN
├── GND           -> 公共地
└── 5V            -> LED灯带电源
```

## 🔄 主动握手协议

V2.0 版本采用**主动握手机制**实现智能设备识别：

### 工作流程

1. **上位机扫描**：程序启动后自动扫描所有串口
2. **发送握手**：向每个串口发送握手命令 `'E'`
3. **设备响应**：AIR001 检测到 `'E'` 后回复 `"BANRGB_OK\n"`
4. **验证连接**：上位机验证响应，只连接正确的设备

### 优势

- ✅ 自动识别BanRGB设备
- ✅ 避免误连其他串口设备
- ✅ 支持多串口并发测试
- ✅ 连接失败自动重试

详细说明请参考：[握手协议文档](out/BanRGB_V2.0/HANDSHAKE_PROTOCOL.md)

## 📖 文档索引

### 上位机文档
- [README_V2.md](out/BanRGB_V2.0/README_V2.md) - 完整使用说明
- [HANDSHAKE_PROTOCOL.md](out/BanRGB_V2.0/HANDSHAKE_PROTOCOL.md) - 握手协议详解
- [PROTOCOL_AIR001.md](out/BanRGB_V2.0/PROTOCOL_AIR001.md) - 通信协议文档
- [UI_DESIGN.md](out/BanRGB_V2.0/UI_DESIGN.md) - UI设计说明
- [QUICKSTART.md](out/BanRGB_V2.0/QUICKSTART.md) - 快速开始指南
- [CHANGELOG.md](out/BanRGB_V2.0/CHANGELOG.md) - 更新日志

### 下位机文档
- [FIRMWARE_UPDATE_GUIDE.md](AIR001_BanRGB_V2/FIRMWARE_UPDATE_GUIDE.md) - 固件更新指南

## 🎨 UI 预览

### 主界面（机甲风格）

- 紧凑式布局，节省屏幕空间
- 赛博朋克配色方案（#00FF41, #FF2D55, #0A0E27）
- 半透明效果，科技感十足
- 自动吸边功能

### 功能区域

1. **状态显示**
   - LED数量指示器（圆形显示）
   - 连接状态（实时更新）
   - 当前模式

2. **颜色控制**
   - RGB滑动条（0-255）
   - 实时颜色预览
   - 十六进制代码显示

3. **模式切换**
   - 屏幕取色模式
   - 自定义颜色模式

4. **调试控制台**（可选）
   - 实时日志输出
   - 串口通信状态

## 🛠️ 开发说明

### 技术栈

**上位机**：
- Python 3.8+
- PyQt6（GUI框架）
- PySerial（串口通信）
- Pillow + MSS（屏幕截图）
- NumPy（数据处理）

**下位机**：
- C语言
- STM32 HAL库
- Keil MDK-ARM
- WS2812B驱动（SPI协议）

### 代码结构

#### 上位机

```python
# main.py - 主程序
class BanRGBApp(QMainWindow):
    - 初始化UI
    - 设置样式表
    - 颜色更新线程
    - 串口连接管理

# SerialAutoConn.py - 串口模块
class SerialThread(QThread):
    - 串口通信
    - 主动握手
    - 错误处理

class SerialPortAutoConnect(QObject):
    - 串口扫描
    - 自动连接
    - 重连管理
```

#### 下位机

```c
// main.c - 主程序
- 系统初始化
- LED模式切换
- 按键处理
- 主循环

// uart.c - 串口驱动
- 串口初始化
- 数据发送接收

// air001xx_it.c - 中断处理
- USART1中断
- 握手命令检测
- 握手响应发送

// ws2812b.c - LED驱动
- WS2812B协议
- SPI+DMA传输
- 颜色数据发送
```

## 🐛 调试指南

### 启用调试模式

在 `config.ini` 中设置：
```ini
[Settings]
sac_debug = 1
```

### 常见问题

#### 1. 无法连接设备
- ✅ 检查USB连接
- ✅ 确认驱动已安装（CH340/CP2102）
- ✅ 检查串口是否被其他程序占用
- ✅ 验证波特率（115200）

#### 2. 握手超时
- ✅ 确认固件已正确烧录
- ✅ 检查下位机响应函数
- ✅ 使用串口调试工具测试

#### 3. LED不亮
- ✅ 检查电源供电
- ✅ 确认WS2812B连接正确
- ✅ 验证LED数量配置

#### 4. 颜色不准确
- ✅ 检查RGB值范围（0-255）
- ✅ 调整屏幕采样区域
- ✅ 验证WS2812B颜色顺序（GRB）

## 📝 版本历史

### V2.0.0 (2024-01)
- ✨ 全新机甲赛博朋克风格UI
- ✨ 实现主动握手机制
- ✨ 上位机自动串口扫描
- ✨ 下位机握手响应功能
- 🐛 修复PyQt6兼容性问题
- 🐛 修复线程退出警告
- 📝 完善文档体系

### V1.0.0 (2023)
- ✨ 基础RGB控制功能
- ✨ 屏幕取色模式
- ✨ 自定义颜色模式
- ✨ 串口自动连接

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

### 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 LICENSE 文件

## 👨‍💻 作者

**BanGO**

## 🙏 致谢

- AIR001 社区
- PyQt6 开发团队
- WS2812B 驱动参考

---

**最后更新**: 2024-01  
**版本**: V2.0.0  
**状态**: 活跃开发中 🚀
