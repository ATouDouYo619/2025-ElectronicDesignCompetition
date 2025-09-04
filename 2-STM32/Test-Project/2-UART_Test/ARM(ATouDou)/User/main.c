#include "SYS.h"

int main(void)
{
	UART1_Init(UART1_Bound);
	UART2_Init(UART2_Bound);
	UART3_Init(UART3_Bound);

	UART1_SendByte(0x01);
	UART2_SendByte(0x02);	
	UART3_SendByte(0x03);
	
    while (1)
    {

    }
}
