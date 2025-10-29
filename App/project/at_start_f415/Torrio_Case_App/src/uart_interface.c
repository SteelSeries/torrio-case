/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_interface.h"
#include "uart_protocol.h"
#include "uart_comm_manager.h"
#include <string.h>
#include <stdlib.h>
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

bool UartInterface_SendDirect(UartInterface_Port_t port,
                              const uint8_t *payload,
                              uint16_t payload_len,
                              uint16_t event_id,
                              uint32_t timeout_ms,
                              uint8_t op)
{
    DEBUG_PRINT("UartInterface_SendDirect(port=%d, len=%d, event_id=0x%04X)\n",
                port, payload_len, event_id);

    UART_CommContext_t *ctx = NULL;
    if (port == UART_INTERFACE_BUD_LEFT)
    {
        ctx = UartCommManager_GetLeftBudContext();
    }
    else if (port == UART_INTERFACE_BUD_RIGHT)
    {
        ctx = UartCommManager_GetRightBudContext();
    }
    else
    {
        return false;
    }

    if (ctx->state != UART_STATE_IDLE)
    {
        DEBUG_PRINT("[UART][DIRECT] Busy (state=%d), cannot send now.\n", ctx->state);
        return false;
    }

    uint16_t total_len = payload_len + 7;

    uint8_t *tx_buf = malloc(total_len);
    if (!tx_buf)
    {
        return false;
    }

    uint16_t out_len = total_len;
    if (!UartProtocol_PackCommand(event_id, &ctx->tx_seqn, payload, payload_len, tx_buf, &out_len))
    {
        free(tx_buf);
        return false;
    }

    ctx->direct_data = tx_buf;
    ctx->direct_len = out_len;
    ctx->direct_event_id = event_id;
    ctx->direct_mode = true;
    ctx->direct_pending = true;
    ctx->direct_timeout_ms = timeout_ms;
    ctx->command_id = op;

    DEBUG_PRINT("[UART][DIRECT] Direct packet prepared (%d bytes)\n", out_len);
    return true;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/