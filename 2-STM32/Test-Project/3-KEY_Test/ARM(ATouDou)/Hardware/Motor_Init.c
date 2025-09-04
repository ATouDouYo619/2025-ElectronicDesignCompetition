#include "Motor_Init.h"
#include "Motor.h"
#include "stm32f10x.h" // Device header

void MOTOR_TIM8_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(STEP_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(DIR_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // 脉冲引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = STEP_Pin_1 | STEP_Pin_2 | STEP_Pin_4 | STEP_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STEP_PORT, &GPIO_InitStructure);

    // 控制引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = DIR_Pin_1 | DIR_Pin_2 | DIR_Pin_3 | DIR_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DIR_PORT, &GPIO_InitStructure);
}

void MOTOR_TIM8_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    TIM_InternalClockConfig(TIM8);

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_PRESCALER;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;

    // 通道1初始化
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_1, TIM_CCx_Disable);

    // 通道2初始化
    TIM_OC2Init(TIM8, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_2, TIM_CCx_Disable);
    
    // 通道3初始化
    TIM_OC3Init(TIM8, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_3, TIM_CCx_Disable);
    
    // 通道4初始化
    TIM_OC4Init(TIM8, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_4, TIM_CCx_Disable);

    // 标志位并启用中断
    TIM_ClearFlag(TIM8, TIM_FLAG_CC1);
    TIM_ITConfig(TIM8, TIM_IT_CC1, ENABLE);

    TIM_ClearFlag(TIM8, TIM_FLAG_CC2);
    TIM_ITConfig(TIM8, TIM_IT_CC2, ENABLE);
    
    // 通道3标志位清除和中断启用
    TIM_ClearFlag(TIM8, TIM_FLAG_CC3);
    TIM_ITConfig(TIM8, TIM_IT_CC3, ENABLE);
    
    // 通道4标志位清除和中断启用
    TIM_ClearFlag(TIM8, TIM_FLAG_CC4);
    TIM_ITConfig(TIM8, TIM_IT_CC4, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

