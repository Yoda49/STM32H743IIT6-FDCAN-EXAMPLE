#include <stm32h7xx.h>
#include "stm32h743_fdcan.h"
#include "stm32h743_usart.h"
#include "stm32h743_gpio.h"

#define LED_ON  GPIOI->BSRR |= (1UL << 16)
#define LED_OFF GPIOI->BSRR |= (1UL <<  0)


struct can_message can_rx_message;
struct can_message can_tx_message;

extern unsigned long pll2_q_value;

unsigned char new_message_received = 0;



// ============================================================================
// FDCAN interface init
// ============================================================================
void FDCAN1_init (void)
{
	unsigned long *ptr, *i;
	unsigned long computed_can_freq;
	double temp;
	
	usart1_send_string("\nFDCAN: Init...");
	
	temp = (double)1 / pll2_q_value;	
	temp = temp * CAN_PRESCALER;
	temp = temp * (CAN_SYNC_SEG + CAN_PHASE_SEG1 + CAN_PHASE_SEG2);
	computed_can_freq = (double)1 /  temp;
	
	usart1_send_string("\nFDCAN: Computed frequency: ");
	usart1_ulong32_10digits (computed_can_freq);
	usart1_send_string(" MHz\n");
	
	gpio_init (PORT_A, 11, MODE_ALT_F, TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_9); // RX
	gpio_init (PORT_A, 12, MODE_ALT_F, TYPE_PUSH_PULL, SPEED_MAX, PULL_NO, ALTF_9); // TX
	
	//CLK Source
	//00: hse_ck clock is selected as FDCAN kernel clock (default after reset)
	//01: pll1_q_ck clock is selected as FDCAN kernel clock
	//10: pll2_q_ck clock is selected as FDCAN
	
	// PLL2_Q used 
	// Input freq = 48 MHz
	RCC->D2CCIP1R |= RCC_D2CCIP1R_FDCANSEL_1;
	
	// enable clock for CAN
	RCC->APB1HENR |= RCC_APB1HENR_FDCANEN; 
    
	//Initialize the CAN module
	//FDCAN1->CCCR reset value is 0x0000 0001
	FDCAN1->CCCR |= FDCAN_CCCR_INIT;               
	while ((FDCAN1->CCCR & FDCAN_CCCR_INIT) == 0) {};
		
	//CCE bit enabled to start configuration
	FDCAN1->CCCR |= FDCAN_CCCR_CCE;

	//Set classic CAN
	FDCAN1->CCCR &= ~(FDCAN_CCCR_FDOE);          

	// Auto retransmission
	FDCAN1->CCCR |= FDCAN_CCCR_DAR; // RETR OFF
		
	// Test mode, Internal loopback
	#ifdef CAN_TESTMODE
		FDCAN1->CCCR |= FDCAN_CCCR_TEST;
		FDCAN1->TEST |= FDCAN_TEST_LBCK;
	#endif

	// Clean message memory
	for (i = (unsigned long*)FDCAN_MEM_START_ADDR; i < (unsigned long*)FDCAN_MEM_END_ADDR; i++) *i = 0;

	// Timings 
	// Note: this calculations fit for PCLK1 = 48MHz, baudrate is set to 500k bit/s
	// Tq = 1 / 48 000 000 MHz
	// Tq * prescaler (6) = 0.000000125 sec
	// Tq * all segments  = 0.000002 sec
	// CAN freq = 1 / 0.000002 = 500 000 KHz
	
	FDCAN1->NBTP  = (CAN_SYNC_JW    - 1) << FDCAN_NBTP_NSJW_Pos;
	FDCAN1->NBTP |= (CAN_PRESCALER  - 1) << FDCAN_NBTP_NBRP_Pos;
	FDCAN1->NBTP |= (CAN_PHASE_SEG1 - 1) << FDCAN_NBTP_NTSEG1_Pos;
	FDCAN1->NBTP |= (CAN_PHASE_SEG2 - 1) << FDCAN_NBTP_NTSEG2_Pos;

	FDCAN1->DBTP  = (CAN_SYNC_JW    - 1) << FDCAN_DBTP_DSJW_Pos;
	FDCAN1->DBTP |= (CAN_PRESCALER  - 1) << FDCAN_DBTP_DBRP_Pos;
	FDCAN1->DBTP |= (CAN_PHASE_SEG1 - 1) << FDCAN_DBTP_DTSEG1_Pos;
	FDCAN1->DBTP |= (CAN_PHASE_SEG2 - 1) << FDCAN_DBTP_DTSEG2_Pos;

	#ifdef CAN_ENABLE_FILTERS
		
		FDCAN1->GFC |= FDCAN_GFC_ANFS; // Reject non-matching frames standard
		FDCAN1->GFC |= FDCAN_GFC_ANFE; // Reject non-matching frames extended
		FDCAN1->GFC |= FDCAN_GFC_RRFS; // Reject all remote frames with 11-bit standard ID
		FDCAN1->GFC |= FDCAN_GFC_RRFE; // Reject all remote frames with 29-bit standard ID
	
		FDCAN1->SIDFC = (FDCAN_11B_FILTER_EL_CNT << FDCAN_SIDFC_LSS_Pos); // standart filter count
		FDCAN1->XIDFC = (FDCAN_29B_FILTER_EL_CNT << FDCAN_XIDFC_LSE_Pos); // extended filter count
	
		FDCAN1->SIDFC |= (FDCAN_11B_FILTER_OFFSET << FDCAN_SIDFC_FLSSA_Pos); // standard filter start address
		FDCAN1->XIDFC |= (FDCAN_29B_FILTER_OFFSET << FDCAN_XIDFC_FLESA_Pos); // extended filter start address
	
	
		// set up standart filters
		ptr = (unsigned long*)FCCAN_11B_FILTER_START_ADDR; // get start address for pointer
		// *ptr++ = (FILTER_TYPE_DUAL << 30) | (FILTER_CFG_STORE_FIFO_0 << 27) | (CAN_TEST_ID_0 << 16) | (CAN_TEST_ID_1);
		// *ptr++ = (FILTER_TYPE_DUAL << 30) | (FILTER_CFG_STORE_FIFO_0 << 27) | (CAN_TEST_ID_2 << 16) | (CAN_TEST_ID_3);

		// set up extended filters
		ptr = (unsigned long*)FCCAN_29B_FILTER_START_ADDR; // get start address for pointer
		*ptr++ = (FILTER_CFG_STORE_FIFO_0 << 29) | (CAN_TEST_ID_0);
		*ptr++ = (FILTER_TYPE_DUAL        << 30) | (CAN_TEST_ID_1);
		
	#else
		FDCAN1->SIDFC = 0;
		FDCAN1->GFC   = 0;
	#endif

	// setup RX FIFO 0 memory offset (32 bit word)	
    FDCAN1->RXF0C  = FDCAN_RX_FIFO_0_OFFSET << FDCAN_RXF0C_F0SA_Pos;
	
	// setup RX FIFO 0 elements count
	FDCAN1->RXF0C |= FDCAN_RX_FIFO_0_EL_CNT << FDCAN_RXF0C_F0S_Pos;
	
	// TX event
	FDCAN1->TXEFC = 0; //No Event
	
	// setup TX buffers (TXBC after reset = 0x00000000)
	FDCAN1->TXBC &= ~(FDCAN_TXBC_TFQM); // FIFO operation
	
	// setup TX FIFO elements count
	FDCAN1->TXBC |= FDCAN_TX_FIFO_EL_CNT << FDCAN_TXBC_TFQS_Pos;
	
	// setup TX FIFO offset
	FDCAN1->TXBC |= FDCAN_TX_FIFO_OFFSET << FDCAN_TXBC_TBSA_Pos;
					
	// RX FIFO 0 new message interrupt enable
	FDCAN1->IE |= FDCAN_IE_RF0NE;
	
	// Interrrupt LINE 0 enable
	FDCAN1->ILE |= FDCAN_ILE_EINT0;

	// Normal operation - exit from setup
	FDCAN1->CCCR &= ~(FDCAN_CCCR_INIT);
	while ((FDCAN1->CCCR & FDCAN_CCCR_INIT) == 1) {};
	
	// enable status change & error interrupt        
	NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
	
	usart1_send_string("FDCAN: Init complete!\n");
}


// ============================================================================
// FDCAN get message
// ============================================================================
void FDCAN1_read_msg (struct can_message* msg, unsigned char idx)
{
	struct can_fifo_element *fifo;

	fifo = (struct can_fifo_element*)(FDCAN_RX_FIFO_0_START_ADDR + idx * FDCAN_RX_FIFO_0_EL_SIZE);

	msg->error   = (unsigned char)((fifo->word0 >> 31) & 0x01);
	msg->format  = (unsigned char)((fifo->word0 >> 30) & 0x01);
	msg->type    = (unsigned char)((fifo->word0 >> 29) & 0x01);
	
	if (msg->format == CAN_EXTENDED_FORMAT)
	{
		msg->id = fifo->word0 & 0x1FFFFFFF;
	}
	else
	{
		msg->id = (fifo->word0 >> 18) & 0x7FF;
	}
	
	msg->length  = (unsigned char)((fifo->word1 >> 16) & 0x0F);
	
	msg->data[0] = (unsigned char)((fifo->word2 >>  0) & 0xFF);
	msg->data[1] = (unsigned char)((fifo->word2 >>  8) & 0xFF);
	msg->data[2] = (unsigned char)((fifo->word2 >> 16) & 0xFF);
	msg->data[3] = (unsigned char)((fifo->word2 >> 24) & 0xFF);
	
	msg->data[4] = (unsigned char)((fifo->word3 >>  0) & 0xFF);
	msg->data[5] = (unsigned char)((fifo->word3 >>  8) & 0xFF);
	msg->data[6] = (unsigned char)((fifo->word3 >> 16) & 0xFF);
	msg->data[7] = (unsigned char)((fifo->word3 >> 24) & 0xFF);
}

// ============================================================================
// FDCAN send msg
// ============================================================================
void FDCAN1_send_msg (struct can_message *msg)
{
	struct can_fifo_element *fifo;
	unsigned char tx_index;
	
	if ((FDCAN1->TXFQS & FDCAN_TXFQS_TFQF) != 0)
	{
		
	}
	
	tx_index = (FDCAN1->TXFQS >> 16) & 0xF;

	fifo = (struct can_fifo_element *)(FDCAN_TX_FIFO_START_ADDR + tx_index * FDCAN_TX_FIFO_EL_SIZE);
	
	if (msg->format == CAN_STANDARD_FORMAT)
	{
		fifo->word0 = (msg->id << 18);
	}
	else
	{
		fifo->word0 = msg->id;
		fifo->word0 |= 1UL << 30; // extended flag
	}
	
	if (msg->type == REMOTE_FRAME) fifo->word0 |= 1UL << 29;
	
	fifo->word1 = (8UL << 16);  //Data size
	fifo->word2 = (msg->data[3] << 24)|(msg->data[2] << 16)|(msg->data[1] << 8)|msg->data[0];
	fifo->word3 = (msg->data[7] << 24)|(msg->data[6] << 16)|(msg->data[5] << 8)|msg->data[4];

	FDCAN1->TXBAR |= (1UL << tx_index);   
}

// ============================================================================
// FDCAN interrupt
// ============================================================================
void FDCAN1_IT0_IRQHandler(void)
{
	unsigned char rx_fifo_get_index;
   
	// new message received
	if((FDCAN1->IR & FDCAN_IR_RF0N) != 0)
	{
		FDCAN1->IR = FDCAN_IR_RF0N; // clear flag
		rx_fifo_get_index = (unsigned char)((FDCAN1->RXF0S >> 8) & 0x3F);
		FDCAN1_read_msg (&can_rx_message, rx_fifo_get_index);
		new_message_received = 1;
		FDCAN1->RXF0A = rx_fifo_get_index;
	};
	
	// message lost
	if((FDCAN1->IR & FDCAN_IR_RF0L) != 0)
	{
		FDCAN1->IR = FDCAN_IR_RF0L; // clear flag
	};

	
	// rx fifo 0 full
	if((FDCAN1->IR & FDCAN_IR_RF0F) != 0)
	{
		while(FDCAN1->RXF0S & FDCAN_RXF0S_F0FL_Msk)
		{
			rx_fifo_get_index= (uint8_t)((FDCAN1->RXF0S >> 8) & 0x1F);
			FDCAN1_read_msg(&can_rx_message, rx_fifo_get_index);

			FDCAN1->RXF0A = rx_fifo_get_index;
		}
		FDCAN1->IR = FDCAN_IR_RF0F;
	};


}


// ============================================================================
// FDCAN show filters configuration
// ============================================================================
void FDCAN1_show_filters_configuration (void)
{
	unsigned char x, value;
	unsigned long *ptr;
	
	usart1_send_string ("\n===================================");
	usart1_send_string ("\nFDCAN1 FILTERS CONFIGURATION");
	usart1_send_string ("\n===================================");
	
	usart1_send_string ("\n\nStandard filters:");
	ptr = (unsigned long*)FCCAN_11B_FILTER_START_ADDR;
	
	for (x = 0; x < FDCAN_11B_FILTER_EL_CNT; x++)
	{
		usart1_send_string ("\n\nFilter #");
		usart1_uchar8_3digits (x);
		
		usart1_send_string ("\nType: ");
		value = (*ptr & (0x3UL << 30)) >> 30;
		if (value == 0) usart1_send_string("Range filter from SFID1 to SFID2");
		if (value == 1) usart1_send_string("Dual ID filter for SFID1 or SFID2");
		if (value == 2) usart1_send_string("Classic filter: SFID1 = filter, SFID2 = mask");
		if (value == 3) usart1_send_string("Filter element disabled");
		
		usart1_send_string ("\nConfiguration: ");
		value = (*ptr & (0x7UL << 27)) >> 27;
		
		if (value == 0) usart1_send_string("Disable filter element");
		if (value == 1) usart1_send_string("Store in Rx FIFO 0 if filter matches");
		if (value == 2) usart1_send_string("Store in Rx FIFO 1 if filter matches");
		if (value == 3) usart1_send_string("Reject ID if filter matches");
		if (value == 4) usart1_send_string("Set priority if filter matches");
		if (value == 5) usart1_send_string("Set priority and store in FIFO 0 if filter matches");
		if (value == 6) usart1_send_string("Set priority and store in FIFO 1 if filter matches");
		if (value == 7) usart1_send_string("Store into Rx buffer or as debug message, configuration of FDCAN_SFT[1:0] ignored");
		
		usart1_send_string ("\nID 1: ");
		value = (*ptr & (0x7FFUL << 16)) >> 16;
		usart1_uint16_5digits (value);
		
		usart1_send_string ("\nID 2: ");
		value = *ptr & 0x7FFUL;
		usart1_uint16_5digits (value);
		
		ptr++;
	}
	
	usart1_send_string ("\n\nExtended filters:");
	ptr = (unsigned long*)FCCAN_29B_FILTER_START_ADDR;
	
	for (x = 0; x < FDCAN_29B_FILTER_EL_CNT; x++)
	{
		usart1_send_string ("\n\nFilter #");
		usart1_uchar8_3digits (x);
		
		ptr++;
		
		usart1_send_string ("\nType: ");
		value = (*ptr & (0x3UL << 30)) >> 30;
		if (value == 0) usart1_send_string("Range filter from EF1ID to EF2ID");
		if (value == 1) usart1_send_string("Dual ID filter for EF1ID or EF2ID");
		if (value == 2) usart1_send_string("Classic filter: EF1ID = filter, EF2ID = mask");
		if (value == 3) usart1_send_string("Range filter from EF1ID to EF2ID, FDCAN_XIDAM mask not applied");
		
		ptr--;
		
		usart1_send_string ("\nConfiguration: ");
		value = (*ptr & (0x7UL << 29)) >> 29;
		
		if (value == 0) usart1_send_string("Disable filter element");
		if (value == 1) usart1_send_string("Store in Rx FIFO 0 if filter matches");
		if (value == 2) usart1_send_string("Store in Rx FIFO 1 if filter matches");
		if (value == 3) usart1_send_string("Reject ID if filter matches");
		if (value == 4) usart1_send_string("Set priority if filter matches");
		if (value == 5) usart1_send_string("Set priority and store in FIFO 0 if filter matches");
		if (value == 6) usart1_send_string("Set priority and store in FIFO 1 if filter matches");
		if (value == 7) usart1_send_string("Store into Rx buffer, configuration of EFT[1:0] ignored");
		
		usart1_send_string ("\nID 1: ");
		value = *ptr & 0x1FFFFFFF;
		usart1_ulong32_10digits (value);
		
		ptr++;
		
		usart1_send_string ("\nID 2: ");
		value = *ptr & 0x1FFFFFFF;
		usart1_ulong32_10digits (value);
		
		ptr++;
	}
	
}










