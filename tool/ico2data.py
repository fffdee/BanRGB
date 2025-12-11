import os
import base64
from pathlib import Path

def icon_to_text(icon_path, output_file=None):
    """
    将图标文件转换为Base64编码的文本数据并保存为TXT文件
    
    Args:
        icon_path: 图标文件路径(.ico)
        output_file: 输出TXT文件名，默认为None(打印到控制台)
    """
    try:
        # 确保图标文件存在
        if not os.path.exists(icon_path):
            print(f"错误：图标文件 {icon_path} 不存在")
            return
        
        # 读取图标文件的二进制数据
        with open(icon_path, 'rb') as f:
            icon_data = f.read()
        
        # 进行Base64编码
        base64_data = base64.b64encode(icon_data).decode('utf-8')
        
        # 生成要保存的文本内容
        icon_name = Path(icon_path).stem
        text_content = f"# {icon_name}.ico 的Base64编码数据\n# 可直接用于PyQt程序中\n\n{base64_data}"
        
        # 输出到文件或控制台
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(text_content)
            print(f"图标数据已保存到 {output_file}")
        else:
            print(text_content)
    
    except Exception as e:
        print(f"处理图标时出错: {str(e)}")


if __name__ == "__main__":
    # 直接指定图标路径和输出文件
    icon_path = "hbbb.ico"  # 修改为你的图标路径
    output_file = "icon_data.txt"  # 修改为你想要的输出文件名，None表示输出到控制台
    
    icon_to_text(icon_path, output_file)