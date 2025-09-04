#include "SYS.h"

int main(void)
{
	LED_Init();
	MotorDriver_Init();
	
	LED_Set(LED1,LED_ON);
	Motor_SetSpeed(MOTOR_A,500);
	Motor_SetSpeed(MOTOR_B,-500);
	
    while (1)
    {

    }
}
