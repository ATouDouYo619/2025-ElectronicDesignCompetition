#include "LED.h"

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = LED_ALL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    
    GPIO_ResetBits(LED_PORT, LED_ALL); // 默认全部熄灭
}

void LED_Set(uint16_t led_pins, uint8_t state)
{
    if(state == LED_ON) 
        GPIO_SetBits(LED_PORT, led_pins);  	// 高电平点亮
	else 
        GPIO_ResetBits(LED_PORT, led_pins); // 低电平熄灭
}
