#include "UART2.h"

// 定义环形缓冲区实例
UART2_DataPacket packet;
UART2_RingBuffer uart2_rx_buffer;

void UART2_MaixCam_Init(uint32_t baudRate)
{
    // 初始化环形缓冲区
    UART2_Buffer_Init(&uart2_rx_buffer);
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // USART2 TX (PA2)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART2 RX (PA3)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    // 使能接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // 配置中断优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    // 使能USART2
    USART_Cmd(USART2, ENABLE);    
}

// 初始化环形缓冲区
void UART2_Buffer_Init(UART2_RingBuffer *buf)
{
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
}

// 向环形缓冲区写入数据
uint8_t UART2_Buffer_Write(UART2_RingBuffer *buf, uint8_t data)
{
    // 检查缓冲区是否已满
    if (buf->count >= UART2_BUFFER_SIZE) 
	{
        return 0;  // 缓冲区满，写入失败
    }
    
    buf->buffer[buf->head] = data;
    buf->head = (buf->head + 1) % UART2_BUFFER_SIZE;
    buf->count++;
    return 1;  // 写入成功
}

// 从环形缓冲区读取数据
uint8_t UART2_Buffer_Read(UART2_RingBuffer *buf, uint8_t *data)
{
    // 检查缓冲区是否为空
    if (buf->count == 0) 
	{
        return 0;  // 缓冲区空，读取失败
    }
    
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % UART2_BUFFER_SIZE;
    buf->count--;
    return 1;  // 读取成功
}

// 获取缓冲区中的数据数量
uint16_t UART2_Buffer_Count(UART2_RingBuffer *buf)
{
    return buf->count;
}

// 清空环形缓冲区
void UART2_Buffer_Clear(UART2_RingBuffer *buf)
{
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
}

// 解析数据包
uint8_t UART2_ParsePacket(UART2_RingBuffer *buf, UART2_DataPacket *packet)
{
    // 状态定义：0-找AA，1-找BB，2-收数据1，3-收数据2，4-收数据3，5-收数据4
    // 6-等结束符1（CC或EE），7-等DD（原包），8-等FF（新包）
    uint8_t state = 0;  
    uint8_t data[8];    // 存储完整数据包（8字节）
    uint16_t temp;
    
    // 确保缓冲区数据足够（两种包都是8字节）
    if (UART2_Buffer_Count(buf) < 8) 
        return 0;
    
    // 重置有效性标志
    packet->valid = 0;
    packet->new_valid = 0;
    
    while (UART2_Buffer_Count(buf) > 0) 
    {
        uint8_t byte;
        UART2_Buffer_Read(buf, &byte);
        
        switch (state) 
        {
            case 0:  // 寻找包头0xAA
                if (byte == 0xAA) 
                {
                    state = 1;
                    data[0] = byte;
                }
                break;
                
            case 1:  // 已找到AA，寻找0xBB
                if (byte == 0xBB) 
                {
                    state = 2;
                    data[1] = byte;
                } 
                else 
                {
                    state = 0; // 错误，重置
                    if (byte == 0xAA) // 若当前是AA，重新开始
                    {
                        state = 1;
                        data[0] = byte;
                    }
                }
                break;
                
            // 接收4个数据字节（两种包数据部分格式相同）
            case 2: data[2] = byte; state = 3; break;
            case 3: data[3] = byte; state = 4; break;
            case 4: data[4] = byte; state = 5; break;
            case 5: data[5] = byte; state = 6; break;
                
            case 6:  // 等待结束符第一字节（CC或EE）
                if (byte == 0xCC) 
                {
                    data[6] = byte;
                    state = 7; // 下一步找DD
                }
                else if (byte == 0xEE) 
                {
                    data[6] = byte;
                    state = 8; // 下一步找FF
                }
                else 
                {
                    state = 0; // 错误，重置
                    if (byte == 0xAA) 
                    {
                        state = 1;
                        data[0] = byte;
                    }
                }
                break;
                
            case 7:  // 验证结束符0xDD
                if (byte == 0xDD) 
                {
                    data[7] = byte;
                    // 解析原包数据
                    temp = (data[2] << 8) | data[3];
                    packet->distance = temp / 100.0f;
                    temp = (data[4] << 8) | data[5];
                    packet->length = temp / 100.0f;
                    packet->valid = 1; // 标记原包有效
                    return 1;
                }
                else 
                {
                    state = 0; // 错误，重置
                    if (byte == 0xAA) state = 1;
                }
                break;
                
            case 8:  // 验证结束符0xFF
                if (byte == 0xFF) 
                {
                    data[7] = byte;
                    // 解析数据（假设数据格式与原包相同）
                    temp = (data[2] << 8) | data[3];
                    packet->new_distance = temp / 100.0f;
                    temp = (data[4] << 8) | data[5];
                    packet->new_length = temp / 100.0f;
                    packet->new_valid = 1; // 标记新包有效
                    return 1;
                }
                else 
                {
                    state = 0; // 错误，重置
                    if (byte == 0xAA) state = 1;
                }
                break;
                
            default: state = 0; break;
        }
    }
    return 0; // 未找到完整包
}

void UART2_MaixCam_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
}
// 串口中断处理函数
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t data = USART_ReceiveData(USART2);  // 读取接收的数据
        UART2_Buffer_Write(&uart2_rx_buffer, data); // 写入环形缓冲区
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);  // 清除接收中断标志
    }
}

