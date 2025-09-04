#include "SYS.h"

int16_t a,b;

int main(void)
{
	LED_Init();
	TIM6_Init();
	Encoder_TIM4_Init();
	Encoder_TIM5_Init();
	
	OLED_Init();
	OLED_ColorTurn(0);
	OLED_DisplayTurn(0);
	OLED_Clear();
	
	LED_Set(LED2,LED_ON);
	
    while (1)
    {
		OLED_ShowNum(0,0,a,4,12,1);
		OLED_ShowNum(0,12,b,4,12,1); 
		OLED_ShowNum(0,24,num,4,12,1); 
		OLED_ShowNum(0,36,EncoderA,4,12,1); 
		OLED_ShowNum(0,48,EncoderB,4,12,1); 
		OLED_Refresh();
    }
}
