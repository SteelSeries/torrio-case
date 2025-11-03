
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
    UART_INTERFACE_BUD_LEFT = 0,
    UART_INTERFACE_BUD_RIGHT,
    UART_INTERFACE_PORT_MAX
} UartInterface_Port_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
bool UartInterface_SendQueue(UartInterface_Port_t port, UartCommandQueue_Command_t *cmd);
bool UartInterface_SendDirect(UartInterface_Port_t port, const uint8_t *payload, uint16_t payload_len, uint16_t event_id, uint32_t timeout_ms, uint8_t op);
void UartInterface_SendBudCommand(UartInterface_Port_t target, uint8_t command_id, const uint8_t *payload, size_t payload_len, uint16_t timeout);
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
