#include "Servo.h"

void Servo_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 1. 禁用JTAG释放PB3
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    // 2. 完全重映射TIM2 (使用PA15和PB3)
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // TIM2_CH2 (PB3), TIM3_CH3 (PB0), TIM3_CH4 (PB1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 20000 - 1;     // 20ms周期
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;     // 1MHz计数频率
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OCInitStructure.TIM_Pulse = 0; 
    
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);  // PA15
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);  // PB3
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);  // PB0
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);  // PB1
    
    // 6. 启用预加载
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    // 7. 使能定时器但保持初始无输出
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}
void Servo_SetAngle(Servo_Channel ch, uint16_t angle) 
{
    angle = (angle > 270) ? 270 : angle;
    
    uint16_t pulse = 500 + (angle * 2000) / 270;
    
    switch(ch) 
	{
        case SERVO_PA15:
            TIM_SetCompare1(TIM2, pulse);
            break;
        case SERVO_PB3:
            TIM_SetCompare2(TIM2, pulse);
            break;
        case SERVO_PB0:
            TIM_SetCompare3(TIM3, pulse);
            break;
        case SERVO_PB1:
            TIM_SetCompare4(TIM3, pulse);
            break;
    }
}


