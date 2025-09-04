#ifndef __Motor_Init_H
#define __Motor_Init_H

#include "Motor.h"
#include "math.h"
#include "stm32f10x.h" // Device header

// 输出比较模式周期设置0xFFFF
#define TIM_PERIOD 0xFFFF

#define STEP_CLK RCC_APB2Periph_GPIOC
#define STEP_PORT GPIOC
#define STEP_Pin_1 GPIO_Pin_6
#define STEP_Pin_2 GPIO_Pin_7
#define STEP_Pin_3 GPIO_Pin_8
#define STEP_Pin_4 GPIO_Pin_9

#define DIR_CLK RCC_APB2Periph_GPIOB
#define DIR_PORT GPIOB
#define DIR_Pin_1 GPIO_Pin_12
#define DIR_Pin_2 GPIO_Pin_13
#define DIR_Pin_3 GPIO_Pin_14
#define DIR_Pin_4 GPIO_Pin_15

void MOTOR_TIM8_GPIO_Init(void);
void MOTOR_TIM8_Init(void);

#endif
