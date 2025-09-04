#include "Motor_Init.h"
#include "Motor.h"
#include "stm32f10x.h" // Device header

void MOTOR_TIM8_GPIO_Init(void)
{
    // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(TIM8_CLK | DIR_CLK, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置定时器通道引脚 (PC6, PC7)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = TIM8_STEP_PIN1 | TIM8_STEP_PIN2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TIM8_STEP_PORT, &GPIO_InitStructure);
    
    // 配置方向控制引脚 (PB12-PB15)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = DIR_PIN1 | DIR_PIN2 | DIR_PIN3 | DIR_PIN4;
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

#if 0   // 注释掉通道3和4
    // 通道3初始化
    TIM_OC3Init(TIM8, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_3, TIM_CCx_Disable);
    
    // 通道4初始化
    TIM_OC4Init(TIM8, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_CCxCmd(TIM8, TIM_Channel_4, TIM_CCx_Disable);
#endif

    // 标志位并启用中断
    TIM_ClearFlag(TIM8, TIM_FLAG_CC1);
    TIM_ITConfig(TIM8, TIM_IT_CC1, ENABLE);

    TIM_ClearFlag(TIM8, TIM_FLAG_CC2);
    TIM_ITConfig(TIM8, TIM_IT_CC2, ENABLE);
	
#if 0   // 注释掉通道3和4    
    // 通道3标志位清除和中断启用
    TIM_ClearFlag(TIM8, TIM_FLAG_CC3);
    TIM_ITConfig(TIM8, TIM_IT_CC3, ENABLE);
    
    // 通道4标志位清除和中断启用
    TIM_ClearFlag(TIM8, TIM_FLAG_CC4);
    TIM_ITConfig(TIM8, TIM_IT_CC4, ENABLE);
#endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
	
	TIM_CtrlPWMOutputs(TIM8, ENABLE);
}

void MOTOR_TIM3_GPIO_Init(void)
{
    // 使能GPIOC时钟和AFIO时钟（重映射需要）
    RCC_APB2PeriphClockCmd(TIM3_CLK | RCC_APB2Periph_AFIO, ENABLE);
    
    // 将TIM3完全重映射到
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置TIM3通道3和4引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = TIM3_STEP_PIN3 | TIM3_STEP_PIN4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TIM3_STEP_PORT, &GPIO_InitStructure);
}

void MOTOR_TIM3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65536-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_PRESCALER;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
		
	TIM_OCStructInit(&TIM_OCInitStructure);	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
		
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_CCxCmd(TIM3,TIM_Channel_1,TIM_CCx_Disable);
	
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Disable); 
	TIM_CCxCmd(TIM3,TIM_Channel_2,TIM_CCx_Disable);

	TIM_ClearFlag(TIM3,TIM_FLAG_CC1);															
	TIM_ITConfig(TIM3,TIM_IT_CC1,ENABLE);														

	TIM_ClearFlag(TIM3,TIM_FLAG_CC2);																
	TIM_ITConfig(TIM3,TIM_IT_CC2,ENABLE);										

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);								
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}
