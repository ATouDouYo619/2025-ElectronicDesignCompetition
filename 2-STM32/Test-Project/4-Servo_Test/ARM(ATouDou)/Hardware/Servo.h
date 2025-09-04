#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"
#include "stdint.h"

typedef enum {
    SERVO_PA15 = 0,  // TIM2_CH1 (PA15)
    SERVO_PB3,       // TIM2_CH2 (PB3)
    SERVO_PB0,       // TIM3_CH3 (PB0)
    SERVO_PB1        // TIM3_CH4 (PB1)
} Servo_Channel;

void Servo_Init(void);
void Servo_SetAngle(Servo_Channel ch, uint16_t angle);

#endif
