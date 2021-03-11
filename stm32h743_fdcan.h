#ifndef __FDCAN_H__
#define __FDCAN_H__

#define CAN_TEST_ID_0 0x01
#define CAN_TEST_ID_1 0x02
#define CAN_TEST_ID_2 0x03
#define CAN_TEST_ID_3 0x04


/* Тестовый режим, без трансивера */
//#define CAN_TESTMODE 1
#define CAN_ENABLE_FILTERS 1

/* 
Частота 48Mhz 
Для изменения скорости достаточно изменить PSC.
Для 500Kb/s 6
Для 250Kb/s 12
Для 125Kb/s 24
*/

#define CAN_PRESCALER   6
#define CAN_SYNC_JW     2
#define CAN_SYNC_SEG    1
#define CAN_PHASE_SEG1 11
#define CAN_PHASE_SEG2  4




/*
Message ram start address = 0x4000AC00
----------------------------------------------------------------
Start addr  | Description    | Elements count   | Max memory
----------------------------------------------------------------
SIDFC.FLSSA | 11-bit filters | 0 - 128 elements | 0 -  128 words
XIDFC.FLESA | 29-bit filters | 0 -  64 elements | 0 -  128 words
RXF0C.F0SA  | Rx FIFO 0      | 0 -  64 elements | 0 - 1152 words
RXF1C.F1SA  | Rx FIFO 1      | 0 -  64 elements | 0 - 1152 words
RXBC.RBSA   | Rx buffer      | 0 -  64 elements | 0 - 1152 words
TXEFC.EFSA  | Tx event FIFO  | 0 -  32 elements | 0 -   64 words
TXBC.TBSA   | Tx buffers     | 0 -  32 elements | 0 -  576 words
TMC.TMSA    | Trigger memory | 0 -  64 elements | 0 -  128 words
----------------------------------------------------------------
Message ram end address = 0x4000D3FF
----------------------------------------------------------------
offset  measured in words (32bit / 4 bytes)
address measured in bytes (8 bit / 1 byte)
*/

#define FDCAN_MEM_START_ADDR          0x4000AC00UL
#define FDCAN_MEM_END_ADDR            0x4000D3FFUL

// 11-bit filters
#define FDCAN_11B_FILTER_EL_CNT       0UL
#define FDCAN_11B_FILTER_EL_SIZE      4UL
#define FDCAN_11B_FILTER_EL_W_SIZE    (FDCAN_11B_FILTER_EL_SIZE / 4)
#define FCCAN_11B_FILTER_START_ADDR   (FDCAN_MEM_START_ADDR)
#define FDCAN_11B_FILTER_OFFSET       0UL

// 29-bit filters
#define FDCAN_29B_FILTER_EL_CNT       4UL
#define FDCAN_29B_FILTER_EL_SIZE      8UL 
#define FDCAN_29B_FILTER_EL_W_SIZE    (FDCAN_29B_FILTER_EL_SIZE / 4) 
#define FCCAN_29B_FILTER_START_ADDR   (FCCAN_11B_FILTER_START_ADDR + FDCAN_11B_FILTER_EL_CNT * FDCAN_11B_FILTER_EL_SIZE)
#define FDCAN_29B_FILTER_OFFSET       (FDCAN_11B_FILTER_OFFSET     + FDCAN_11B_FILTER_EL_CNT * FDCAN_11B_FILTER_EL_W_SIZE)

// Rx FIFO 0
#define FDCAN_RX_FIFO_0_EL_CNT        10
#define FDCAN_RX_FIFO_0_HEAD_SIZE     8UL
#define FDCAN_RX_FIFO_0_DATA_SIZE     8UL
#define FDCAN_RX_FIFO_0_EL_SIZE       (FDCAN_RX_FIFO_0_HEAD_SIZE   + FDCAN_RX_FIFO_0_DATA_SIZE)
#define FDCAN_RX_FIFO_0_EL_W_SIZE     (FDCAN_RX_FIFO_0_EL_SIZE / 4)
#define FDCAN_RX_FIFO_0_START_ADDR    (FCCAN_29B_FILTER_START_ADDR + FDCAN_29B_FILTER_EL_CNT * FDCAN_29B_FILTER_EL_SIZE)
#define FDCAN_RX_FIFO_0_OFFSET        (FDCAN_29B_FILTER_OFFSET     + FDCAN_29B_FILTER_EL_CNT * FDCAN_29B_FILTER_EL_W_SIZE)

// Rx FIFO 1
#define FDCAN_RX_FIFO_1_EL_CNT        0UL
#define FDCAN_RX_FIFO_1_HEAD_SIZE     0UL
#define FDCAN_RX_FIFO_1_DATA_SIZE     0UL
#define FDCAN_RX_FIFO_1_EL_SIZE       (FDCAN_RX_FIFO_1_HEAD_SIZE + FDCAN_RX_FIFO_1_DATA_SIZE)
#define FDCAN_RX_FIFO_1_EL_W_SIZE     (FDCAN_RX_FIFO_1_EL_SIZE / 4)
#define FDCAN_RX_FIFO_1_START_ADDR    (FDCAN_RX_FIFO_0_START_ADDR + FDCAN_RX_FIFO_0_EL_CNT * FDCAN_RX_FIFO_0_EL_SIZE)
#define FDCAN_RX_FIFO_1_OFFSET        (FDCAN_RX_FIFO_0_OFFSET     + FDCAN_RX_FIFO_0_EL_CNT * FDCAN_RX_FIFO_0_EL_W_SIZE)

// Rx buffer
#define FDCAN_RX_BUFFER_EL_CNT        0UL
#define FDCAN_RX_BUFFER_HEAD_SIZE     0UL
#define FDCAN_RX_BUFFER_DATA_SIZE     0UL
#define FDCAN_RX_BUFFER_EL_SIZE       (FDCAN_RX_BUFFER_HEAD_SIZE + FDCAN_RX_BUFFER_DATA_SIZE)
#define FDCAN_RX_BUFFER_EL_W_SIZE     (FDCAN_RX_BUFFER_EL_SIZE / 4)
#define FDCAN_RX_BUFFER_START_ADDR    (FDCAN_RX_FIFO_1_START_ADDR + FDCAN_RX_FIFO_1_EL_CNT * FDCAN_RX_FIFO_1_EL_SIZE)
#define FDCAN_RX_BUFFER_OFFSET        (FDCAN_RX_FIFO_1_OFFSET     + FDCAN_RX_FIFO_1_EL_CNT * FDCAN_RX_FIFO_1_EL_W_SIZE)

// Tx event FIFO
#define FDCAN_TX_EVENT_FIFO_EL_CNT    0UL
#define FDCAN_TX_EVENT_FIFO_EL_SIZE   8UL
#define FDCAN_TX_EVENT_FIFO_EL_W_SIZE (FDCAN_TX_EVENT_FIFO_EL_SIZE / 4) 
#define FDCAN_TX_EVENT_START_ADDR     (FDCAN_RX_BUFFER_START_ADDR + FDCAN_RX_BUFFER_EL_CNT * FDCAN_RX_BUFFER_EL_SIZE)
#define FDCAN_TX_EVENT_OFFSET         (FDCAN_RX_BUFFER_OFFSET     + FDCAN_RX_BUFFER_EL_CNT * FDCAN_RX_BUFFER_EL_W_SIZE)

// TX buffers (FIFO)
#define FDCAN_TX_FIFO_EL_CNT          10UL
#define FDCAN_TX_FIFO_HEAD_SIZE       8UL
#define FDCAN_TX_FIFO_DATA_SIZE       8UL
#define FDCAN_TX_FIFO_EL_SIZE         (FDCAN_TX_FIFO_HEAD_SIZE + FDCAN_TX_FIFO_DATA_SIZE)
#define FDCAN_TX_FIFO_EL_W_SIZE       (FDCAN_TX_FIFO_EL_SIZE / 4)
#define FDCAN_TX_FIFO_START_ADDR      (FDCAN_TX_EVENT_START_ADDR + FDCAN_TX_EVENT_FIFO_EL_CNT * FDCAN_TX_EVENT_FIFO_EL_SIZE)
#define FDCAN_TX_FIFO_OFFSET          (FDCAN_TX_EVENT_OFFSET     + FDCAN_TX_EVENT_FIFO_EL_CNT * FDCAN_TX_EVENT_FIFO_EL_W_SIZE)

// Trigger memory
#define FDCAN_TRIG_MEM_EL_CNT          10UL
#define FDCAN_TRIG_MEM_EL_SIZE         8UL
#define FDCAN_TRIG_MEM_EL_W_SIZE       (FDCAN_TRIG_MEM_EL_SIZE / 4)
#define FDCAN_TRIG_MEM_START           (FDCAN_TX_FIFO_START_ADDR + FDCAN_TX_FIFO_EL_CNT * FDCAN_TX_FIFO_EL_SIZE)
#define FDCAN_TRIG_MEM_OFFSET          (FDCAN_TX_FIFO_OFFSET     + FDCAN_TX_FIFO_EL_CNT * FDCAN_TX_FIFO_EL_W_SIZE)







// FILTER TYPE
#define FILTER_TYPE_RANGE       0UL
#define FILTER_TYPE_DUAL        1UL
#define FILTER_TYPE_CLASSIC     2UL
#define FILTER_TYPE_DISABLE     3UL

/* FILTER CONFIG */
#define FILTER_CFG_DISABLED     0UL
#define FILTER_CFG_STORE_FIFO_0 1UL
#define FILTER_CFG_STORE_FIFO_1 2UL
#define FILTER_CFG_REJECT       3UL



/* MESSAGES */
#define CAN_STANDARD_FORMAT     0UL
#define CAN_EXTENDED_FORMAT     1UL

#define DATA_FRAME              0UL
#define REMOTE_FRAME            1UL










struct can_message
{
	unsigned char error;
	unsigned long id;
	unsigned char data[8];
	unsigned char length;
	unsigned char format;
	unsigned char type;
};

struct can_fifo_element
{
	unsigned long word0;
	unsigned long word1;
    unsigned long word2;
    unsigned long word3;
};




void FDCAN1_init                       (void);
void FDCAN1_read_msg                   (struct can_message* msg, unsigned char idx);
void FDCAN1_send_msg                   (struct can_message *msg);
void FDCAN1_show_filters_configuration (void);


#endif



