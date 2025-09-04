#include "Buzz.h"

void Buzz_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // 配置PC12为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 初始状态关闭蜂鸣器
	GPIO_SetBits(GPIOC, GPIO_Pin_12);
}

void Buzz_On(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_12);  // 低电平触发
}

// 关闭蜂鸣器
void Buzz_Off(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_12);    // 高电平关闭
}

// 蜂鸣指定时间(阻塞式)
void Buzz_Beep(uint32_t duration_ms)
{
    Buzz_On();
    Delay_ms(duration_ms);
    Buzz_Off();
}

