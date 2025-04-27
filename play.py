import sounddevice as sd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import queue

# 定义参数
SAMPLERATE = 44100  # 采样率
BLOCKSIZE = 1024    # 每次读取的帧数

# 创建用于存储音频数据的队列
data_queue = queue.Queue()

# 定义音频回调函数
def audio_callback(indata, frames, time, status):
    if status:
        print(status)
    data_queue.put(indata.copy())  # 将音频数据放入队列

# 初始化图形
fig, ax = plt.subplots()
x = np.arange(0, 2 * BLOCKSIZE, 2)
line, = ax.plot(x, np.random.rand(BLOCKSIZE), '-')  # 初始化线条
ax.set_ylim(-32768, 32767)  # 设置Y轴范围（16位音频）
ax.set_xlim(0, 2 * BLOCKSIZE)  # 设置X轴范围
plt.title("实时扬声器音频波形")
plt.xlabel("样本")
plt.ylabel("振幅")

# 更新图形的函数
def update(frame):
    try:
        # 从队列中获取音频数据
        indata = data_queue.get(timeout=0.1)  # 等待获取数据
        line.set_ydata(indata[:, 0])  # 假设是单通道
    except queue.Empty:
        pass
    return line,

# 创建动画
ani = FuncAnimation(fig, update, interval=10, blit=True)

if __name__ == "__main__":
    try:
        # 启动音频流
        with sd.InputStream(samplerate=SAMPLERATE, channels=1, blocksize=BLOCKSIZE, callback=audio_callback):
            print("正在捕获扬声器音频数据...")
            plt.show()  # 显示图形
    except KeyboardInterrupt:
        print("停止捕获音频...")
    except Exception as e:
        print(f"发生错误: {e}")
