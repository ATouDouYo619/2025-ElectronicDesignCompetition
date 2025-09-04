#include "SYS.h"

int main(void)
{
    // 系统初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    LED_Init();
    OLED_Init();

//	UART1_Scrn_Init(Scrn_Bound);	
    MotorDriver_Init();
    Encoder_TIM4_Init();  // 左电机编码器
    Encoder_TIM5_Init();  // 右电机编码器
    
    // OLED初始化设置
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Clear();
    LED_Set(LED1, LED_ON);

    // 初始化电机控制器
    MotorController_Init(&motorLeft);
    MotorController_Init(&motorRight);

    while (1)
    {
        // OLED显示数据
        OLED_ShowSignedNum(0, 12, motorRight.currentSpeed, 3, 12, 1, 1);
        OLED_ShowSignedNum(0, 24, motorLeft.currentSpeed, 3, 12, 1, 1);
        OLED_ShowSignedNum(0, 36, motorRight.expectSpeed, 3, 12, 1, 1);
        OLED_ShowSignedNum(0, 48, motorLeft.expectSpeed, 3, 12, 1, 1);
		
//        printf("%.1f,%.1f,%.1f,%.1f\r\n",motorRight.currentSpeed,motorLeft.currentSpeed,motorRight.expectSpeed,motorLeft.expectSpeed);
		
        OLED_Refresh();
    }
}

// TIM3中断服务程序 - 用于PID控制周期执行
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
    {
        time++;
        
        if(time == 10)
        {
            // 读取编码器速度
            motorRight.currentSpeed = Encoder_Get(ENCODER1);
            motorLeft.currentSpeed = Encoder_Get(ENCODER2);
            
            // 计算PID并设置电机速度
            pwmRight = MotorController_CalcPID(&motorRight);
            pwmLeft = MotorController_CalcPID(&motorLeft);
            
            right_motor((int16_t)pwmRight);
            left_motor((int16_t)pwmLeft);
            time = 0;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
