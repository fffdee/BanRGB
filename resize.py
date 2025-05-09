from PIL import ImageGrab
from PIL import Image

def capture_screen():
    # 获取整个屏幕的截图
    screen = ImageGrab.grab()
    
    # 调整大小为 20x10
    resized_screen = screen.resize((20, 10), Image.Resampling.LANCZOS)
    
    # 保存截图
    resized_screen.save('screen_capture.png')
    return resized_screen

if __name__ == "__main__":
    captured_image = capture_screen()
    print("Screen captured and saved as 'screen_capture.png'")