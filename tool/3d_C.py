import sys
import numpy as np
import pyqtgraph as pg
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
                             QComboBox, QPushButton, QLabel)
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QColor

class SerialPlotter(QMainWindow):
    def __init__(self):
        super().__init__()
        
        # 初始化窗口
        self.setWindowTitle("串口数据绘图工具")
        self.setGeometry(100, 100, 800, 600)
        
        # 初始化串口
        self.init_serial_port()
        
        # 初始化 UI
        self.init_ui()
        
        # 定时器，用于定期更新图表
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(100)  # 每100毫秒更新一次
        
        # 存储数据的缓冲区
        self.data_buffer = []
        self.max_data_points = 100  # 最多显示100个数据点
        
        # 颜色列表
        self.colors = [
            QColor(255, 0, 0),     # 红色
            QColor(0, 255, 0),     # 绿色
            QColor(0, 0, 255),     # 蓝色
            QColor(255, 255, 0),   # 黄色
            QColor(255, 0, 255),   # 品红色
            QColor(0, 255, 255),   # 青色
            QColor(255, 165, 0),   # 橙色
            QColor(128, 0, 128),   # 紫色
            QColor(0, 128, 0),     # 深绿色
            QColor(128, 0, 0)      # 深红色
        ]
        
        # 临时缓冲区，用于存储未解析的串口数据
        self.temp_buffer = ""
        
        # 数据名称列表
        self.data_names = []
    
    def init_serial_port(self):
        """初始化串口"""
        self.serial_port = QSerialPort()
        self.serial_port.readyRead.connect(self.read_serial_data)
        
        # 获取可用的串口列表
        self.available_ports = [port.portName() for port in QSerialPortInfo.availablePorts()]
    
    def init_ui(self):
        """初始化用户界面"""
        # 创建主部件和布局
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        main_layout = QVBoxLayout(main_widget)
        
        # 顶部控制面板
        control_panel = QWidget()
        control_layout = QHBoxLayout(control_panel)
        
        # 串口选择
        self.port_combo = QComboBox()
        self.port_combo.addItems(self.available_ports)
        control_layout.addWidget(QLabel("串口:"))
        control_layout.addWidget(self.port_combo)
        
        # 波特率选择
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(['9600', '115200', '19200', '38400', '57600'])
        self.baud_combo.setCurrentIndex(1)  # 默认选择115200
        control_layout.addWidget(QLabel("波特率:"))
        control_layout.addWidget(self.baud_combo)
        
        # 打开/关闭串口按钮
        self.open_button = QPushButton("打开串口")
        self.open_button.clicked.connect(self.toggle_serial_port)
        control_layout.addWidget(self.open_button)
        
        main_layout.addWidget(control_panel)
        
        # 创建图表区域
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setLabels(left='值', bottom='数据点')
        
        # 添加到主布局
        main_layout.addWidget(self.plot_widget)
        
        # 创建曲线对象
        self.plot_curves = []
    
    def toggle_serial_port(self):
        """打开或关闭串口"""
        if self.serial_port.isOpen():
            self.serial_port.close()
            self.open_button.setText("打开串口")
            print("串口已关闭")
        else:
            port_name = self.port_combo.currentText()
            baud_rate = int(self.baud_combo.currentText())
            
            self.serial_port.setPortName(port_name)
            self.serial_port.setBaudRate(baud_rate)
            
            if self.serial_port.open(QSerialPort.ReadWrite):
                self.open_button.setText("关闭串口")
                print(f"成功打开串口 {port_name}")
            else:
                print(f"无法打开串口 {port_name}")
    
    def read_serial_data(self):
        """读取串口数据"""
        # 读取所有可用数据
        while self.serial_port.bytesAvailable() > 0:
            data = self.serial_port.readAll().data().decode('utf-8', errors='ignore').strip()
            self.temp_buffer += data  # 将读取到的数据添加到临时缓冲区
            
            # 尝试从缓冲区中解析完整数据包
            self.parse_data_packets()
    
    def parse_data_packets(self):
        """从缓冲区中解析完整数据包"""
        while 'S' in self.temp_buffer and 'E' in self.temp_buffer[self.temp_buffer.index('S'):]:
            # 找到第一个 'S' 的位置
            start_index = self.temp_buffer.index('S')
            
            # 找到 'S' 之后的第一个 'E' 的位置
            end_index = self.temp_buffer.index('E', start_index)
            
            # 提取数据包
            packet = self.temp_buffer[start_index:end_index + 1]
            
            # 从缓冲区中移除已处理的数据
            self.temp_buffer = self.temp_buffer[end_index + 1:]
            
            # 解析数据包
            self.parse_data_packet(packet)
    
    def parse_data_packet(self, packet):
        """解析单个数据包"""
        # 去掉包头和包尾
        data_str = packet[1:-1]
        
        # 按逗号分隔数据
        data_points = data_str.split(',')
        
        # 解析数据名称和值
        data_dict = {}
        for point in data_points:
            if ':' in point:
                key, value = point.split(':', 1)
                key = key.strip()
                value = value.strip()
                try:
                    data_dict[key] = float(value)
                except ValueError:
                    print(f"无法解析数据点: {point}")
        
        if data_dict:
            self.data_buffer.append(data_dict)
            
            # 更新数据名称列表
            for key in data_dict.keys():
                if key not in self.data_names:
                    self.data_names.append(key)
            
            # 限制数据缓冲区大小
            if len(self.data_buffer) > self.max_data_points:
                self.data_buffer.pop(0)
    
    def update_plot(self):
        """更新图表"""
        if not self.data_buffer:
            return
        
        # 清空现有曲线
        self.plot_widget.clear()
        
        # 如果有多个信号（数据点）
        if self.data_buffer:
            # 获取所有数据点，按信号分组
            data_dict_list = self.data_buffer
            
            # 准备存储每个信号的数据
            y_values_dict = {name: [] for name in self.data_names}
            
            for data_dict in data_dict_list:
                for name in self.data_names:
                    y_values_dict[name].append(data_dict.get(name, 0.0))
            
            for signal_idx, name in enumerate(self.data_names):
                y_values = y_values_dict[name]
                x_values = list(range(len(y_values)))
                
                # 使用不同的颜色为每个信号绘制曲线
                color = self.colors[signal_idx % len(self.colors)]
                
                # 确保数据是 NumPy 数组
                x_array = np.array(x_values)
                y_array = np.array(y_values)
                
                curve = pg.PlotCurveItem(x_array, y_array, pen=color)
                self.plot_widget.addItem(curve)
                
                # 添加图例
                if len(self.plot_curves) > signal_idx:
                    self.plot_widget.removeItem(self.plot_curves[signal_idx])
                self.plot_curves.append(curve)
            
            # 添加图例
            legend = self.plot_widget.addLegend()
            for curve, name in zip(self.plot_curves, self.data_names):
                legend.addItem(curve, name)
    
if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    # 启用 pyqtgraph 的反锯齿绘制
    pg.setConfigOptions(antialias=True)
    
    window = SerialPlotter()
    window.show()
    sys.exit(app.exec_())