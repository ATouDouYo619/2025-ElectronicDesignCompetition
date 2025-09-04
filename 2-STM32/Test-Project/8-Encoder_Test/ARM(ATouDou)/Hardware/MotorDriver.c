#include "MotorDriver.h"

void MotorDriver_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 2. 配置PWM引脚 (PA6, PA7)
    GPIO_InitStructure.GPIO_Pin = MOTOR_A_PWM_PIN | MOTOR_B_PWM_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PWM_PORT, &GPIO_InitStructure);
    
    // 3. 配置方向控制引脚
    // 电机A方向控制 (PC0, PC1)
    GPIO_InitStructure.GPIO_Pin = MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_A_DIR_PORT, &GPIO_InitStructure);
    
    // 电机B方向控制 (PA4, PA5)
    GPIO_InitStructure.GPIO_Pin = MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN;
    GPIO_Init(MOTOR_B_DIR_PORT, &GPIO_InitStructure);
    
    // 4. 初始化所有方向引脚为低电平
    GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
    GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
    
    // 5. 配置TIM3 PWM
    // 计算预分频值 (72MHz时钟)
    uint16_t prescaler = SystemCoreClock / (MOTOR_PWM_FREQ * 1000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;         // 1000级分辨率
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;      // 设置PWM频率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(MOTOR_PWM_TIM, &TIM_TimeBaseStructure);
    
    // 6. 配置PWM输出
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比0%
    
    // 通道1配置 (PA6 - 电机A)
    TIM_OC1Init(MOTOR_PWM_TIM, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(MOTOR_PWM_TIM, TIM_OCPreload_Enable);
    
    // 通道2配置 (PA7 - 电机B)
    TIM_OC2Init(MOTOR_PWM_TIM, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(MOTOR_PWM_TIM, TIM_OCPreload_Enable);
    
    // 7. 启动定时器
    TIM_Cmd(MOTOR_PWM_TIM, ENABLE);
}

// 设置电机速度和方向
void Motor_SetSpeed(Motor_Channel channel, int16_t speed)
{
    // 限制速度范围 (-1000 到 +1000)
    if (speed > 1000) speed = 1000;
    if (speed < -1000) speed = -1000;
    
    // 根据电机通道处理
    switch (channel) 
	{
        case MOTOR_A:
            if (speed > 0) 
			{
                // 正转
                GPIO_SetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN);
                GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN);
            } 
			else if (speed < 0) 
			{
                // 反转
                GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN);
                GPIO_SetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN);
            } 
			else 
			{
                // 停止
                GPIO_ResetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
            }
            // 设置PWM占空比 (取绝对值)
            TIM_SetCompare1(MOTOR_PWM_TIM, (speed < 0) ? -speed : speed);
            break;
        case MOTOR_B:
            if (speed > 0) 
			{
                // 正转
                GPIO_SetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN);
                GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN);
            } 
			else if (speed < 0) 
			{
                // 反转
                GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN);
                GPIO_SetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN);
            } 
			else 
			{
                // 停止
                GPIO_ResetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
            }
            // 设置PWM占空比
            TIM_SetCompare2(MOTOR_PWM_TIM, (speed < 0) ? -speed : speed);
            break;
            
        default:
            // 无效通道处理
            break;
    }
}
// 停止电机（自由停止）
void Motor_Stop(Motor_Channel channel)
{
    Motor_SetSpeed(channel, 0);
}

// 刹车（短接电机两端）
void Motor_Brake(Motor_Channel channel)
{
    if (channel == MOTOR_A) 
	{
        GPIO_SetBits(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
        TIM_SetCompare1(MOTOR_PWM_TIM, 0);
    } 
	else if (channel == MOTOR_B) 
	{
        GPIO_SetBits(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
        TIM_SetCompare2(MOTOR_PWM_TIM, 0);
    }
}
