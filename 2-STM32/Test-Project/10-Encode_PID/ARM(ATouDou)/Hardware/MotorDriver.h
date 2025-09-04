#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H

#include "stm32f10x.h"
#include "stdint.h"
#include "Motor_PID.h"

// 引脚定义
#define MOTOR_A_PWM_PIN     GPIO_Pin_6    // PA6
#define MOTOR_B_PWM_PIN     GPIO_Pin_7    // PA7
#define MOTOR_A_IN1_PIN     GPIO_Pin_0    // PC0
#define MOTOR_A_IN2_PIN     GPIO_Pin_1    // PC1
#define MOTOR_B_IN1_PIN     GPIO_Pin_4    // PA4
#define MOTOR_B_IN2_PIN     GPIO_Pin_5    // PA5

// 端口定义
#define MOTOR_PWM_PORT      GPIOA
#define MOTOR_A_DIR_PORT    GPIOC
#define MOTOR_B_DIR_PORT    GPIOA

// PWM配置
#define MOTOR_PWM_TIM       TIM3
#define MOTOR_PWM_ARR       100 - 1
#define MOTOR_PWM_PSC       720 - 1

// 函数声明
void MotorDriver_Init(void);
void left_motor(int16_t speed);
void right_motor(int16_t speed);
void left_stop(void);
void right_stop(void);
void all_motors_stop(void);

#endif
