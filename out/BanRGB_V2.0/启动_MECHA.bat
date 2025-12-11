@echo off
chcp 65001 >nul
title BanRGB V2.0 MECHA EDITION
color 0A

echo.
echo ╔══════════════════════════════════════════════════╗
echo ║   ◢ BAN·RGB MECHA CONTROL SYSTEM ◣             ║
echo ╚══════════════════════════════════════════════════╝
echo.
echo [INFO] 正在检查 Python 环境...
python --version
if errorlevel 1 (
    echo [ERROR] Python 未安装或未添加到 PATH
    pause
    exit /b 1
)

echo.
echo [INFO] 正在检查依赖库...
python -c "import PyQt6; print('[OK] PyQt6 已安装')" 2>nul
if errorlevel 1 (
    echo [WARN] PyQt6 未安装，正在安装...
    pip install PyQt6
)

python -c "import serial; print('[OK] pyserial 已安装')" 2>nul
if errorlevel 1 (
    echo [WARN] pyserial 未安装，正在安装...
    pip install pyserial
)

python -c "from PIL import Image; print('[OK] Pillow 已安装')" 2>nul
if errorlevel 1 (
    echo [WARN] Pillow 未安装，正在安装...
    pip install Pillow
)

echo.
echo [INFO] 启动机甲控制系统...
echo.
python main.py

if errorlevel 1 (
    echo.
    echo [ERROR] 程序运行出错，请检查错误信息
    pause
)
