#ifndef __Motor_Init_H
#define __Motor_Init_H

#include "Motor.h"
#include "math.h"
#include "stm32f10x.h" // Device header

// 输出比较模式周期设置0xFFFF
#define TIM_PERIOD 0xFFFF

#define TIM8_CLK RCC_APB2Periph_GPIOC
#define TIM8_STEP_PORT GPIOC
#define TIM8_STEP_PIN1 GPIO_Pin_6  // CH1
#define TIM8_STEP_PIN2 GPIO_Pin_7  // CH2

#define DIR_CLK RCC_APB2Periph_GPIOB
#define DIR_PORT GPIOB
#define DIR_PIN1 GPIO_Pin_12
#define DIR_PIN2 GPIO_Pin_13
#define DIR_PIN3 GPIO_Pin_14
#define DIR_PIN4 GPIO_Pin_15

// 定时器3引脚定义
#define TIM3_CLK RCC_APB2Periph_GPIOB
#define TIM3_STEP_PORT GPIOB
#define TIM3_STEP_PIN3 GPIO_Pin_4  // CH1
#define TIM3_STEP_PIN4 GPIO_Pin_5  // CH2

void MOTOR_TIM8_GPIO_Init(void);
void MOTOR_TIM8_Init(void);
void MOTOR_TIM3_GPIO_Init(void);
void MOTOR_TIM3_Init(void);

#endif
