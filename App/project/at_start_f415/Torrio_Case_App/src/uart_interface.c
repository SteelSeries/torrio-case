/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_interface.h"
#include "uart_protocol.h"
#include "uart_comm_manager.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
bool UartInterface_SendCommand(UartInterface_Port_t port, UartCommandQueue_Command_t *cmd)
{
    DEBUG_PRINT("UartInterface_SendCommand called with port=%d, cmd->command_id=0x%02X, length=%d\n", port, cmd->command_id, cmd->length);

    UART_CommContext_t *ctx = NULL;
    if (port == UART_INTERFACE_BUD_LEFT)
    {
        ctx = UartCommManager_GetLeftBudContext();
        DEBUG_PRINT("Using Left Bud UART Context\n");
    }
    else if (port == UART_INTERFACE_BUD_RIGHT)
    {
        ctx = UartCommManager_GetRightBudContext();
        DEBUG_PRINT("Using Right Bud UART Context\n");
    }
    else
    {
        DEBUG_PRINT("Invalid port specified: %d\n", port);
        return false;
    }

    uint16_t out_len = CMD_MAX_DATA_LEN;
    uint8_t tx_buf[CMD_MAX_DATA_LEN] = {0};

    bool pack_result = UartProtocol_PackCommand(CMD_ONE_WIRE_UART_ACK, &ctx->tx_seqn, cmd->data, cmd->length, tx_buf, &out_len);
    if (!pack_result)
    {
        DEBUG_PRINT("UartProtocol_PackCommand failed: buffer too small or invalid input\n");
        return false;
    }
    DEBUG_PRINT("UartProtocol_PackCommand succeeded: packed length = %d\n", out_len);

    UartCommandQueue_Command_t tx_cmd;
    memcpy(tx_cmd.data, tx_buf, out_len);
    tx_cmd.length = out_len;
    tx_cmd.command_id = cmd->command_id;
    tx_cmd.timeout_ms = cmd->timeout_ms;

    bool enqueue_result = UartCommandQueue_Enqueue(&ctx->cmd_queue, &tx_cmd);
    if (!enqueue_result)
    {
        DEBUG_PRINT("UartCommandQueue_Enqueue failed: queue full\n");
        return false;
    }
    DEBUG_PRINT("Command enqueued successfully. Queue count = %d\n", ctx->cmd_queue.count);

    return true;
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/