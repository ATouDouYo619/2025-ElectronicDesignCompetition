#ifndef __BUZZ_H
#define __BUZZ_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"
#include "Delay.h"

void Buzz_Init(void);
void Buzz_On(void);
void Buzz_Off(void);

void Buzz_Beep(uint32_t duration_ms);

#endif
