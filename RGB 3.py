import sys
import time
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox, QColorDialog
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage, QIcon
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal
import configparser
import os
import pyaudio
import numpy as np
import ctypes
import colorsys
import librosa

np.set_printoptions(threshold=np.inf)
# 设置线程模式为多线程
ctypes.windll.ole32.CoInitializeEx(None, 0)  # COINIT_MULTITHREADED

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
        self.mode_combo.addItems(["Screen Color", "Custom", "Music Rhythm"])  # 添加音乐节奏模式
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
            time.sleep(0.02)
            self.update_led_color(i, (self.selected_color.red(), self.selected_color.green(), self.selected_color.blue()))


class MusicRhythmMode:
    def __init__(self, parent):
        self.parent = parent
        self.sample_rate = 44100
        self.fft_size = 2048
        self.audio = pyaudio.PyAudio()
        self.stream = None
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_colors)
        self.timer.start(200)  # 每200毫秒触发一次
        self.open_stream()

    def open_stream(self):
        # 自动选择名字中包含“CABLE Output”的设备
        device_index = self.find_cable_output_device()
        if device_index is None:
            raise ValueError("No device with 'CABLE Output' found. Please check your device names.")

        # 打开音频输入流
        self.stream = self.audio.open(format=pyaudio.paInt16,
                                      channels=2,  # 假设是立体声
                                      rate=self.sample_rate,
                                      input=True,
                                      input_device_index=device_index,
                                      frames_per_buffer=self.fft_size)

    def find_cable_output_device(self):
        info = self.audio.get_host_api_info_by_index(0)
        num_devices = info.get('deviceCount')
        for i in range(num_devices):
            device_info = self.audio.get_device_info_by_host_api_device_index(0, i)
            if "CABLE Output" in device_info.get('name'):
                print(f"Selected device: {device_info.get('name')} (Index: {i})")
                return i
        return None

    def update_colors(self):
        if not self.stream.is_active():
            self.open_stream()
        data = np.frombuffer(self.stream.read(self.fft_size), dtype=np.int16)
        print(f"Audio data: {data}")  # 打印部分音频数据

        # FFT 分析
        fft_data = np.fft.fft(data)
        fft_magnitude = np.abs(fft_data[:self.fft_size // 2])
        fft_magnitude = np.log(fft_magnitude + 1)  # 避免除零

        # 分析不同频率范围
        low_freq_magnitude = np.mean(fft_magnitude[:self.fft_size // 8])  # 低频
        mid_freq_magnitude = np.mean(fft_magnitude[self.fft_size // 8:self.fft_size // 4])  # 中频
        high_freq_magnitude = np.mean(fft_magnitude[self.fft_size // 4:self.fft_size // 2])  # 高频

        # 检测节奏
        rhythm = self.detect_rhythm(data)

        # 综合频率信息和节奏信息生成颜色和亮度
        color, brightness = self.get_color_and_brightness(low_freq_magnitude, mid_freq_magnitude, high_freq_magnitude, rhythm)

        print(f"Generated color: {color}, Brightness: {brightness}")  # 打印生成的颜色和亮度值

        # 根据响度调整亮灯的数量
        num_lights = int(brightness / 255 * self.parent.led_count)
        for i in range(self.parent.led_count):
            if i < num_lights:
                self.parent.updateColorsThread.colorUpdated.emit(i, color)
            else:
                self.parent.updateColorsThread.colorUpdated.emit(i, (0, 0, 0))  # 关闭其他灯

    def detect_rhythm(self, data):
        # 确保数据是浮点数类型
        data = data.astype(np.float32)
        
        # 归一化音频数据
        data = data / np.max(np.abs(data))
        
        # 检查并修复非有限值
        if not np.all(np.isfinite(data)):
            print("Warning: Audio data contains non-finite values. Replacing with zero.")
            data = np.where(np.isfinite(data), data, 0)
        
        # 使用 librosa 检测节奏
        try:
            tempo, beats = librosa.beat.beat_track(y=data, sr=self.sample_rate, hop_length=self.fft_size // 2)
            return len(beats) > 0  # 如果检测到节奏，返回 True
        except Exception as e:
            print(f"Error detecting rhythm: {e}")
            return False

    def get_color_and_brightness(self, low_freq, mid_freq, high_freq, rhythm):
        threshold = 30
        max_magnitude = 5000  # 调整这个值以适应你的音频信号范围

        # 根据频率信息生成颜色
        total_magnitude = low_freq + mid_freq + high_freq
        if total_magnitude < threshold:
            return (0, 0, 0), 0  # 如果总能量低于阈值，返回黑色

        hue = (total_magnitude - threshold) / (max_magnitude - threshold)
        hue = hue % 1.0  # 确保 hue 在 [0, 1] 范围内

        saturation = 1.0
        value = 1.0
        rgb = colorsys.hsv_to_rgb(hue, saturation, value)
        red = int(rgb[0] * 255)
        green = int(rgb[1] * 255)
        blue = int(rgb[2] * 255)

        # 根据节奏信息调整亮度
        if rhythm:
            brightness = 255  # 有节奏时，亮度最高
        else:
            brightness = int(total_magnitude / max_magnitude * 255)  # 根据音频强度调整亮度

        return (red, green, blue), brightness


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

            for y in range(region_height):
                for x in range(region_width):
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
        self.music_rhythm_mode = MusicRhythmMode(parent)

    def run(self):
        while True:
            if self.parent.serial_port is None or not self.parent.serial_port.is_open:
                self.msleep(10)  # 等待10ms
                continue

            if self.parent.mode == "Screen Color":
                self.screen_color_mode.update_colors()
            elif self.parent.mode == "Music Rhythm":
                self.music_rhythm_mode.update_colors()

            self.msleep(10)  # 减少等待时间，提高刷新速度


if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())