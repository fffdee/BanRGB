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
        self.led_count = 64  # 默认灯数量
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

        self.serial_layout.addWidget(self.serial_combo)
        self.serial_layout.addWidget(self.baudrate_edit)
        self.serial_layout.addWidget(self.connect_button)

        self.led_count_layout.addWidget(QLabel("LED Count:"))
        self.led_count_layout.addWidget(self.led_count_edit)
        self.led_count_layout.addWidget(self.led_count_button)

        self.main_layout.addLayout(self.grid_layout)
        self.main_layout.addLayout(self.serial_layout)
        self.main_layout.addLayout(self.led_count_layout)

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
        except ValueError:
            QMessageBox.critical(self, "Error", "Invalid LED count. Please enter a positive integer.")

    def update_mode(self, mode):
        self.mode = mode
        print(f"Mode changed to: {mode}")

class UpdateColorsThread(QThread):
    colorUpdated = pyqtSignal(int, tuple)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent

    def run(self):
        while True:
            if self.parent.serial_port is None or not self.parent.serial_port.is_open:
                self.msleep(10)  # 等待10ms
                continue

            screen_geometry = QGuiApplication.primaryScreen().geometry()
            screen_width = screen_geometry.width()
            screen_height = screen_geometry.height()

            # Divide the screen into regions based on LED count
            rows = int(self.parent.led_count ** 0.5)
            cols = rows
            region_width = screen_width // cols
            region_height = screen_height // rows

            # 从中间开始向两边扩展的顺序
            middle = self.parent.led_count // 2  # 中间索引
            for offset in range(middle):
                for direction in [1, -1]:
                    led_index = middle + offset * direction
                    if led_index < 0 or led_index >= self.parent.led_count:
                        continue  # 跳过无效索引

                    # 计算对应的行列
                    row = led_index // cols
                    col = led_index % cols
                    x = col * region_width
                    y = row * region_height
                    pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
                    image = pixmap.toImage()

                    # 计算区域的平均颜色
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
                    self.colorUpdated.emit(led_index, rgb888)

            self.msleep(10)  # 减少等待时间，提高刷新速度

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())