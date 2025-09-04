#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H

#include "stm32f10x.h"
#include "stdint.h"

#define MOTOR_A_PWM_PIN     GPIO_Pin_6    // PA6
#define MOTOR_B_PWM_PIN     GPIO_Pin_7    // PA7
#define MOTOR_A_IN1_PIN     GPIO_Pin_0     // PC0
#define MOTOR_A_IN2_PIN     GPIO_Pin_1     // PC1
#define MOTOR_B_IN1_PIN     GPIO_Pin_4     // PA4
#define MOTOR_B_IN2_PIN     GPIO_Pin_5     // PA5

#define MOTOR_PWM_PORT      GPIOA
#define MOTOR_A_DIR_PORT     GPIOC
#define MOTOR_B_DIR_PORT     GPIOA

#define MOTOR_PWM_TIM        TIM3
#define MOTOR_PWM_FREQ       20000       // 20kHz PWM频率

// 电机通道定义
typedef enum 
{
    MOTOR_A = 0,
    MOTOR_B
} Motor_Channel;

void MotorDriver_Init(void);								// 初始化函数
void Motor_SetSpeed(Motor_Channel channel, int16_t speed);	// 设置电机速度和方向
void Motor_Stop(Motor_Channel channel);						// 停止电机
void Motor_Brake(Motor_Channel channel);					// 刹车（短接电机两端）

#endif /* __MOTOR_DRIVER_H */

