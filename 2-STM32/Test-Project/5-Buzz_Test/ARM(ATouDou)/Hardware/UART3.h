#ifndef __UART3_H
#define __UART3_H

#include "stm32f10x.h"
#include "stdint.h"

#define Code_Bound 		115200

void UART3_Code_Init(int bound);

#endif
