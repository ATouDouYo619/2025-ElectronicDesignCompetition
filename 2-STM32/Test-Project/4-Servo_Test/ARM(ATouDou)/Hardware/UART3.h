#ifndef __UART3_H
#define __UART3_H

#include "stm32f10x.h"
#include "stdint.h"

#define UART3_Bound 		115200

void UART3_Init(int bound);
void UART3_SendByte(uint8_t Byte);

#endif
