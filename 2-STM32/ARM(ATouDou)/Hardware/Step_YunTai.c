#include "Step_YunTai.h"
#include "math.h"

// 全局云台对象
static StepYunTai_TypeDef yunTai;

float BiLi;

/**
 * @brief 初始化云台
 */
void YunTai_Init(void) 
{
    yunTai.current_x = 0;
    yunTai.current_y = 0;
    yunTai.target_x = 0;
    yunTai.target_y = 0;
    yunTai.delta_x = 0;
    yunTai.delta_y = 0;
    
    // 圆弧参数初始化
    yunTai.center_x = 0;
    yunTai.center_y = 0;
    yunTai.radius = 0;
    yunTai.start_angle = 0;
    yunTai.end_angle = 0;
    yunTai.arc_direction = ARC_CW;
    yunTai.segment_count = 0;
    yunTai.current_segment = 0;
    
    yunTai.state = YUNTAI_IDLE;
    yunTai.interpolation_mode = LINEAR_INTERPOLATION;
    yunTai.speed = 2;    // 默认基准速度
    yunTai.accel = 1;     // 默认加速度
    yunTai.decel = 1;     // 默认减速度
}

/**
 * @brief 设置云台运动参数
 * @param speed 基准速度参数
 * @param accel 加速度参数
 * @param decel 减速度参数
 */
void YunTai_SetSpeed(uint32_t speed, uint32_t accel, uint32_t decel) 
{
    yunTai.speed = speed;
    yunTai.accel = accel;
    yunTai.decel = decel;
}

/**
 * @brief 移动到绝对位置
 * @param x 目标X坐标(步数)
 * @param y 目标Y坐标(步数)
 */
void YunTai_MoveTo(int32_t x, int32_t y) 
{
    if (yunTai.state == YUNTAI_MOVING) return; // 运动中不响应新指令
    
    // 更新目标位置和位移量
    yunTai.target_x = x;
    yunTai.target_y = y;
    yunTai.delta_x = x - yunTai.current_x;
    yunTai.delta_y = y - yunTai.current_y;
    
    // 执行直线插补
    yunTai.interpolation_mode = LINEAR_INTERPOLATION;
    YunTai_LinearInterpolation();
}

/**
 * @brief 相对当前位置移动
 * @param dx X方向相对步数
 * @param dy Y方向相对步数
 */
void YunTai_MoveRelative(int32_t dx, int32_t dy) 
{
    YunTai_MoveTo(yunTai.current_x + dx, yunTai.current_y + dy);
}

/**
 * @brief 圆弧运动
 * @param center_x 圆心X坐标
 * @param center_y 圆心Y坐标
 * @param radius 圆弧半径
 * @param start_angle 起始角度(弧度)
 * @param end_angle 终止角度(弧度)
 * @param direction 圆弧方向(ARC_CW顺时针, ARC_CCW逆时针)
 */
void YunTai_MoveArc(int32_t center_x, int32_t center_y, uint32_t radius, 
                   float start_angle, float end_angle, uint8_t direction)
{
    if (yunTai.state == YUNTAI_MOVING) return; // 运动中不响应新指令
    
    // 设置圆弧参数
    yunTai.center_x = center_x;
    yunTai.center_y = center_y;
    yunTai.radius = radius;
    yunTai.start_angle = start_angle;
    yunTai.end_angle = end_angle;
    yunTai.arc_direction = direction;
    
    // 根据弧度差计算段数，每1度一段
    float angle_diff = fabs(end_angle - start_angle);
    if (angle_diff < 0.0175f) angle_diff = 6.283f; // 约360度
    
    // 计算需要的段数，确保足够平滑
    yunTai.segment_count = (uint32_t)(angle_diff * 57.3f); // 弧度转角度
    if (yunTai.segment_count < 10) yunTai.segment_count = 10; // 最少10段
    
    yunTai.current_segment = 0;
    yunTai.interpolation_mode = CIRCULAR_INTERPOLATION;
    
    // 开始圆弧插补
    YunTai_CircularInterpolation();
}

/**
 * @brief 获取云台当前状态
 * @return 云台状态：YUNTAI_IDLE 或 YUNTAI_MOVING
 */
uint8_t YunTai_GetState(void) 
{
    YunTai_UpdatePosition();
    return yunTai.state;
}

/**
 * @brief 更新当前位置和运动状态
 */
void YunTai_UpdatePosition(void) 
{
    // 检查两轴是否都已停止
    uint8_t motor1_busy = (motor_sta != STOP);
    uint8_t motor4_busy = (motor_sta2 != STOP);
    
    if (motor1_busy || motor4_busy) 
    {
        yunTai.state = YUNTAI_MOVING;
    } 
    else 
    {
        // 运动完成，更新当前位置
        yunTai.current_x = yunTai.target_x;
        yunTai.current_y = yunTai.target_y;
        
        // 如果是圆弧插补且还有段未完成，则继续下一段
        if (yunTai.interpolation_mode == CIRCULAR_INTERPOLATION && 
            yunTai.current_segment < yunTai.segment_count)
        {
            YunTai_ContinueArc();
        }
        else
        {
            yunTai.state = YUNTAI_IDLE;
        }
    }
}

/**
 * @brief 继续圆弧插补的下一段
 */
void YunTai_ContinueArc(void)
{
    if (yunTai.current_segment >= yunTai.segment_count)
    {
        yunTai.state = YUNTAI_IDLE;
        return;
    }
    
    YunTai_CircularInterpolation();
}

/**
 * @brief 获取X轴当前位置
 * @return X轴位置(步数)
 */
int32_t YunTai_GetCurrentX(void) 
{
    return yunTai.current_x;
}

/**
 * @brief 获取Y轴当前位置
 * @return Y轴位置(步数)
 */
int32_t YunTai_GetCurrentY(void) 
{
    return yunTai.current_y;
}

/**
 * @brief 直线插补算法实现
 * 保证XY轴运动时间相同，实现严格直线轨迹
 */
void YunTai_LinearInterpolation(void) 
{
    if (yunTai.delta_x == 0 && yunTai.delta_y == 0) return;
    
    int32_t abs_dx = abs(yunTai.delta_x);
    int32_t abs_dy = abs(yunTai.delta_y);
    int32_t max_steps = (abs_dx > abs_dy) ? abs_dx : abs_dy;
    
    // 计算XY轴速度（保持比例）
    uint32_t x_speed = 0, y_speed = 0;
    if (max_steps > 0) 
    {
        x_speed = (yunTai.speed * abs_dx + max_steps - 1) / max_steps;
        y_speed = (yunTai.speed * abs_dy + max_steps - 1) / max_steps;
    }
    
    // 确保最小速度
    if (abs_dx > 0 && x_speed == 0) x_speed = 1;
    if (abs_dy > 0 && y_speed == 0) y_speed = 1;
    
    yunTai.state = YUNTAI_MOVING;
    
    // 启动X轴
    if (abs_dx > 0)
        Motor1_Move(yunTai.delta_x, x_speed, x_speed, x_speed);
    
    // 启动Y轴
    if (abs_dy > 0) 
        Motor4_Move(yunTai.delta_y, y_speed, y_speed, y_speed);
}

/**
 * @brief 圆弧插补算法实现
 * 将圆弧分成多段直线，逐段运动形成平滑圆弧
 */
void YunTai_CircularInterpolation(void) 
{
    if (yunTai.radius == 0) return;
    
    // 计算当前段的角度
    float angle_step = (yunTai.end_angle - yunTai.start_angle) / yunTai.segment_count;
    if (yunTai.arc_direction == ARC_CW && angle_step > 0)
        angle_step = -angle_step;
    if (yunTai.arc_direction == ARC_CCW && angle_step < 0)
        angle_step = -angle_step;
    
    // 计算当前段的起始和结束角度
    float current_angle = yunTai.start_angle + yunTai.current_segment * angle_step;
    float next_angle = current_angle + angle_step;
    
    // 计算圆弧上的点坐标
    int32_t current_x = yunTai.center_x + (int32_t)(yunTai.radius * cos(current_angle));
    int32_t current_y = yunTai.center_y + (int32_t)(yunTai.radius * sin(current_angle));
    int32_t next_x = yunTai.center_x + (int32_t)(yunTai.radius * cos(next_angle));
    int32_t next_y = yunTai.center_y + (int32_t)(yunTai.radius * sin(next_angle));
    
    // 设置目标位置和位移量
    yunTai.target_x = next_x;
    yunTai.target_y = next_y;
    yunTai.delta_x = next_x - current_x;
    yunTai.delta_y = next_y - current_y;
    
    // 使用直线插补运动到下一个点
    YunTai_LinearInterpolation();
    
    // 更新当前段索引
    yunTai.current_segment++;
}

//

