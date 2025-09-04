#ifndef __UART2_H
#define __UART2_H

#include "stm32f10x.h"
#include "stdint.h"
#include <stddef.h>

#define MaixCam_Bound 115200

// 环形缓冲区大小定义
#define UART2_BUFFER_SIZE 512

typedef struct 
{
    // 结束符0xCC、0xDD）
    float distance;    // 距离 D (cm)
    float length;      // 边长 X (cm)
    uint8_t valid;     // 原包有效性标志
    
    // 结束符0xEE、0xFF）
    float new_distance;// 距离
    float new_length;  // 边长
    uint8_t new_valid; // 有效性标志
} UART2_DataPacket;

// 环形缓冲区结构定义
typedef struct 
{
    uint8_t buffer[UART2_BUFFER_SIZE];
    uint16_t head;   // 写入指针
    uint16_t tail;   // 读取指针
    uint16_t count;  // 数据数量
} UART2_RingBuffer;

// 初始化函数
void UART2_MaixCam_Init(uint32_t baudRate);
void UART2_MaixCam_SendByte(uint8_t Byte);

// 环形缓冲区操作函数
void UART2_Buffer_Init(UART2_RingBuffer *buf);
uint8_t UART2_Buffer_Write(UART2_RingBuffer *buf, uint8_t data);
uint8_t UART2_Buffer_Read(UART2_RingBuffer *buf, uint8_t *data);
uint16_t UART2_Buffer_Count(UART2_RingBuffer *buf);
void UART2_Buffer_Clear(UART2_RingBuffer *buf);

// 数据包解析函数
uint8_t UART2_ParsePacket(UART2_RingBuffer *buf, UART2_DataPacket *packet);

// 外部声明环形缓冲区
extern UART2_RingBuffer uart2_rx_buffer;
extern UART2_DataPacket packet;
#endif

