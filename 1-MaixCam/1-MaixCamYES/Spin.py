from maix import image
import cv2
import numpy as np

"""
这个就是发挥4的旋转了,没什么好说的，透视变换
"""

A4_PAPER_WIDTH_MM = 210.0     # A4纸宽度（短边，单位：毫米）
A4_PAPER_HEIGHT_MM = 297.0    # A4纸高度（长边，单位：毫米）
A4_BLACK_BORDER_WIDTH_MM = 20.0  # 黑边宽度（2cm）
A4_MIN_CONTOUR_AREA_THRESH = 1200  # 最小轮廓面积阈值（过滤噪声）
a4_distance_filter_queue = []  # 距离滤波队列
A4_DISTANCE_FILTER_WINDOW_SIZE = 10  # 均值滤波窗口
a4_prev_contour = None  # 用于存储上一帧的A4纸轮廓
A4_CONTOUR_STABILITY_THRESHOLD = 0.6  # 轮廓稳定性阈值，适应旋转导致的变化

def detect_a4_contour(image):
    """检测A4纸轮廓（增强对2cm黑边的检测，处理非平面旋转畸变）"""
    # 转换为HSV色彩空间，增强黑色边框检测
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # 优化黑色范围（针对2cm黑边调整，更精确捕捉黑色）
    lower_black = np.array([0, 0, 0])
    upper_black = np.array([180, 255, 60])  # 稍微提高亮度阈值，确保黑边被检测
    
    # 创建黑色区域的掩码
    mask = cv2.inRange(hsv, lower_black, upper_black)
    
    # 增强形态学操作：使用更大的kernel处理可能断裂的黑边
    kernel = np.ones((7, 7), np.uint8)
    mask = cv2.dilate(mask, kernel, iterations=2)  # 更强的膨胀，连接黑边
    mask = cv2.erode(mask, kernel, iterations=1)
    
    # 查找轮廓
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    max_area = 0
    best_approx = None

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < A4_MIN_CONTOUR_AREA_THRESH:  # 过滤小轮廓
            continue
            
        # 多边形逼近：使用更大的容差适应非平面旋转导致的畸变
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.15 * peri, True)  # 增大容差至0.15，适应严重畸变
        
        # 检查是否为四边形（梯形也是四边形）
        if len(approx) == 4:
            # 检查是否为凸四边形
            if cv2.isContourConvex(approx):
                # 计算面积比例（过滤过小或过大的轮廓）
                img_area = image.shape[0] * image.shape[1]
                area_ratio = area / img_area
                
                # 调整比例范围，考虑黑边存在时的合理范围
                if 0.05 < area_ratio < 0.95:
                    if area > max_area:
                        max_area = area
                        best_approx = approx
    
    # 增强轮廓稳定性处理，适应旋转场景
    global a4_prev_contour
    if best_approx is None and a4_prev_contour is not None:
        # 检查上一帧轮廓是否在当前帧中仍然可见
        mask_prev = np.zeros_like(mask)
        cv2.drawContours(mask_prev, [a4_prev_contour], -1, 255, -1)
        overlap = cv2.bitwise_and(mask, mask_prev)
        
        # 计算重叠比例
        prev_area = cv2.contourArea(a4_prev_contour)
        if prev_area > 0:
            overlap_ratio = cv2.countNonZero(overlap) / prev_area
            
            if overlap_ratio > A4_CONTOUR_STABILITY_THRESHOLD:
                best_approx = a4_prev_contour
    
    # 更新上一帧轮廓
    if best_approx is not None:
        a4_prev_contour = best_approx
    
    return best_approx, mask

def order_points(pts):
    """对四个点进行排序（左上、右上、右下、左下）"""
    # 初始化坐标点
    rect = np.zeros((4, 2), dtype="float32")
    
    # 左上角点：x+y最小
    # 右下角点：x+y最大
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]
    rect[2] = pts[np.argmax(s)]
    
    # 右上角点：x-y最小
    # 左下角点：x-y最大
    diff = np.diff(pts, axis=1)
    
    # 确保我们有有效的点
    if diff.size > 0:
        rect[1] = pts[np.argmin(diff)]
        rect[3] = pts[np.argmax(diff)]
    else:
        # 如果无法计算diff，使用简单排序
        rect = pts[np.argsort(pts[:, 0])]
    
    return rect

def perspective_transform(image, contour):
    """透视变换（增强对非平面旋转畸变的处理）"""
    # 提取四点并转换格式
    pts = contour.reshape(4, 2).astype(np.float32)
    
    # 对点进行排序
    rect = order_points(pts)
    
    # 计算目标尺寸（保持竖直A4实际比例）
    target_width = A4_PAPER_WIDTH_MM
    target_height = A4_PAPER_HEIGHT_MM

    # 创建目标点（映射到标准A4尺寸的矩形）
    dst = np.array([
        [0, 0],
        [target_width - 1, 0],
        [target_width - 1, target_height - 1],
        [0, target_height - 1]
    ], dtype="float32")

    # 计算变换矩阵
    M = cv2.getPerspectiveTransform(rect, dst)

    # 应用透视变换
    warped = cv2.warpPerspective(image, M, (int(target_width), int(target_height)))
    return warped, M

def is_equilateral_triangle(approx):
    """判断三角形是否为等边三角形"""
    if len(approx) != 3:
        return False
        
    # 计算三边长度
    side1 = np.linalg.norm(approx[0] - approx[1])
    side2 = np.linalg.norm(approx[1] - approx[2])
    side3 = np.linalg.norm(approx[2] - approx[0])
    
    # 计算平均边长
    avg_side = (side1 + side2 + side3) / 3
    
    # 检查三边是否接近（允许10%的误差）
    tolerance = 0.2
    if (abs(side1 - avg_side) / avg_side < tolerance and
        abs(side2 - avg_side) / avg_side < tolerance and
        abs(side3 - avg_side) / avg_side < tolerance):
        return True
    return False

def detect_shapes(warped):
    """检测所有形状并返回尺寸信息（针对2cm黑边内的图形优化）"""
    # 转换为灰度并反色（形状为黑色，背景白色）
    if len(warped.shape) == 3:
        gray = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
    else:
        gray = warped
    
    # 自适应阈值处理，适应光照变化
    binary_img = cv2.adaptiveThreshold(
        gray, 255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV, 31, 10
    )
    
    # 查找所有轮廓
    contours, _ = cv2.findContours(binary_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    
    results = {
        "square": {"size": 0, "contour": None},
        "equilateral_triangle": {"size": 0, "contour": None},  # 专门检测等边三角形
        "circle": {"size": 0, "contour": None}
    }
    
    # 计算有效区域（扣除2cm黑边）
    border_pixels = int(A4_BLACK_BORDER_WIDTH_MM)  # 透视变换后单位为毫米，直接使用mm作为像素数
    valid_width = warped.shape[1] - 2 * border_pixels
    valid_height = warped.shape[0] - 2 * border_pixels
    
    if valid_width <= 0 or valid_height <= 0:
        return results
    
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < 10:  # 过滤微小噪声
            continue
            
        # 计算轮廓中心
        M = cv2.moments(cnt)
        if M["m00"] == 0:
            continue
        cX = int(M["m10"] / M["m00"])
        cY = int(M["m01"] / M["m00"])
        
        # 确保图形在2cm黑边以内的有效区域
        if (cX < border_pixels or cX > warped.shape[1] - border_pixels or 
            cY < border_pixels or cY > warped.shape[0] - border_pixels):
            continue
            
        # 排除过大的轮廓（可能是干扰）
        max_valid_area = 0.8 * valid_width * valid_height  # 最大不超过有效区域的80%
        if area > max_valid_area:
            continue
            
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.03 * peri, True)
        num_sides = len(approx)
        
        # 计算轮廓的边界框
        x, y, w, h = cv2.boundingRect(cnt)
        
        # 计算特征尺寸（毫米，因为透视变换后单位为毫米）
        if num_sides == 4:  # 正方形
            # 检查是否为正方形（边长比例接近1）
            aspect_ratio = w / float(h)
            if 0.85 < aspect_ratio < 1.15:  # 放宽比例范围
                # 使用边界框的宽度作为特征尺寸
                size = w
                if size > results["square"]["size"]:
                    results["square"]["size"] = size
                    results["square"]["contour"] = cnt
                
        elif num_sides == 3:  # 三角形
            # 检查是否为等边三角形
            if is_equilateral_triangle(approx):
                # 使用边界框的高度作为特征尺寸
                size = h
                if size > results["equilateral_triangle"]["size"]:
                    results["equilateral_triangle"]["size"] = size
                    results["equilateral_triangle"]["contour"] = cnt
                    
        else:  # 圆形
            # 计算圆度（圆形度）
            circularity = 4 * np.pi * area / (peri * peri)
            if circularity > 0.7:  # 圆度大于0.7认为是圆形
                # 使用等效直径作为特征尺寸
                size = np.sqrt(4 * area / np.pi)
                if size > results["circle"]["size"]:
                    results["circle"]["size"] = size
                    results["circle"]["contour"] = cnt
                
    return results

def Find_Spin_Shape(disp, img, focal_length):
    global a4_distance_filter_queue  
    Cv_img = image.image2cv(img)
    h, w = Cv_img.shape[:2]

    filtered_distance = 0.0  # A4纸距离（毫米）
    shape_size = 0.0         # 形状尺寸（毫米）
    shape_type = ""          # 形状类型

    # 初始化显示参数
    shape_info_text = ""
    shape_info_color = (255, 255, 255)
    shape_detected = False
    detected_contour = None
    shape_color = (0, 0, 0)

    # 1. 检测A4纸轮廓
    Contour_img = Cv_img.copy()
    outer_approx, mask = detect_a4_contour(Cv_img)

    if outer_approx is not None and focal_length is not None:
        # 2. 计算A4纸距离（使用已标定的焦距）
        # 使用最小外接矩形计算长边（竖直A4的长边为297mm）
        rect = cv2.minAreaRect(outer_approx)
        pixel_long = max(rect[1])
        
        if pixel_long > 10:  # 有效轮廓
            # 用长边计算距离（A4长边为297mm）
            raw_distance = (A4_PAPER_HEIGHT_MM * focal_length) / pixel_long
            a4_distance_filter_queue.append(raw_distance)
            if len(a4_distance_filter_queue) > A4_DISTANCE_FILTER_WINDOW_SIZE:
                a4_distance_filter_queue.pop(0)
        
        # 3. 透视变换校正A4区域（处理梯形畸变）
        try:
            warped, M = perspective_transform(Cv_img, outer_approx)
        except Exception as e:
            print(f"透视变换失败: {e}")
            warped = None
            M = None

        # 4. 检测所有形状
        if warped is not None and M is not None:
            shape_results = detect_shapes(warped)
            
            # 优先显示检测到的形状
            if shape_results["square"]["size"] > 0:
                shape_size = shape_results["square"]["size"]
                shape_type = "Square"
                shape_info_text = f"Square: {shape_size:.2f}mm"
                shape_info_color = (0, 0, 255)
                shape_detected = True
                detected_contour = shape_results["square"]["contour"]
                shape_color = (0, 0, 255)
            elif shape_results["equilateral_triangle"]["size"] > 0:
                shape_size = shape_results["equilateral_triangle"]["size"]
                shape_type = "EquiTriangle"
                shape_info_text = f"Triangle: {shape_size:.2f}mm"
                shape_info_color = (0, 255, 0)
                shape_detected = True
                detected_contour = shape_results["equilateral_triangle"]["contour"]
                shape_color = (0, 255, 0)
            elif shape_results["circle"]["size"] > 0:
                shape_size = shape_results["circle"]["size"]
                shape_type = "Circle"
                shape_info_text = f"Circle: {shape_size:.2f}mm"
                shape_info_color = (0, 255, 255)
                shape_detected = True
                detected_contour = shape_results["circle"]["contour"]
                shape_color = (0, 255, 255)

            # 5. 将形状轮廓映射回原始图像
            if detected_contour is not None:
                M_inv = cv2.invert(M)[1]  # 逆变换矩阵
                warped_points = detected_contour.reshape(-1, 2).astype(np.float32)
                # 添加第三个维度以满足perspectiveTransform要求
                warped_points = np.expand_dims(warped_points, axis=0)
                original_points = cv2.perspectiveTransform(warped_points, M_inv)[0]
                original_points = original_points.astype(np.int32)
                original_contour = original_points.reshape(-1, 1, 2)
                cv2.drawContours(Contour_img, [original_contour], -1, shape_color, 2, lineType=cv2.LINE_AA)

        # 绘制A4纸外框
        cv2.drawContours(Contour_img, [outer_approx], -1, (255, 255, 0), 4, lineType=cv2.LINE_AA)
    
    # 显示信息
    if focal_length is not None:
        cv2.putText(Contour_img, f"Focal: {focal_length:.3f}mm", 
                   (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
    
  
    if shape_detected:
        cv2.putText(Contour_img, shape_info_text, 
                   (10, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.6, shape_info_color, 2)
    
    # 显示结果
    shape_size = shape_size*1.0643
    Maix_img = image.cv2image(Contour_img)
    disp.show(Maix_img)
    
    # 返回检测结果（距离：毫米，形状尺寸：毫米）
    return round(filtered_distance, 3), round(shape_size/10, 3)