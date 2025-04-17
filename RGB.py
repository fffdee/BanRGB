import sys
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal
import random

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
        self.initUI()
        self.updateColorsThread = UpdateColorsThread(self)
        self.updateColorsThread.colorUpdated.connect(self.update_led_color)
        self.updateColorsThread.start()

    def initUI(self):
        self.setWindowTitle('Screen Color Picker')
        self.setGeometry(100, 100, 100, 100)

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.main_layout = QVBoxLayout()
        self.central_widget.setLayout(self.main_layout)

        # 创建 8x8 的 LED 网格
        self.grid_layout = QGridLayout()
        for i in range(8):
            for j in range(8):
                label = QLabel()
                label.setFixedSize(10, 10)
                label.setStyleSheet("background-color: black;")
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)  # 将 label 添加到 led_labels 列表中

        # 串口选择和波特率设置
        self.serial_layout = QHBoxLayout()
        self.serial_combo = QComboBox()
        self.baudrate_edit = QLineEdit("115200")
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_serial)

        self.serial_layout.addWidget(self.serial_combo)
        self.serial_layout.addWidget(self.baudrate_edit)
        self.serial_layout.addWidget(self.connect_button)

        self.main_layout.addLayout(self.grid_layout)
        self.main_layout.addLayout(self.serial_layout)

        self.update_serial_ports()

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
        self.led_labels[led_index].setStyleSheet(f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});")
        if self.serial_port:
            set_led_color(led_index, color, self.serial_port)

class UpdateColorsThread(QThread):
    colorUpdated = pyqtSignal(int, tuple)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent

    def run(self):
        while True:
            if self.parent.serial_port is None or not self.parent.serial_port.is_open:
                print("Serial port not connected or not open")
                self.msleep(10)  # 等待10ms
                continue

            screen_geometry = QGuiApplication.primaryScreen().geometry()
            screen_width = screen_geometry.width()
            screen_height = screen_geometry.height()

            # Divide the screen into 64 regions (8x8 grid)
            region_width = screen_width // 8
            region_height = screen_height // 8

            # 从中间开始向两边扩展的顺序
            middle = 32  # 中间索引
            for offset in range(32):
                for direction in [1, -1]:
                    led_index = middle + offset * direction
                    if led_index < 0 or led_index >= 64:
                        continue  # 跳过无效索引

                    # 计算对应的行列
                    row = led_index // 8
                    col = led_index % 8
                    x = col * region_width
                    y = row * region_height
                    pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
                    image = pixmap.toImage()

                    # 计算区域的平均颜色
                    total_red = 0
                    total_green = 0
                    total_blue = 0
                    pixel_count = 0

                    for y in range(30):
                        for x in range(30):
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
                    self.colorUpdated.emit(led_index, rgb888)

            self.msleep(50)  # 减少等待时间，提高刷新速度

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())