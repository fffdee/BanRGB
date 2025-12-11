# 🚀 快速开始指南

## 安装依赖（仅首次）

```bash
pip install PyQt6 pyserial Pillow
```

## 启动程序

```bash
python main.py
```

## 快捷键

- **Ctrl+C** - 终止程序（在终端中）
- **Alt+F4** - 关闭窗口

## 配置文件

### config.ini - 串口配置
```ini
[ConnCommand]
command1 = F00E01        # 初始化命令
command2 = F00C00        # 心跳命令

[MatchCommand]
command = F10C           # 配对识别

[Heartbeat]
interval = 5000          # 心跳间隔(毫秒)
```

### rgb_config.ini - RGB 配置
```ini
[DEFAULT]
ledcount = 61            # LED 数量
debug = 0                # 0=关闭 1=开启
mode = Screen Color      # 工作模式
refreshmode = 0          # 刷新模式
```

## 故障排除

### 无法连接设备
1. 检查 USB 连接
2. 确认驱动已安装
3. 查看 config.ini 命令是否正确

### LED 颜色错误
1. 检查 LED 数量设置
2. 尝试切换刷新模式
3. 启用 Debug 模式查看数据

### 程序崩溃
```bash
# 查看详细错误
python main.py
```

## 日志位置

程序输出在终端/控制台中
- `[debug]` 开头：调试信息
- 错误信息会直接显示

## 联系支持

- 查看 `README_V2.md` 详细文档
- 查看 `CHANGELOG.md` 更新日志
- 查看 `UI_DESIGN.md` 界面说明
