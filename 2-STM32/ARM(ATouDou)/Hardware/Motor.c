#include "Motor.h"
#include "Dir.h"
#include "Motor_Init.h"
#include "stm32f10x.h" // Device header
#include <math.h>

//uint32_t set_speed  = 1; 		// 最大速度,	为speed*0.1*rad/sec
//uint32_t step_accel = 1; 		// 加速度,		为decel*0.1*rad/sec^2
//uint32_t step_decel = 1; 		// 减速度,		为accel*0.1*rad/sec^2

speedRampData srd = {STOP, 0, 0, 0, 0, 0, 0}; // 加减速变量
speedRampData srd2 = {STOP, 0, 0, 0, 0, 0, 0}; // 加减速变量

uint8_t motor_sta = 0; // 电机状态
uint8_t motor_sta2 = 0; // 电机状态

/**
 * @brief 电机加减速
 * @param step   步数
 * @param accel  加速度，	为accel*0.1*rad/sec^2  10倍且2个脉冲
 * @param decel  减速度，	为decel*0.1*rad/sec^2
 * @param speed  最大速度，	为speed*0.1*rad/sec
 * @return null
 */

void MOTOR_Move(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed)
{

    uint16_t tim_count;     // 用于存放中断时刻的计数值
    unsigned int max_s_lim; // 达到最大速度时的步数
    unsigned int accel_lim; // 必须开始减速的步数（如果还没有加速度到最大速度时）

    if (motor_sta != STOP) // 只允许步进电机在停止时才能继续
        return;
    if (step < 0) // 逆时针
    {
        step = -step;
    }
    if (step == 1) // 如果只移动一步
    {
        srd.accel_count = -1;  // 只走一步
        srd.run_state = DECEL; // 减速状态
        srd.step_delay = 1000; // 短延时
    }
    else if (step != 0) // 步数为0时才能移动
    {
        srd.min_delay = (int32_t)(A_T_x10 / speed);								// 计算最大速度对应的最小延时
        srd.step_delay = (int32_t)((T1_FREQ_148 * sqrt(A_SQ / accel)) / 10);	// 计算初始延时（基于加速度）
        // tep_delay = 1/tt * sqrt(2*alpha/accel)
        // step_delay = ( tfreq*0.69/10 )*10 * sqrt( (2*alpha*100000) / (accel*10) )/100
		
		/**
			​​最大速度可达性判断​​：
			max_s_lim：达到设定最大速度speed所需的最小步数（加速度为accel时）
			accel_lim：在总步数step约束下，为了平稳停止而必须开始减速的位置
			​​运行模式选择​​：
			​​情况1（三角形模式）​​：当max_s_lim > accel_lim时
			意味着在加速到设定最大速度前就必须开始减速（距离不够）
			此时运动是三角形速度曲线（只有加速和减速，没有匀速段）
			减速起点 = step - accel_lim
			​​情况2（梯形模式）​​：当max_s_lim <= accel_lim时
			可以正常加速到最大速度，并保持一段匀速后再减速
			减速起点 = step - (max_s_lim + (max_s_lim * accel / decel))
		**/
		// 1. 计算达到最大速度所需的最小步数（加速段持续时间）
        max_s_lim = (uint32_t)(speed * speed / (A_x200 * accel / 10));			

        if (max_s_lim == 0)
        {
            max_s_lim = 1;
        }
		// 2. 计算必须开始减速的步数位置（减速段起点）
        accel_lim = (uint32_t)(step * decel / (accel + decel));					// 计算必须开始减速的步数位置

        if (accel_lim == 0)
        {
            accel_lim = 1;
        }

        if (accel_lim <= max_s_lim)
        {
			// 情况1：无法达到设定的最大速度（三角形模式）
            srd.decel_val = accel_lim - step;
        }
        else
        {
			// 情况2：能达到最大速度（梯形模式）
            srd.decel_val = -(max_s_lim * accel / decel);
        }

        if (srd.decel_val == 0)
        {
            srd.decel_val = -1;
        }
        srd.decel_start = step + srd.decel_val;									// 确定减速点

        if (srd.step_delay <= srd.min_delay)
        {
            srd.step_delay = srd.min_delay;
            srd.run_state = RUN;
        }
        else
        {
            srd.run_state = ACCEL;
        }

        srd.accel_count = 0;
    }
    motor_sta = 1;
	
    tim_count = TIM_GetCounter(TIM8);
    TIM_SetCompare1(TIM8, tim_count + srd.step_delay / 2);
    TIM_ITConfig(TIM8, TIM_IT_CC1, ENABLE);
    TIM_CCxCmd(TIM8, TIM_Channel_1, TIM_CCx_Enable);

    TIM_SetCompare2(TIM8, tim_count + srd.step_delay / 2);
    TIM_ITConfig(TIM8, TIM_IT_CC2, ENABLE);
    TIM_CCxCmd(TIM8, TIM_Channel_2, TIM_CCx_Enable);
	
#if 0
    TIM_SetCompare1(TIM8, tim_count + srd.step_delay / 2);
    TIM_ITConfig(TIM8, TIM_IT_CC3, ENABLE);
    TIM_CCxCmd(TIM8, TIM_Channel_3, TIM_CCx_Enable);

    TIM_SetCompare2(TIM8, tim_count + srd.step_delay / 2);
    TIM_ITConfig(TIM8, TIM_IT_CC4, ENABLE);
    TIM_CCxCmd(TIM8, TIM_Channel_4, TIM_CCx_Enable);
#endif	
	
    TIM_Cmd(TIM8, ENABLE);

}

/**
 * @brief T型加减速决策
 * 放在中断中执行
 */
void speed_decision(void)
{
    __IO uint32_t tim_count = 0;
    __IO uint32_t tmp = 0;
    uint16_t new_step_delay = 0;
    __IO static uint16_t last_accel_delay = 0;
    __IO static uint32_t step_count = 0;
    __IO static int32_t rest = 0;
    __IO static uint8_t i = 0;

    // 检查所有四个通道的中断标志
    uint8_t int_flag = (TIM_GetITStatus(TIM8, TIM_IT_CC1) == SET) || 
                       (TIM_GetITStatus(TIM8, TIM_IT_CC2) == SET);

    if (int_flag)
    {
        // 清除所有四个通道的中断标志
        TIM_ClearITPendingBit(TIM8, TIM_IT_CC1 | TIM_IT_CC2);
        
        tim_count = TIM_GetCounter(TIM8);
        tmp = tim_count + srd.step_delay / 2;
        
        // 更新所有四个通道的比较值
        TIM_SetCompare1(TIM8, tmp);
        TIM_SetCompare2(TIM8, tmp);
        
        i++;
        if (i == 2)
        {
            i = 0;
            switch (srd.run_state)
            {
            case STOP:
                step_count = 0;
                rest = 0;
                
                // 禁用所有四个通道
				TIM_ClearITPendingBit(TIM8, TIM_IT_CC1);
                TIM_CCxCmd(TIM8, TIM_Channel_1, TIM_CCx_Disable);
				TIM_ClearITPendingBit(TIM8, TIM_IT_CC2);
				TIM_CCxCmd(TIM8, TIM_Channel_2, TIM_CCx_Disable);

                
                TIM_Cmd(TIM8, DISABLE);
                motor_sta = 0;
                break;

            case ACCEL:
                step_count++;
                srd.accel_count++;
                new_step_delay = srd.step_delay - (((2 * srd.step_delay) + rest) / (4 * srd.accel_count + 1));
                rest = ((2 * srd.step_delay) + rest) % (4 * srd.accel_count + 1);

                if (step_count >= srd.decel_start)
                {
                    srd.accel_count = srd.decel_val;
                    srd.run_state = DECEL;
                }
                else if (new_step_delay <= srd.min_delay)
                {
                    last_accel_delay = new_step_delay;
                    new_step_delay = srd.min_delay;
                    rest = 0;
                    srd.run_state = RUN;
                }
                break;

            case RUN:
                step_count++;
                new_step_delay = srd.min_delay;
                if (step_count >= srd.decel_start)
                {
                    srd.accel_count = srd.decel_val;
                    new_step_delay = last_accel_delay;
                    srd.run_state = DECEL;
                }
                break;

            case DECEL:
                step_count++;
                srd.accel_count++;
                new_step_delay = srd.step_delay - (((2 * srd.step_delay) + rest) / (4 * srd.accel_count + 1));
                rest = ((2 * srd.step_delay) + rest) % (4 * srd.accel_count + 1);
                if (srd.accel_count >= 0)
                {
                    srd.run_state = STOP;
                }
                break;
            }
            srd.step_delay = new_step_delay;
        }
    }
}
void TIM8_CC_IRQHandler(void) 
{
    // 通道1触发决策
    if(TIM_GetITStatus(TIM8, TIM_IT_CC1) == SET) 
	{
        TIM_ClearITPendingBit(TIM8, TIM_IT_CC1);
        speed_decision();
    }
    // 清除其他通道中断
    if(TIM_GetITStatus(TIM8, TIM_IT_CC2) == SET) TIM_ClearITPendingBit(TIM8, TIM_IT_CC2);
}
//
void MOTOR_Move3(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed)
{
		
    uint16_t tim_count; 														//用于存放中断时刻的计数值
    unsigned int max_s_lim;                                     				//达到最大速度时的步数
    unsigned int accel_lim;														//必须开始减速的步数（如果还没有加速度到最大速度时）

    if(motor_sta2!= STOP)  														//只允许步进电机在停止时才能继续
			return;			
    if(step < 0)   																//逆时针
		{
			step = -step;      
		}		     
    if(step == 1)   															//如果只移动一步
    {       
      srd2.accel_count = -1; 													//只走一步
      srd2.run_state = DECEL;													//减速状态
      srd2.step_delay = 1000;													//短延时
     }
	 else if(step != 0)  														//步数为0时才能移动
    {					
				srd2.min_delay = (int32_t)(A_T_x10/speed);
				srd2.step_delay = (int32_t)((T1_FREQ_148 * sqrt(A_SQ / accel))/10);
																																						 //tep_delay = 1/tt * sqrt(2*alpha/accel)
																																						 //step_delay = ( tfreq*0.69/10 )*10 * sqrt( (2*alpha*100000) / (accel*10) )/100 
			  max_s_lim = (uint32_t)(speed*speed/(A_x200*accel/10));
    
				if(max_s_lim == 0)																							
				{
					max_s_lim = 1;
				}    
				accel_lim = (uint32_t)(step*decel/(accel+decel)); 									 
   
				if(accel_lim == 0) 																									
				{
					accel_lim = 1;
				}
   
				if(accel_lim <= max_s_lim)																					 
				{
					srd2.decel_val = accel_lim - step;																	
				}
				else
				{
					srd2.decel_val = -(max_s_lim*accel/decel);													 
				}
				
				if(srd2.decel_val == 0) 																						
				{
					srd2.decel_val = -1;
				}    
				srd2.decel_start = step + srd2.decel_val;															
				
				if(srd2.step_delay <= srd2.min_delay)																	
				{
					srd2.step_delay = srd2.min_delay;
					srd2.run_state = RUN;
				}
				else
				{
					srd2.run_state = ACCEL;
				}
    
				srd2.accel_count = 0;																								
		
			}
			motor_sta2 = 1;  																										
			tim_count = TIM_GetCounter(TIM3);																			
			
			TIM_SetCompare1(TIM3,tim_count+srd2.step_delay/2);												
			TIM_ITConfig(TIM3,TIM_IT_CC1,ENABLE);																		
			TIM_CCxCmd(TIM3,TIM_Channel_1,TIM_CCx_Enable);
			
			TIM_SetCompare2(TIM3,tim_count+srd2.step_delay/2);												
			TIM_ITConfig(TIM3,TIM_IT_CC2,ENABLE);															
			TIM_CCxCmd(TIM3,TIM_Channel_2,TIM_CCx_Enable);
			
			TIM_Cmd(TIM3, ENABLE);																							
}

void speed_decision3(void)                                       
{
		__IO uint32_t tim_count=0;
		__IO uint32_t tmp = 0;  
	  uint16_t new_step_delay=0;                                         
	  __IO static uint16_t last_accel_delay=0;                             
	  __IO static uint32_t step_count = 0; 															
	  __IO static int32_t rest = 0;																						
	  __IO static uint8_t i=0;																								
	 
	  if ((TIM_GetITStatus(TIM3, TIM_IT_CC1)== SET) ||(TIM_GetITStatus(TIM3, TIM_IT_CC2)== SET))
	  {	  

			TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);															
			TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
		  tim_count = TIM_GetCounter(TIM3);	
			tmp = tim_count+srd2.step_delay/2;

			TIM_SetCompare1(TIM3,tmp);																				
			TIM_SetCompare2(TIM3,tmp);
			i++; 
			if(i==2)																														
			{
				i=0; 
				switch(srd2.run_state)
				{
					case STOP:																											
						step_count = 0;
						rest = 0;
					
						TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
					  TIM_CCxCmd(TIM3,TIM_Channel_1,TIM_CCx_Disable);
						TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
					  TIM_CCxCmd(TIM3,TIM_Channel_2,TIM_CCx_Disable);

					  TIM_Cmd(TIM3, DISABLE);					                     
						motor_sta2 = 0;  
						break;
					
					case ACCEL:																									
					step_count++;
					srd2.accel_count++;
					new_step_delay = srd2.step_delay - (((2 *srd2.step_delay) + rest)/(4 * srd2.accel_count + 1));
					rest = ((2 * srd2.step_delay)+rest)%(4 * srd2.accel_count + 1);	   
					
					if(step_count >= srd2.decel_start) 														
					{
						srd2.accel_count = srd2.decel_val;															
						srd2.run_state = DECEL;																			
					}
					
					else if(new_step_delay <= srd2.min_delay)												
					{
						last_accel_delay = new_step_delay;														
						new_step_delay = srd2.min_delay;   														
						rest = 0;            																		     
						srd2.run_state = RUN;																			
					}
					break;
					
					case RUN:
			  step_count++;  																									  
			  new_step_delay = srd2.min_delay;   																	 
			  if(step_count >= srd2.decel_start)   														
						{
				srd2.accel_count = srd2.decel_val;  																
				new_step_delay = last_accel_delay;																	
				srd2.run_state = DECEL;           															
			  }
			  break;
						
					case DECEL:
			  step_count++;  																				

			  srd2.accel_count++; 																						
			  new_step_delay = srd2.step_delay - (((2 * srd2.step_delay) + rest)/(4 * srd2.accel_count + 1)); 
			  rest = ((2 * srd2.step_delay)+rest)%(4 * srd2.accel_count + 1);			
			  if(srd2.accel_count >= 0) 																					
			  {
					srd2.run_state = STOP;
			  }
			  break;
				}
			srd2.step_delay = new_step_delay; 																
			}
		}
}
//void Motor1_Move(int32_t step)
//{
//    if (step < 0) // 逆时针
//    {
//		SetMotor_Direction(MOTOR1, Fun);
//        step = -step;
//    }
//	else
//	{
//		SetMotor_Direction(MOTOR1, Zheng);
//        step = step;
//	}
//	MOTOR_Move(step,step_accel,step_decel,set_speed);
//}
//void Motor2_Move(int32_t step)
//{
//    if (step < 0) // 逆时针
//    {
//		SetMotor_Direction(MOTOR2, Fun);
//        step = -step;
//    }
//	else
//	{
//		SetMotor_Direction(MOTOR2, Zheng);
//        step = step;
//	}
//	MOTOR_Move(step,step_accel,step_decel,set_speed);
//}
//void Motor3_Move(int32_t step)
//{
//    if (step < 0) // 逆时针
//    {
//		SetMotor_Direction(MOTOR3, Fun);
//        step = -step;
//    }
//	else
//	{
//		SetMotor_Direction(MOTOR3, Zheng);
//        step = step;
//	}
//	MOTOR_Move3(step,step_accel,step_decel,set_speed);
//}
//void Motor4_Move(int32_t step)
//{
//    if (step < 0) // 逆时针
//    {
//		SetMotor_Direction(MOTOR4, Fun);
//        step = -step;
//    }
//	else
//	{
//		SetMotor_Direction(MOTOR4, Zheng);
//        step = step;
//	}
//	MOTOR_Move3(step,step_accel,step_decel,set_speed);
//}

void Motor1_Move(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed) {
    if (step < 0) { // 逆时针
        SetMotor_Direction(MOTOR1, Fun);
        step = -step;
    } else {
        SetMotor_Direction(MOTOR1, Zheng);
    }
    // 直接使用传入的参数，而非全局变量
    MOTOR_Move(step, accel, decel, speed);
}

void Motor4_Move(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed) {
    if (step < 0) { // 逆时针
        SetMotor_Direction(MOTOR4, Fun);
        step = -step;
    } else {
        SetMotor_Direction(MOTOR4, Zheng);
    }
    // 直接使用传入的参数，而非全局变量
    MOTOR_Move3(step, accel, decel, speed);
}

void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_CC1) || TIM_GetITStatus(TIM3, TIM_IT_CC2)) 
        speed_decision3();
}
