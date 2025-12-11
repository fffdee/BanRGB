import serial
import serial.tools.list_ports
import time
import configparser
from PyQt6.QtCore import QThread, pyqtSignal, QObject, QTimer

# 环形缓冲区类定义
class CircularBuffer:
    def __init__(self, size=1024):
        self.size = size
        self.buffer = [None] * size
        self.head = 0
        self.tail = 0
        self.count = 0

    def is_full(self):
        return self.count == self.size

    def is_empty(self):
        return self.count == 0

    def enqueue(self, item):
        """向缓冲区添加数据，缓冲区满时覆盖最旧的数据"""
        if self.is_full():
            self.head = (self.head + 1) % self.size
        else:
            self.count += 1
        self.buffer[self.tail] = item
        self.tail = (self.tail + 1) % self.size

    def dequeue(self):
        """从缓冲区读取数据"""
        if self.is_empty():
            return None
        item = self.buffer[self.head]
        self.head = (self.head + 1) % self.size
        self.count -= 1
        return item

    def read_all(self):
        """读取缓冲区中所有数据"""
        result = []
        while not self.is_empty():
            item = self.dequeue()
            if item:
                result.append(item)
        return ''.join(result)


class SerialThread(QThread):
    """串口通信线程类，增强异常处理和资源释放，支持主动握手"""
    
    data_received = pyqtSignal(str, str)  # 信号：(端口, 数据)
    thread_finished = pyqtSignal(str)     # 信号：线程结束
    connection_status = pyqtSignal(str, bool)  # 信号：(端口, 连接状态)
    serial_error = pyqtSignal(str, int, str)  # 串口错误信号
    debug_log = pyqtSignal(str)  # 调试日志信号
    handshake_response = pyqtSignal(str, str)  # 信号：(端口, 握手响应)
    
    # 错误码定义
    ERROR_OPEN_FAILED = 1001
    ERROR_PORT_BUSY = 1002
    ERROR_PERMISSION = 1003
    ERROR_PORT_NOT_FOUND = 1004
    ERROR_HANDSHAKE_TIMEOUT = 1005
    ERROR_HANDSHAKE_FAILED = 1006
    
    def __init__(self, port, baudrate, commands, auto_connect, debug=False, handshake_cmd="E", handshake_timeout=1.0):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.commands = commands
        self.debug = debug
        self.serial_port = None
        self.running = False
        self.last_heartbeat_time = time.time()
        self.connection_attempts = 0
        self.response_buffer = CircularBuffer()
        self.exit_requested = False
        self.exception_occurred = False
        self.port_opened = False
        self.auto_connect = auto_connect
        self.handshake_cmd = handshake_cmd  # 主动握手命令（默认为'E'）
        self.handshake_timeout = handshake_timeout  # 握手超时时间（秒）
        self.handshake_success = False  # 握手是否成功
        
    def run(self):
        """线程运行函数，增强异常处理和资源管理，支持主动握手机制"""
        try:
            self.connection_attempts += 1
            debug_msg = f"[debug] 尝试连接串口 {self.port} (第 {self.connection_attempts} 次)"
            if self.debug:
                print(debug_msg)
            
            # 打开串口
            self.serial_port = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=0.5,  # 缩短超时时间以加快握手检测
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                write_timeout=1
            )
            
            self.port_opened = True
            debug_msg = f"[debug] 串口 {self.port} 已打开，准备握手"
            if self.debug:
                print(debug_msg)
            
            # 清空串口缓冲区
            time.sleep(0.1)
            if self.serial_port.in_waiting > 0:
                self.serial_port.read(self.serial_port.in_waiting)
            
            # 主动发送握手命令
            if self.handshake_cmd:
                debug_msg = f"[debug] 向 {self.port} 发送握手命令: {repr(self.handshake_cmd)}"
                if self.debug:
                    print(debug_msg)
                self.serial_port.write(self.handshake_cmd.encode('utf-8'))
                
                # 等待握手响应
                handshake_response = ""
                start_time = time.time()
                
                while time.time() - start_time < self.handshake_timeout:
                    if self.serial_port.in_waiting > 0:
                        data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='replace')
                        handshake_response += data
                        debug_msg = f"[debug] {self.port} 握手响应: {repr(data)}"
                        if self.debug:
                            print(debug_msg)
                        
                        # 检查是否收到有效响应（'BANRGB_OK'）
                        if 'BANRGB_OK' in handshake_response:
                            self.handshake_success = True
                            debug_msg = f"[debug] {self.port} 握手成功！响应: {repr(handshake_response)}"
                            if self.debug:
                                print(debug_msg)
                            self.handshake_response.emit(self.port, handshake_response)
                            break
                    time.sleep(0.05)
                
                # 握手失败处理
                if not self.handshake_success:
                    debug_msg = f"[debug] {self.port} 握手超时或失败，响应: {repr(handshake_response)}"
                    if self.debug:
                        print(debug_msg)
                    self.serial_error.emit(self.port, self.ERROR_HANDSHAKE_TIMEOUT, 
                                         f"握手超时 (响应: {handshake_response[:50]})")
                    self.close_port()
                    return
            
            # 握手成功后继续正常通信
            self.running = True
            self.connection_status.emit(self.port, True)
            debug_msg = f"[debug] {self.port} 进入正常通信模式"
            if self.debug:
                print(debug_msg)
            
            # 发送初始化命令（如果有）
            for cmd in self.commands:
                if cmd:  # 只发送非空命令
                    debug_msg = f"[debug] 向 {self.port} 发送初始化命令: {cmd}"
                    if self.debug:
                        print(debug_msg)
                    self.serial_port.write(cmd.encode('utf-8'))
                    time.sleep(0.05)
                
            # 进入数据接收循环
            while self.running and not self.exit_requested and not self.exception_occurred:
                try:
                    if self.serial_port and self.serial_port.is_open:
                        if self.serial_port.in_waiting > 0:
                            data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='replace')
                            for char in data:
                                self.response_buffer.enqueue(char)
                            self.data_received.emit(self.port, data)
                            debug_msg = f"[debug] 从 {self.port} 接收到数据: {repr(data)}"
                            if self.debug:
                                print(debug_msg)
                            self.last_heartbeat_time = time.time()
                except serial.SerialException as e:
                    debug_msg = f"[debug] 串口读取错误: {str(e)}"
                    if self.debug:
                        print(debug_msg)
                    error_code, error_msg = self._get_error_info(e)
                    self.serial_error.emit(self.port, error_code, error_msg)
                    self.connection_status.emit(self.port, False)
                    self.running = False
                    self.exception_occurred = True
                    break
                except Exception as e:
                    debug_msg = f"[debug] 数据处理异常: {str(e)}"
                    if self.debug:
                        print(debug_msg)
                    self.running = False
                    self.exception_occurred = True
                    break
                    
                time.sleep(0.05)
                
        except serial.SerialException as e:
            error_code, error_msg = self._get_error_info(e)
            debug_msg = f"[debug] 串口 {self.port} 打开错误: {error_msg}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            
            self.serial_error.emit(self.port, error_code, error_msg)
            self.connection_status.emit(self.port, False)
            self.port_opened = False
          
        except Exception as e:
            debug_msg = f"[debug] 串口线程异常: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            self.connection_status.emit(self.port, False)
            self.port_opened = False
        finally:
            self.close_port()
            self.thread_finished.emit(self.port)
            
    def _get_error_info(self, exception):
        error_msg = str(exception)
        error_code = self.ERROR_OPEN_FAILED
        
        if "port" in error_msg and "not found" in error_msg:
            error_code = self.ERROR_PORT_NOT_FOUND
            error_msg = "端口不存在或未连接设备"
        elif "permission" in error_msg.lower():
            error_code = self.ERROR_PERMISSION
            error_msg = "权限不足，无法访问串口"
        elif "busy" in error_msg.lower():
            error_code = self.ERROR_PORT_BUSY
            error_msg = "端口被其他程序占用"
            
        return error_code, error_msg
    
    def close_port(self):
        self.exit_requested = True
        self.running = False
        
        if self.serial_port:
            try:
                if self.serial_port.is_open:
                    self.serial_port.flush()
                    self.serial_port.close()
                    debug_msg = f"[debug] 串口 {self.port} 已关闭"
                    if self.debug:
                        print(debug_msg)
                        # self.debug_log.emit(debug_msg)
            except Exception as e:
                debug_msg = f"[debug] 关闭串口异常: {str(e)}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
            finally:
                self.serial_port = None
                self.port_opened = False
    
    def is_alive(self):
        if not self.port_opened or self.exit_requested or self.exception_occurred:
            return False
        
        try:
            current_time = time.time()
            if current_time - self.last_heartbeat_time > 3.0:
                if self.serial_port and self.serial_port.is_open:
                    self.serial_port.write("F00C00".encode('utf-8'))
                    time.sleep(0.1)
                    if self.serial_port.in_waiting > 0:
                        self.serial_port.read(self.serial_port.in_waiting)
                    return True
                return False
            return True
        except Exception as e:
            debug_msg = f"[debug] 检查连接状态异常: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            return False


class SerialPortAutoConnect(QObject):
    """串口自动连接类，支持主动握手机制"""
    
    connected = pyqtSignal(str)          
    disconnected = pyqtSignal(str)       
    data_available = pyqtSignal(str)     
    debug_log = pyqtSignal(str)           # 调试日志信号
    error_occurred = pyqtSignal(str, int, str)
    
    def __init__(self, config_file):
        super().__init__()
        self.config_file = config_file
        self.config = configparser.ConfigParser()
        self.debug = True
        self.threads = {}
        self.connected_port = None
        self.match_command = "E"  # 默认握手响应标识
        self.heartbeat_command = ""
        self.heartbeat_timer = QTimer()
        self.retry_timer = QTimer()
        self.retry_attempts = 0
        self.max_retry_attempts = 0
        self.connection_check_timer = QTimer()
        self.global_buffer = CircularBuffer()
        self.baudrate = 115200
        self.conn_command = []  # 初始化命令
        self.heartbeat_flag = False
        self.reconnect_delay = 2
        self.heartbeat_count = 0
        self.handshake_cmd = "E"  # 主动握手命令（默认为'E'）
        self.handshake_timeout = 1.0  # 握手超时时间（秒）
        self.get_config()
        self.get_match_command()
        self.get_heartbeat_command()
        self._get_debug_mode()
        self._get_handshake_config()  # 读取握手配置
        self.retry_timer.timeout.connect(self._retry_connect)
    
    def _get_debug_mode(self):
        try:
            self.config.read(self.config_file)
            if 'Settings' in self.config and 'sac_debug' in self.config['Settings']:
                self.debug = bool(int(self.config['Settings']['sac_debug']))
                debug_msg = "[debug] 调试模式已启用"
                if self.debug:
                    print(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 读取调试模式错误: {str(e)}"
            if self.debug:
                print(debug_msg)
    
    def _get_handshake_config(self):
        """读取握手配置"""
        try:
            self.config.read(self.config_file)
            if 'Handshake' in self.config:
                self.handshake_cmd = self.config['Handshake'].get('command', 'BANRGB?\r\n').strip()
                self.handshake_timeout = self.config['Handshake'].getfloat('timeout', 1.0)
                debug_msg = f"[debug] 读取握手配置 - 命令: {repr(self.handshake_cmd)}, 超时: {self.handshake_timeout}s"
                if self.debug:
                    print(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 读取握手配置错误: {str(e)}，使用默认值"
            if self.debug:
                print(debug_msg)
    
    def get_config(self):
        try:
            self.config.read(self.config_file)
            if 'ConnCommand' in self.config:
                # 安全地构建命令列表
                command1 = self.config['ConnCommand'].get('command1', 'F00E01').strip()
                command2 = self.config['ConnCommand'].get('command2', 'F00C00').strip()
                
                self.conn_command = [command1, command2]
                
                debug_msg = f"[debug] 读取到连接命令: {self.conn_command}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 读取配对命令错误: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
    
    def get_match_command(self):
        try:
            self.config.read(self.config_file)
            if 'MatchCommand' in self.config and 'command' in self.config['MatchCommand']:
                self.match_command = self.config['MatchCommand']['command'].strip()
                debug_msg = f"[debug] 读取到配对命令: {self.match_command}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 读取配对命令错误: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
    
    def get_heartbeat_command(self):
        try:
            self.config.read(self.config_file)
            if 'Heartbeat' in self.config and 'command' in self.config['Heartbeat']:
                self.heartbeat_command = self.config['Heartbeat']['command'].strip()
                debug_msg = f"[debug] 读取到心跳命令: {self.heartbeat_command}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 读取心跳命令错误: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
    
    def get_all_possible_ports(self):
        ports = []
        try:
            for port in serial.tools.list_ports.comports():
                ports.append(port.device)
            debug_msg = f"[debug] 找到以下串口: {ports}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
        except Exception as e:
            debug_msg = f"[debug] 获取串口列表错误: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
        return ports
    
    def auto_connect(self, retry_mode=True, retry_interval=5000, heartbeat_interval=3000):
        debug_msg = "[debug] 开始自动连接串口..."
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
        
        self.retry_mode = retry_mode
        self.retry_interval = retry_interval
        self.heartbeat_interval = heartbeat_interval
        self.retry_attempts = 0
        
        if retry_mode:
             self._start_connect()
    
    def _start_connect(self):
        debug_msg = "[debug] 开始连接流程（主动握手模式）"
        if self.debug:
            print(debug_msg)
        
        try:
            self._safe_close_all_ports()
            ports = self.get_all_possible_ports()
            
            if not ports:
                debug_msg = "[debug] 未找到任何可用串口"
                if self.debug:
                    print(debug_msg)
                return
            
            for port in ports:
                if port not in self.threads:
                    debug_msg = f"[debug] 创建线程连接 {port}，准备主动握手"
                    if self.debug:
                        print(debug_msg)
                    
                    # 创建线程时传入握手配置
                    thread = SerialThread(
                        port, 
                        self.baudrate, 
                        self.conn_command, 
                        self, 
                        self.debug,
                        handshake_cmd=self.handshake_cmd,
                        handshake_timeout=self.handshake_timeout
                    )
                    
                    # 连接握手响应信号
                    thread.handshake_response.connect(self.on_handshake_response)
                    thread.data_received.connect(self.on_data_received)
                    thread.thread_finished.connect(self.on_thread_finished)
                    thread.serial_error.connect(self.on_serial_out)
                    thread.debug_log.connect(self.debug_log.emit)
                    
                    self.threads[port] = thread
                    thread.start()
            
            debug_msg = f"[debug] 已尝试向 {len(ports)} 个串口发起握手"
            if self.debug:
                print(debug_msg)
                
        except Exception as e:
            debug_msg = f"[debug] 连接流程异常: {str(e)}"
            if self.debug:
                print(debug_msg)
                
        if self.connected_port is None:
            debug_msg = f"[debug] [_start_connect] 未找到可用串口,启动重连"
            if self.debug:
                print(debug_msg)
            self.retry_timer.start(self.retry_interval)
        
    def _retry_connect(self):
        self.retry_attempts += 1
        if self.retry_mode:
            debug_msg = f"[debug] 正在重试连接串口... (尝试 {self.retry_attempts}/{self.max_retry_attempts or '∞'})"
            if self.debug:
                print(debug_msg)
            self._start_connect()
        else:
            debug_msg = "[debug] 已达到最大重试次数，停止重试"
            if self.debug:
                print(debug_msg)
            self.retry_timer.stop()

    def on_handshake_response(self, port, response):
        """处理握手响应，只有收到正确响应才建立连接"""
        debug_msg = f"[debug] 收到 {port} 的握手响应: {repr(response)}"
        if self.debug:
            print(debug_msg)
        
        # 检查响应是否包含正确的握手响应（BANRGB_OK）
        if 'BANRGB_OK' in response:
            debug_msg = f"[debug] {port} 握手成功，收到正确响应: BANRGB_OK"
            if self.debug:
                print(debug_msg)
            
            # 如果已经有连接的端口，先关闭其他端口
            if self.connected_port and self.connected_port != port:
                debug_msg = f"[debug] 已有连接 {self.connected_port}，关闭其他端口"
                if self.debug:
                    print(debug_msg)
                self.close_all_ports()
            
            # 建立新连接
            self.connected_port = port
            self.connected.emit(port)
            self.retry_timer.stop()
            
            # 关闭其他线程
            for p, thread in list(self.threads.items()):
                if p != port:
                    thread.close_port()
                    if p in self.threads:
                        del self.threads[p]
            
            self.retry_attempts = 0
            self.heartbeat_flag = True
            self._start_heartbeat()
            
            # 清空全局缓冲区
            self.global_buffer = CircularBuffer()
            debug_msg = "[debug] 全局缓冲区已清空，连接建立完成"
            if self.debug:
                print(debug_msg)
        else:
            debug_msg = f"[debug] {port} 握手响应不匹配，期望: BANRGB_OK，实际收到: {repr(response)}"
            if self.debug:
                print(debug_msg)
    
    def on_data_received(self, port, data):
        """处理接收到的数据，不再进行被动匹配（已改为主动握手）"""
        self.data_available.emit(data)
        
        for char in data:
            self.global_buffer.enqueue(char)
        
        debug_msg = f"[debug] 从 {port} 收到数据，全局缓冲区: {self.global_buffer.count}/{self.global_buffer.size}"
        if self.debug:
            print(debug_msg)
    
    def on_serial_out(self, port, error_code, error_msg):
        debug_msg = f"[debug] 接收到串口错误 - 端口: {port}, 错误码: {error_code}, 信息: {error_msg}"
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
        self.retry_timer.start(self.retry_interval)
        self.connected_port = None
        debug_msg = "[debug] 检测到串口断开，准备重连"
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
        self.error_occurred.emit(port, error_code, error_msg)
        self.disconnected.emit(port)
    
    def _start_heartbeat(self):
        if not self.heartbeat_command:
            debug_msg = "[debug] 心跳命令未设置，无法启动心跳检测"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            return
            
        if self.connected_port:
            try:
                self.config.read(self.config_file)
                heartbeat_interval = self.config['Heartbeat'].getint('interval', 5000)
            except:
                heartbeat_interval = 5000
                
            debug_msg = f"[debug] 启动心跳检测，间隔: {heartbeat_interval}ms，命令: {self.heartbeat_command}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            
            try:
                self.heartbeat_timer.timeout.disconnect()
            except:
                pass
                
            self.heartbeat_timer.timeout.connect(self._heartbeat)
            self.heartbeat_timer.start(heartbeat_interval)
        else:
            debug_msg = "[debug] 未连接到串口，无法启动心跳检测"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
    
    def _heartbeat(self):
        if not self.connected_port or self.connected_port not in self.threads:
            debug_msg = f"[debug] 未连接到串口，心跳检测失败"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            self._reconnect()
            return
            
        try:
            debug_msg = f"[debug] 发送心跳命令到 {self.connected_port}: {self.heartbeat_command}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            
            response = self.send_data(self.heartbeat_command)
            print(response)
            if response and "F10C" in response:
                debug_msg = f"[debug] 心跳检测成功，收到F10C响应: {response}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
                self.heartbeat_count = 0
                return
                
            if response:
                debug_msg = f"[debug] 心跳检测收到非F10C响应: {response}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
            else:
                debug_msg = f"[debug] 心跳检测无响应"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
            
            raise Exception("心跳检测失败，未收到F10C响应")
            
        except Exception as e:
            self.heartbeat_count += 1
            debug_msg = f"[debug] 心跳检测错误 {self.heartbeat_count} 次: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            

        if self.heartbeat_count  >= 3:   
            self._reconnect()
    
    def _reconnect(self):
        debug_msg = f"[debug] 连接已断开，准备重新连接..."
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
        
        disconnected_port = self.connected_port
        self.connected_port = None
        self.disconnected.emit(disconnected_port)
        
        try:
            self.heartbeat_timer.stop()
            self.heartbeat_timer.timeout.disconnect()
        except Exception as e:
            debug_msg = f"[debug] 停止心跳检测异常: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
        
        self._safe_close_all_ports()
        

        if self.retry_mode:
            debug_msg = f"[debug] [reconnect]..."
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            self.retry_timer.start(self.retry_interval)
    
    def on_thread_finished(self, port):
        if port in self.threads:
            debug_msg = f"[debug] 线程 {port} 已结束，从列表中移除"
            
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            del self.threads[port]
        print(f"[debug] 串口： {self.connected_port}") 
        if port == self.connected_port:
            debug_msg = f"[debug] [on_thread_finished] 已连接端口 {port} 的线程结束，准备重连"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            self.retry_timer.start(self.retry_interval)
    
    def _safe_close_all_ports(self):
        debug_msg = "[debug] 开始安全关闭所有端口"
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
        
        threads_to_close = list(self.threads.values())
        
        for thread in threads_to_close:
            try:
                if thread.isRunning():
                    debug_msg = f"[debug] 尝试正常关闭线程: {thread.port}"
                    if self.debug:
                        print(debug_msg)
                        # self.debug_log.emit(debug_msg)
                    thread.exit_requested = True
                    
                    time.sleep(0.1)
                    if thread.isRunning():
                        debug_msg = f"[debug] 线程 {thread.port} 未在5秒内结束，强制终止"
                        if self.debug:
                            print(debug_msg)
                            # self.debug_log.emit(debug_msg)
                        try:
                            thread.terminate()
                            thread.wait(1000)
                        except Exception as e:
                            debug_msg = f"[debug] 强制终止线程异常: {str(e)}"
                            if self.debug:
                                print(debug_msg)
                                # self.debug_log.emit(debug_msg)
                        
            except Exception as e:
                debug_msg = f"[debug] 关闭线程异常: {str(e)}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
        
        try:
            self.heartbeat_timer.stop()
            self.retry_timer.stop()
            self.connection_check_timer.stop()
        except Exception as e:
            debug_msg = f"[debug] 停止定时器异常: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
        
        debug_msg = "[debug] 所有端口已安全关闭"
        if self.debug:
            print(debug_msg)
            # self.debug_log.emit(debug_msg)
    
    def close_all_ports(self):
        self._safe_close_all_ports()
    
    def send_data(self, command):
        if not self.connected_port or self.connected_port not in self.threads:
            debug_msg = "[debug] 没有连接的串口，无法发送数据"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            return None
        
        try:
            thread = self.threads[self.connected_port]
            if not thread.serial_port or not thread.serial_port.is_open:
                debug_msg = f"[debug] 串口 {self.connected_port} 未打开，无法发送数据"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
                return None
                
            debug_msg = f"[debug] 发送命令: {command} 到 {self.connected_port}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            
            thread.serial_port.write(command.encode('utf-8'))
            
            response = ""
            time.sleep(0.01)
                
            if thread.serial_port.in_waiting > 0:
                data = thread.serial_port.read(thread.serial_port.in_waiting).decode('utf-8', errors='replace')
                response += data

            if response:
                debug_msg = f"[debug] 接收到响应: {response}"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
                return response
            else:
                debug_msg = "[debug] 发送数据后无响应"
                if self.debug:
                    print(debug_msg)
                    # self.debug_log.emit(debug_msg)
                return None
                
        except Exception as e:
            debug_msg = f"[debug] 发送数据错误: {str(e)}"
            if self.debug:
                print(debug_msg)
                # self.debug_log.emit(debug_msg)
            
        return None
    
    def read_data(self, size=None):
        if size is None:
            return self.global_buffer.read_all()
        
        result = []
        for _ in range(size):
            data = self.global_buffer.dequeue()
            if data is not None:
                result.append(data)
            else:
                break
        return ''.join(result)
    
    def read_line(self, delimiter='\n'):
        result = []
        while not self.global_buffer.is_empty():
            char = self.global_buffer.dequeue()
            result.append(char)
            if char == delimiter:
                break
        return ''.join(result)
    
    def get_buffer_size(self):
        return self.global_buffer.count
    
    def is_connected(self):
        return self.connected_port is not None