# BanRGB V2.0 MECHA EDITION - 安装指南

## 🤖 机甲风格特性

- ✅ 酷炫的机甲科幻UI设计
- ✅ 霓虹绿色主题 (#00ff41)
- ✅ 发光阴影效果
- ✅ 动态状态指示器
- ✅ 赛博朋克风格面板
- ✅ PyQt6 现代化框架

## 📦 安装依赖

### 1. 卸载旧版本 PyQt5（如果已安装）
```bash
pip uninstall PyQt5 -y
```

### 2. 安装 PyQt6 和其他依赖
```bash
pip install PyQt6 pyserial Pillow
```

### 3. 验证安装
```bash
python -c "from PyQt6.QtWidgets import QApplication; print('PyQt6 安装成功!')"
```

## 🚀 运行程序

```bash
cd c:\Users\BanGO\Desktop\BanRGB\out\BanRGB_V2.0
python main.py
```

## 🎨 UI 特性说明

### 配色方案
- **主色调**: 霓虹绿 (#00ff41) - 机甲能量色
- **高亮色**: 青色 (#00ffff) - 系统提示色
- **警告色**: 红色 (#ff0040) - 断开/错误提示
- **背景色**: 深蓝渐变 (#0a0e27 → #16213e)

### 视觉效果
- **发光阴影**: 连接状态会发出霓虹光效
- **渐变背景**: 多层次渐变增强科技感
- **边框高亮**: 鼠标悬停时边框颜色变化
- **字体**: 等宽字体模拟终端界面

### 状态指示器
| 状态 | 颜色 | 说明 |
|------|------|------|
| ● DETECTING... | 橙色 | 正在扫描设备 |
| ● CONNECTED >> COMx | 绿色发光 | 已成功连接 |
| ● DISCONNECTED | 红色发光 | 连接已断开 |

## 🎮 界面布局

```
┌────────────────────────────────────┐
│   ◢ RGB CONTROL SYSTEM ◣          │
├────────────────────────────────────┤
│  ┌──────────────────────────────┐  │
│  │   [ SYSTEM STATUS ]          │  │
│  │   ● CONNECTED >> COM3        │  │  ← 发光状态
│  └──────────────────────────────┘  │
├────────────────────────────────────┤
│  ┌──────────────────────────────┐  │
│  │   [ CONTROL PANEL ]          │  │
│  │   [ MODE ] [Screen Color ▼]  │  │
│  │   [ REFRESH ] [Mode 0 ▼]     │  │
│  │   [ SELECT COLOR ]           │  │  ← 自定义模式
│  └──────────────────────────────┘  │
├────────────────────────────────────┤
│   [ LED COUNT: 61 ]                │
├────────────────────────────────────┤
│  ┌──────────────────────────────┐  │
│  │   [ DEBUG CONSOLE ]          │  │  ← Debug模式
│  │   LED Grid Display           │  │
│  │   RX/TX Data Monitor         │  │
│  └──────────────────────────────┘  │
├────────────────────────────────────┤
│   ◢ V2.0 MECHA EDITION ◣          │
└────────────────────────────────────┘
```

## 🔧 配置文件

### rgb_config.ini
```ini
[DEFAULT]
mode = Screen Color
debug = 1              # 设置为 1 启用机甲 DEBUG 界面
ledcount = 61
refreshmode = 0
```

## ⚙️ PyQt5 → PyQt6 主要变化

### 导入变化
```python
# PyQt5
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication

# PyQt6
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QApplication
```

### 枚举变化
```python
# PyQt5
Qt.AlignCenter

# PyQt6
Qt.AlignmentFlag.AlignCenter
```

## 🐛 故障排除

### ImportError: No module named 'PyQt6'
```bash
pip install PyQt6 --upgrade
```

### 样式不显示
- 检查是否正确应用了 `MECHA_STYLE`
- 确保所有 QFrame 的 objectName 设置正确

### 字体显示异常
- 系统需要支持 'Consolas' 或 'Courier New' 等宽字体
- Windows 系统默认包含这些字体

### 发光效果不显示
- 确保 `QGraphicsDropShadowEffect` 正常工作
- 某些旧显卡可能不支持图形效果

## 📝 自定义样式

可以在代码中修改 `MECHA_STYLE` 变量来自定义配色：

```python
# 修改主色调（当前：霓虹绿）
# 搜索 #00ff41 并替换为你喜欢的颜色
# 例如：#ff00ff (洋红), #00ffff (青色), #ffff00 (黄色)
```

## 🎯 推荐配色方案

### 赛博朋克粉
```python
主色: #ff00ff (洋红)
高亮: #ff88ff (浅粉)
背景: #1a001a → #330033
```

### 冰蓝机甲
```python
主色: #00ccff (冰蓝)
高亮: #66ddff (浅蓝)
背景: #001a2e → #002040
```

### 暗黑红警
```python
主色: #ff3333 (亮红)
高亮: #ff6666 (橙红)
背景: #2e0a0a → #401010
```

## 💡 性能优化

- 屏幕取色模式下，刷新率约 20 FPS
- Debug 模式会增加 CPU 使用率
- 建议在不需要时关闭 Debug 模式

## 🌟 特殊效果

### 连接动画
连接成功时，状态栏会有：
- 颜色渐变切换
- 霓虹发光效果
- 阴影扩散动画

### 悬停效果
鼠标悬停在控件上时：
- 边框颜色变为青色
- 背景亮度增加
- 光标变为手型（按钮）

## 📞 技术支持

遇到问题？
1. 检查 Python 版本 ≥ 3.8
2. 确认所有依赖已安装
3. 查看终端错误信息
4. 启用 Debug 模式查看数据流

---

**◢ ENJOY YOUR MECHA RGB EXPERIENCE ◣**
