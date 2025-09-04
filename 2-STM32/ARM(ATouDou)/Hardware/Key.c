#include "Key.h"

uint8_t last_key_num = 0;
uint8_t last_key_num2 = 0;
uint8_t last_key_num3 = 0;
uint8_t last_key_num4 = 0;  // 新增按键4状态变量
uint8_t last_key_num5 = 0;  // 新增按键5状态变量
u8 key_flag = 1;
u8 Key_num = 0;

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能时钟（已包含GPIOA，无需修改）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置KEY1 (PC3) 和 KEY2 (PC2)
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT1, &GPIO_InitStructure);
    
    // 配置KEY3 (PA12)、KEY4 (PA4)、KEY5 (PA5) 新增按键4和5的配置
    GPIO_InitStructure.GPIO_Pin = KEY3_PIN | KEY4_PIN | KEY5_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT2, &GPIO_InitStructure);
}

void Key_Scan(void)
{
    if(key_flag == KEY_RELEASED && 
       (!GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN) ||   // KEY1按下（低电平）
        !GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN) ||   // KEY2按下（低电平）
        !GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN) ||   // KEY3按下（低电平）
        !GPIO_ReadInputDataBit(KEY_PORT2, KEY4_PIN) ||   // KEY4按下（低电平）新增
        !GPIO_ReadInputDataBit(KEY_PORT2, KEY5_PIN)))    // KEY5按下（低电平）新增
    {
        key_flag = KEY_PRESSED;
        Delay_ms(20);  // 消抖
        
        if(!GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN))      Key_num = 1;
        else if(!GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN)) Key_num = 2;
        else if(!GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN)) Key_num = 3;
        else if(!GPIO_ReadInputDataBit(KEY_PORT2, KEY4_PIN)) Key_num = 4;  // 新增按键4判断
        else if(!GPIO_ReadInputDataBit(KEY_PORT2, KEY5_PIN)) Key_num = 5;  // 新增按键5判断
    }
    else if(GPIO_ReadInputDataBit(KEY_PORT1, KEY1_PIN) &&   // KEY1释放（高电平）
            GPIO_ReadInputDataBit(KEY_PORT1, KEY2_PIN) &&   // KEY2释放（高电平）
            GPIO_ReadInputDataBit(KEY_PORT2, KEY3_PIN) &&   // KEY3释放（高电平）
            GPIO_ReadInputDataBit(KEY_PORT2, KEY4_PIN) &&   // KEY4释放（高电平）新增
            GPIO_ReadInputDataBit(KEY_PORT2, KEY5_PIN))     // KEY5释放（高电平）新增
    {
        key_flag = KEY_RELEASED;
        Key_num = 0; 
    }
}

void Base_component(void)
{
    // 按键1处理逻辑
    if (Key_num == 1 && last_key_num != 1)
    {
        UART2_MaixCam_SendByte(0x01);
        last_key_num = 1;
    }
    else if (Key_num == 0)
    {
        last_key_num = 0;
    }
    
    // 按键2处理逻辑
    if (Key_num == 2 && last_key_num2 != 1)
    {
        UART2_MaixCam_SendByte(0x02);
        last_key_num2 = 1;
    }
    else if (Key_num == 0)
    {
        last_key_num2 = 0;
    }

    // 按键3处理逻辑
    if (Key_num == 3 && last_key_num3 != 1)
    {
        UART2_MaixCam_SendByte(0x03);
        last_key_num3 = 3;
    }
    else if (Key_num == 0)
    {
        last_key_num3 = 0;
    }
    
    // 按键4处理逻辑
    if (Key_num == 4 && last_key_num4 != 1)
    {
        UART2_MaixCam_SendByte(0x04);  // 发送0x04指令
        last_key_num4 = 1;
    }
    else if (Key_num == 0)
    {
        last_key_num4 = 0;
    }
    
    // 按键5处理逻辑
    if (Key_num == 5 && last_key_num5 != 1)
    {
        UART2_MaixCam_SendByte(0x05);  // 发送0x05指令
        last_key_num5 = 1;
    }
    else if (Key_num == 0)
    {
        last_key_num5 = 0;
    }
}
