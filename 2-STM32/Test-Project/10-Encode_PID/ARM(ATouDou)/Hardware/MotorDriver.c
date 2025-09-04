#include "MotorDriver.h"

// 电机驱动初始化
void MotorDriver_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 2. 配置PWM引脚 (PA6, PA7)
    GPIO_InitStructure.GPIO_Pin = MOTOR_A_PWM_PIN | MOTOR_B_PWM_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PWM_PORT, &GPIO_InitStructure);
    
    // 3. 配置方向控制引脚
    GPIO_InitStructure.GPIO_Pin = MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_A_DIR_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN;
    GPIO_Init(MOTOR_B_DIR_PORT, &GPIO_InitStructure);
    
    // 初始化方向引脚
    GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
    GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
    
    // 4. 配置TIM3为PWM模式
    TIM_TimeBaseStructure.TIM_Period = MOTOR_PWM_ARR;        // ARR 
    TIM_TimeBaseStructure.TIM_Prescaler = MOTOR_PWM_PSC;     // PSC 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 配置PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比0%
    
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    // 5. 配置TIM3更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM3, ENABLE);
}

void left_motor(int16_t speed) 
{
    uint8_t pwm = (uint8_t)myabs(speed);
    
    if (pwm > 99) pwm = 99;
    TIM_SetCompare1(MOTOR_PWM_TIM, pwm);
    
    if (speed > 0) 
	{
        GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN);
        GPIO_SetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN);
    } 
	else if (speed < 0) 
	{
        GPIO_SetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN);
        GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN);
    } 
	else 
	{
        left_stop();
    }
}

// 右电机控制
void right_motor(int16_t speed) 
{
    uint8_t pwm = (uint8_t)myabs(speed);
    
    if (pwm > 99) pwm = 99;
    TIM_SetCompare2(MOTOR_PWM_TIM, pwm);
    
    if (speed > 0) 
	{
        GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN);
        GPIO_SetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN);
    } 
	else if (speed < 0) 
	{
        GPIO_SetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN);
        GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN);
    } 
	else 
	{
        right_stop();
    }
}

// 停止函数保持不变
void left_stop(void)
{
    TIM_SetCompare1(MOTOR_PWM_TIM, 0);
    GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
}

void right_stop(void)
{
    TIM_SetCompare2(MOTOR_PWM_TIM, 0);
    GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
}

void all_motors_stop(void)
{
    left_stop();
    right_stop();
}

