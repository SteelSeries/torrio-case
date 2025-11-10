
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
#define BUD_CMD_PREVENT_SLEEP           0x50U
#define BUD_CMD_BUTTON_AND_MODE         0x71U
#define BUD_CMD_DEEP_POWER_OFF          0x77U
#define BUD_CMD_FW_VERSION              0x10U
#define BUD_CMD_MODEL_AND_COLOR         0x6CU
#define BUD_CMD_SERIAL_NUMBER           0x12U
#define BUD_CMD_BUD_STATE               0x7BU
#define BUD_CMD_BATTERY_STATE           0x65U
#define BUD_CMD_CHARGE_SETING           0x67U
#define BUD_CMD_SYNC_CASE_LID_STATE     0x3CU

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
