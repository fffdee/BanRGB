import sys
import threading
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QGridLayout, QWidget
from PyQt5.QtGui import QGuiApplication, QColor, QPixmap, QImage
from PyQt5.QtCore import QTimer, Qt, pyqtSignal, QObject
from concurrent.futures import ThreadPoolExecutor
import random

class Worker(QObject):
    update_colors = pyqtSignal(list)

    def __init__(self):
        super().__init__()
        self.executor = ThreadPoolExecutor(max_workers=8)  # 创建一个线程池，最多8个线程

    def run(self):
        screen_geometry = QGuiApplication.primaryScreen().geometry()
        screen_width = screen_geometry.width()
        screen_height = screen_geometry.height()

        # Divide the screen into 64 regions (8x8 grid)
        region_width = screen_width // 8
        region_height = screen_height // 8

        futures = []
        for i in range(8):
            for j in range(8):
                x = j * region_width
                y = i * region_height
                future = self.executor.submit(self.process_region, x, y, region_width, region_height)
                futures.append(future)

        colors = [future.result() for future in futures]
        self.update_colors.emit(colors)

    def process_region(self, x, y, region_width, region_height):
        pixmap = QGuiApplication.primaryScreen().grabWindow(0, x, y, region_width, region_height)
        
        image = pixmap.toImage()

        total_red = 0
        total_green = 0
        total_blue = 0
        pixel_count = 0

        for y in range(random.randint(20, region_height)):
            for x in range(random.randint(20, region_width)):
                color = QColor(image.pixel(x, y))
                total_red += color.red()
                total_green += color.green()
                total_blue += color.blue()
                pixel_count += 1

        if pixel_count > 0:
            avg_red = total_red // pixel_count
            avg_green = total_green // pixel_count
            avg_blue = total_blue // pixel_count
        else:
            avg_red = 0
            avg_green = 0
            avg_blue = 0

        return (avg_red, avg_green, avg_blue)


class ColorPicker(QMainWindow):
    def __init__(self):
        super().__init__()
        self.led_labels = []  # 初始化 led_labels 为一个空列表
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Screen Color Picker')
        self.setGeometry(100, 100, 200, 150)

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.grid_layout = QGridLayout()
        self.central_widget.setLayout(self.grid_layout)

        # 创建 64 个 LED 灯
        for i in range(64):
            label = QLabel()
            label.setFixedSize(10, 10)
            label.setStyleSheet("background-color: black;")
            self.grid_layout.addWidget(label, i // 8, i % 8)
            self.led_labels.append(label)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.updateColors)
        self.timer.start(60)  # Update every 200ms，减少更新频率

        self.worker = Worker()
        self.worker.update_colors.connect(self.update_leds)

    def updateColors(self):
        if not hasattr(self, 'worker_thread') or not self.worker_thread.is_alive():
            self.worker_thread = threading.Thread(target=self.worker.run)
            self.worker_thread.start()

    def update_leds(self, colors):
        for i, color in enumerate(colors):
            self.led_labels[i].setStyleSheet(f"background-color: rgb({color[0]}, {color[1]}, {color[2]});")
        print("ok")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    picker = ColorPicker()
    picker.show()
    sys.exit(app.exec_())