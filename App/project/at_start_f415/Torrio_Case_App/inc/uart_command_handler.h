
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "uart_comm_manager.h"
#include "uart_protocol.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
// clang-format off
#define BUD_CMD_PREVENT_SLEEP       0x50
#define BUD_CMD_BUTTON_AND_MODE     0x71
#define BUD_CMD_DEEP_POWER_OFF      0x77
#define BUD_CMD_GET_FW_VERSION      0x10
#define BUD_CMD_GET_MODEL_AND_COLOR 0x6C
#define BUD_CMD_READ_SERIAL_NUMBER  0x12
#define BUD_CMD_READ_BUD_STATE      0x7B


// clang-format on
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartCommandsHandle_CommandsHandle(UART_CommContext_t *ctx, UartProtocol_Packet_t rx_packet);
void UartCommandsHandle_CommandsHandleTimeout(UART_CommContext_t *ctx);
