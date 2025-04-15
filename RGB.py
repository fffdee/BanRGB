import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QGridLayout
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage
from PyQt5.QtCore import QTimer, Qt

# 假设你有一个函数来控制 RGB 灯带
def set_led_color(led_index, color):
    # 这里应该是你的代码，用于设置第 led_index 个 LED 的颜色
    # 例如，通过串口发送数据到你的硬件设备
    print(f"LED {led_index}: {color}")

class ColorPicker(QMainWindow):
    def __init__(self):
        super().__init__()
        self.led_labels = []  # 初始化 led_labels 为一个空列表
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Screen Color Picker')
        self.setGeometry(100, 100, 640, 480)

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.grid_layout = QGridLayout()
        self.central_widget.setLayout(self.grid_layout)

        # 创建 8x8 的 LED 网格
        for i in range(8):
            for j in range(8):
                label = QLabel()
                label.setFixedSize(50, 50)
                label.setStyleSheet("background-color: black;")
                self.grid_layout.addWidget(label, i, j)
                self.led_labels.append(label)  # 将 label 添加到 led_labels 列表中

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.updateColors)
        self.timer.start(500)  # Update every 1000ms

    def updateColors(self):
        screen_geometry = QGuiApplication.primaryScreen().geometry()
        screen_width = screen_geometry.width()
        screen_height = screen_geometry.height()
        print("screen_width:",screen_width," screen_height:",screen_height)
        # Divide the screen into 64 regions (8x8 grid)
        region_width = screen_width // 8
        region_height = screen_height // 8

        for i in range(8):
            for j in range(8):
                x = j * region_width
                y = i * region_height
                pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
                image = pixmap.toImage()

                # Calculate the average color of the region
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
                print(avg_red,avg_green,avg_blue)
                rgb888 = (avg_red, avg_green, avg_blue)
                led_index = i * 8 + j
                # set_led_color(led_index, rgb888)

                # 更新 GUI 中的 LED 颜色
                self.led_labels[led_index].setStyleSheet(f"background-color: rgb({avg_red}, {avg_green}, {avg_blue});")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())