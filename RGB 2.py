import sys
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout, QComboBox, QLineEdit, QPushButton, QHBoxLayout, QMessageBox
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal

# 假设你有一个函数来控制 RGB 灯带
def set_led_colors(colors, serial_port):
    # 将颜色列表转换为字节数据
    data = bytearray()
    for color in colors:
        red = color[0]
        green = color[1]
        blue = color[2]
        data.extend([0xEA, red, green, blue])  # 每个颜色数据前加上 0xEA
    serial_port.write(data)
    print(f"Sent {len(colors)} LED colors: {data}")

class ColorPicker(QMainWindow):
    def __init__(self):
        super().__init__()
        self.led_labels = []  # 初始化 led_labels 为一个空列表
        self.serial_port = None
        self.led_count = 64  # 默认灯数量
        self.colors = []  # 用于存储所有LED的颜色
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
        self.led_count_edit = QLineEdit("64")
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
        if led_index < len(self.led_labels):
            self.led_labels[led_index].setStyleSheet(f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});")
        if self.serial_port:
            self.colors.append(color)
            if len(self.colors) == self.led_count:
                self.send_colors_to_serial()

    def send_colors_to_serial(self):
        if self.serial_port:
            set_led_colors(self.colors, self.serial_port)
            self.colors.clear()

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
                self.colors.clear()  # 清空颜色列表
                self.create_led_grid()
        except ValueError:
            QMessageBox.critical(self, "Error", "Invalid LED count. Please enter a positive integer.")

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

            rows = int(self.parent.led_count ** 0.5)
            cols = rows
            region_width = screen_width // cols
            region_height = screen_height // rows

            for i in range(rows):
                for j in range(cols):
                    led_index = i * cols + j
                    x = j * region_width
                    y = i * region_height
                    pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
                    image = pixmap.toImage()

                    total_red = 0
                    total_green = 0
                    total_blue = 0
                    pixel_count = 0

                    for y in range(50):
                        for x in range(50):
                            color = QColor(image.pixel(x, y))
                            total_red += color.red()
                            total_green += color.green()
                            total_blue += color.blue()
                            pixel_count += 1

                    avg_red = total_red // pixel_count
                    avg_green = total_green // pixel_count
                    avg_blue = total_blue // pixel_count

                    rgb888 = (avg_red, avg_green, avg_blue)

                    self.colorUpdated.emit(led_index, rgb888)

            self.msleep(10)  # 减少等待时间，提高刷新速度

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())