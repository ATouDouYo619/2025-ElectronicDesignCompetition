#include "Encoder_Init.h"

void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    
    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 2. 配置GPIO引脚
    // 编码器1 (PA0-PA1) - TIM5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 编码器2 (PB6-PB7) - TIM4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 3. 配置TIM5 (Encoder1)
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 65535;        // ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;         // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);
    
    // 4. 配置TIM4 (Encoder2)
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    
    // 5. 配置输入捕获
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 0xF;  // 最大滤波
    
    // TIM5通道1
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM5, &TIM_ICInitStructure);
    
    // TIM5通道2
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM5, &TIM_ICInitStructure);
    
    // TIM4通道1
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    
    // TIM4通道2
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    
    // 6. 配置编码器模式
    TIM_EncoderInterfaceConfig(TIM5, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    // 7. 启动编码器
    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    
    // 8. 初始化计数器
    TIM_SetCounter(TIM4, 0);
    TIM_SetCounter(TIM5, 0);
}

// 获取编码器计数值（有符号16位）
int16_t Encoder_Get(Encoder_ID id)
{
    int16_t count;
    
    switch(id) {
        case ENCODER1:
            count = TIM_GetCounter(TIM5);
            TIM_SetCounter(TIM5, 0);
            break;
            
        case ENCODER2:
            count = TIM_GetCounter(TIM4);
            TIM_SetCounter(TIM4, 0);
            break;
            
        default:
            count = 0;
            break;
    }
    
    return count;
}
