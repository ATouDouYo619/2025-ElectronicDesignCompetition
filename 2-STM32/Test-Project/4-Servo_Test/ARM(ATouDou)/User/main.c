#include "SYS.h"

int main(void)
{
	Servo_Init();
	
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
	Servo_SetAngle(SERVO_PA15,200);
	Servo_SetAngle(SERVO_PB3,135);
	Servo_SetAngle(SERVO_PB0,90);
	Servo_SetAngle(SERVO_PB1,10);
	
    while (1)
    {

    }
}
