#ifndef __OLED_H
#define __OLED_H

// #include "sys.h"
#include "stdlib.h"
#include "stm32f10x.h" // Device header

//-----------------OLED端口定义----------------
#define OLED_SCL_PIN GPIO_Pin_8
#define OLED_SCL_PORT GPIOB
#define OLED_SCL_CLK RCC_APB2Periph_GPIOB

#define OLED_SDA_PIN GPIO_Pin_9
#define OLED_SDA_PORT GPIOB
#define OLED_SDA_CLK RCC_APB2Periph_GPIOB

#define OLED_RES_PIN GPIO_Pin_2
#define OLED_RES_PORT GPIOA
#define OLED_RES_CLK RCC_APB2Periph_GPIOA

#define OLED_SCL_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_8) // SCL
#define OLED_SCL_Set() GPIO_SetBits(GPIOB, GPIO_Pin_8)

#define OLED_SDA_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_9) // DIN
#define OLED_SDA_Set() GPIO_SetBits(GPIOB, GPIO_Pin_9)

#define OLED_RES_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_2) // RES
#define OLED_RES_Set() GPIO_SetBits(GPIOA, GPIO_Pin_2)

#define OLED_CMD 0  // 写命令
#define OLED_DATA 1 // 写数据

void OLED_ClearPoint(u8 x, u8 y);
void OLED_ColorTurn(u8 i);   // 反显函数
void OLED_DisplayTurn(u8 i); // 屏幕旋转180度
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u8 dat);
void OLED_WR_Byte(u8 dat, u8 mode);
void OLED_DisPlay_On(void);                                               // 开启OLED显示
void OLED_DisPlay_Off(void);                                              // 关闭OLED显示
void OLED_Refresh(void);                                                  // 更新显存到OLED
void OLED_Clear(void);                                                    // 清屏函数
void OLED_DrawPoint(u8 x, u8 y, u8 t);                                    // 画点
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode);                  // 画线
void OLED_DrawCircle(u8 x, u8 y, u8 r);                                   // 画圆
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1, u8 mode);                // 显示一个字符
void OLED_ShowChar6x8(u8 x, u8 y, u8 chr, u8 mode);                       //
void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1, u8 mode);             // 显示字符串
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode);        // 显示数字
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1, u8 mode);             // 显示汉字
void OLED_ScrollDisplay(u8 num, u8 space, u8 mode);                       // num 显示汉字的个数
void OLED_ShowPicture(u8 x, u8 y, u8 sizex, u8 sizey, u8 BMP[], u8 mode); // 显示图片
void OLED_Init(void);

void OLED_ShowFNum(u8 x, u8 y, float num, u8 frac_len, u8 size1, u8 mode); // 显示浮点数
void OLED_ShowSignedNum(u8 x, u8 y, s32 num, u8 len, u8 size1, u8 mode, u8 show_plus);

#endif
