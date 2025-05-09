import sys
import time
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox, QColorDialog
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
    print(f"LED {led_index}: {color}")

class ColorPicker(QMainWindow):
    def __init__(self):
        super().__init__()
        self.led_labels = []  # 初始化 led_labels 为一个空列表
        self.serial_port = None
        self.length = 8  # 默认长
        self.width = 8  # 默认宽
        self.rows = 10  # 默认行数
        self.cols = 6  # 默认列数
        self.led_count = (self.length // 2) * ((self.width // 2) + 1)  # 根据新公式计算默认灯数量
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

    def load_config(self):
        self.config = configparser.ConfigParser()
        if not os.path.exists(self.config_file):
            # 如果配置文件不存在，创建默认配置
            self.config['DEFAULT'] = {
                'Mode': 'Screen Color',
                'CustomColorRed': '255',
                'CustomColorGreen': '255',
                'CustomColorBlue': '255',
                'Length': '20',  # 默认长
                'Width': '10',    # 默认宽
                'Rows': '10',      # 默认行数
                'Cols': '6'       # 默认列数
            }
            with open(self.config_file, 'w') as configfile:
                self.config.write(configfile)
            self.length = 20
            self.width = 10
            self.rows = 10
            self.cols = 6
            self.led_count = (self.length * 2) + (self.width * 2)
        else:
            # 读取配置文件
            self.config.read(self.config_file)
            self.mode = self.config['DEFAULT']['Mode']
            self.selected_color = QColor(
                int(self.config['DEFAULT']['CustomColorRed']),
                int(self.config['DEFAULT']['CustomColorGreen']),
                int(self.config['DEFAULT']['CustomColorBlue'])
            )
            self.length = int(self.config['DEFAULT']['Length'])
            self.width = int(self.config['DEFAULT']['Width'])
            self.rows = int(self.config['DEFAULT']['Rows'])
            self.cols = int(self.config['DEFAULT']['Cols'])
            self.led_count = (self.length * 2) + (self.width * 2)

    def save_config(self):
        self.config['DEFAULT'] = {
            'Mode': self.mode,
            'CustomColorRed': str(self.selected_color.red()),
            'CustomColorGreen': str(self.selected_color.green()),
            'CustomColorBlue': str(self.selected_color.blue()),
            'Length': str(self.length),
            'Width': str(self.width),
            'Rows': str(self.rows),
            'Cols': str(self.cols)
        }
        with open(self.config_file, 'w') as configfile:
            self.config.write(configfile)
        self.led_count = (self.length * 2) + (self.width * 2)

    def initUI(self):
        self.setWindowTitle('BanRGB')
        self.setGeometry(100, 100, 300, 200)
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

        # 模式选择
        self.mode_layout = QHBoxLayout()
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Screen Color", "Custom"])
        self.mode_combo.currentTextChanged.connect(self.update_mode)

        self.serial_layout.addWidget(self.serial_combo)
        self.serial_layout.addWidget(self.baudrate_edit)
        self.serial_layout.addWidget(self.connect_button)

        self.mode_layout.addWidget(QLabel("Mode:"))
        self.mode_layout.addWidget(self.mode_combo)

        self.main_layout.addLayout(self.grid_layout)
        self.main_layout.addLayout(self.serial_layout)
        self.main_layout.addLayout(self.mode_layout)

        self.update_serial_ports()
        self.mode_combo.setCurrentText(self.mode)  # 设置模式选择框的当前模式
        self.toggle_color_picker()

    def update_led_count_and_config(self):
        try:
            self.length = int(self.length_edit.text())
            self.width = int(self.width_edit.text())
            self.rows = int(self.rows/2)
            self.cols = int(self.cols/2)+1
            self.led_count = (self.length * 2)+ (self.width*2)
            self.total_label.setText(f"Total: {self.led_count}")
            self.create_led_grid()
            self.save_config()
        except ValueError:
            QMessageBox.critical(self, "Error", "Invalid input. Please enter positive integers for length and width.")

    def update_serial_ports(self):
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
        # 读取当前目录下的 .ini 文件中的 LED_MAP 数据
        folder_path = os.path.dirname(__file__)
        all_data = []
        for filename in os.listdir(folder_path):
            if filename.endswith(".ini"):
                file_path = os.path.join(folder_path, filename)
                config = configparser.ConfigParser()
                config.read(file_path)
                if 'LED_MAP' in config:
                    # 如果文件中有 [LED_MAP] 部分，读取其中的 data
                    led_map_data = config['LED_MAP'].get('data', '')
                    if led_map_data:
                        # 将数据转换为字节
                        data_bytes = bytes(map(int, led_map_data.split(',')))
                        all_data.extend(data_bytes)
        
        # 构造发送的数据格式：0xEC, 数据长度, 数据
        if all_data:
            data_length = len(all_data)
            send_data = bytes([0xEC, data_length]) + bytes(all_data)
            self.serial_port.write(send_data)
            print(f"Sent LED_MAP data: {send_data}")

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
        rows = self.rows  # 使用配置文件中的行数
        cols = self.cols  # 使用配置文件中的列数
        print("rows is : ",rows )
        print("cols is : ",cols )
        for i in range(rows):
            for j in range(cols):
                label = QLabel()
                label.setFixedSize(10, 10)
                label.setStyleSheet("background-color: black;")
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)

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
            if i < len(self.led_labels):
                self.update_led_color(i, (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue()))

    def init_timer(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.send_keepalive_message)
    #     self.timer.start(1000)  # 120000毫秒 = 2分钟

    def send_keepalive_message(self):
        # 仅在屏幕模式下发送消息
        if self.mode == "Custom":
         for i in range(self.led_count):
            if i < len(self.led_labels):
                self.update_led_color(i, (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue()))
        print(self.led_count)

class ScreenColorMode:
    def __init__(self, parent):
        self.parent = parent

    def update_colors(self):
        # 捕获整个屏幕的截图
        screen = ImageGrab.grab()
        # 调整图像大小为 rows*cols
        resized_screen = screen.resize((self.parent.cols, self.parent.rows ), Image.Resampling.LANCZOS)
        resized_screen.save('screen_capture.png')
        # 遍历每个像素，获取颜色值
        for led_index in range(self.parent.led_count):
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
                except serial.SerialException as e:
                    print("Error reading serial port:", e)
                    self.parent.serial_port.close()  # 关闭串口
                    self.parent.serial_port = None  # 确保串口对象为 None
                    QMessageBox.information(self.parent, "Disconnected", "Serial port disconnected unexpectedly.")
                    self.parent.serial_reader_thread = None  # 确保串口读取线程为 None
            self.msleep(10)  # 等待10ms

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())