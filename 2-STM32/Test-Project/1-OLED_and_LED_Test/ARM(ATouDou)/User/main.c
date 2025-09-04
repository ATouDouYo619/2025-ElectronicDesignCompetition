#include "SYS.h"

int main(void)
{
	LED_Init();
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
	LED_Set(LED1 | LED2 | LED3,LED_ON);
    while (1)
    {
		OLED_ShowNum(0,0,123,3,12,1);
		OLED_Refresh();
    }
}
