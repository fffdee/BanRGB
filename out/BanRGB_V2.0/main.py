import os
import sys
from SerialAutoConn import SerialPortAutoConnect
from PyQt6.QtWidgets import (QApplication, QMainWindow, QLabel, QTextEdit, 
                             QHBoxLayout, QVBoxLayout, QWidget, QMessageBox, 
                             QLineEdit, QPushButton, QGridLayout, QComboBox, 
                             QColorDialog, QCheckBox, QFrame, QGraphicsDropShadowEffect)
from PyQt6.QtCore import QTimer, Qt, QThread, pyqtSignal, QPropertyAnimation, QEasingCurve, QSize
from PyQt6.QtGui import QColor, QFont, QPixmap, QIcon, QPalette, QPainter, QLinearGradient
import data_commands
import base64
import configparser
from PIL import ImageGrab
from PIL import Image

# 机甲风格样式表
MECHA_STYLE = """
QMainWindow {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #0a0e27, stop:1 #16213e);
}

QWidget {
    background-color: transparent;
    color: #00ff41;
    font-family: 'Consolas', 'Courier New', monospace;
}

QLabel {
    color: #00ff41;
    font-weight: bold;
    padding: 5px;
}

QLabel#titleLabel {
    color: #00ffff;
    font-size: 24px;
    font-weight: bold;
    letter-spacing: 3px;
}

QLabel#statusLabel {
    border: 2px solid #00ff41;
    border-radius: 6px;
    padding: 6px 12px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #1a1a2e, stop:1 #16213e);
    font-size: 12px;
    font-weight: bold;
}

QLabel#connectedStatus {
    border: 2px solid #00ff41;
    border-radius: 6px;
    padding: 6px 12px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #00ff41, stop:1 #00cc33);
    color: #000000;
    font-weight: bold;
    font-size: 12px;
}

QLabel#disconnectedStatus {
    border: 2px solid #ff0040;
    border-radius: 6px;
    padding: 6px 12px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff0040, stop:1 #cc0033);
    color: #ffffff;
    font-weight: bold;
    font-size: 12px;
}

QLabel#detectingStatus {
    border: 2px solid #ffaa00;
    border-radius: 6px;
    padding: 6px 12px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ffaa00, stop:1 #ff8800);
    color: #000000;
    font-weight: bold;
    font-size: 12px;
}

QComboBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #1e3a5f, stop:1 #0f1b2e);
    border: 2px solid #00ff41;
    border-radius: 4px;
    padding: 5px 8px;
    color: #00ff41;
    font-size: 11px;
    font-weight: bold;
    min-width: 100px;
    max-height: 28px;
}

QComboBox:hover {
    border: 2px solid #00ffff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2a4a6f, stop:1 #1a2a3e);
}

QComboBox::drop-down {
    border: none;
    background: #00ff41;
    width: 30px;
    border-radius: 4px;
}

QComboBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 8px solid #000000;
    margin-right: 8px;
}

QComboBox QAbstractItemView {
    background: #1a1a2e;
    border: 2px solid #00ff41;
    selection-background-color: #00ff41;
    selection-color: #000000;
    color: #00ff41;
}

QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00ff41, stop:1 #00cc33);
    border: 2px solid #00ff41;
    border-radius: 4px;
    padding: 5px 12px;
    color: #000000;
    font-weight: bold;
    font-size: 11px;
    min-width: 80px;
    max-height: 28px;
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00ffff, stop:1 #00cccc);
    border: 2px solid #00ffff;
}

QPushButton:pressed {
    background: #00cc33;
    border: 2px solid #009922;
}

QPushButton:disabled {
    background: #333333;
    border: 2px solid #555555;
    color: #666666;
}

QLineEdit {
    background: #1a1a2e;
    border: 2px solid #00ff41;
    border-radius: 6px;
    padding: 8px;
    color: #00ff41;
    font-size: 12px;
    selection-background-color: #00ff41;
    selection-color: #000000;
}

QLineEdit:focus {
    border: 2px solid #00ffff;
}

QTextEdit {
    background: #0a0e27;
    border: 2px solid #00ff41;
    border-radius: 6px;
    padding: 8px;
    color: #00ff41;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 11px;
}

QCheckBox {
    color: #00ff41;
    spacing: 8px;
}

QCheckBox::indicator {
    width: 20px;
    height: 20px;
    border: 2px solid #00ff41;
    border-radius: 4px;
    background: #1a1a2e;
}

QCheckBox::indicator:checked {
    background: #00ff41;
    image: none;
}

QCheckBox::indicator:hover {
    border: 2px solid #00ffff;
}

QFrame#panel {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #1a1a2e, stop:1 #0f1b2e);
    border: 2px solid #00ff41;
    border-radius: 12px;
    padding: 15px;
}

QFrame#divider {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 transparent, 
                                stop:0.4 #00ff41, 
                                stop:0.6 #00ff41, 
                                stop:1 transparent);
    max-height: 2px;
    min-height: 2px;
}
"""

# RGB 灯带控制函数
def set_led_color(led_index, color, auto_connect):
    """发送LED颜色控制命令"""
    red = color[0]
    green = color[1]
    blue = color[2]
    data = bytes([0xEA, led_index, red, green, blue])
    # 使用自动连接发送数据
    if auto_connect.is_connected():
        thread = auto_connect.threads.get(auto_connect.connected_port)
        if thread and thread.serial_port and thread.serial_port.is_open:
            thread.serial_port.write(data)


class SerialPortUI(QMainWindow):
    """
    RGB灯带控制界面类 - 集成自动连接功能
    """

    def __init__(self, config_path):
        super().__init__()
        
        self.config_path = config_path
        self.led_labels = []
        self.selected_color = QColor(255, 255, 255)
        self.mode = "Screen Color"
        self.color_picker_button = None
        self.rgb_config_file = "rgb_config.ini"
        
        # 加载RGB配置
        self.load_rgb_config()
        
        # 初始化自动连接
        self.auto_connect = SerialPortAutoConnect(config_path)
        
        # 初始化UI
        self.init_ui()
        self.setup_serial_connections()
        
        # 启动颜色更新线程
        self.update_colors_thread = UpdateColorsThread(self)
        self.update_colors_thread.colorUpdated.connect(self.update_led_color)
        self.update_colors_thread.start()
        
        # 启动自动连接
        self.auto_connect.auto_connect(retry_mode=True, retry_interval=2000, heartbeat_interval=5000)
        
        self.debug_mode = self.rgb_config['DEFAULT'].getboolean('Debug', False)
        if self.debug_mode:
            self.create_debug_widgets()
    
    def load_rgb_config(self):
        """加载RGB配置文件"""
        self.rgb_config = configparser.ConfigParser()
        if not os.path.exists(self.rgb_config_file):
            # 创建默认配置
            self.rgb_config['DEFAULT'] = {
                'Mode': 'Screen Color',
                'CustomColorRed': '255',
                'CustomColorGreen': '255',
                'CustomColorBlue': '255',
                'LedCount': '61',
                'Debug': '0',
                'RefreshMode': '0'
            }
            self.rgb_config['LED_MAP0'] = {
                'Rows': '10',
                'Cols': '6',
                'Data': '0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59'
            }
            self.rgb_config['LED_MAP1'] = {
                'Rows': '12',
                'Cols': '20',
                'Data': '0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59'
            }
            with open(self.rgb_config_file, 'w') as configfile:
                self.rgb_config.write(configfile)
        else:
            self.rgb_config.read(self.rgb_config_file)
        
        # 读取配置
        self.mode = self.rgb_config['DEFAULT'].get('Mode', 'Screen Color')
        self.selected_color = QColor(
            int(self.rgb_config['DEFAULT'].get('CustomColorRed', '255')),
            int(self.rgb_config['DEFAULT'].get('CustomColorGreen', '255')),
            int(self.rgb_config['DEFAULT'].get('CustomColorBlue', '255'))
        )
        self.led_count = int(self.rgb_config['DEFAULT'].get('LedCount', '61'))
        self.debug_mode = self.rgb_config['DEFAULT'].getboolean('Debug', False)
        self.refresh_mode = int(self.rgb_config['DEFAULT'].get('RefreshMode', '0'))
        self.load_led_map()
    
    def load_led_map(self):
        """加载LED映射配置"""
        led_map_section = f'LED_MAP{self.refresh_mode}'
        if led_map_section in self.rgb_config:
            self.rows = int(self.rgb_config[led_map_section].get('Rows', '10'))
            self.cols = int(self.rgb_config[led_map_section].get('Cols', '6'))
            self.led_map_data = list(map(int, self.rgb_config[led_map_section]['Data'].split(',')))
        else:
            self.rows = 10
            self.cols = 6
            self.led_map_data = list(range(60))
    
    def save_rgb_config(self):
        """保存RGB配置"""
        self.rgb_config['DEFAULT'] = {
            'Mode': self.mode,
            'CustomColorRed': str(self.selected_color.red()),
            'CustomColorGreen': str(self.selected_color.green()),
            'CustomColorBlue': str(self.selected_color.blue()),
            'LedCount': str(self.led_count),
            'Debug': '1' if self.debug_mode else '0',
            'RefreshMode': str(self.refresh_mode)
        }
        with open(self.rgb_config_file, 'w') as configfile:
            self.rgb_config.write(configfile)
        
    
    def init_ui(self):
        """初始化机甲风格界面"""
        self.setWindowTitle('◢ BAN·RGB MECHA ◣')
        self.setGeometry(100, 100, 380, 170)  # 缩小窗口尺寸
        self.setWindowIcon(self.create_icon_from_data(data_commands.base64_data))
        
        # 应用机甲风格样式
        self.setStyleSheet(MECHA_STYLE)
        
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QVBoxLayout()
        self.main_layout.setSpacing(8)  # 减小间距
        self.main_layout.setContentsMargins(12, 12, 12, 12)  # 减小边距
        self.central_widget.setLayout(self.main_layout)
        
        # ========== 连接状态 ==========
        self.conn_label = QLabel("● DETECTING...")
        self.conn_label.setObjectName("detectingStatus")
        self.conn_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.conn_label.setMinimumHeight(35)  # 减小高度
        self.main_layout.addWidget(self.conn_label)
        
        # ========== 控制面板 ==========
        # 模式选择
        self.mode_layout = QHBoxLayout()
        self.mode_layout.setSpacing(6)
        
        mode_label = QLabel("MODE")
        mode_label.setStyleSheet("font-size: 11px; color: #00ffff; min-width: 45px;")
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Screen Color", "Custom"])
        self.mode_combo.setCurrentText(self.mode)
        self.mode_combo.currentTextChanged.connect(self.update_mode)
        
        self.mode_layout.addWidget(mode_label)
        self.mode_layout.addWidget(self.mode_combo)
        self.main_layout.addLayout(self.mode_layout)
        
        # 刷新模式选择
        self.refresh_mode_layout = QHBoxLayout()
        self.refresh_mode_layout.setSpacing(6)
        
        refresh_label = QLabel("REFRESH")
        refresh_label.setStyleSheet("font-size: 11px; color: #00ffff; min-width: 45px;")
        self.refresh_mode_combo = QComboBox()
        self.refresh_mode_combo.addItems(["Mode 0", "Mode 1"])
        self.refresh_mode_combo.setCurrentIndex(self.refresh_mode)
        self.refresh_mode_combo.currentIndexChanged.connect(self.change_refresh_mode)
        
        self.refresh_mode_layout.addWidget(refresh_label)
        self.refresh_mode_layout.addWidget(self.refresh_mode_combo)
        self.main_layout.addLayout(self.refresh_mode_layout)
        
        # LED 数量显示
        led_info_label = QLabel(f"LED: {self.led_count}")
        led_info_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        led_info_label.setStyleSheet("font-size: 10px; color: #00ff41; margin-top: 5px;")
        self.main_layout.addWidget(led_info_label)
        
        self.main_layout.addStretch()
        
        self.toggle_color_picker()
        
        # 添加阴影效果
        self.add_shadow_effects()

       
    def create_icon_from_data(self, base64_data):
        """从Base64数据创建图标"""
        try:
            icon_bytes = base64.b64decode(base64_data)
            pixmap = QPixmap()
            pixmap.loadFromData(icon_bytes)
            return QIcon(pixmap)
        except Exception as e:
            print(f"图标加载错误: {str(e)}")
            return QIcon()
    
    def add_shadow_effects(self):
        """添加阴影效果增强机甲感"""
        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(15)
        shadow.setColor(QColor(0, 255, 65, 80))
        shadow.setOffset(0, 0)
        self.conn_label.setGraphicsEffect(shadow)
    
    def create_debug_widgets(self):
        """创建调试模式控件 - 机甲风格"""
        divider = QFrame()
        divider.setObjectName("divider")
        self.main_layout.addWidget(divider)
        
        debug_panel = QFrame()
        debug_panel.setObjectName("panel")
        self.debug_layout = QVBoxLayout()
        debug_panel.setLayout(self.debug_layout)
        
        debug_title = QLabel("[ DEBUG CONSOLE ]")
        debug_title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        debug_title.setStyleSheet("font-size: 14px; color: #ff0040; letter-spacing: 2px; margin-bottom: 10px;")
        self.debug_layout.addWidget(debug_title)
        
        # LED 网格
        grid_container = QFrame()
        grid_container.setStyleSheet("background: #0a0e27; border: 1px solid #00ff41; border-radius: 6px; padding: 10px;")
        self.grid_layout = QGridLayout()
        grid_container.setLayout(self.grid_layout)
        self.create_led_grid()
        self.debug_layout.addWidget(grid_container)
        
        # 接收数据显示
        rx_label = QLabel("[ RX DATA ]")
        rx_label.setStyleSheet("font-size: 11px; color: #00ffff; margin-top: 10px;")
        self.debug_layout.addWidget(rx_label)
        
        self.received_text_edit = QTextEdit()
        self.received_text_edit.setReadOnly(True)
        self.received_text_edit.setPlaceholderText("Waiting for data...")
        self.received_text_edit.setMaximumHeight(100)
        self.debug_layout.addWidget(self.received_text_edit)
        
        # 发送数据区域
        tx_label = QLabel("[ TX DATA ]")
        tx_label.setStyleSheet("font-size: 11px; color: #00ffff; margin-top: 10px;")
        self.debug_layout.addWidget(tx_label)
        
        self.send_text_edit = QLineEdit()
        self.send_text_edit.setPlaceholderText("Enter command...")
        self.debug_layout.addWidget(self.send_text_edit)
        
        tx_controls = QHBoxLayout()
        self.hex_checkbox = QCheckBox("HEX MODE")
        self.send_button = QPushButton("[ SEND ]")
        self.send_button.clicked.connect(self.send_debug_data)
        tx_controls.addWidget(self.hex_checkbox)
        tx_controls.addWidget(self.send_button)
        self.debug_layout.addLayout(tx_controls)
        
        self.main_layout.addWidget(debug_panel)
    
    def create_led_grid(self):
        """创建LED网格显示 - 机甲风格"""
        # 清空现有网格
        while self.grid_layout.count():
            child = self.grid_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()
        
        self.led_labels = []
        for i in range(self.rows):
            for j in range(self.cols):
                label = QLabel()
                label.setFixedSize(12, 12)
                label.setStyleSheet("""
                    background-color: #000000;
                    border: 1px solid #00ff41;
                    border-radius: 2px;
                """)
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)
    
    def toggle_color_picker(self):
        """切换颜色选择器显示 - 机甲风格"""
        if self.color_picker_button:
            self.mode_layout.removeWidget(self.color_picker_button)
            self.color_picker_button.deleteLater()
            self.color_picker_button = None
        
        if self.mode == "Custom":
            self.color_picker_button = QPushButton("COLOR")
            self.color_picker_button.clicked.connect(self.select_color)
            self.mode_layout.addWidget(self.color_picker_button)
            self.update_custom_colors()
    
    def select_color(self):
        """选择自定义颜色"""
        color = QColorDialog.getColor(self.selected_color, self, "Select Color")
        if color.isValid():
            self.selected_color = color
            self.update_custom_colors()
            self.save_rgb_config()
    
    def update_custom_colors(self):
        """更新自定义颜色到所有LED"""
        for i in range(self.led_count):
            if i < len(self.led_labels) and self.debug_mode:
                self.led_labels[i].setStyleSheet(
                    f"background-color: rgb({self.selected_color.red()}, {self.selected_color.green()}, {self.selected_color.blue()});"
                )
        # 发送颜色数据到灯带
        if self.auto_connect.is_connected():
            for _ in range(3):  # 发送3次确保接收
                self.send_color_data_once()
    
    def send_color_data_once(self):
        """发送一次颜色数据到所有LED"""
        if self.auto_connect.is_connected():
            color = (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue())
            for i in range(self.led_count):
                set_led_color(i, color, self.auto_connect)
    
    def update_mode(self, mode):
        """更新工作模式"""
        self.mode = mode
        self.save_rgb_config()
        self.toggle_color_picker()
    
    def change_refresh_mode(self, index):
        """切换刷新模式"""
        self.refresh_mode = index
        self.save_rgb_config()
        self.load_led_map()
        if self.debug_mode:
            self.create_led_grid()
    
    def send_debug_data(self):
        """发送调试数据"""
        if self.auto_connect.is_connected():
            data_str = self.send_text_edit.text()
            if self.hex_checkbox.isChecked():
                try:
                    self.auto_connect.send_data(data_str)
                except ValueError:
                    QMessageBox.critical(self, "Error", "Invalid hex data")
            else:
                self.auto_connect.send_data(data_str)
    
    def setup_serial_connections(self):
        """设置串口连接的信号与槽"""
        self.auto_connect.connected.connect(self.on_connected)
        self.auto_connect.disconnected.connect(self.on_disconnected)
        self.auto_connect.data_available.connect(self.on_data_received)
    
    def on_connected(self, port_name):
        """串口连接成功的回调函数 - 机甲风格动画"""
        print(f"已连接到串口: {port_name}")
        self.conn_label.setText(f"● CONNECTED >> {port_name}")
        self.conn_label.setObjectName("connectedStatus")
        self.conn_label.setStyleSheet(self.conn_label.styleSheet())  # 触发样式更新
        
        # 添加连接成功的发光效果
        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(25)
        shadow.setColor(QColor(0, 255, 65, 150))
        shadow.setOffset(0, 0)
        self.conn_label.setGraphicsEffect(shadow)
        
        # 发送LED映射数据
        self.send_led_map_data()
    
    def on_disconnected(self, port_name):
        """串口断开连接的回调函数 - 机甲风格"""
        print(f"已断开与串口: {port_name} 的连接")
        self.conn_label.setText("● DISCONNECTED")
        self.conn_label.setObjectName("disconnectedStatus")
        self.conn_label.setStyleSheet(self.conn_label.styleSheet())  # 触发样式更新
        
        # 添加断开连接的警告效果
        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(25)
        shadow.setColor(QColor(255, 0, 64, 150))
        shadow.setOffset(0, 0)
        self.conn_label.setGraphicsEffect(shadow)
    
    def on_data_received(self, data):
        """数据接收回调函数"""
        if data:
            print(f"接收到数据: {data}")
            if self.debug_mode and hasattr(self, 'received_text_edit'):
                self.received_text_edit.append(str(data))
            # 如果接收到特定数据，重新发送LED映射
            if "F10" in data:
                self.send_led_map_data()
    
    def send_led_map_data(self):
        """发送LED映射配置数据"""
        if self.auto_connect.is_connected():
            thread = self.auto_connect.threads.get(self.auto_connect.connected_port)
            if thread and thread.serial_port and thread.serial_port.is_open:
                send_data = bytes([0xEC, self.led_count, 127 + self.led_count, 0xCE])
                thread.serial_port.write(send_data)
                print(f"Sent LED_MAP data: {send_data}")
    
    def update_led_color(self, led_index, color):
        """更新LED颜色"""
        avg_red, avg_green, avg_blue = color
        if 0 <= led_index < len(self.led_labels) and self.debug_mode:
            self.led_labels[led_index].setStyleSheet(
                f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});"
            )
        if self.auto_connect.is_connected():
            # 使用 led_map_data 中的索引更新灯的颜色
            for i in range(self.led_count):
                if led_index == self.led_map_data[i]:
                    set_led_color(i, color, self.auto_connect)
    
    def closeEvent(self, event):
        """处理程序关闭事件"""
        print("程序关闭，正在关闭所有串口...")
        # 停止颜色更新线程
        self.update_colors_thread.terminate()
        self.update_colors_thread.wait()
        # 关闭串口
        self.auto_connect.close_all_ports()
        event.accept()


class ScreenColorMode:
    """屏幕取色模式"""
    def __init__(self, parent):
        self.parent = parent
    
    def update_colors(self):
        """捕获屏幕颜色并更新LED"""
        screen = ImageGrab.grab()
        resized_screen = screen.resize((self.parent.cols, self.parent.rows), Image.Resampling.LANCZOS)
        
        for led_index in range(self.parent.rows * self.parent.cols):
            y = led_index // self.parent.cols
            x = led_index % self.parent.cols
            pixel = resized_screen.getpixel((x, y))
            r, g, b = pixel[0], pixel[1], pixel[2]
            self.parent.update_colors_thread.colorUpdated.emit(led_index, (r, g, b))


class UpdateColorsThread(QThread):
    """颜色更新线程"""
    colorUpdated = pyqtSignal(int, tuple)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent
        self.screen_color_mode = ScreenColorMode(parent)
    
    def run(self):
        while True:
            if not self.parent.auto_connect.is_connected():
                self.msleep(10)
                continue
            
            if self.parent.mode == "Screen Color":
                self.screen_color_mode.update_colors()
                self.msleep(50)
            else:
                self.msleep(100)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    current_dir = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(current_dir, "config.ini")
    
    window = SerialPortUI(config_path)
    window.show()
    
    sys.exit(app.exec())