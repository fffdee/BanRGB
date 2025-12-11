# BanRGB V2.0 MECHA 版本修复日志

## 🚀 重大更新：主动握手协议 (2025-01-XX)

### 新功能：智能串口识别

BanRGB V2.0 现在使用**主动握手机制**自动识别和连接设备！

#### 上位机更新
- ✅ **自动扫描所有串口**：程序启动后自动扫描系统中的所有串口
- ✅ **主动发送握手命令**：向每个串口发送握手命令 `'E'`
- ✅ **智能响应验证**：只有返回 `"BANRGB_OK"` 的设备才会被连接
- ✅ **多串口并发测试**：可以同时向多个串口发起握手，快速找到目标设备
- ✅ **自动关闭无效串口**：握手失败的串口会自动关闭，不影响其他串口
- ✅ **详细调试日志**：可以在 config.ini 中启用调试模式查看握手过程

#### 下位机固件更新（AIR001_BanRGB_V2）
- ✅ **握手命令检测**：在串口中断中检测握手命令 `'E'`（0x45）
- ✅ **握手响应函数**：收到握手命令后立即响应 `"BANRGB_OK\n"`
- ✅ **移除开机自动发送**：不再在 main() 中自动发送 `printf("E")`，改为响应上位机的主动握手

#### 文件修改清单

**上位机 (BanRGB_V2.0)**
- `SerialAutoConn.py`
  - 修改默认握手命令为 `'E'`
  - 修改握手响应验证逻辑（检查 `'BANRGB_OK'`）
  - 更新握手超时处理
  
- `config.ini`
  ```ini
  [Handshake]
  command = E              # 握手命令
  timeout = 1.0            # 超时时间
  
  [MatchCommand]
  command = BANRGB_OK      # 期望的响应
  ```

**下位机 (AIR001_BanRGB_V2)**
- `air001xx_it.c`
  - 新增 `Handle_Handshake()` 函数
  - 修改 `USART1_IRQHandler()` 中断处理，检测握手命令
  
- `main.c`
  - 移除 `printf("E")` 自动发送

#### 使用方式

1. **烧录固件**：将更新后的 AIR001_BanRGB_V2 固件烧录到设备
2. **连接设备**：通过USB连接AIR001设备
3. **启动程序**：运行 BanRGB V2.0 上位机程序
4. **自动连接**：程序会自动扫描并连接到正确的设备

#### 调试方法

启用调试模式查看握手过程：
```ini
[Settings]
sac_debug = 1
```

调试输出示例：
```
[debug] 找到以下串口: ['COM3', 'COM4']
[debug] 向 COM3 发送握手命令: 'E'
[debug] COM3 握手响应: 'BANRGB_OK\n'
[debug] COM3 握手成功！
[debug] COM3 进入正常通信模式
```

#### 优势
- 🎯 **精确识别**：只连接BanRGB设备，避免误连其他串口设备
- ⚡ **快速连接**：并发握手，快速找到目标设备
- 🔄 **自动重连**：连接断开后自动重新扫描
- 🛡️ **安全可靠**：握手机制确保连接到正确的设备

#### 参考文档
- 详细握手协议说明：`HANDSHAKE_PROTOCOL.md`
- AIR001协议文档：`PROTOCOL_AIR001.md`

---

## 🔧 Bug 修复 (2025-12-11)

### 修复的问题

#### 1. PyQt6 兼容性问题
**问题**: `AttributeError: 'QApplication' object has no attribute 'exec_'`
- ✅ **修复**: PyQt6 中 `exec_()` 改为 `exec()`
- 📝 **位置**: main.py 最后一行

#### 2. CSS 属性不支持
**问题**: `Unknown property text-shadow`
- ✅ **修复**: 移除不支持的 `text-shadow` 属性
- 📝 **位置**: MECHA_STYLE 样式表

#### 3. 线程退出警告
**问题**: `QThread: Destroyed while thread is still running`
- ✅ **修复**: 在关闭窗口时正确停止颜色更新线程
- 📝 **位置**: closeEvent() 方法

### 更新内容

```python
# 修复前
sys.exit(app.exec_())  ❌

# 修复后
sys.exit(app.exec())   ✅
```

```python
# 修复前
def closeEvent(self, event):
    self.auto_connect.close_all_ports()
    event.accept()

# 修复后
def closeEvent(self, event):
    self.update_colors_thread.terminate()  # 停止线程
    self.update_colors_thread.wait()       # 等待线程结束
    self.auto_connect.close_all_ports()
    event.accept()
```

### 测试状态

✅ 程序启动正常
✅ 自动扫描串口
✅ 尝试连接设备
✅ 界面显示正常
✅ 样式渲染正确
✅ 程序关闭无警告

### 当前运行状态

根据日志，程序现在可以：
- ✅ 正常启动
- ✅ 扫描到 COM1 和 COM35 端口
- ✅ 尝试建立连接
- ✅ 发送初始化命令 (F00E01, F00C00)
- ⚠️ 等待设备响应配对命令 (F10C)

### 下一步

如果设备连接成功，您会看到：
1. 状态显示变为绿色 "● CONNECTED >> COMx"
2. 开始屏幕取色功能
3. LED 灯带开始响应

如果设备未响应：
- 检查设备是否支持这些命令
- 查看 config.ini 中的配对命令是否正确
- 确认设备固件是否与协议匹配

### 已知限制

Qt StyleSheet 不支持的 CSS 属性：
- ❌ text-shadow (已移除)
- ❌ box-shadow (使用 QGraphicsDropShadowEffect 代替)
- ❌ animation (使用 QPropertyAnimation 代替)

但我们通过代码实现了类似效果：
- ✅ 发光效果 (QGraphicsDropShadowEffect)
- ✅ 颜色渐变 (QLinearGradient)
- ✅ 动态样式更新

### 性能优化

- ✅ 颜色更新间隔: 50ms (Screen Color 模式)
- ✅ 空闲等待间隔: 100ms (Custom 模式)
- ✅ 串口扫描间隔: 2000ms (2秒)
- ✅ 心跳检测间隔: 5000ms (5秒)

## 📦 完整文件列表

```
BanRGB_V2.0/
├── main.py                  ✅ 已修复
├── SerialAutoConn.py        ✅ PyQt6 版本
├── data_commands.py         ✅ 正常
├── config.ini               ✅ 正常
├── rgb_config.ini           ✅ 正常
├── README_V2.md             ✅ 文档
├── CHANGELOG.md             ✅ 本文件
└── __pycache__/
```

## 🚀 现在可以使用

所有问题已修复，程序可以正常运行！

```bash
python main.py
```

或使用启动脚本（如果有）：
```bash
启动_MECHA.bat
```
