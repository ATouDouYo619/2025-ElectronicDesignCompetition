#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
#include "stdint.h"

#define LED_PORT    GPIOC
#define LED1        GPIO_Pin_13
#define LED2        GPIO_Pin_14
#define LED3        GPIO_Pin_15
#define LED_ALL     (LED1 | LED2 | LED3)

#define LED_ON      1
#define LED_OFF     0

void LED_Init(void);
void LED_Set(uint16_t led_pins, uint8_t state);

#endif

