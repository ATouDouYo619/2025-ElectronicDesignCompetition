#ifndef __MOTOR_PID_H
#define __MOTOR_PID_H

#include "stm32f10x.h"
#include "stdint.h"
#include "MotorDriver.h"

#define MAX_INTEGRAL 100.0f
#define PWM_MAX 99.0f
#define PWM_MIN -99.0f

// 电机控制结构体
typedef struct 
{
    float expectSpeed;    // 期望速度
    float currentSpeed;   // 当前速度
    float kp, ki, kd;     // PID参数
    float err;            // 累计误差
    float lastErr;        // 上次误差
    float pwm;            // PWM输出值
    int direction;        // 方向状态
    float integral;       // 积分项累计值
} Motor_TypeDef;

// 函数声明
void MotorController_Init(Motor_TypeDef *motor);
float MotorController_CalcPID(Motor_TypeDef *motor);
int16_t myabs(int a);

extern Motor_TypeDef motorLeft;
extern Motor_TypeDef motorRight;

#endif
