#include "SYS.h"

int main(void)
{
	LED_Init();
	UART4_Init(JY61_Bound);
	
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
	LED_Set(LED3,LED_ON);
	Jy61P_Init_Zero();
	
    while (1)
    {
		OLED_ShowNum(0,0,1234,4,12,1);
		OLED_ShowFNum(12,12,Roll,2,12,1);
		OLED_ShowFNum(12,24,Pitch,2,12,1);
		OLED_ShowFNum(12,36,Yaw,2,12,1);
		OLED_Refresh();
    }
}
