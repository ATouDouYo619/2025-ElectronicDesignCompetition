#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"
#include "Delay.h"

#define KEY_PRESSED  1
#define KEY_RELEASED 0

// 按键配置 (PC2/PC3/PA12)
#define KEY_PORT1     GPIOC
#define KEY_PORT2     GPIOA
#define KEY1_PIN      GPIO_Pin_2    // PC2
#define KEY2_PIN      GPIO_Pin_3    // PC3
#define KEY3_PIN      GPIO_Pin_12   // PA12

void KEY_Init(void);
void Key_Scan(void);

extern u8 Key_num;

#endif
