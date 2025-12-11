"""
测试主动握手功能
用于验证 SerialAutoConn.py 的主动握手机制是否正常工作
"""

import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QTextEdit, QVBoxLayout, QWidget
from PyQt6.QtCore import QTimer
from SerialAutoConn import SerialPortAutoConnect

class HandshakeTestWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BanRGB 主动握手测试")
        self.setGeometry(100, 100, 800, 600)
        
        # 创建日志显示区域
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setStyleSheet("""
            QTextEdit {
                background-color: #1a1a1a;
                color: #00ff00;
                font-family: 'Consolas', 'Courier New', monospace;
                font-size: 10pt;
                border: 2px solid #00ff00;
                padding: 5px;
            }
        """)
        
        # 创建布局
        layout = QVBoxLayout()
        layout.addWidget(self.log_text)
        
        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)
        
        # 创建自动连接实例
        self.serial_conn = SerialPortAutoConnect('config.ini')
        
        # 连接信号
        self.serial_conn.connected.connect(self.on_connected)
        self.serial_conn.disconnected.connect(self.on_disconnected)
        self.serial_conn.data_available.connect(self.on_data_available)
        self.serial_conn.error_occurred.connect(self.on_error)
        
        # 添加日志
        self.add_log("=" * 80)
        self.add_log("BanRGB V2.0 主动握手测试工具")
        self.add_log("=" * 80)
        self.add_log(f"握手命令: {repr(self.serial_conn.handshake_cmd)}")
        self.add_log(f"握手超时: {self.serial_conn.handshake_timeout}s")
        self.add_log(f"匹配标识: {self.serial_conn.match_command}")
        self.add_log(f"波特率: {self.serial_conn.baudrate}")
        self.add_log("=" * 80)
        self.add_log("")
        
        # 延迟启动自动连接
        QTimer.singleShot(500, self.start_auto_connect)
    
    def add_log(self, message):
        """添加日志到显示区域"""
        self.log_text.append(message)
        # 自动滚动到底部
        cursor = self.log_text.textCursor()
        cursor.movePosition(cursor.MoveOperation.End)
        self.log_text.setTextCursor(cursor)
    
    def start_auto_connect(self):
        """启动自动连接"""
        self.add_log("[INFO] 开始扫描并主动握手...")
        self.add_log("")
        self.serial_conn.auto_connect(retry_mode=True, retry_interval=5000, heartbeat_interval=3000)
    
    def on_connected(self, port):
        """连接成功回调"""
        self.add_log("")
        self.add_log("=" * 80)
        self.add_log(f"✓ 握手成功！已连接到: {port}")
        self.add_log("=" * 80)
        self.add_log("")
    
    def on_disconnected(self, port):
        """断开连接回调"""
        self.add_log("")
        self.add_log("=" * 80)
        self.add_log(f"✗ 连接断开: {port}")
        self.add_log("=" * 80)
        self.add_log("")
    
    def on_data_available(self, data):
        """数据接收回调"""
        self.add_log(f"[DATA] {repr(data)}")
    
    def on_error(self, port, error_code, error_msg):
        """错误回调"""
        self.add_log(f"[ERROR] 端口: {port}, 错误码: {error_code}, 信息: {error_msg}")
    
    def closeEvent(self, event):
        """窗口关闭事件"""
        self.add_log("")
        self.add_log("[INFO] 正在关闭连接...")
        self.serial_conn.close_all_ports()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    # 设置应用样式
    app.setStyle('Fusion')
    
    window = HandshakeTestWindow()
    window.show()
    
    sys.exit(app.exec())
