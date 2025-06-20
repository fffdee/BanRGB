import sys
import time
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox, QColorDialog, QTextEdit, QCheckBox
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage, QIcon
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal
import configparser
import os
from PIL import ImageGrab
from PIL import Image

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
        self.selected_color = QColor(255, 255, 255)  # 默认颜色为白色
        self.mode = "Screen Color"  # 默认模式
        self.color_picker_button = None  # 调色盘按钮
        self.config_file = "config.ini"
        self.load_config()
        self.initUI()
        self.update_colors_thread = UpdateColorsThread(self)
        self.update_colors_thread.colorUpdated.connect(self.update_led_color)
        self.update_colors_thread.start()
        self.init_timer()
        self.screen_mode = True  # 添加一个布尔变量，用于记录是否处于屏幕模式
        self.serial_reader_thread = SerialReaderThread(self)  # 添加串口读取线程
        self.serial_reader_thread.start()
        self.debug_mode = self.config['DEFAULT'].getboolean('Debug', False)  # 获取 debug 模式状态
        if self.debug_mode:
            self.create_debug_widgets()

    def load_config(self):
        self.config = configparser.ConfigParser()
        config_files = [f for f in os.listdir() if f.endswith('.ini')]  # 获取当前目录下所有 .ini 文件
        if not config_files:
            # 如果没有找到任何 .ini 文件，创建默认配置
            self.config['DEFAULT'] = {
                'Mode': 'Screen Color',
                'CustomColorRed': '255',
                'CustomColorGreen': '255',
                'CustomColorBlue': '255',
                'Length': '20',  # 默认长
                'Width': '10',  # 默认宽
                'LedCount': '61',  # 默认灯的数量
                'Debug': '0',  # 默认关闭 debug 模式
                'RefreshMode': '0'  # 默认刷新模式
            }
            self.config['LED_MAP0'] = {
                'Rows': '10',
                'Cols': '6',
                'Data': '0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59'
            }
            self.config['LED_MAP1'] = {
                'Rows': '8',
                'Cols': '8',
                'Data': '0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59'
            }
            with open('default_config.ini', 'w') as configfile:
                self.config.write(configfile)
            self.length = 20
            self.width = 10
            self.led_count = int(self.config['DEFAULT']['LedCount'])  # 从配置文件读取灯的数量
            self.refresh_mode = int(self.config['DEFAULT']['RefreshMode'])  # 读取刷新模式
        else:
            # 读取第一个找到的 .ini 文件
            self.config.read(config_files[0])
            self.mode = self.config['DEFAULT']['Mode']
            self.selected_color = QColor(
                int(self.config['DEFAULT']['CustomColorRed']),
                int(self.config['DEFAULT']['CustomColorGreen']),
                int(self.config['DEFAULT']['CustomColorBlue'])
            )
            self.length = int(self.config['DEFAULT']['Length'])
            self.width = int(self.config['DEFAULT']['Width'])
            self.led_count = int(self.config['DEFAULT']['LedCount'])  # 从配置文件读取灯的数量
            self.debug_mode = self.config['DEFAULT'].getboolean('Debug', False)  # 读取 Debug 模式状态
            self.refresh_mode = int(self.config['DEFAULT']['RefreshMode'])  # 读取刷新模式
            self.load_led_map()

    def load_led_map(self):
        refresh_mode = self.refresh_mode
        led_map_section = f'LED_MAP{refresh_mode}'
        if led_map_section in self.config:
            self.rows = int(self.config[led_map_section].get('Rows', '10'))
            self.cols = int(self.config[led_map_section].get('Cols', '6'))
            self.led_map_data = list(map(int, self.config[led_map_section]['Data'].split(',')))
        else:
            self.rows = 10
            self.cols = 6
            self.led_map_data = list(range(60))  # 默认的 LED 映射数据
        print(self.led_map_data)

    def save_config(self):
        self.config['DEFAULT'] = {
            'Mode': self.mode,
            'CustomColorRed': str(self.selected_color.red()),
            'CustomColorGreen': str(self.selected_color.green()),
            'CustomColorBlue': str(self.selected_color.blue()),
            'Length': str(self.length),
            'Width': str(self.width),
            'LedCount': str(self.led_count),  # 保存灯的数量
            'Debug': '1' if self.debug_mode else '0',
            'RefreshMode': str(self.refresh_mode)  # 保存刷新模式
        }
        with open('default_config.ini', 'w') as configfile:
            self.config.write(configfile)

    def initUI(self):
        self.setWindowTitle('BanRGB')
        self.setGeometry(100, 100, 300, 100)
        self.setWindowIcon(QIcon('BanGO.png'))
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.main_layout = QVBoxLayout()
        self.central_widget.setLayout(self.main_layout)

        # 串口选择和波特率设置
        self.serial_layout = QHBoxLayout()
        self.serial_combo = QComboBox()
        self.baudrate_edit = QLineEdit("115200")
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_serial)

        # 模式选择
        self.mode_layout = QHBoxLayout()
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Screen Color", "Custom"])
        self.mode_combo.currentTextChanged.connect(self.update_mode)

        # 创建刷新模式选择框
        self.refresh_mode_layout = QHBoxLayout()
        self.refresh_mode_label = QLabel("Refresh Mode:")
        self.refresh_mode_combo = QComboBox()
        self.refresh_mode_combo.addItems(["Mode 0", "Mode 1"])  # 假设有两种刷新模式
        self.refresh_mode_combo.setCurrentIndex(self.refresh_mode)
        self.refresh_mode_combo.currentIndexChanged.connect(self.change_refresh_mode)
        self.refresh_mode_layout.addWidget(self.refresh_mode_label)
        self.refresh_mode_layout.addWidget(self.refresh_mode_combo)

        self.serial_layout.addWidget(self.serial_combo)
        self.serial_layout.addWidget(self.baudrate_edit)
        self.serial_layout.addWidget(self.connect_button)

        self.mode_layout.addWidget(QLabel("Mode:"))
        self.mode_layout.addWidget(self.mode_combo)

        self.main_layout.addLayout(self.serial_layout)
        self.main_layout.addLayout(self.mode_layout)
        self.main_layout.addLayout(self.refresh_mode_layout)

        self.update_serial_ports()
        self.mode_combo.setCurrentText(self.mode)  # 设置模式选择框的当前模式
        self.toggle_color_picker()

    def create_debug_widgets(self):
        # 创建 debug 模式下的控件
        self.debug_layout = QVBoxLayout()

        # 创建 LED 网格
        self.grid_layout = QGridLayout()
        self.create_led_grid()
        self.debug_layout.addLayout(self.grid_layout)

        # 创建文本显示框
        self.received_text_edit = QTextEdit()
        self.received_text_edit.setReadOnly(True)
        self.received_text_edit.setPlaceholderText("Received Data")
        self.debug_layout.addWidget(self.received_text_edit)

        # 创建文本输入框
        self.send_text_edit = QLineEdit()
        self.send_text_edit.setPlaceholderText("Enter Data to Send")
        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.send_data)
        self.hex_checkbox = QCheckBox("Send as Hex")
        self.hex_checkbox.setChecked(False)  # 默认不以16进制格式发送
        self.debug_layout.addWidget(self.send_text_edit)
        self.debug_layout.addWidget(self.send_button)
        self.debug_layout.addWidget(self.hex_checkbox)

        # 将 debug 布局添加到主布局中
        self.main_layout.addLayout(self.debug_layout)

    def create_led_grid(self):
        # 清空现有的 LED 网格
        for i in range(self.grid_layout.count()):
            self.grid_layout.itemAt(i).widget().deleteLater()

        self.led_labels = []
        # 根据刷新模式获取行数和列数
        refresh_mode = self.refresh_mode
        led_map_section = f'LED_MAP{refresh_mode}'
        if led_map_section in self.config:
            self.rows = int(self.config[led_map_section].get('Rows', '10'))
            self.cols = int(self.config[led_map_section].get('Cols', '6'))
        else:
            self.rows = 10
            self.cols = 6

        for i in range(self.rows):
            for j in range(self.cols):
                label = QLabel()
                label.setFixedSize(10, 10)
                label.setStyleSheet("background-color: black;")
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)

    def update_led_count_and_config(self):
        try:
            self.length = int(self.length_edit.text())
            self.width = int(self.width_edit.text())
            self.rows = int(self.rows / 2)
            self.cols = int(self.cols / 2) + 1
            self.led_count = (self.length * 2) + (self.width * 2)
            self.total_label.setText(f"Total: {self.led_count}")
            self.create_led_grid()
            self.save_config()
        except ValueError:
            QMessageBox.critical(self, "Error", "Invalid input. Please enter positive integers for length and width.")

    def update_serial_ports(self):
        self.serial_combo.clear()  # 清空当前的串口列表
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.serial_combo.addItem(port.device)

    def connect_serial(self):
        port_name = self.serial_combo.currentText()
        baudrate = int(self.baudrate_edit.text())
        try:
            self.serial_port = serial.Serial(port_name, baudrate, timeout=1)
            # 读取 LED_MAP 数据并发送
            self.send_led_map_data()
            QMessageBox.information(self, "Connected", f"Connected to {port_name} at {baudrate} baud")
        except serial.SerialException as e:
            QMessageBox.critical(self, "Error", f"Failed to connect: {e}")

    def send_led_map_data(self):
        send_data = bytes([0xEC, self.led_count,127+self.led_count,0xCE])
        self.serial_port.write(send_data)
        print(f"Sent LED_MAP data: {send_data}")

    def update_led_color(self, led_index, color):
        avg_red, avg_green, avg_blue = color
        if 0 <= led_index < len(self.led_labels) and self.debug_mode:
            self.led_labels[led_index].setStyleSheet(f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});")
        if self.serial_port:
            # 使用 led_map_data 中的索引更新灯的颜色
            for i in range(self.led_count):
                if led_index == self.led_map_data[i]:
                    set_led_color(i, color, self.serial_port)

    def change_refresh_mode(self, index):
        self.refresh_mode = index
        self.save_config()
        self.load_led_map()
        if self.debug_mode:
            self.create_led_grid()

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
            if i < len(self.led_labels) and self.debug_mode:
                self.led_labels[i].setStyleSheet(f"background-color: rgb({self.selected_color.red()}, {self.selected_color.green()}, {self.selected_color.blue()});")
        # 发送颜色数据到灯带
        self.send_color_data_once()
        self.send_color_data_once()
        self.send_color_data_once()

    def send_color_data_once(self):
        if self.serial_port and self.serial_port.is_open:
            color = (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue())
            for i in range(self.led_count):
                set_led_color(i, color, self.serial_port)
            print("Sent color data once")

    def init_timer(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.send_keepalive_message)
        # self.timer.start(1000)  # 120000毫秒 = 2分钟

    def send_keepalive_message(self):
        # 仅在屏幕模式下发送消息
        if self.mode == "Screen Color":
            for i in range(self.led_count):
                if i < len(self.led_labels):
                    self.update_led_color(i, (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue()))
    

    def send_data(self):
        if self.serial_port and self.serial_port.is_open:
            data = self.send_text_edit.text()
            if self.hex_checkbox.isChecked():
                # 如果选中了16进制发送，将输入的字符串转换为字节
                try:
                    data = bytes.fromhex(data.replace(" ", ""))
                except ValueError:
                    QMessageBox.critical(self, "Error", "Invalid hex data")
                    return
            else:
                # 否则，将字符串直接编码为字节
                data = data.encode()
            self.serial_port.write(data)
            print(f"Sent data: {data}")

class ScreenColorMode:
    def __init__(self, parent):
        self.parent = parent

    def update_colors(self):
        # 捕获整个屏幕的截图
        screen = ImageGrab.grab()
        # 调整图像大小为 rows*cols
        resized_screen = screen.resize((self.parent.cols, self.parent.rows), Image.Resampling.LANCZOS)
        # resized_screen.save('screen_capture.png')
        # 遍历每个像素，获取颜色值
        for led_index in range(self.parent.rows * self.parent.cols):
            y = led_index // self.parent.cols
            x = led_index % self.parent.cols
            pixel = resized_screen.getpixel((x, y))
            r, g, b = pixel[0], pixel[1], pixel[2]
            # 发出信号更新 GUI
            self.parent.update_colors_thread.colorUpdated.emit(led_index, (r, g, b))

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
                self.msleep(50)

class SerialReaderThread(QThread):
    portsUpdated = pyqtSignal()  # 添加一个信号，用于通知主线程刷新串口设备列表

    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent

    def run(self):
        while True:
            if self.parent.serial_port is not None and self.parent.serial_port.is_open:
                try:
                    # 读取串口数据
                    if self.parent.serial_port.in_waiting > 0:
                        data = self.parent.serial_port.read(self.parent.serial_port.in_waiting)
                        # 打印接收到的数据
                        print("Received data:", data)
                        self.parent.send_led_map_data()
                        if self.parent.debug_mode:
                            self.parent.received_text_edit.append(str(data))
                except serial.SerialException as e:
                    print("Error reading serial port:", e)
                    self.parent.serial_port.close()  # 关闭串口
                    self.parent.serial_port = None  # 确保串口对象为 None
                    QMessageBox.information(self.parent, "Disconnected", "Serial port disconnected unexpectedly.")
                    self.parent.serial_reader_thread = None  # 确保串口读取线程为 None
            self.msleep(10)  # 等待10ms
            self.portsUpdated.emit()  # 发出信号，通知主线程刷新串口设备列表

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())