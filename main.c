#include <stm32h7xx.h>

#include "stm32h743_gpio.h"
#include "stm32h743_sdram.h"
#include "stm32h743_usart.h"
#include "stm32h743_timers.h"
#include "stm32h743_ltdc.h"
#include "stm32h743_sdmmc.h"
#include "stm32h743_dmamux1.h"
#include "stm32h743_dma.h"
#include "stm32h743_fdcan.h"
#include <stdlib.h>

#include "gfx.h"
#include "gui.h"
#include "sdcard.h"
#include "common.h"
#include "file_system.h"
//#include "stm32h743_dma.h"

#define LED_ON  GPIOI->BSRR |= (1UL << 16)
#define LED_OFF GPIOI->BSRR |= (1UL <<  0)

#define H12_OFF GPIOH->BSRR |= (1UL << 28)
#define H12_ON  GPIOH->BSRR |= (1UL << 12)

#define BUTTON (GPIOH->IDR & (1UL << 9))
 
unsigned short led_flag = 0;

unsigned short milliseconds = 0;
unsigned long distance = 132443;
unsigned char gearbox_value = 0;


unsigned char  ltdc_error_cnt = 0;
unsigned char  ltdc_error_stg = 0;
unsigned short ltdc_transfer_error_cnt = 0;
unsigned short ltdc_fifo_underrun_cnt  = 0;

unsigned long button_timer = 0;
unsigned char center_window_mode_switch_timer = 0;

unsigned char x = 0;
unsigned char y = 0;
unsigned char z = 0;


unsigned long icon_counter = 0;
unsigned char icons [34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
unsigned short mseconds = 0;
	
	
	
extern unsigned char new_message_received;
extern struct can_message can_tx_message;
extern struct can_message can_rx_message;

	
// *************************************
// SOFTWARE DELAY
// *************************************
void software_delay (volatile unsigned long time)
{
	for (;time > 1; time--) __ASM("NOP");
}



// *************************************
// BUZZER
// *************************************
unsigned long buzzer_time = 3;

void buzzer (unsigned short freq, unsigned short time)
{
	buzzer_time = time;
	
	// off
	TIM8->CR1 &= ~TIM_CR1_CEN;
	
	// set freq
	TIM8->PSC = freq;
	
	// on
	TIM8->CR1 |= TIM_CR1_CEN;
}



// *************************************
// HARD FAULT
// *************************************
void HardFault_Handler (void)
{
	usart1_send_string("\nHARD FAULT!");
	
	while (1) {};
}


// *************************************
// TIM2
// *************************************
void TIM2_IRQHandler (void)
{
	if (TIM2->SR & 1UL)
	{
		TIM2->SR = 0;
		odo_draw_flag = 1;
		gearbox_draw_flag = 1;
	}
}

// *************************************
// TIM3
// *************************************
void TIM3_IRQHandler (void)
{
	if (TIM3->SR & 1UL)
	{
		TIM3->SR = 0;
		distance++;
		
		if (++gearbox_value == 4) gearbox_value = 0;
	}
}

// *************************************
// TIM5
// *************************************
void TIM5_IRQHandler (void)
{
	if (TIM5->SR & 1UL)
	{
		TIM5->SR = 0;
	}
}

// *************************************
// TIM8 TIM12 IRQ
// *************************************
void TIM8_BRK_TIM12_IRQHandler (void)
{
	if (TIM12->SR & 1UL)
	{
		TIM12->SR = 0;
		
		// allow gui changes
		if (++x > 230) x = 0;
		if (++y > 230) y = 0;

		icon_counter++;
		if (center_window_mode_switch_timer < 250) center_window_mode_switch_timer++;
	}
}


// *************************************
// LTDC ERROR INTERRUPT
// *************************************
void LTDC_ER_IRQHandler (void)
{
	if (LTDC->ISR & LTDC_ISR_FUIF)
	{
		LTDC->ICR |= LTDC_ICR_CFUIF;
		ltdc_fifo_underrun_cnt++;
	}
	
	if (LTDC->ISR & LTDC_ISR_TERRIF)
	{
		LTDC->ICR |= LTDC_ICR_CTERRIF;
		ltdc_transfer_error_cnt++;
	}
}	



// *************************************
// SYS TICK
// *************************************
void SysTick_Handler (void)
{
	milliseconds++;

	// switch off buzzer
	//if (buzzer_time  > 0) buzzer_time--;
	//if (buzzer_time == 1) TIM8->CR1 &= ~TIM_CR1_CEN;
	
	if (++mseconds == 999)
	{
		LED_OFF;
		mseconds = 0;
	}
	else if (mseconds == 500)
	{
		LED_ON;
		FDCAN1_send_msg (&can_tx_message);
	}
}





void icons_show (unsigned char num)
{	
	if (icons[num] == 0) icons[num] = 1; else icons[num] = 0;
	
	if (num == 0)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 0, 25, 12, COLOR_RGB565_GRAY_3);
		else gfx_draw_icon_48x48c (25 + 52 * 0, 25, 12, COLOR_RGB565_YELLOW);
	}
	else if (num == 1)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 1, 25, 19, COLOR_RGB565_GRAY_3);
		else gfx_draw_icon_48x48c (25 + 52 * 1, 25, 19, COLOR_RGB565_YELLOW);
	}
	else if (num == 2)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 2, 25, 32, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 2, 25, 32, COLOR_RGB565_YELLOW);
	}
	else if (num == 3)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 3, 25, 33, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 3, 25, 33, COLOR_RGB565_GREEN);
	}
	else if (num == 4)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 4, 25, 17, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 4, 25, 17, COLOR_RGB565_RED);
	}
	else if (num == 5)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 5, 25, 43, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 5, 25, 43, COLOR_RGB565_YELLOW);
	}
	else if (num == 6)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 0, 77, 24, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 0, 77, 19, COLOR_RGB565_GREEN);
	}
	else if (num == 7)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 1, 77, 15, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 1, 77, 32, COLOR_RGB565_RED);
	}
	else if (num == 8)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 0, 129, 38, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 0, 129, 12, COLOR_RGB565_YELLOW);
	}
	else if (num == 9)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (25 + 52 * 1, 129, 34, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (25 + 52 * 1, 129, 19, COLOR_RGB565_YELLOW);
	}
	else if (num == 10)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 0, 25, 10, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 0, 25, 10, COLOR_RGB565_RED);
	}
	else if (num == 11)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 1, 25, 21, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 1, 25, 21, COLOR_RGB565_RED);
	}
	else if (num == 12)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 2, 25, 31, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 2, 25, 31, COLOR_RGB565_YELLOW);
	}
	else if (num == 13)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 3, 25, 40, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 3, 25, 40, COLOR_RGB565_YELLOW);
	}
	else if (num == 14)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 4, 25, 41, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 4, 25, 41, COLOR_RGB565_YELLOW);
	}
	else if (num == 15)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 5, 25, 28, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 5, 25, 28, COLOR_RGB565_YELLOW);
	}
	else if (num == 16)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 0, 77, 29, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 0, 77, 29, COLOR_RGB565_YELLOW);
	}
	else if (num == 17)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 1, 77,  8, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 1, 77,  8, COLOR_RGB565_YELLOW);
	}
	else if (num == 18)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 2, 77, 11, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 2, 77, 11, COLOR_RGB565_GREEN);
	}
	else if (num == 19)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 0, 129, 47, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 0, 129, 47, COLOR_RGB565_GREEN);
	}
	else if (num == 20)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 1, 129,  6, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 1, 129,  6, COLOR_RGB565_YELLOW);
	}
	else if (num == 21)
	{
		if (icons[num] == 0) gfx_draw_icon_48x48c (1205 - 52 * 0, 181, 35, COLOR_RGB565_GRAY_2);
		else gfx_draw_icon_48x48c (1205 - 52 * 0, 181, 35, COLOR_RGB565_GREEN);
	}
	
	
	if (center_window_num <= 1)
	{
		if (num == 22)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 0, 210, 30, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 0, 210, 30, COLOR_RGB565_YELLOW);
		}
		else if (num == 23)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 1, 210, 44, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 1, 210, 44, COLOR_RGB565_RED);
		}
		else if (num == 24)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 2, 210, 45, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 2, 210, 45, COLOR_RGB565_RED);
		}
		else if (num == 25)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 3, 210,  1, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 3, 210,  1, COLOR_RGB565_RED);
		}
		else if (num == 26)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 4, 210,  0, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 4, 210,  0, COLOR_RGB565_RED);
		}
		else if (num == 27)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 5, 210, 25, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 5, 210, 25, COLOR_RGB565_GREEN);
		}
		else if (num == 28)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 6, 210, 46, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 6, 210, 46, COLOR_RGB565_GREEN);
		}
		else if (num == 29)
		{
			if (icons[num] == 0) gfx_draw_icon_48x48a (427 + 54 * 7, 210, 42, COLOR_RGB565_GRAY_3);
			else gfx_draw_icon_48x48a (427 + 54 * 7, 210, 42, COLOR_RGB565_YELLOW);
		}
	}
}
	
void icons_init (void)
{
	signed char y;
	
	for (y = -40; y != 1; y++)
	{
		gfx_draw_icon_48x48c (25 + 52 * 0, 25 + y / 8, 12, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 1, 25 + y / 8, 19, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 2, 25 + y / 8, 32, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 3, 25 + y / 8, 33, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 4, 25 + y / 8, 17, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 5, 25 + y / 8, 43, COLOR_RGB565_GRAY_2);
		
		gfx_draw_icon_48x48c (25 + 52 * 0, 77 + y / 4, 24, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 1, 77 + y / 4, 15, COLOR_RGB565_GRAY_2);
		
		gfx_draw_icon_48x48c (25 + 52 * 0, 129 + y, 38, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (25 + 52 * 1, 129 + y, 34, COLOR_RGB565_GRAY_2);
		
		
		
		
		gfx_draw_icon_48x48c (1205 - 52 * 0, 25 + y / 8, 10, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 1, 25 + y / 8, 21, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 2, 25 + y / 8, 31, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 3, 25 + y / 8, 40, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 4, 25 + y / 8, 41, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 5, 25 + y / 8, 28, COLOR_RGB565_GRAY_2);
		
		gfx_draw_icon_48x48c (1205 - 52 * 0, 77 + y / 4, 29, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 1, 77 + y / 4,  8, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 2, 77 + y / 4, 11, COLOR_RGB565_GRAY_2);
		
		gfx_draw_icon_48x48c (1205 - 52 * 0, 129 + y, 47, COLOR_RGB565_GRAY_2);
		gfx_draw_icon_48x48c (1205 - 52 * 1, 129 + y,  6, COLOR_RGB565_GRAY_2);
		
		gfx_draw_icon_48x48c (1205 - 52 * 0, 181 + y * 2, 35, COLOR_RGB565_GRAY_2);
		
		software_delay (500000);
	}
	
	gfx_draw_icon_48x48a (427 + 54 * 0, 210, 30, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 1, 210, 44, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 2, 210, 45, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 3, 210,  1, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 4, 210,  0, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 5, 210, 25, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 6, 210, 46, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 7, 210, 42, COLOR_RGB565_GRAY_3);
	
	gfx_draw_icon_48x48a (427 + 54 * 0, 264, 18, COLOR_RGB565_WHITE);
	gfx_draw_icon_48x48a (427 + 54 * 1, 264, 13, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 6, 264, 26, COLOR_RGB565_GRAY_3);
	gfx_draw_icon_48x48a (427 + 54 * 7, 264, 20, COLOR_RGB565_GRAY_3);
	
	gfx_fill_rectangle (427 + 54 * 0, 264 + 48, 48, 4, COLOR_RGB565_GREEN);
}


// *************************************
// MAIN
// *************************************
int main (void)
{
	unsigned char drawing_stage = 0;
	volatile unsigned char animation_select = 0;
	unsigned char y1;
	
	volatile unsigned short ltdc_y_pos = 0;
	
	gpio_init (PORT_G, 13,  MODE_OUTPUT, TYPE_PUSH_PULL, SPEED_MAX, PULL_DOWN, ALTF_0); // test
	gpio_init (PORT_H, 9,   MODE_INPUT,  TYPE_PUSH_PULL, SPEED_MAX, PULL_DOWN, ALTF_0); // BUTTON
	
	//gpio_init (PORT_C, 9,  MODE_ALT_F,  TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_0); // MCO2
	gpio_init (PORT_I, 0,  MODE_OUTPUT, TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_0); // LED
	gpio_init (PORT_I, 6,  MODE_ALT_F,  TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_3); // BUZZER
	
	//gpio_init (PORT_H,  0, MODE_ALT_F,  TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_0); // OSC IN
	//gpio_init (PORT_H,  1, MODE_ALT_F,  TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_0); // OSC OUT
	
	//gpio_init (PORT_A,  6, MODE_ALT_F,  TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_2); // TIM3 PWM
	
	//RCC->CFGR |= RCC_CFGR_MCO2PRE; // set prescaler to 15
	
	LED_ON;
	
	
	SetSysClockTo400mHz ();
	SystemCoreClockUpdate ();
	SCB_EnableICache ();
	SCB_EnableDCache ();
	
	SysTick_Config(SystemCoreClock/1000);
	

	usart1_init ();
	usart1_show_cpu_info ();
	
	//SDRAM_init ();
	//SDRAM_test ();
	
	FDCAN1_init ();
	FDCAN1_show_filters_configuration ();
	
	//SD_Init     ();
	//SDRAM_clear ();
	//SD_Test     ();
	
	//LTDC_init ();

	
	timer2_init  ();
	timer3_init  ();
	timer4_init  ();
	timer5_init  (); // precision delay
	//timer8_init  (); // buzzer
	timer12_init ();
	
	

	
	//NVIC_EnableIRQ(TIM1_UP_IRQn);
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_EnableIRQ(TIM3_IRQn);
	//NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_EnableIRQ(TIM5_IRQn);
	//NVIC_EnableIRQ(TIM6_DAC_IRQn);
	//NVIC_EnableIRQ(TIM7_IRQn);
	//NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
	NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
	//NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
	//NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
	//NVIC_EnableIRQ(TIM15_IRQn);
	//NVIC_EnableIRQ(TIM16_IRQn);
	//NVIC_EnableIRQ(TIM17_IRQn);
	NVIC_EnableIRQ(LTDC_ER_IRQn);
	NVIC_EnableIRQ (USART1_IRQn);
	__enable_irq();

	
	
	can_tx_message.id = 1;
	can_tx_message.format = CAN_EXTENDED_FORMAT;
	can_tx_message.type = DATA_FRAME;
	can_tx_message.length = 8;
	can_tx_message.data[0] = 20;
	can_tx_message.data[1] = 21;
	can_tx_message.data[2] = 22;
	can_tx_message.data[3] = 23;
	can_tx_message.data[4] = 24;
	can_tx_message.data[5] = 25;
	can_tx_message.data[6] = 26;
	can_tx_message.data[7] = 27;
	
	while (1)
	{
		if (new_message_received == 1)
		{
			usart1_send_string ("\nNew message! ID: ");
			usart1_uchar8_3digits (can_rx_message.id);
			
			for (y1 = 0; y1 < 8; y1++)
			{
				usart1_send_string (" ");
				usart1_uchar8_3digits (can_rx_message.data[y1]);	
			}
			new_message_received = 0;
		}
	}
	
	
	/*
	


	SD_ReadMultipleBlock_POLLING ((unsigned char*)BUFFER1_ADDR, 1950 * 1, 1950);
	SD_ReadMultipleBlock_POLLING ((unsigned char*)BUFFER2_ADDR, 1950 * 0, 1950);
	SD_ReadMultipleBlock_POLLING ((unsigned char*)BUFFER3_ADDR, 1950 * 7, 1950);
	SD_ReadMultipleBlock_POLLING ((unsigned char*)BUFFER5_ADDR, 1950 * 6, 1950);
	SD_ReadMultipleBlock_POLLING ((unsigned char*)BUFFER4_ADDR, 1950 * 8, 1950);
	
	
	
	START:
		
	if (animation_select == 0)
	{
		gui_fx_logo_maz_1 ();
		gui_fx_slide_start_1 ();
	}
	else if (animation_select == 1)
	{
		gui_fx_logo_maz_2 ();
		gui_fx_crt_start  ();
	}
	else if (animation_select == 2)
	{
		gui_fx_logo_maz_3 ();
		gui_fx_slide_start_2 ();
	}
	gui_init ();
	icons_init ();


	//buzzer (3000, 500);
	

	while (1)
	{
		

		
		if (BUTTON != 0)
		{
			button_timer = 0;
			software_delay (100000);
			while (BUTTON != 0)
			{
				if (button_timer <= 20000000) button_timer++;
			}
			
			if (button_timer > 10000000) gui_icons_show_list (48);
			if (++animation_select == 3) animation_select = 0;
			goto START;
		}
		
		ltdc_y_pos = LTDC->CPSR & 0xFFFF;
		
		if (center_window_mode_switch_timer > 100)
		{
			if (ltdc_y_pos > 420 && ltdc_y_pos < 490)
			{
				center_window_mode_switch_timer	= 0;
				gfx_dma2d_copy_window (BUFFER0_ADDR, BUFFER5_ADDR, 530, 578, 290, 306);
				gui_center_window ();
			}
		}
		
		
		
		ltdc_y_pos = LTDC->CPSR & 0xFFFF;
		
		if (ltdc_y_pos > 420 && ltdc_y_pos < 490)
		{
			if (drawing_stage == 0)
			{
				gui_speedometer ((float)x / 3, COLOR_RGB565_RED);
				
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 0;}
				drawing_stage++;
			}
			else if (drawing_stage == 1)
			{
				gui_tachometer  (y, COLOR_RGB565_RED);
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 1;}
				drawing_stage++;
			}
			else if (drawing_stage == 2)
			{
				gui_brake1_level (x / 2, COLOR_RGB565_RED);
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 2;}
				drawing_stage++;
			}
			else if (drawing_stage == 3)
			{
				gui_brake2_level (y / 2, COLOR_RGB565_RED);
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 3;}
				drawing_stage++;
			}
			else if (drawing_stage == 4)
			{
				gui_odometer_draw ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 4;}
				drawing_stage++;
			}
			else if (drawing_stage == 5)
			{
				gui_gearbox_draw  ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 5;}
				drawing_stage++;
			}
			else if (drawing_stage == 6)
			{
				gui_fuel_level (x, COLOR_RGB565_RED);
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 6;}
				drawing_stage++;
			}
			else if (drawing_stage == 7)
			{
				gui_cool_temp  (x, COLOR_RGB565_RED);
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 7;}
				drawing_stage++;
			}
			else if (drawing_stage == 8)
			{
				if (icon_counter > 10)
				{
					y1 = rand() / 963 - 1;
					icons_show (y1);
					icon_counter = 0;
				}
							
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 8;}
				drawing_stage++;
			}
			else if (drawing_stage == 9)
			{
				gui_addblue_level (x / 2, RGB565(0, 150, 150));
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 9;}
				drawing_stage++;
			}
			else if (drawing_stage == 10)
			{
				gui_voltmeter_draw   ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 10;}
				drawing_stage++;
			}
			else if (drawing_stage == 11)
			{
				gui_temperature_draw ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 11;}
				drawing_stage++;
			}
			else if (drawing_stage == 12)
			{
				gui_oil_level     (x / 2, RGB565(0, 150, 150));
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 12;}
				drawing_stage++;
			}
			else if (drawing_stage == 13)
			{
				if (center_window_num == 0)
				{
					gui_text_draw ();
					gui_doors_draw ();
				}
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 13;}
				drawing_stage++;
			}
			else if (drawing_stage == 14)
			{
				gui_date_draw   ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 14;}
				drawing_stage++;
			}
			else if (drawing_stage == 15)
			{
				gui_icons_shift ();
								
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 15;}
				drawing_stage++;
			}
			else if (drawing_stage == 16)
			{
				// center window timer
				gfx_draw_uchar_16x16 (890, 64, center_window_mode_switch_timer, 3, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
				
				gfx_draw_uchar_16x16 (890, 80, ltdc_fifo_underrun_cnt, 3, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);				
				gfx_draw_uchar_16x16 (890, 96, ltdc_transfer_error_cnt, 3, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
				
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 16;}
				drawing_stage++;
			}
			else if (drawing_stage == 17)
			{
				// show errors
				gfx_draw_uchar_16x16  (890, 32, ltdc_error_stg, 2, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
				gfx_draw_uchar_16x16  (890, 48, ltdc_error_cnt, 3, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
				
				ltdc_y_pos = LTDC->CPSR & 0xFFFF;
				if (ltdc_y_pos < 420) {ltdc_error_cnt++; ltdc_error_stg = 16;}
				drawing_stage = 0;
			}

		}
		
		// LINE < 420 ERROR :: STAGE NUM
		// LINE < 420 ERROR :: COUNT
		// CENTER WINDOW COUNTER
		// LTDC FIFO UNDERRUN ERROR COUNTER
		// LTDC TRANSFER ERROR COUNTER

		gui_odometer_check ();
		gui_gearbox_check  ();
	};
	
*/	
	

}

