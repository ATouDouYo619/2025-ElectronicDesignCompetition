#include "SYS.h"

volatile uint16_t num = 0;  
volatile uint8_t Send_flag = 1;
extern float filtered;

int main(void)
{
    // 系统初始化 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    KEY_Init();
    OLED_Init();
	TIM6_Init();
    INA240_AD_Init(); 
    UART1_Scrn_Init(Scrn_Bound);
    UART2_MaixCam_Init(MaixCam_Bound);
        
    while (1)
    {
        Key_Scan();  // 扫描按键，更新Key_num
        current = Read_Current();					
        Calculate_Power(); 							
        filtered_current = Read_FilteredCurrent();	
		
        /* 调试显示 */
        OLED_ShowFNum(0, 0, packet.distance,  3, 12, 1);
        OLED_ShowFNum(0, 12, packet.length,  3, 12, 1);
        OLED_ShowFNum(0, 36, packet.new_distance,  3, 12, 1);
        OLED_ShowFNum(0, 48, packet.new_length,  3, 12, 1);		
        OLED_ShowNum(100,0,Key_num,1,12,1);
        OLED_Refresh();
        
		if (Send_flag == 0)
		{
			Send_HMI();
			Send_flag = 1;  
		}		
//		printf("%.3f\r\n",filtered);	// VOFA 调试函数
		
		Base_component();	// 按键触发采集,基础部分和发挥	
		
        if(UART2_ParsePacket(&uart2_rx_buffer, &packet))	
        {
            if(packet.valid)
            {
                HMI_send_float("t3.txt", packet.distance);
                HMI_send_float("t4.txt", packet.length);
            }
			if(packet.new_valid)
			{
				HMI_send_float("t10.txt", packet.new_distance);
				HMI_send_float("t11.txt", packet.new_length);
			}
        }
    }
}

void TIM6_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) 
    {
        if (num++ >= 20)
        {
            Send_flag = 0;  // 触发发送
            num = 0;
        }
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
