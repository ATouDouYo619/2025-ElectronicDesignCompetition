#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "stdint.h"
#include "UART.h"
#include "Delay.h"
#include <string.h>

// 原有定义保持不变
#define INA240_GAIN           	20.0f      
#define SHUNT_RESISTANCE      	0.01f       
#define ADC_REFERENCE_VOLTAGE 	3.3f      
#define ADC_RESOLUTION        	4096.0f    
#define Input_voltage         	5.0f
#define k						1.0245f

// 最大功率阈值
#define MAX_POWER_THRESHOLD   10.0f  // 最大允许功率10W

// 全局变量存储当前功率
extern float currentPower;
extern float INA240_VM,INA240_VM_2,INA240_Val;
extern float current;
extern float maxPower;  // 存储历史最大功率

void INA240_AD_Init(void);
uint16_t ADC_Read(uint8_t channel);
float ADC_ConvertToVoltage(uint16_t adcValue);
float Calculate_Current(uint16_t adcShuntValue);
float Read_BusVoltage(void);
float Read_ShuntVoltage(void);

float Read_Current(void);
  
// 功率计算和判断函数声明
float Calculate_Power(void);       // 计算当前功率
uint8_t IsPowerOverMax(void);      // 判断是否超过最大功率
void Send_HMI(void);

float Read_FilteredCurrent(void);

#endif
