from maix import image
import cv2
import numpy as np

"""
发挥部分的分离情况，和前面类似，不多讲了
"""

# 统一为毫米）
D_mm = 1000.0    # 初始距离（标定用，单位：毫米）
W_mm = 296.5     # A4纸实际宽度（单位：毫米）
MIN_CONTOUR_AREA = 1200  # 最小轮廓面积阈值（过滤噪声）
calculated_F = None  # 焦距（毫米）
distance_history = [] # 距离滤波队列
FILTER_SIZE = 10       # 均值滤波窗口

# A4纸检测核心函数
def calculate_pixel_width(contour):
    """使用最小外接矩形计算短边（A4纸像素宽度）"""
    rect = cv2.minAreaRect(contour)
    width, height = rect[1]
    return max(width, height)  # 返回长边作为参考

def detect_a4_contour(image):
    """检测A4纸轮廓（返回最大凸四边形）"""
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    # 自适应阈值处理 - 增强黑色边框
    thresh = cv2.adaptiveThreshold(
        gray, 255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV, 59, 20
    )
    thresh_copy = thresh.copy()
    kernel = np.ones((3, 3), np.uint8)
    mask = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)

    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    max_area = 0
    best_approx = None

    for cnt in contours:
        # 跳过点数不足的轮廓
        if len(cnt) < 4:
            continue
            
        # 计算周长和多边形逼近
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.03 * peri, True)
        
        # 检查是否为凸四边形
        if len(approx) == 4 and cv2.isContourConvex(approx):
            area = cv2.contourArea(approx)
            # 保留面积最大的轮廓
            if area > max_area:
                max_area = area
                best_approx = approx
    
    # 返回检测结果
    if best_approx is not None:
        return best_approx, thresh_copy
    return None, None

def perspective_transform(image, contour):
    """将A4纸区域透视变换为标准矩形，返回校正图像和变换矩阵M"""
    # 将轮廓点排序为（左上、右上、右下、左下）
    pts = contour.reshape(4, 2)
    rect = np.zeros((4, 2), dtype="float32")

    # 计算轮廓点中心
    center = np.mean(pts, axis=0)

    # 根据点与中心的相对位置排序
    for point in pts:
        if point[0] < center[0] and point[1] < center[1]:
            rect[0] = point  # 左上
        elif point[0] > center[0] and point[1] < center[1]:
            rect[1] = point  # 右上
        elif point[0] > center[0] and point[1] > center[1]:
            rect[2] = point  # 右下
        else:
            rect[3] = point  # 左下

    # 计算目标矩形尺寸（保持A4比例）
    width = max(
        np.linalg.norm(rect[0] - rect[1]),
        np.linalg.norm(rect[2] - rect[3])
    )
    height = max(
        np.linalg.norm(rect[0] - rect[3]),
        np.linalg.norm(rect[1] - rect[2])
    )

    # 创建目标点
    dst = np.array([
        [0, 0],
        [width - 1, 0],
        [width - 1, height - 1],
        [0, height - 1]
    ], dtype="float32")

    # 计算变换矩阵
    M = cv2.getPerspectiveTransform(rect, dst)

    # 应用透视变换
    warped = cv2.warpPerspective(image, M, (int(width), int(height)))
    return warped, M  # 同时返回校正图像和变换矩阵


# 形状检测核心函数
def calculate_angle(p1, p2, p3):
    """计算三点形成的角度"""
    v1 = p1 - p2
    v2 = p3 - p2
    cos_theta = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
    return np.degrees(np.arccos(cos_theta))

def detect_squares(warped, scale):
    """检测正方形，返回（实际边长, 轮廓）"""
    inverted_gray = cv2.bitwise_not(warped)
    _, binary_img = cv2.threshold(inverted_gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    contours, _ = cv2.findContours(binary_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < 50:  # 过滤小轮廓
            continue

        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.02 * peri, True)
        if len(approx) != 4:
            continue

        # 正方形特征验证（边长比例+角度）
        sides = []
        for i in range(4):
            pt1 = approx[i][0]
            pt2 = approx[(i+1)%4][0]
            sides.append(np.linalg.norm(pt2 - pt1))

        max_side, min_side = max(sides), min(sides)
        aspect_ratio = abs(max_side - min_side) / ((max_side + min_side)/2)
        if aspect_ratio > 0.15:
            continue

        # 角度检查（85-95度）
        angles_valid = True
        for i in range(4):
            pt1 = approx[i][0]
            pt2 = approx[(i+1)%4][0]
            pt3 = approx[(i+2)%4][0]
            angle = calculate_angle(pt1, pt2, pt3)
            if not (85 <= angle <= 95):
                angles_valid = False
                break
        if not angles_valid:
            continue

        # 计算实际边长（毫米）
        avg_side = np.mean(sides)
        return avg_side * scale, approx  # 返回尺寸和轮廓
    return None, None

def detect_triangles(warped, scale):
    """检测等边三角形，返回（实际边长, 轮廓）"""
    # 预处理（使用OTSU阈值）
    inverted = cv2.bitwise_not(warped)
    _, binary_img = cv2.threshold(inverted, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    contours, _ = cv2.findContours(binary_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    best_side = None
    best_contour = None
    max_area = 0  # 记录最大面积，筛选最优三角形

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < 50 or area <= max_area:  # 过滤小面积或小于当前最大面积的轮廓
            continue

        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.04 * peri, True)
        if len(approx) != 3:
            continue

        # 等边三角形验证（同上）
        points = approx.reshape(3, 2)
        sides = [
            np.linalg.norm(points[1] - points[0]),
            np.linalg.norm(points[2] - points[1]),
            np.linalg.norm(points[0] - points[2])
        ]
        max_side, min_side = max(sides), min(sides)
        aspect_ratio = abs(max_side - min_side) / ((max_side + min_side)/2)
        if aspect_ratio > 0.15:
            continue

        # 角度检查
        angles_valid = True
        for i in range(3):
            a, b, c = points[i], points[(i+1)%3], points[(i+2)%3]
            angle = calculate_angle(a, b, c)
            if not (55 <= angle <= 65):
                angles_valid = False
                break
        if not angles_valid:
            continue

        # 更新最优三角形（面积更大）
        avg_side = np.mean(sides)
        max_area = area  # 记录当前最大面积
        best_side = avg_side * scale
        best_contour = approx

    return best_side, best_contour  # 未检测到时返回(None, None)

def detect_circles(warped, scale):
    """检测圆形，返回（实际直径, 轮廓）"""
    # 预处理：反色+自适应二值化（提高鲁棒性）
    if len(warped.shape) == 3:
        gray = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
    else:
        gray = warped
    inverted_gray = cv2.bitwise_not(gray)
    # 用OTSU阈值自动适应光照
    _, binary_img = cv2.threshold(inverted_gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

    contours, _ = cv2.findContours(binary_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    best_diameter = None  # 初始化最佳直径为None
    best_contour = None   # 初始化最佳轮廓为None
    max_area = 0          # 用于筛选面积最大的圆形

    for contour in contours:
        area = cv2.contourArea(contour)
        # 过滤过小轮廓（噪声）
        if area < 50:
            continue

        perimeter = cv2.arcLength(contour, True)
        if perimeter == 0:  # 避免除零错误
            continue

        # 角点数量辅助判断（曲线轮廓近似为多角形）
        approx = cv2.approxPolyDP(contour, 0.03 * perimeter, True)
        corners = len(approx)
        if corners < 6:  # 角点太少，不是圆形
            continue

        # 圆度核心判断（越接近1越圆）
        circularity = (4 * np.pi * area) / (perimeter **2)
        if circularity < 0.75:
            continue

        # 计算直径（像素→毫米）
        diameter_mm = np.sqrt(4 * area / np.pi) * scale

        # 筛选面积最大的圆形（最优结果）
        if area > max_area:
            max_area = area
            best_diameter = diameter_mm
            best_contour = contour

    return best_diameter, best_contour  # 直接返回初始化的None（未检测到时）

def Find_Shape(disp, img):
    global calculated_F, distance_history  
    Cv_img = image.image2cv(img)
    h, w = Cv_img.shape[:2]
    kernel = np.ones((3, 3), np.uint8)

    filtered_distance = 0.0  # 单位 mm
    Line = 0.0
    
    # 初始化形状信息
    shape_info_text = ""
    shape_info_color = (255, 255, 255)  # 默认白色
    shape_detected = False  # 标记是否检测到形状
    detected_contour = None  # 保存检测到的形状轮廓（校正图像坐标）
    shape_color = (0, 0, 0)  # 形状轮廓颜色

    # 1. 检测A4纸轮廓
    Contour_img = Cv_img.copy()
    outer_approx, thresh_copy = detect_a4_contour(Cv_img)

    if outer_approx is not None:
        # 2. A4纸参数计算（距离+焦距）
        pixel_width = calculate_pixel_width(outer_approx)

        # 焦距标定
        if calculated_F is None and pixel_width > 10:  
            calculated_F = (pixel_width * D_mm) / W_mm
        
        # 距离计算（均值滤波）
        if calculated_F is not None and pixel_width > 10:
            raw_distance = (W_mm * calculated_F) / pixel_width
            distance_history.append(raw_distance)
            if len(distance_history) > FILTER_SIZE:
                distance_history.pop(0)
            filtered_distance = sum(distance_history) / len(distance_history)
        
        # 3. 透视变换校正A4区域（获取变换矩阵M）
        try:
            warped, M = perspective_transform(thresh_copy, outer_approx)
            warped_gray = cv2.cvtColor(warped, cv2.COLOR_GRAY2BGR) if len(warped.shape) == 2 else warped
            warped_h, warped_w = warped.shape[:2]
            if warped_h == 0:
                raise ValueError("校正后图像宽度为0")
            scale = W_mm / warped_h  # 像素到毫米的缩放比例
        except:
            warped = None
            M = None
            scale = None

        # 4. 校正区域内形状检测（并获取轮廓）
        if warped is not None and scale is not None and M is not None:
            # 检测正方形（优先级1）
            square_side, square_contour = detect_squares(warped, scale)
            if square_side:
                Line = square_side
                shape_info_text = f"S: {Line:.2f}mm"
                shape_info_color = (0, 0, 255)
                shape_detected = True
                detected_contour = square_contour
                shape_color = (0, 0, 255)  # 正方形用红色
            else:
                # 检测三角形（优先级2）
                triangle_side, triangle_contour = detect_triangles(warped, scale)
                if triangle_side:
                    Line = triangle_side
                    shape_info_text = f"T: {Line:.2f}mm"
                    shape_info_color = (0, 255, 0)
                    shape_detected = True
                    detected_contour = triangle_contour
                    shape_color = (0, 255, 0)  # 三角形用绿色
                else:
                    # 检测圆形（优先级3）
                    circle_dia, circle_contour = detect_circles(warped, scale)
                    if circle_dia:
                        Line = circle_dia
                        shape_info_text = f"C: {Line:.2f}mm"
                        shape_info_color = (0, 255, 255)
                        shape_detected = True
                        detected_contour = circle_contour
                        shape_color = (0, 255, 255)  # 圆形用黄色

            # 将检测到的轮廓从warped图像映射回原始图像
            if detected_contour is not None:
                # 计算透视变换逆矩阵
                M_inv = cv2.invert(M)[1]
                # 转换轮廓点格式
                warped_points = detected_contour.reshape(-1, 2).astype(np.float32)
                # 透视变换：warped点 → 原始图像点
                original_points = cv2.perspectiveTransform(warped_points[None, :, :], M_inv)[0]
                original_points = original_points.astype(np.int32)
                # 重塑为轮廓格式并绘制
                original_contour = original_points.reshape(-1, 1, 2)
                cv2.drawContours(Contour_img, [original_contour], -1, shape_color, 2, lineType=cv2.LINE_AA)

        # 5. 绘制A4纸外框
        cv2.drawContours(Contour_img, [outer_approx], -1, (255, 255, 0), 4, lineType=cv2.LINE_AA)
    
    # 6. 显示全局信息（焦距和A4距离）
    if calculated_F is not None:
        cv2.putText(Contour_img, f"F: {calculated_F:.3f}mm", 
                   (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
        cv2.putText(Contour_img, f"A4: {filtered_distance/10:.2f}cm", 
                   (10, 220), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
        cv2.putText(Contour_img, f"basics", 
                   (10, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
   
    # 7. 显示形状信息
    if shape_detected:
        cv2.putText(Contour_img, shape_info_text, 
                   (180, 220), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, shape_info_color, 2)
    
    # 绘制中心点标记
    # cv2.drawMarker(Contour_img, (int(w/2), int(h/2)), color=[255, 255, 0], thickness=2, 
    #               markerType=cv2.MARKER_CROSS, line_type=cv2.LINE_AA, markerSize=10)    

    # 显示结果
    Maix_img = image.cv2image(Contour_img)
    disp.show(Maix_img)
    
    # 返回值格式化处理
    if filtered_distance >= 176:
        filtered_distance = filtered_distance*1.0144
    filtered_distance = round(filtered_distance/10, 3)
    Line = round(Line/10, 3)
    calculated_F_rounded = round(calculated_F/10, 4) if calculated_F is not None else None
    
    return filtered_distance, Line, calculated_F_rounded

"""************************************************************************************"""
def detect_A4_frame(img):
    """检测A4纸外框和计算边框参数"""
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    lower_black = np.array([0, 0, 0])
    upper_black = np.array([180, 255, 60])
    mask = cv2.inRange(hsv, lower_black, upper_black)
    
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None, None, None, None
    
    # 找到最大轮廓（A4纸外框）
    max_contour = max(contours, key=cv2.contourArea)
    epsilon = 0.02 * cv2.arcLength(max_contour, True)
    approx = cv2.approxPolyDP(max_contour, epsilon, True)
    
    # 计算边框参数
    rect = cv2.minAreaRect(approx)
    (w_rect, h_rect) = rect[1]
    box_size = min(w_rect, h_rect)  # A4纸短边像素尺寸
    
    # 计算A4纸长边像素尺寸
    long_side = max(w_rect, h_rect)
    
    border_pixels = int((20 / 210) * box_size)  # 计算2cm边框的像素尺寸
    
    return approx, border_pixels, mask, box_size, long_side

def extract_inner_region(img, approx, border_pixels, mask):
    """提取A4纸内部区域"""
    inner_mask = np.zeros_like(mask)
    cv2.drawContours(inner_mask, [approx], -1, 255, -1)
    
    # 创建腐蚀内核确保完全去除黑框
    erosion_kernel = np.ones((2 * border_pixels, 2 * border_pixels), np.uint8)
    inner_mask = cv2.erode(inner_mask, erosion_kernel)
    
    return cv2.bitwise_and(img, img, mask=inner_mask), inner_mask

def detect_squares_2(roi_img):
    """在ROI区域检测所有正方形"""
    gray_roi = cv2.cvtColor(roi_img, cv2.COLOR_BGR2GRAY)
    _, thresh_roi = cv2.threshold(gray_roi, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    
    squares = []
    contours, _ = cv2.findContours(thresh_roi, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < 100:
            continue
            
        peri = cv2.arcLength(cnt, True)
        poly = cv2.approxPolyDP(cnt, 0.04 * peri, True)
        
        if len(poly) == 4 and cv2.isContourConvex(poly):
            rect = cv2.minAreaRect(poly)
            (w_rect, h_rect) = rect[1]
            aspect_ratio = min(w_rect, h_rect) / max(w_rect, h_rect)
            rect_area = w_rect * h_rect
            area_ratio = area / rect_area if rect_area > 0 else 0
            
            if aspect_ratio > 0.85 and area_ratio > 0.85:
                squares.append((poly, area, w_rect, h_rect))
    
    return squares

def find_min_square(squares):
    """找出面积最小的正方形"""
    min_square = None
    min_side = 0.0
    
    if squares:
        min_square, min_area, w_rect, h_rect = min(squares, key=lambda x: x[1])
        min_side = (w_rect + h_rect) / 2.0  # 取平均边长
    
    return min_square, min_side

def render_results(img, approx, inner_mask, min_square, min_side, actual_size, distance):
    """绘制检测结果并添加标注"""
    h, w = img.shape[:2]
    result_img = img.copy()
    
    # 绘制A4纸外框
    cv2.drawContours(result_img, [approx], -1, (0, 255, 255), 2)
    
    # 绘制内部区域边界
    inner_contours, _ = cv2.findContours(inner_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    for cnt in inner_contours:
        cv2.drawContours(result_img, [cnt], -1, (255, 0, 0), 1, lineType=cv2.LINE_AA)
    
    # 绘制最小正方形（如果找到）
    if min_square is not None:
        cv2.drawContours(result_img, [min_square], -1, (255, 255, 0), 2, lineType=cv2.LINE_AA)
        
        # 在正方形中心添加尺寸信息
        M = cv2.moments(min_square)
        if M["m00"] != 0:
            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])
            # 显示实际尺寸（单位：mm）
            cv2.putText(result_img, f"{actual_size:.2f}mm", 
                       (cX - 50, cY), cv2.FONT_HERSHEY_SIMPLEX, 
                       0.6, (0, 0, 255), 2)
    
    # 添加参数信息
    cv2.putText(result_img, f"F: {min_side:.3f}", (10, 30), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
    
    # 显示距离信息
    cv2.putText(result_img, f"Dist: {distance:.1f}mm", (10, 220), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
    
    # 固定UI元素
    cv2.putText(result_img, "add=1", (10, 110), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
    cv2.drawMarker(result_img, (w//2, h//2), color=[0, 255, 255], 
                  thickness=2, markerType=cv2.MARKER_CROSS, 
                  markerSize=10, line_type=cv2.LINE_AA)
    
    # 没有找到正方形时显示提示
    if min_square is None:
        cv2.putText(result_img, "No Squares", (w//2 - 50, h - 30), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
    
    return result_img

def Fine_Mazy_Shape(disp, img, focal_length):
    try:
        # 转换图像格式
        Cv_img = image.image2cv(img)
        h, w = Cv_img.shape[:2]
        
        # 1. 定位A4纸外框
        approx, border_pixels, mask, box_size, long_side = detect_A4_frame(Cv_img)
        if approx is None:
            # 未检测到A4纸时显示原始图像
            disp.show(img)
            return 0.0, 0.0
        
        # 2. 提取内部区域
        roi_img, inner_mask = extract_inner_region(Cv_img, approx, border_pixels, mask)
        
        # 3. 检测所有正方形
        squares = detect_squares_2(roi_img)
        
        # 4. 找出面积最小的正方形及其像素尺寸
        min_square, min_side = find_min_square(squares)
        
        # 5. 计算实际边长（毫米）和距离
        # 使用A4纸物理尺寸计算比例尺
        A4_short_side_mm = 210.0  # A4纸短边物理尺寸（210mm）
        A4_long_side_mm = 297.0   # A4纸长边物理尺寸（297mm）
        
        # 计算像素比例（使用短边）
        pixel_per_mm = A4_short_side_mm / box_size
        
        # 计算实际边长
        actual_size = min_side * pixel_per_mm  # 实际边长（毫米）
        
        # 计算镜头到A4纸的距离（毫米）
        # 使用长边计算更准确（因为长边像素更多）
        distance = (focal_length * A4_long_side_mm) / long_side
        if distance >= 160:
            distance = distance*0.98
        # 6. 绘制结果
        result_img = render_results(Cv_img, approx, inner_mask, 
                                   min_square, focal_length, actual_size, distance)
        
        # 7. 显示结果
        Maix_img = image.cv2image(result_img)
        disp.show(Maix_img)
        
        # 返回实际边长和距离
        return round(distance, 3), round(actual_size/10, 3)
        
    except Exception as e:
        print("Error in Fine_Mazy_Shape:", e)
        disp.show(img)
        return 0.0, 0.0