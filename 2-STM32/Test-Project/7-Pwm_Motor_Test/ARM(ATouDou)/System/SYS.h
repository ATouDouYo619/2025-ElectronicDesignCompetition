#ifndef __SYS_H
#define __SYS_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include <string.h>

//关于电机
#include "Dir.h"
#include "Motor.h"
#include "Motor_Init.h"
#include "Encoder_Init.h"
#include "MotorDriver.h"

//串口通讯
#include "UART.h"
#include "UART2.h"
#include "UART3.h"
#include "UART4.h"

//其他外设
#include "OLED.h"
#include "Servo.h"
#include "LED.h"
#include "Buzz.h"
#include "Key.h"
#include "jy61p.h"

#endif
