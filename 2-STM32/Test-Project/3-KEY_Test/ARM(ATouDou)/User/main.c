#include "SYS.h"

int main(void)
{
	KEY_Init();
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
    while (1)
    {
		Key_Scan();
		OLED_ShowNum(0,0,Key_num,1,12,1);
		OLED_Refresh();
    }
}
