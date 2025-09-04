#ifndef __TIME6_H
#define __TIME6_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"
#include "Encoder_Init.h"
#include "MotorDriver.h"
#include "Motor_PID.h"

void TIM6_Init(void);

extern u32 num;
extern int16_t SpeedA, SpeedB;
extern int16_t Expect_SpeedA;
extern int16_t Expect_SpeedB;

#endif
