#include "SYS.h"

int main(void)
{
	MOTOR_TIM8_GPIO_Init();
	MOTOR_TIM8_Init();
	LED_Init();	
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
	LED_Set(LED1,LED_ON);	
	SetAllMotorsDirection(Zheng,Zheng,Zheng,Zheng);
	MOTOR_Move(12800*0.2, step_accel, step_decel, set_speed);
	
    while (1)
    {
		OLED_ShowNum(0,0,123,3,12,1);
		OLED_Refresh();
    }
}
