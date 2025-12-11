# BanRGB V2.0 - 自动连接增强版

## 🎉 版本说明

本版本在 V1.0 的基础上集成了 V2.0 的**自动连接功能**，实现了最佳的用户体验！

## ✨ 主要特性

### 🔌 自动连接功能（来自 V2.0）
- ✅ 自动扫描并连接 RGB 设备
- ✅ 智能心跳检测
- ✅ 断线自动重连
- ✅ 完善的错误处理
- ✅ 环形缓冲区数据管理

### 🎨 RGB 控制功能（来自 V1.0）
- ✅ 屏幕取色模式 - 实时捕获屏幕颜色映射到 LED
- ✅ 自定义颜色模式 - 手动选择固定颜色
- ✅ 支持两种刷新模式（Mode 0 / Mode 1）
- ✅ Debug 模式 - LED 网格可视化 + 数据监控
- ✅ 灵活的 LED 映射配置

## 📁 文件说明

```
BanRGB_V2.0/
├── main.py              # 主程序（整合版）
├── SerialAutoConn.py    # 自动连接模块
├── data_commands.py     # 数据命令和图标
├── config.ini           # 串口自动连接配置
├── rgb_config.ini       # RGB 控制配置
└── README_V2.md         # 本说明文档
```

## ⚙️ 配置文件

### config.ini - 串口自动连接配置
```ini
[Settings]
sac_debug = 1            # 串口调试模式 (0=关闭, 1=开启)

[ConnCommand]
command1 = F00E01        # 连接命令1
command2 = F00C00        # 连接命令2
baudrate = 115200        # 波特率

[MatchCommand]
command = F10C           # 配对成功识别命令

[Heartbeat]
command = F00C00         # 心跳命令
interval = 5000          # 心跳间隔(毫秒)
```

### rgb_config.ini - RGB 控制配置
```ini
[DEFAULT]
mode = Screen Color      # 工作模式
customcolorred = 255     # 自定义颜色-红
customcolorgreen = 255   # 自定义颜色-绿
customcolorblue = 255    # 自定义颜色-蓝
ledcount = 61            # LED 总数
debug = 0                # Debug 模式 (0=关闭, 1=开启)
refreshmode = 0          # 刷新模式 (0/1)

[LED_MAP0]               # 刷新模式0的LED映射
rows = 6
cols = 10
data = 30,30,29,28,...   # LED 物理排列映射

[LED_MAP1]               # 刷新模式1的LED映射
rows = 12
cols = 20
data = 0,1,2,3,...
```

## 🚀 使用方法

### 1. 安装依赖
```bash
pip install PyQt5 pyserial Pillow
```

### 2. 运行程序
```bash
python main.py
```

### 3. 使用流程
1. **启动程序** - 自动扫描并连接设备
2. **等待连接** - 状态栏显示"已连接 COMx"（绿色）
3. **选择模式**:
   - **Screen Color**: 实时屏幕取色
   - **Custom**: 选择固定颜色
4. **切换刷新模式** - 根据硬件布局选择 Mode 0 或 Mode 1

## 🐛 Debug 模式

启用 Debug 模式（rgb_config.ini 中设置 `debug = 1`）后：
- 显示 LED 网格可视化
- 显示接收数据监控
- 支持手动发送命令
- 支持 HEX 格式数据发送

## 🔧 通信协议

### LED 颜色控制
```
命令格式: [0xEA, led_index, red, green, blue]
- 0xEA: 命令头
- led_index: LED 索引 (0-60)
- red, green, blue: RGB 值 (0-255)
```

### LED 映射配置
```
命令格式: [0xEC, led_count, checksum, 0xCE]
- 0xEC: 配置命令头
- led_count: LED 总数
- checksum: 127 + led_count
- 0xCE: 命令尾
```

## 📊 工作模式对比

| 模式 | 说明 | 应用场景 |
|------|------|----------|
| **Screen Color** | 实时捕获屏幕内容并映射到 LED | 游戏氛围灯、视频环境光 |
| **Custom** | 手动选择固定颜色 | 单色照明、氛围灯 |

## 🆚 版本对比

| 功能 | V1.0 | V2.0（本版本） |
|------|------|----------------|
| RGB 控制 | ✅ | ✅ |
| 屏幕取色 | ✅ | ✅ |
| 手动连接 | ✅ | ❌ |
| **自动连接** | ❌ | ✅ |
| **心跳检测** | 简单 | 完善 |
| **自动重连** | ❌ | ✅ |
| **错误处理** | 基础 | 完善 |

## ⚠️ 注意事项

1. **端口占用**: 确保设备未被其他程序占用
2. **权限问题**: Windows 下可能需要管理员权限
3. **驱动安装**: 确保设备驱动已正确安装
4. **配置匹配**: 确保 LED 数量和映射配置与硬件一致

## 🔍 故障排除

### 无法连接设备
- 检查设备是否正确连接
- 检查驱动是否安装
- 查看 config.ini 中的连接命令是否正确

### LED 颜色不正确
- 检查 rgb_config.ini 中的 LED 映射数据
- 尝试切换刷新模式
- 启用 Debug 模式查看数据传输

### 程序崩溃
- 检查 Python 依赖是否完整安装
- 查看控制台错误信息
- 启用 sac_debug 查看详细日志

## 📝 更新日志

### V2.0 (2025-12-11)
- ✅ 集成自动连接功能
- ✅ 保留完整 RGB 控制功能
- ✅ 优化界面布局
- ✅ 改进错误处理
- ✅ 添加详细文档

## 👨‍💻 开发者

- 基于 V1.0 RGB 控制系统
- 集成 V2.0 自动连接功能
- 优化整合版本

## 📄 License

本项目仅供学习和个人使用。
