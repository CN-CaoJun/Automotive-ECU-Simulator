import os

def fill_file_with_ff(filename, target_size, suffix):
    current_size = os.path.getsize(filename)
    bytes_to_fill = target_size - current_size
    
    if bytes_to_fill <= 0:
        print(f"File {filename} is already larger than or equal to the target size.")
        return
    
    print(f"Filling file {filename} with {bytes_to_fill} bytes of 0xFF...")
    with open(filename, 'ab') as file:
        # 使用 0xFF 字节填充文件
        file.write(bytes([0x00] * (bytes_to_fill - len(suffix))))
        # 写入特定字节序列
        file.write(suffix)
    
    print("File filled successfully.")

# 设置要填充的文件名、目标大小（以字节为单位）和特定字节序列
file_to_fill = 'rtthread.bin'
target_size = 512 * 1024  # 1MB
suffix = b'\xaa\xbb\xcc\xdd'

fill_file_with_ff(file_to_fill, target_size, suffix)
