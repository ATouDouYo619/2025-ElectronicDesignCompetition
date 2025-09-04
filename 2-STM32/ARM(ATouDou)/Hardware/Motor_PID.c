#include "Motor_PID.h"

Motor_TypeDef motorLeft = {.expectSpeed = 20,.kp = 4.22f,.ki = 0.753f,.kd = 0.001f};
Motor_TypeDef motorRight = {.expectSpeed = 30,.kp = 4.22f,.ki = 0.753f,.kd = 0.001f};

void MotorController_Init(Motor_TypeDef *motor) 
{
    motor->currentSpeed = 0.0f;
    motor->err = 0.0f;
    motor->lastErr = 0.0f;
    motor->pwm = 0.0f;
    motor->direction = 0;
    motor->integral = 0.0f; // 初始化积分项
}

// 绝对值函数
int16_t myabs(int a)
{ 		   
    return (a < 0) ? -a : a;
}

// PID计算
float MotorController_CalcPID(Motor_TypeDef *motor)
{
if (motor->expectSpeed > 0.1f) {
        motor->direction = 1;
    } else if (motor->expectSpeed < -0.1f) {
        motor->direction = 2;
    } else {
        motor->direction = 0;
    }

    float currentErr = motor->expectSpeed - motor->currentSpeed;
    
    motor->integral += currentErr;
    
    if(motor->integral > MAX_INTEGRAL) motor->integral = MAX_INTEGRAL;
    if(motor->integral < -MAX_INTEGRAL) motor->integral = -MAX_INTEGRAL;
    
    float derivative = currentErr - motor->lastErr;
    
    // PID计算
    float pTerm = motor->kp * currentErr;
    float iTerm = motor->ki * motor->integral;
    float dTerm = motor->kd * derivative;
    
    // 更新PWM输出
    motor->pwm = pTerm + iTerm + dTerm;
    
    // PWM限幅

    if(motor->pwm > PWM_MAX) motor->pwm = PWM_MAX;
    if(motor->pwm < PWM_MIN) motor->pwm = PWM_MIN;
    

    motor->lastErr = motor->err;
    motor->err = currentErr;
    

    
    return motor->pwm;
}
