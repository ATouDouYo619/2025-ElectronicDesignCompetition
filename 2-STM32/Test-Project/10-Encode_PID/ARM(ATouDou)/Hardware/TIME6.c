#include "TIME6.h"

u32 num;
int16_t SpeedA, SpeedB;
int16_t Expect_SpeedA = 20;
int16_t Expect_SpeedB = 30;

// TIM6初始化 (10ms定时)
void TIM6_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    
    TIM_TimeBaseInitStructure.TIM_Period = 1000-1;       // ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;   // PSC
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);
    
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;        // TIM6中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM6, ENABLE);
}

// TIM6中断服务函数
void TIM6_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) 
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
