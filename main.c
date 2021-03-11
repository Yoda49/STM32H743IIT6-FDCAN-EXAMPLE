#include <stm32h7xx.h>
#include "stm32h743_gpio.h"
#include "stm32h743_fdcan.h"

extern unsigned char new_message_received;
extern struct can_message can_tx_message;
extern struct can_message can_rx_message;

unsigned short mseconds = 0;

// *************************************
// SYS TICK
// *************************************
void SysTick_Handler (void)
{
	milliseconds++;

	if (++mseconds == 999)
	{
		mseconds = 0;
	}
	else if (mseconds == 500)
	{
		FDCAN1_send_msg (&can_tx_message);
	}
}





// *************************************
// MAIN
// *************************************
int main (void)
{
	unsigned char y1;
	
	SetSysClockTo400mHz ();
	SystemCoreClockUpdate ();
	SCB_EnableICache ();
	SCB_EnableDCache ();
	
	SysTick_Config(SystemCoreClock/1000);
	
	FDCAN1_init ();
	FDCAN1_show_filters_configuration ();
	
	can_tx_message.id      = 1;
	can_tx_message.format  = CAN_EXTENDED_FORMAT;
	can_tx_message.type    = DATA_FRAME;
	can_tx_message.length  = 10;
	can_tx_message.data[0] = 20;
	can_tx_message.data[1] = 30;
	can_tx_message.data[2] = 40
	can_tx_message.data[3] = 50
	can_tx_message.data[4] = 60
	can_tx_message.data[5] = 70
	can_tx_message.data[6] = 80
	can_tx_message.data[7] = 90
	
	while (1)
	{
		if (new_message_received == 1)
		{
			for (y1 = 0; y1 < 8; y1++)
			{
				usart1_uchar8_3digits (can_rx_message.data[y1]);	
			}
			new_message_received = 0;
		}
	}
}

