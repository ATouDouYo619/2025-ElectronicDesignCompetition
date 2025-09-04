#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header
#include "stdint.h"
#include "Delay.h"
#include "UART2.h"

#define KEY_PRESSED  1
#define KEY_RELEASED 0

// 按键配置 (PC2/PC3/PA12/PA4/PA5)
#define KEY_PORT1     GPIOC
#define KEY_PORT2     GPIOA
#define KEY1_PIN      GPIO_Pin_3    // PC3
#define KEY2_PIN      GPIO_Pin_2    // PC2
#define KEY3_PIN      GPIO_Pin_12   // PA12
#define KEY4_PIN      GPIO_Pin_4    // PA4 
#define KEY5_PIN      GPIO_Pin_5    // PA5 

void KEY_Init(void);
void Key_Scan(void);
void Base_component(void);

extern u8 Key_num;
extern uint8_t last_key_num;
extern uint8_t last_key_num2;
extern uint8_t last_key_num3;
extern uint8_t last_key_num4;  
extern uint8_t last_key_num5;  
#endif

