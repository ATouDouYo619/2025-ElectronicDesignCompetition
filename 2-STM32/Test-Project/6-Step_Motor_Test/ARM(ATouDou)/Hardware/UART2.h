#ifndef __UART2_H
#define __UART2_H

#include "stm32f10x.h"
#include "stdint.h"
#include <stddef.h>

#define MaixCam_Bound 9600

void UART2_MaixCam_Init(uint32_t baudRate);

#endif

