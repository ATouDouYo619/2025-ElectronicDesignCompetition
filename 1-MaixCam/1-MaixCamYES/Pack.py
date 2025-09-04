

def print_hex(data: bytes):  # 打印十六进制数据
    for i in data:
        print(f"0x{i:02X}", end=" ")
    print("")

def pack_float_data(distance, line):
    """
    将浮点数打包成数据帧
    格式：[起始符AA 55] [距离高8位] [距离低8位] [边长高8位] [边长低8位] [焦距高8位] [焦距低8位] [结束符0D 0A] [校验和]
    
    参数:
        distance: 距离值 (2位小数的浮点数)
        line: 边长值 (2位小数的浮点数)
        focal: 焦距值 (2位小数的浮点数,可选)
    
    返回:
        打包后的字节数组
    """
    # 将浮点数转换为整数（乘以100保留2位小数）
    distance_int = int(distance * 100) if distance is not None else 0
    line_int = int(line * 100) if line is not None else 0

    # 分割为高8位和低8位
    dis_high = (distance_int >> 8) & 0xFF
    dis_low = distance_int & 0xFF
    
    line_high = (line_int >> 8) & 0xFF
    line_low = line_int & 0xFF
    

    # 构建数据包（包含起始符和结束符）
    packet = [
        0xAA, 0xBB,             # 起始符
        dis_high, dis_low,      # 距离值
        line_high, line_low,    # 边长值
        0xCC, 0xDD              # 结束符
    ]

    return bytes(packet)

def pack_float_data_Mazy(distance, line):
    """
    将浮点数打包成数据帧
    格式：[起始符AA 55] [距离高8位] [距离低8位] [边长高8位] [边长低8位] [焦距高8位] [焦距低8位] [结束符0D 0A] [校验和]
    
    参数:
        distance: 距离值 (2位小数的浮点数)
        line: 边长值 (2位小数的浮点数)
        focal: 焦距值 (2位小数的浮点数,可选)
    
    返回:
        打包后的字节数组
    """
    # 将浮点数转换为整数（乘以100保留2位小数）
    distance_int = int(distance * 100) if distance is not None else 0
    line_int = int(line * 100) if line is not None else 0

    # 分割为高8位和低8位
    dis_high = (distance_int >> 8) & 0xFF
    dis_low = distance_int & 0xFF
    
    line_high = (line_int >> 8) & 0xFF
    line_low = line_int & 0xFF
    

    # 构建数据包（包含起始符和结束符）
    packet = [
        0xAA, 0xBB,             # 起始符
        dis_high, dis_low,      # 距离值
        line_high, line_low,    # 边长值
        0xEE, 0xFF              # 结束符
    ]

    return bytes(packet)
