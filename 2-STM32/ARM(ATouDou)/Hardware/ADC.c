#include "ADC.h"

float INA240_VM,INA240_VM_2,INA240_Val;
float currentPower;
float current;
float maxPower = 0.0f;
float filtered = 0.0f;

// ADC初始化函数
void INA240_AD_Init(void)
{
    // 使能ADC1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置ADC时钟，PCLK2分频6，ADC工作频率为12MHz
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    
    // 配置GPIOA的A0和A1为模拟输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;      // 模拟输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;  // A0和A1引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 速度不影响模拟输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置ADC1
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // 关闭扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;             // 转换通道数量
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // 使能ADC1
    ADC_Cmd(ADC1, ENABLE);
    
    // ADC校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));  // 等待校准重置完成
    
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));      // 等待校准完成
}

// 读取指定通道的ADC值
uint16_t ADC_Read(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 
    return ADC_GetConversionValue(ADC1);
}

// 将ADC值转换为电压值
float ADC_ConvertToVoltage(uint16_t adcValue)
{
    // 计算公式：电压 = (ADC值 / ADC分辨率) * 参考电压

    return (adcValue / ADC_RESOLUTION) * ADC_REFERENCE_VOLTAGE;
}

// 计算电流值
float Calculate_Current(uint16_t adcShuntValue)
{
    // 1. 将ADC值转换为电压
    float vout = ADC_ConvertToVoltage(adcShuntValue);
    
    const float vref = ADC_REFERENCE_VOLTAGE / 2.0f;
    
    // 3. 计算电流（单位：安培A）
    //    I = (Vout - Vref) / (Rshunt × Gain)
    float current = (vout - vref) / (SHUNT_RESISTANCE * INA240_GAIN);
    
    return current;
}

// 读取总线电压
float Read_BusVoltage(void)
{
    uint16_t adcValue = ADC_Read(ADC_Channel_1);  // A1通道读取总线电压
	INA240_VM = ADC_ConvertToVoltage(adcValue);
	INA240_VM_2 = INA240_VM/2;
    return INA240_VM;
}

// 读取分流电压(INA240输出)
float Read_ShuntVoltage(void)
{
    uint16_t adcValue = ADC_Read(ADC_Channel_0);  // A0通道读取INA240输出
	INA240_Val = ADC_ConvertToVoltage(adcValue);
    return INA240_Val;
}

// 直接读取电流值
float Read_Current(void)
{
    uint16_t shuntAdc = ADC_Read(ADC_Channel_0);  // A0读取INA240输出
    uint16_t busAdc = ADC_Read(ADC_Channel_1);    // A1读取总线电压
    return Calculate_Current(shuntAdc);
}
// 计算当前功率（功率 = 总线电压 × 电流，单位：瓦特W）
float Calculate_Power(void)
{
    // 读取最新的电流
    float current = Read_Current();     		// 电流（A）
    currentPower = Input_voltage * current;   	// 功率计算：P = U × I

    // 比较当前功率与历史最大功率，若更大则更新
    if (currentPower > maxPower)
    {
        maxPower = currentPower;  // 更新历史最大功率
    }

    return currentPower;
}

// 判断当前功率是否超过最大功率阈值
uint8_t IsPowerOverMax(void)
{
    // 返回1表示超过，0表示未超过
    return (currentPower > MAX_POWER_THRESHOLD) ? 1 : 0;
}

//发送数据
void Send_HMI(void)
{
	HMI_send_float("t5.txt", filtered);
	Delay_ms(20);
	HMI_send_float("t12.txt", currentPower);
	Delay_ms(20);
    HMI_send_float("t13.txt", maxPower);
}

float Read_FilteredCurrent(void) 
{
    static float filter_buffer[5] = {0};  // 原始值缓存
    static float median_buffer[5] = {0};  // 中值结果缓存
    static uint8_t index = 0;
    static uint8_t median_index = 0;      // 中值缓存索引
    
    // 1. 获取原始值
    float raw = Read_Current()*0.96;
    
    // 2. 中值滤波
    filter_buffer[index] = raw;
    index = (index + 1) % 5;
    
    float temp[5];
    memcpy(temp, filter_buffer, sizeof(temp));
    for(int i=0; i<4; i++) 
    {
        for(int j=i+1; j<5; j++) 
        {
            if(temp[j] < temp[i]) 
            {
                float swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }
    float median = temp[2];
    
    // 3. 均值滤波（对中值结果求平均）
    median_buffer[median_index] = median;
    median_index = (median_index + 1) % 5;
    float mean_sum = 0;
    for(int i = 0; i < 5; i++) {
        mean_sum += median_buffer[i];
    }
    filtered = mean_sum / 5;  // 双重滤波最终值
    
    return filtered;
}
//

