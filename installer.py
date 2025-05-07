# -*- coding: utf-8 -*-
"""
将 .py 文件打包成 .exe（带自动 PNG 转 ICO 功能）
"""
import os
import subprocess
import sys
from PIL import Image

def png_to_ico(png_path, ico_path):
    """将 PNG 文件转换为 ICO 文件"""
    img = Image.open(png_path)
    # 调整大小为 256x256，因为 ICO 格式支持的最大尺寸是 256x256
    img = img.resize((256, 256), Image.Resampling.LANCZOS)
    img.save(ico_path)
    print(f"Converted {png_path} to {ico_path}")

def create_exe_with_icon():
    # 你的 Python 文件路径
    python_file = "RGB.py"
    # 你的 PNG 图标文件路径
    png_icon_file = "BanGO.png"
    # 生成的 ICO 图标文件路径
    ico_icon_file = "BanGO.ico"

    # 将 PNG 转换为 ICO
    png_to_ico(png_icon_file, ico_icon_file)

    # 打包命令
    command = f"pyinstaller -F -w --icon={ico_icon_file} {python_file}"

    # 执行打包命令
    subprocess.call(command, shell=True)

    # 删除临时生成的 ICO 文件（可选）
    os.remove(ico_icon_file)
    print("Cleaned up temporary ICO file")

if __name__ == "__main__":
    create_exe_with_icon()