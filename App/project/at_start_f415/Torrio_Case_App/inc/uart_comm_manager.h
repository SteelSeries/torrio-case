
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "uart_command_queue.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    UART_STATE_IDLE,
    UART_STATE_SENDING,
    UART_STATE_WAITING_RESPONSE,
    UART_STATE_PROCESSING,
    UART_STATE_TIMEOUT,
    UART_STATE_ERROR
} Uart_State_t;

typedef enum
{
    UART_BUD_LEFT = 0,
    UART_BUD_RIGHT,
    UART_BUD_UNKNOWN
} Uart_BudSide_t;

typedef struct
{
    usart_type *uart;
    UartCommandQueue_Queue_t cmd_queue;
    Uart_State_t state;
    uint32_t timeout_tick;
    uint16_t current_timeout_ms;
    uint8_t tx_buffer[CMD_MAX_DATA_LEN];
    uint16_t tx_len;
    uint8_t rx_buffer[CMD_MAX_DATA_LEN];
    uint16_t rx_index;
    uint16_t expected_len; // Expected total length of the incoming packet
    bool sync_detected;    // Indicates whether the CMD_SYNC_BYTE (start byte) has been detected
    bool packet_ready;     // Set to true when a complete packet has been received in the interrupt
    uint8_t retry_count;
    uint8_t tx_seqn;
    Uart_BudSide_t side; // Indicates which bud this context belongs to (Left or Right)
    uint8_t command_id;
    bool direct_mode;
    bool direct_pending;
    uint8_t *direct_data;
    uint16_t direct_len;
    uint16_t direct_event_id;
    uint32_t direct_timeout_ms;
} UART_CommContext_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartCommManager_Init(void);
void UartCommManager_RunningTask(void);
UART_CommContext_t *UartCommManager_GetLeftBudContext(void);
UART_CommContext_t *UartCommManager_GetRightBudContext(void);
