#ifndef __TIME6_H
#define __TIME6_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"
#include "Encoder_Init.h"

void TIM6_Init(void);

extern u32 num;
extern int16_t EncoderA, EncoderB;

#endif
