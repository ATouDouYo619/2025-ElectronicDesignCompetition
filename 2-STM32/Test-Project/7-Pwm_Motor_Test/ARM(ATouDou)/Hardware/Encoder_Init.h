#ifndef __ENCODER_INIT_H
#define __ENCODER_INIT_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"

// 编码器枚举
typedef enum 
{
    ENCODER1 = 0,  // TIM5 (PA0-PA1)
    ENCODER2        // TIM4 (PB6-PB7)
} Encoder_ID;

// 初始化函数
void Encoder_Init(void);

// 获取编码器计数值（有符号16位）
int16_t Encoder_Get(Encoder_ID id);

#endif
