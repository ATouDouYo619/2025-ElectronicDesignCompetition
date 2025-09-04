from maix import time, display, app,  camera, pinmap, uart
from Pack import print_hex,pack_float_data,pack_float_data_Mazy
from Shape import Find_Shape,Fine_Mazy_Shape
from Overlap import Find_Over_Shape
from num import Find_Num_Shape
from Spin import Find_Spin_Shape

pinmap.set_pin_function("A18", "UART1_RX")
pinmap.set_pin_function("A19", "UART1_TX")
device = "/dev/ttyS1"
serial1 = uart.UART(device, 115200)

disp = display.Display()
cam = camera.Camera(320, 240)

# 状态变量
calibrated = False
focal_length = 0
base_count = 0  # 基础部分计数（3次）
in_advanced_mode = False  # 是否进入发挥部分模式

# 发挥4专用计数
advanced4_count = 0  # 已采集次数，达到2次则完成
advanced4_started = False  # 是否已进入发挥4阶段

# 发挥部分完成状态
advanced1_done = False  
advanced2_done = False  
advanced3_done = False  
advanced4_done = False  

# 指令定义
BASE_TRIGGER_CMD = b'\x01'              
ADVANCED_TRIGGER_CMD_1 = b'\x02'        
ADVANCED_TRIGGER_CMD_2 = b'\x03'        
ADVANCED_TRIGGER_CMD_3 = b'\x04'        
ADVANCED_TRIGGER_CMD_4 = b'\x05'        
CALIB_DONE_SIGNAL = b'\xAA\xBB\x00\x01\x00\x00\xCC\xDD'

# 焦距标定函数
def calibrate_focal_length():
    global calibrated, focal_length
    while not calibrated:
        img = cam.read()
        _, _, focal = Find_Shape(disp, img)
        if focal > 0:
            focal_length = focal
            calibrated = True
            print(f"焦距F = {focal_length}")
        else:
            time.sleep(0.1)

calibrate_focal_length()


while not app.need_exit():  
    img = cam.read()
    RxData = serial1.read(1)

    # 基础部分逻辑
    if RxData == BASE_TRIGGER_CMD and base_count < 3:                   
        base_count += 1
        basics_distance, basics_line, _ = Find_Shape(disp, img)
        serial1.write(pack_float_data(basics_distance, basics_line))
        print(f"基础部分已采集 {base_count}/3 次")

    # 基础部分完成后进入发挥模式
    if base_count >= 3 and not in_advanced_mode:
        in_advanced_mode = True
        print("进入发挥模式")

    # 发挥模式核心逻辑
    if in_advanced_mode:
        # 处理发挥1和2
        if not advanced1_done and not advanced2_done:
            if RxData == ADVANCED_TRIGGER_CMD_1:
                advanced1_done = True
                exert_distance, exert_line = Fine_Mazy_Shape(disp, img, focal_length)
                serial1.write(pack_float_data_Mazy(exert_distance, exert_line))
                print("发挥1完成")
            elif RxData == ADVANCED_TRIGGER_CMD_2:
                advanced2_done = True
                Over_distance, Over_line = Find_Over_Shape(disp, img, focal_length)
                serial1.write(pack_float_data_Mazy(Over_distance, Over_line))
                print("发挥2完成")

        elif advanced1_done and not advanced2_done:
            if RxData == ADVANCED_TRIGGER_CMD_2:
                advanced2_done = True
                Over_distance, Over_line = Find_Over_Shape(disp, img, focal_length)
                serial1.write(pack_float_data_Mazy(Over_distance, Over_line))
                print("发挥2完成")

        elif advanced2_done and not advanced1_done:
            if RxData == ADVANCED_TRIGGER_CMD_1:
                advanced1_done = True
                exert_distance, exert_line = Fine_Mazy_Shape(disp, img, focal_length)
                serial1.write(pack_float_data_Mazy(exert_distance, exert_line))
                print("发挥1完成")

        # 处理发挥3
        if advanced1_done and advanced2_done and not advanced3_done:
            if RxData == ADVANCED_TRIGGER_CMD_3:
                Num_distance, Num_Line = Find_Num_Shape(disp, img, focal_length)
                serial1.write(pack_float_data_Mazy(Num_distance, Num_Line))
                advanced3_done = True
                advanced4_started = True  # 进入发挥4阶段
                advanced4_count = 0  # 初始化计数
                print("发挥3完成，进入发挥4（需采集2次）")

        if advanced3_done and not advanced4_done and advanced4_started:
            # 仅在此处调用一次图像处理函数
            spin_img = img.copy()  # 创建图像副本避免污染
            display_result = Find_Spin_Shape(disp, spin_img, focal_length)  # 用于显示
            
            # 使用相同处理结果 ==== 避免二次计算 ====
            if RxData == ADVANCED_TRIGGER_CMD_4 and advanced4_count < 2:
                advanced4_count += 1
                
                if display_result:  # 确保有有效数据
                    Spin_distance, Spin_Line = display_result
                    serial1.write(pack_float_data_Mazy(Spin_distance, Spin_Line))
                else:  # 处理识别失败
                    serial1.write(pack_float_data_Mazy(0, 0))
                
                
                if advanced4_count >= 2:
                    advanced4_done = True
    # 显示逻辑
    if base_count < 3:
        Find_Shape(disp, img)   
    elif advanced4_done:
        img.draw_string(20, 220, "The End" ,(0,0,255) ,1,0.8)
        disp.show(img)
    else:
        if in_advanced_mode:
            if not advanced1_done and not advanced2_done:
                disp.show(img)
            elif advanced1_done and not advanced2_done:
                Find_Over_Shape(disp, img, focal_length)
            elif advanced2_done and not advanced1_done:
                Fine_Mazy_Shape(disp, img, focal_length)
            elif advanced1_done and advanced2_done and not advanced3_done:
                Find_Num_Shape(disp, img, focal_length)
            elif advanced3_done and not advanced4_done:
                Find_Spin_Shape(disp, img, focal_length)

    time.sleep(0.01)
