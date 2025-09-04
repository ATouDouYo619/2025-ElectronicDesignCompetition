#ifndef __UART2_H
#define __UART2_H

#include "stm32f10x.h"
#include "stdint.h"
#include <stddef.h>

#define UART2_Bound 115200

void UART2_Init(uint32_t baudRate);
void UART2_SendByte(uint8_t Byte);

#endif

