#ifndef __DIR_H
#define __DIR_H

#include "stm32f10x.h"
#include "stdint.h"

#define GPIO_Type_DIR		GPIOB
#define GPIO_PIN1_DIR		GPIO_Pin_12
#define GPIO_PIN2_DIR		GPIO_Pin_13
#define GPIO_PIN3_DIR		GPIO_Pin_14
#define GPIO_PIN4_DIR		GPIO_Pin_15

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} MotorControl;

typedef enum {
    MOTOR1,
    MOTOR2,
    MOTOR3,
    MOTOR4
} MotorType;

typedef enum{
	Fun   = 0,
	Zheng = 1
}MotorDir;

typedef enum{
	Forward, 
	Backward,
	Right,
	Left,
}MoveDir;

void SetMotor_Direction(MotorType motor, MotorDir dir);
void SetAllMotorsDirection(MotorDir dir1, MotorDir dir2, MotorDir dir3, MotorDir dir4);	//设置多个步进电机方向;
void MoveSetDirection(MoveDir Behavior);

#endif

