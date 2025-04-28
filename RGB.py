import sys
import time
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox, QColorDialog
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage, QIcon
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal
import configparser
import os

# 假设你有一个函数来控制 RGB 灯带
def set_led_color(led_index, color, serial_port):
    # 将颜色转换为字节数据
    red = color[0]
    green = color[1]
    blue = color[2]
    data = bytes([0xEA, led_index, red, green, blue])
    serial_port.write(data)
    # print(f"LED {led_index}: {color}")

class ColorPicker(QMainWindow):
    def __init__(self):
        super().__init__()
        self.led_labels = []  # 初始化 led_labels 为一个空列表
        self.serial_port = None
        self.led_count = 64  # 默认灯数量
        self.selected_color = QColor(255, 255, 255)  # 默认颜色为白色
        self.mode = "Screen Color"  # 默认模式
        self.color_picker_button = None  # 调色盘按钮
        self.config_file = "config.ini"
        self.load_config()
        self.initUI()
        self.updateColorsThread = UpdateColorsThread(self)
        self.updateColorsThread.colorUpdated.connect(self.update_led_color)
        self.updateColorsThread.start()
        self.init_timer()
        self.screen_mode = True  # 添加一个布尔变量，用于记录是否处于屏幕模式

    def load_config(self):
        self.config = configparser.ConfigParser()
        if not os.path.exists(self.config_file):
            # 如果配置文件不存在，创建默认配置
            self.config['DEFAULT'] = {
                'Mode': 'Screen Color',
                'LEDCount': '64',
                'CustomColorRed': '255',
                'CustomColorGreen': '255',
                'CustomColorBlue': '255'
            }
            with open(self.config_file, 'w') as configfile:
                self.config.write(configfile)
        else:
            # 读取配置文件
            self.config.read(self.config_file)
            self.mode = self.config['DEFAULT']['Mode']
            self.led_count = int(self.config['DEFAULT']['LEDCount'])
            self.selected_color = QColor(
                int(self.config['DEFAULT']['CustomColorRed']),
                int(self.config['DEFAULT']['CustomColorGreen']),
                int(self.config['DEFAULT']['CustomColorBlue'])
            )

    def save_config(self):
        self.config['DEFAULT'] = {
            'Mode': self.mode,
            'LEDCount': str(self.led_count),
            'CustomColorRed': str(self.selected_color.red()),
            'CustomColorGreen': str(self.selected_color.green()),
            'CustomColorBlue': str(self.selected_color.blue())
        }
        with open(self.config_file, 'w') as configfile:
            self.config.write(configfile)

    def initUI(self):
        self.setWindowTitle('BanRGB')
        self.setGeometry(100, 100, 100, 100)
        self.setWindowIcon(QIcon('BanGO.png'))
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.main_layout = QVBoxLayout()
        self.central_widget.setLayout(self.main_layout)

        # 创建 LED 网格
        self.grid_layout = QGridLayout()
        self.create_led_grid()

        # 串口选择和波特率设置
        self.serial_layout = QHBoxLayout()
        self.serial_combo = QComboBox()
        self.baudrate_edit = QLineEdit("115200")
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_serial)

        # 灯数量设置
        self.led_count_layout = QHBoxLayout()
        self.led_count_edit = QLineEdit(str(self.led_count))
        self.led_count_edit.setFixedWidth(50)
        self.led_count_button = QPushButton("Update")
        self.led_count_button.clicked.connect(self.update_led_count)

        # 模式选择
        self.mode_layout = QHBoxLayout()
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Screen Color", "Custom"])
        self.mode_combo.currentTextChanged.connect(self.update_mode)

        self.serial_layout.addWidget(self.serial_combo)
        self.serial_layout.addWidget(self.baudrate_edit)
        self.serial_layout.addWidget(self.connect_button)

        self.led_count_layout.addWidget(QLabel("LED Count:"))
        self.led_count_layout.addWidget(self.led_count_edit)
        self.led_count_layout.addWidget(self.led_count_button)

        self.mode_layout.addWidget(QLabel("Mode:"))
        self.mode_layout.addWidget(self.mode_combo)

        self.main_layout.addLayout(self.grid_layout)
        self.main_layout.addLayout(self.serial_layout)
        self.main_layout.addLayout(self.led_count_layout)
        self.main_layout.addLayout(self.mode_layout)

        self.update_serial_ports()
        self.mode_combo.setCurrentText(self.mode)  # 设置模式选择框的当前模式
        self.toggle_color_picker()

    def update_serial_ports(self):
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.serial_combo.addItem(port.device)

    def connect_serial(self):
        port_name = self.serial_combo.currentText()
        baudrate = int(self.baudrate_edit.text())
        try:
            self.serial_port = serial.Serial(port_name, baudrate, timeout=1)
            QMessageBox.information(self, "Connected", f"Connected to {port_name} at {baudrate} baud")
        except serial.SerialException as e:
            QMessageBox.critical(self, "Error", f"Failed to connect: {e}")

    def update_led_color(self, led_index, color):
        avg_red, avg_green, avg_blue = color
        if 0 <= led_index < len(self.led_labels):
            self.led_labels[led_index].setStyleSheet(f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});")
        if self.serial_port:
            set_led_color(led_index, color, self.serial_port)

    def create_led_grid(self):
        # 清空现有的 LED 网格
        for i in range(self.grid_layout.count()):
            self.grid_layout.itemAt(i).widget().deleteLater()

        self.led_labels = []
        rows = int(self.led_count ** 0.5)
        cols = rows

        for i in range(rows):
            for j in range(cols):
                label = QLabel()
                label.setFixedSize(10, 10)
                label.setStyleSheet("background-color: black;")
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)

    def update_led_count(self):
        try:
            new_led_count = int(self.led_count_edit.text())
            if new_led_count > 0:
                self.led_count = new_led_count
                self.create_led_grid()
                self.save_config()
        except ValueError:
            QMessageBox.critical(self, "Error", "Invalid LED count. Please enter a positive integer.")

    def update_mode(self, mode):
        self.mode = mode
        self.screen_mode = mode == "Screen Color"  # 更新屏幕模式状态
        self.save_config()
        self.toggle_color_picker()

    def toggle_color_picker(self):
        if self.color_picker_button:
            self.mode_layout.removeWidget(self.color_picker_button)
            self.color_picker_button.deleteLater()
            self.color_picker_button = None

        if self.mode == "Custom":
            self.color_picker_button = QPushButton("Select Color")
            self.color_picker_button.clicked.connect(self.select_color)
            self.mode_layout.addWidget(self.color_picker_button)
            self.update_custom_colors()  # 确保自定义模式的颜色正确显示

    def select_color(self):
        color = QColorDialog.getColor(self.selected_color, self, "Select Color")
        if color.isValid():
            self.selected_color = color
            self.update_custom_colors()
            self.save_config()

    def update_custom_colors(self):
        # 更新所有LED为选定的颜色
        for i in range(self.led_count):
            self.update_led_color(i, (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue()))

    def init_timer(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.send_keepalive_message)
        self.timer.start(20000)  # 120000毫秒 = 2分钟

    def send_keepalive_message(self):
        # 仅在屏幕模式下发送消息
        if self.screen_mode and self.serial_port and self.serial_port.is_open:
            message = bytes([0xEB, 0xFF, 0xBE])
            self.serial_port.write(message)
            # print("Sent keepalive message: EB FF BE")

class ScreenColorMode:
    def __init__(self, parent):
        self.parent = parent

    def update_colors(self):
        screen_geometry = QGuiApplication.primaryScreen().geometry()
        screen_width = screen_geometry.width()
        screen_height = screen_geometry.height()

        rows = int(self.parent.led_count ** 0.5)
        cols = rows
        region_width = screen_width // cols
        region_height = screen_height // rows

        for led_index in range(self.parent.led_count):
            row = led_index // cols
            col = led_index % cols
            x = col * region_width
            y = row * region_height
            pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
            image = pixmap.toImage()

            total_red = 0
            total_green = 0
            total_blue = 0
            pixel_count = 0

            for y in range(100):
                for x in range(100):
                    color = QColor(image.pixel(x, y))
                    total_red += color.red()
                    total_green += color.green()
                    total_blue += color.blue()
                    pixel_count += 1

            avg_red = total_red // pixel_count
            avg_green = total_green // pixel_count
            avg_blue = total_blue // pixel_count

            rgb888 = (avg_red, avg_green, avg_blue)
            
            # 发出信号更新 GUI
            self.parent.updateColorsThread.colorUpdated.emit(led_index, rgb888)

class UpdateColorsThread(QThread):
    colorUpdated = pyqtSignal(int, tuple)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent
        self.screen_color_mode = ScreenColorMode(parent)

    def run(self):
        while True:
            if self.parent.serial_port is None or not self.parent.serial_port.is_open:
                self.msleep(10)  # 等待10ms
                continue

            if self.parent.mode == "Screen Color":
                self.screen_color_mode.update_colors()

            self.msleep(5)  # 减少等待时间，提高刷新速度

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())