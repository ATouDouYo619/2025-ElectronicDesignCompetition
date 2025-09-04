#include "Key.h"

u8 key_flag = 1;
u8 Key_num = 0;

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置KEY1 (PC2) 和 KEY2 (PC3)
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT1, &GPIO_InitStructure);
    
    // 配置KEY3 (PA12)
    GPIO_InitStructure.GPIO_Pin = KEY3_PIN;
    GPIO_Init(KEY_PORT2, &GPIO_InitStructure);
}

void Key_Scan(void)
{
    if(key_flag == KEY_RELEASED && 
       (!GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN) ||   
        !GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN) || 
         GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN))) 
    {
        key_flag = KEY_PRESSED;
        Delay_ms(20);  // 消抖
        
        if(!GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN))			Key_num = 1;
        else if(!GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN))	Key_num = 2;
        else if(GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN))		Key_num = 3;
    }
    else if(GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN) && 
            GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN) &&
           !GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN))
    {
        key_flag = KEY_RELEASED;
        Key_num = 0; 
    }
}


