#ifndef __STEP_YUNTAI_H
#define __STEP_YUNTAI_H

#include "stm32f10x.h"
#include "stdint.h"
#include "Motor.h"
#include "stdlib.h"
#include "math.h"  // 用于数学计算

// 定义圆周率常量
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// 轴标识定义
#define X_AXIS 0
#define Y_AXIS 1

// 云台状态定义
#define YUNTAI_IDLE     0
#define YUNTAI_MOVING   1

// 插补模式
#define LINEAR_INTERPOLATION 0
#define CIRCULAR_INTERPOLATION 1  // 新增圆弧插补模式

// 圆弧方向
#define ARC_CW 0      // 顺时针
#define ARC_CCW 1     // 逆时针

// 云台状态结构体
typedef struct {
    int32_t current_x;       // X轴当前位置(步数)
    int32_t current_y;       // Y轴当前位置(步数)
    int32_t target_x;        // X轴目标位置(步数)
    int32_t target_y;        // Y轴目标位置(步数)
    int32_t delta_x;         // X轴需要移动的步数
    int32_t delta_y;         // Y轴需要移动的步数
    
    // 圆弧插补参数
    int32_t center_x;        // 圆心X坐标
    int32_t center_y;        // 圆心Y坐标
    uint32_t radius;         // 圆弧半径
    float start_angle;       // 起始角度(弧度)
    float end_angle;         // 终止角度(弧度)
    uint8_t arc_direction;   // 圆弧方向
    
    uint8_t state;           // 云台状态
    uint8_t interpolation_mode; // 插补模式
    uint32_t speed;          // 基准移动速度
    uint32_t accel;          // 加速度
    uint32_t decel;          // 减速度
    uint32_t segment_count;  // 圆弧段数
    uint32_t current_segment;// 当前段索引
} StepYunTai_TypeDef;

// 函数声明
void YunTai_Init(void);
void YunTai_SetSpeed(uint32_t speed, uint32_t accel, uint32_t decel);
void YunTai_MoveTo(int32_t x, int32_t y);
void YunTai_MoveRelative(int32_t dx, int32_t dy);
uint8_t YunTai_GetState(void);
void YunTai_UpdatePosition(void);
int32_t YunTai_GetCurrentX(void);
int32_t YunTai_GetCurrentY(void);
void YunTai_LinearInterpolation(void);

// 圆弧插补相关函数
void YunTai_MoveArc(int32_t center_x, int32_t center_y, uint32_t radius, 
                   float start_angle, float end_angle, uint8_t direction);
void YunTai_CircularInterpolation(void);
void YunTai_ContinueArc(void);

#endif
 
