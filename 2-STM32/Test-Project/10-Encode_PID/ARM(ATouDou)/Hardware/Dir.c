#include "Dir.h"

MotorControl motors[4] = 
{
    {GPIO_Type_DIR, GPIO_PIN1_DIR}, 	// MOTOR1
    {GPIO_Type_DIR, GPIO_PIN2_DIR}, 	// MOTOR2
    {GPIO_Type_DIR, GPIO_PIN3_DIR}, 	// MOTOR3
    {GPIO_Type_DIR, GPIO_PIN4_DIR}, 	// MOTOR4
};										// Dir

void SetMotor_Direction(MotorType motor, MotorDir dir)	//设置单个步进电机方向
{
    if (dir == Fun) 
    {
		GPIO_SetBits(motors[motor].port, motors[motor].pin); // Set high for forward
	}
	else if (dir == Zheng) 
    {
		GPIO_ResetBits(motors[motor].port, motors[motor].pin); // Set low for backward
	}
}

void SetAllMotorsDirection(MotorDir dir1, MotorDir dir2, MotorDir dir3, MotorDir dir4)	//设置多个步进电机方向
{
    SetMotor_Direction(MOTOR1, dir1);
    SetMotor_Direction(MOTOR2, dir2);
    SetMotor_Direction(MOTOR3, dir3);
    SetMotor_Direction(MOTOR4, dir4);
}

//void MoveSetDirection(MoveDir Behavior)	
//{
//    switch (Behavior) 
//	{
//        case Forward:
//            SetAllMotorsDirection(Fun, Fun);
//            break;
//        case Backward:
//            SetAllMotorsDirection(Zheng, Zheng);
//            break;
//        case Right:
//            SetAllMotorsDirection(Fun, Zheng);
//            break;
//		case Left:
//            SetAllMotorsDirection(Zheng, Fun);
//			break;
//	}
//}

