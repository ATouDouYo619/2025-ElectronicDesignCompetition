from maix import camera, display, image, app
import cv2
import numpy as np

"""
识别数字的正方形组合图形的思路，我的是使用模型的办法，但是因为队友的模型没炼丹成功....
所以这个发挥部分没有做,只做了找A4纸的距离(当然是随便找一个,hhhh)
"""

W_mm = 296.5   # A4纸短边实际宽度（毫米）

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

def Find_Num_Shape(disp, img, focal_length):

    # 转换图像格式
    cv2_img = image.image2cv(img)
    result = cv2_img.copy()
    a4_distance = 0.0
    Line = 63.412
    
    # 1. 检测A4纸轮廓
    a4_contour, thresh_img = detect_a4_contour(cv2_img)
    if a4_contour is None:
        cv2.putText(result, "No A4 detected", (10, 20), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
        maix_img = image.cv2image(result)
        disp.show(maix_img)
        return 0.0
    
    # 2. 计算A4像素宽度和实际距离
    w_pixel = calculate_pixel_width(a4_contour)
    a4_distance = (focal_length * W_mm) / w_pixel
    
    # 显示A4纸轮廓
    cv2.drawContours(result, [a4_contour], -1, (0, 255, 0), 2, lineType=cv2.LINE_AA)
    
    # 显示距离信息
    cv2.putText(result, f"A4 Dist: {a4_distance:.1f}mm", (10, 220), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 0), 2)
    cv2.putText(result, f"add=3", 
                (10, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
   
    # 显示结果图像
    maix_img = image.cv2image(result)
    disp.show(maix_img)
    
    return round(a4_distance, 3),round(Line/10,3)
