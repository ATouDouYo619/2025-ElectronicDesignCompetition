/*******************/

Name:ATouDou
Time:2025\6\25
Project:National College Students' Electronic Design Contest

/*******************/

/* On-chip peripherals */
Stepper_motor：
	1.DIR：
		MOTOR1 ---> PB12
		MOTOR2 ---> PB13
		MOTOR3 ---> PB14
		MOTOR4 ---> PB15
	2.STP:
		MOTOR1 ---> TIM8_CH1 --> PC6
		MOTOR2 ---> TIM8_CH2 --> PC7
		MOTOR3 ---> TIM8_CH3 --> PC8
		MOTOR4 ---> TIM8_CH4 --> PC9

TB6612(TIME8):
	PWM:
		A ---> TIM3_CH1 --> PA6
		B ---> TIM3_CH2 --> PA7
	AIN:
		A1 ---> PC0
		A2 ---> PC1
		B1 ---> PA4
		B2 ---> PA5

Encoder:
	1:
		A: ---> TIM5_CH1 --> PA0
		B: ---> TIM5_CH2 --> PA1
	2:
		A: ---> TIM4_CH1 --> PB6
		B: ---> TIM4_CH2 --> PB7
Servo:
		CH1 ---> TIM2_CH1 --> PA15
		CH2 ---> TIM2_CH2 --> PB3
		CH3 ---> TIM2_CH3 --> PB0
		CH4 ---> TIM2_CH4 --> PB1

OLED(Software_IIC):
		SCL ---> PB8
		SDA ---> PB9
		
UART1(115200):
		TX: ---> PA9
		RX: ---> PA10

UART2:
		TX: ---> PA2
		RX: ---> PA3

UART3:
		TX: ---> PB10
		RX: ---> PB11
		
UART4_JY61P(9600):
		TX: ---> PC10
		RX: ---> PC11

LED:
		LED1 ---> PC13
		LED2 ---> PC14
		LED3 ---> PC15
		
KEY:
		KEY1 ---> PC2
		KEY2 ---> PC3
		KEY3 ---> PA12
		
Buzz:
		BUZZ ---> PC12