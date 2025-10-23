
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
} UART_State_t;

typedef struct
{
    usart_type *uart;
    UartCommandQueue_Queue_t cmd_queue;
    UART_State_t state;
    uint32_t timeout_tick;
    uint16_t current_timeout_ms;
    uint8_t tx_buffer[CMD_MAX_DATA_LEN];
    uint8_t rx_buffer[CMD_MAX_DATA_LEN];
    uint8_t retry_count;
    uint8_t tx_seqn;
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
