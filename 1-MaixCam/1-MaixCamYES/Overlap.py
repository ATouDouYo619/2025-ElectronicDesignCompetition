from maix import camera, display, image, app
import cv2
import numpy as np

"""
这个是发挥部分的重叠部分，主要的思路是，先找轮廓，然后确定有效点，此时就会有有效点（红色）
无效点（绿色）。这个时候就可以进行画圆了，但是注意！！！只会两个有效点连在一起的情况，
这个时候要排除掉红绿红的情况，这样才是有效边长，但是似乎我这个地方限制并没有做的很好（大伙可以优化优化）
不过一些简单的已经足够。
"""

W_mm = 296.5   # A4纸短边实际宽度（毫米）

def calculate_pixel_width(contour):
    """使用最小外接矩形计算长边（A4纸像素参考）"""
    rect = cv2.minAreaRect(contour)
    width, height = rect[1]
    return max(width, height)

def detect_a4_contour(image):
    """检测A4纸轮廓（返回最大凸四边形）"""
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    thresh = cv2.adaptiveThreshold(
        gray, 255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV, 59, 20
    )
    kernel = np.ones((3, 3), np.uint8)
    mask = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)

    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    max_area = 0
    best_approx = None
    for cnt in contours:
        if len(cnt) < 4:
            continue
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.03 * peri, True)
        if len(approx) == 4 and cv2.isContourConvex(approx):
            area = cv2.contourArea(approx)
            if area > max_area:
                max_area = area
                best_approx = approx
    return best_approx, thresh

def create_a4_mask(image, a4_contour):
    """基于A4纸轮廓创建掩码"""
    mask = np.zeros(image.shape[:2], np.uint8)
    if a4_contour is not None:
        cv2.drawContours(mask, [a4_contour], -1, 255, -1)
    return mask

def is_point_in_a4(point, a4_contour):
    """判断点是否在A4纸内部"""
    if a4_contour is None:
        return False
    pt = (int(point[0]), int(point[1]))
    return cv2.pointPolygonTest(a4_contour, pt, False) >= 0

def sort_points_by_angle(points):
    """排序点并返回0~2π的极角"""
    if len(points) < 2:
        return points, np.mean(points, axis=0) if len(points) > 0 else None, []
    # 计算中心点
    center = np.mean(points, axis=0)
    # 计算极角并转换为0~2π（避免负角度）
    angles = np.arctan2(points[:, 1] - center[1], points[:, 0] - center[0])
    angles = np.where(angles < 0, angles + 2 * np.pi, angles)  # 负角转正（0~2π）
    sorted_indices = np.argsort(angles)
    return points[sorted_indices], center, angles[sorted_indices]  # 返回排序点、中心、正角度

def angle_between(angle, angle1, angle2):
    """判断角度是否在0~2π的区间内"""
    if angle1 <= angle2:
        # 普通区间（如30°~60°）
        return angle1 <= angle <= angle2
    else:
        # 跨0°区间（如350°~10°，包含350°~360°和0°~10°）
        return angle >= angle1 or angle <= angle2

def Find_Over_Shape(disp, img, focal_length):
    """检测A4纸及其内部形状
    参数:
        disp: 显示对象
        img: 输入图像
        focal_length: 相机焦距（毫米）
    
    返回值:
        a4_distance: A4纸到相机的距离（毫米）
        min_edge_mm: 检测到的最小边长（毫米）
    """
    # 转换图像格式
    cv2_img = image.image2cv(img)
    result = cv2_img.copy()
    a4_distance = 0.0
    min_edge_mm = 0.0
    
    # 1. 检测A4纸轮廓
    a4_contour, a4_thresh = detect_a4_contour(cv2_img)
    if a4_contour is None:
        cv2.putText(result, "No A4 detected", (10, 20), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
        maix_img = image.cv2image(result)
        disp.show(maix_img)
        return 0.0, 0.0
    
    # 2. 计算A4像素宽度和实际距离
    w_pixel = calculate_pixel_width(a4_contour)
    a4_distance = (focal_length * W_mm) / w_pixel
    
    # 显示A4纸轮廓
    cv2.drawContours(result, [a4_contour], -1, (0, 255, 0), 2, lineType=cv2.LINE_AA)
    
    # 3. 创建A4掩码
    a4_mask = create_a4_mask(cv2_img, a4_contour)
    
    # 4. A4内部阈值与轮廓检测
    gray = cv2.cvtColor(cv2_img, cv2.COLOR_BGR2GRAY)
    _, thresh = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)
    thresh_a4 = cv2.bitwise_and(thresh, thresh, mask=a4_mask)
    
    contours, hierarchy = cv2.findContours(
        thresh_a4, 
        cv2.RETR_TREE,
        cv2.CHAIN_APPROX_SIMPLE
    )
    
    # 5. 筛选外轮廓
    max_area = 0
    outer_contour = None
    if contours and hierarchy is not None:
        for i, cnt in enumerate(contours):
            if hierarchy[0][i][3] == -1 and cv2.contourArea(cnt) > 0:
                M = cv2.moments(cnt)
                if M["m00"] == 0:
                    continue
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])
                if is_point_in_a4((cX, cY), a4_contour):
                    area = cv2.contourArea(cnt)
                    if area > max_area:
                        max_area = area
                        outer_contour = cnt
    
    # 6. 轮廓分类
    cnt_useful = []
    cnt_number = []
    cnt_useless = []
    if outer_contour is not None and max_area > 0:
        for cnt in contours:
            if cnt is outer_contour:
                continue  
            M = cv2.moments(cnt)
            if M["m00"] == 0:
                continue
            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])
            if not is_point_in_a4((cX, cY), a4_contour):
                continue
            
            area = cv2.contourArea(cnt)
            ratio = area / max_area if max_area !=0 else 0
            if 0.05 < ratio < 0.9:
                perimeter = cv2.arcLength(cnt, True)
                epsilon = 0.02 * perimeter
                approx = cv2.approxPolyDP(cnt, epsilon, True)
                if len(approx) == 4:
                    cnt_useful.append(cnt)
                else:
                    cnt_number.append(cnt)
            else:
                cnt_useless.append(cnt)
    
    cv2.drawContours(result, cnt_number, -1, (0, 255, 255), 2, lineType=cv2.LINE_AA)
    
    # 7. 顶点标记（红/绿点）
    half_size = 8
    red_corners = []    # 红点（有效的外90度角）
    green_corners = []  # 绿点（其他顶点）

    for contour in cnt_number:
        perimeter = cv2.arcLength(contour, True)
        epsilon = 0.02 * perimeter
        approx = cv2.approxPolyDP(contour, epsilon, True)
        vertices = [tuple(point[0]) for point in approx]
        num_vertices = len(vertices)
        if num_vertices < 3:
            continue
        
        for i in range(num_vertices):
            x, y = vertices[i]
            if not is_point_in_a4((x, y), a4_contour):
                continue
            
            # 获取相邻点
            prev_idx = (i - 1) % num_vertices
            next_idx = (i + 1) % num_vertices
            p_prev = vertices[prev_idx]
            p_next = vertices[next_idx]
            
            # 计算两个向量：当前点->前一点，当前点->后一点
            v1 = (p_prev[0] - x, p_prev[1] - y)
            v2 = (p_next[0] - x, p_next[1] - y)
            
            # 计算向量长度
            len1 = np.sqrt(v1[0]**2 + v1[1]**2)
            len2 = np.sqrt(v2[0]**2 + v2[1]**2)
            
            # 计算点积和角度 (以度为单位的夹角)
            if len1 > 0 and len2 > 0:
                dot_product = v1[0]*v2[0] + v1[1]*v2[1]
                cosine = dot_product / (len1 * len2)
                # 处理可能的小数点精度误差
                cosine = max(-1.0, min(1.0, cosine))
                angle = np.degrees(np.arccos(cosine))
            else:
                angle = 180  # 默认设为钝角
                
            # 检查凸性（向量叉积）
            cross = v1[0]*v2[1] - v1[1]*v2[0]
            is_convex = cross > 0
            
            # 顶点分类逻辑
            roi_x1 = max(0, x - half_size)
            roi_y1 = max(0, y - half_size)
            roi_x2 = min(thresh.shape[1]-1, x + half_size)
            roi_y2 = min(thresh.shape[0]-1, y + half_size)
            if roi_x2 > roi_x1 and roi_y2 > roi_y1:
                roi = thresh[roi_y1:roi_y2, roi_x1:roi_x2]
                white_pixels = cv2.countNonZero(roi)
                total_pixels = roi.shape[0] * roi.shape[1]
                ratio = white_pixels / total_pixels if total_pixels != 0 else 0
                
                # 仅当是凸点且角度接近90度时标记为红点 (78°-105°范围)
                if is_convex and 80 <= angle <= 105:
                    min_edge_length = 10
                    if len1 > min_edge_length and len2 > min_edge_length:
                        cv2.circle(result, (x, y), 3, (0, 0, 255), -1)
                        red_corners.append([x, y])
                    else:
                        # 边太短，标记为绿点
                        cv2.circle(result, (x, y), 3, (0, 255, 0), -1)
                        green_corners.append([x, y])
                # 其他情况标记为绿点
                else:
                    cv2.circle(result, (x, y), 3, (0, 255, 0), -1)  # 绿色标记
                    green_corners.append([x, y])
    
    # 8. 红点排序与圆绘制
    all_circles = []
    min_radius_circle = None
    min_radius = float('inf')
    
    if len(red_corners) >= 2:
        red_corners_np = np.array(red_corners)
        # 排序红点并获取0~2π的极角
        sorted_red, center, red_angles = sort_points_by_angle(red_corners_np)
        if center is None:
            center = [0, 0]
        
        # 计算绿点极角（统一转换为0~2π，与红点角度范围一致）
        green_angles = []
        for (gx, gy) in green_corners:
            ga = np.arctan2(gy - center[1], gx - center[0])
            if ga < 0:
                ga += 2 * np.pi  # 转为0~2π范围
            green_angles.append(ga)
        
        # 遍历相邻红点对
        for i in range(len(sorted_red)):
            p1 = sorted_red[i]
            p2 = sorted_red[(i + 1) % len(sorted_red)]
            angle1 = red_angles[i]
            angle2 = red_angles[(i + 1) % len(sorted_red)]
            
            # 过滤：检查两个红点之间是否有绿点
            has_green_between = False
            for ga in green_angles:
                if angle_between(ga, angle1, angle2):
                    has_green_between = True
                    break
            
            # 若中间有绿点，跳过该红点对
            if has_green_between:
                continue
            
            # 验证圆心在A4内
            center_x = int((p1[0] + p2[0]) / 2)
            center_y = int((p1[1] + p2[1]) / 2)
            if not is_point_in_a4((center_x, center_y), a4_contour):
                continue
            
            # 绘制符合条件的圆（仅红红相连）
            radius = int(np.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])** 2) / 2)
            circle_info = {
                'center': (center_x, center_y),
                'radius': radius,
                'points': (tuple(p1), tuple(p2))
            }
            all_circles.append(circle_info)
            cv2.circle(result, (center_x, center_y), radius, (0, 0, 255), 1, lineType=cv2.LINE_AA)
            
            if radius < min_radius:
                min_radius = radius
                min_radius_circle = circle_info
    
    # 9. 绘制最小半径圆并计算物理尺寸
    if min_radius_circle:
        center = min_radius_circle['center']
        radius = min_radius_circle['radius']
        if is_point_in_a4(center, a4_contour):
            # 计算物理尺寸（毫米）：(像素尺寸 × 实际距离) / 焦距
            min_edge_pixel = radius * 2  # 像素长度（直径）
            min_edge_mm = (min_edge_pixel * a4_distance) / focal_length
            
            # 在图像上标注结果
            cv2.circle(result, center, radius, (255, 255, 0), 2, lineType=cv2.LINE_AA)
            cv2.putText(result, f"R:{min_edge_mm:.1f}mm", 
                       (center[0] + 10, center[1]),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.4, 
                       (0, 255, 255), 1)
    
    # 显示距离信息
    cv2.putText(result, f"A4 Dist: {a4_distance:.1f}mm", (10, 220), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 0), 2)
    
    min_edge_mm = min_edge_mm*1.1213

    cv2.putText(result, f"add=2", 
                (10, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

    # 显示结果图像
    maix_img = image.cv2image(result)
    disp.show(maix_img)
    
    return round(a4_distance,3), round(min_edge_mm/10,3)
