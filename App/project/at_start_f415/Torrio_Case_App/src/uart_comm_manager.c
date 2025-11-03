/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_comm_manager.h"
#include "task_scheduler.h"
#include "timer2.h"
#include "uart_driver.h"
#include "at32f415_int.h"
#include "uart_protocol.h"
#include "uart_command_handler.h"
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
static UART_CommContext_t bud_left_ctx = {
    .Connect = UART_BUDS_CONNT_UNKNOW,
    .detect_state = UART_BUDS_IO_UNKNOW,
    .detect_state_pre = UART_BUDS_IO_UNKNOW,
};
static UART_CommContext_t bud_right_ctx = {
    .Connect = UART_BUDS_CONNT_UNKNOW,
    .detect_state = UART_BUDS_IO_UNKNOW,
    .detect_state_pre = UART_BUDS_IO_UNKNOW,
};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void CommInit(UART_CommContext_t *ctx, usart_type *usart_x);
static void CommTask(UART_CommContext_t *ctx);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
UART_CommContext_t *UartCommManager_GetLeftBudContext(void)
{
    return &bud_left_ctx;
}

UART_CommContext_t *UartCommManager_GetRightBudContext(void)
{
    return &bud_right_ctx;
}

void UartCommManager_Init(void)
{
    CommInit(&bud_left_ctx, USART2);
    CommInit(&bud_right_ctx, USART3);
    Interrupt_BudsCtxInit();
    UartDrive_BudsCtxInit();
}

void UartCommManager_DisconnectReinit(UART_CommContext_t *ctx)
{
    CommInit(ctx, ctx->uart);
}

void UartCommManager_RunningTask(void)
{

    if (bud_left_ctx.Connect == UART_BUDS_CONNT_CONNECT)
    {
        CommTask(&bud_left_ctx);
    }

    if (bud_right_ctx.Connect == UART_BUDS_CONNT_CONNECT)
    {
        CommTask(&bud_right_ctx);
    }

    if (TaskScheduler_AddTask(UartCommManager_RunningTask, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add uart running task fail\n");
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void CommInit(UART_CommContext_t *ctx, usart_type *usart_x)
{
    ctx->uart = usart_x;
    if (usart_x == USART2)
    {
        ctx->side = UART_BUD_LEFT;
    }
    else if (usart_x == USART3)
    {
        ctx->side = UART_BUD_RIGHT;
    }

    UartCommandQueue_Init(&ctx->cmd_queue);
    ctx->state = UART_STATE_IDLE;
    ctx->timeout_tick = 0;
    ctx->current_timeout_ms = 0;
    ctx->retry_count = 0;
    ctx->tx_seqn = 0;
    ctx->tx_len = 0;
    ctx->rx_index = 0;
    ctx->expected_len = 0;
    ctx->sync_detected = false;
    ctx->packet_ready = false;
    ctx->command_id = 0;
    ctx->direct_mode = false;
    ctx->direct_pending = false;
    ctx->direct_data = NULL;
    ctx->direct_len = 0;
    ctx->direct_event_id = 0;
    ctx->direct_timeout_ms = 0;
    ctx->detect_debounce = 0;
    ctx->mode = UART_BUDS_WORK_MODE_UNKNOW;
    memset(ctx->button_io_state, UART_BUDS_BUTTON_IO_UNKNOW, sizeof(ctx->button_io_state));
    memset(ctx->tx_buffer, 0, sizeof(ctx->tx_buffer));
    memset(ctx->rx_buffer, 0, sizeof(ctx->rx_buffer));
}

static void CommTask(UART_CommContext_t *ctx)
{
    switch (ctx->state)
    {
    case UART_STATE_IDLE:
    {
        if (ctx->direct_pending)
        {
            DEBUG_PRINT("[UART][IDLE] Direct send triggered (event_id=0x%04X, len=%d)\n",
                        ctx->direct_event_id, ctx->tx_len);

            UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_SEND_MODE);
            UartDrive_SendData(ctx);
            UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_RECEIVE_MODE);

            ctx->timeout_tick = Timer2_GetTick() + MS_TO_TICKS(ctx->direct_timeout_ms);
            ctx->current_timeout_ms = ctx->direct_timeout_ms;
            ctx->retry_count = 0;
            ctx->direct_pending = false;
            ctx->state = UART_STATE_WAITING_RESPONSE;

            DEBUG_PRINT("[UART][STATE] -> WAITING_RESPONSE (direct)\n");
        }
        else if (!UartCommandQueue_IsEmpty(&ctx->cmd_queue))
        {
            UartCommandQueue_Command_t cmd;
            if (UartCommandQueue_Dequeue(&ctx->cmd_queue, &cmd))
            {
                DEBUG_PRINT("%lu [UART][IDLE] Dequeued command -> ID: 0x%02X, Len: %d\n", Timer2_GetTick(), cmd.command_id, cmd.length);

                DEBUG_PRINT("[UART][IDLE] Data: ");
                for (uint8_t i = 0; i < cmd.length; i++)
                {
                    DEBUG_PRINT("0x%02X ", cmd.data[i]);
                }
                DEBUG_PRINT("\r\n");

                memcpy(&ctx->tx_buffer, cmd.data, cmd.length);
                ctx->tx_len = cmd.length;
                ctx->command_id = cmd.command_id;
                UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_SEND_MODE);
                UartDrive_SendData(ctx);
                UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_RECEIVE_MODE);

                ctx->current_timeout_ms = cmd.timeout_ms;
                ctx->timeout_tick = Timer2_GetTick() + MS_TO_TICKS(ctx->current_timeout_ms);
                ctx->state = UART_STATE_WAITING_RESPONSE;
                DEBUG_PRINT("%lu [UART][STATE] -> WAITING_RESPONSE (current timeout @ %lu)(timeout @ %lu)\n", Timer2_GetTick(), ctx->current_timeout_ms, ctx->timeout_tick);
            }
            else
            {
                DEBUG_PRINT("[UART][IDLE] Queue empty but IsEmpty returned false!?\n");
            }
        }
        break;
    }

    case UART_STATE_WAITING_RESPONSE:
    {
        if (ctx->packet_ready)
        {
            ctx->packet_ready = false;
            DEBUG_PRINT("[UART][WAIT] Response packet ready, len=%d\n", ctx->rx_index);
            DEBUG_PRINT("Received data (%d bytes): ", ctx->rx_index);
            for (uint16_t i = 0; i < ctx->rx_index; i++)
            {
                DEBUG_PRINT("%02X ", ctx->rx_buffer[i]);
            }
            DEBUG_PRINT("\n");

            UartProtocol_Packet_t rx_packet;
            if (UARTProtocol_UnpackCommand(ctx->rx_buffer, ctx->rx_index, &rx_packet))
            {
                DEBUG_PRINT("[UART][WAIT] Response OK, EventID=0x%04X, ReceicedEventID=0x%04X, Seq=%d, PayloadLen=%d\n",
                            rx_packet.event_id, rx_packet.received_event_id, rx_packet.tx_seq, rx_packet.payload_len);

                UartCommandsHandle_CommandsHandle(ctx, rx_packet);
                ctx->state = UART_STATE_IDLE;
                ctx->retry_count = 0;

                if (ctx->direct_mode && ctx->direct_data)
                {
                    free(ctx->direct_data);
                    ctx->direct_data = NULL;
                    ctx->direct_len = 0;
                    ctx->direct_mode = false;
                    ctx->direct_pending = false;
                }
            }
            else
            {
                DEBUG_PRINT("[UART][WAIT] CRC/Error detected -> UART_STATE_ERROR\n");

                if (ctx->rx_index < ctx->expected_len)
                {
                    DEBUG_PRINT("Error: Incomplete packet. Expected %d bytes, got %d bytes.\n",
                                ctx->expected_len, ctx->rx_index);
                }
                else
                {
                    DEBUG_PRINT("Error: CRC mismatch or invalid data.\n");
                }

                ctx->state = UART_STATE_ERROR;
            }

            ctx->rx_index = 0;
            ctx->expected_len = 0;
            ctx->sync_detected = false;
        }
        else if (Timer2_GetTick() > ctx->timeout_tick)
        {
            DEBUG_PRINT("[UART][TIMEOUT] No data received. Current index=%d, expected_len=%d\n",
                        ctx->rx_index, ctx->expected_len);
            DEBUG_PRINT("[UART][WAIT] Timeout detected (tick=%lu, limit=%lu)\n",
                        Timer2_GetTick(), ctx->timeout_tick);
            ctx->state = UART_STATE_TIMEOUT;
        }
        break;
    }

    case UART_STATE_TIMEOUT:
    {

        if (++ctx->retry_count < 3)
        {
            UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_SEND_MODE);
            UartDrive_SendData(ctx);
            UartDrive_SetOneWireMode(ctx, UART_ONEWIRE_RECEIVE_MODE);

            ctx->timeout_tick = Timer2_GetTick() + MS_TO_TICKS(ctx->current_timeout_ms);
            ctx->state = UART_STATE_WAITING_RESPONSE;
            DEBUG_PRINT("[UART][STATE] -> WAITING_RESPONSE (retry)\n");
            DEBUG_PRINT("[UART][STATE] Timeout detected (tick=%lu, limit=%lu current timeout:%lu)\n",
                        Timer2_GetTick(), ctx->timeout_tick, ctx->current_timeout_ms);
        }
        else
        {
            DEBUG_PRINT("[UART][TIMEOUT] Max retry reached. Reset to IDLE.\n");
            UartCommandsHandle_CommandsHandleTimeout(ctx);
            ctx->state = UART_STATE_IDLE;
            ctx->retry_count = 0;
            if (ctx->direct_mode && ctx->direct_data)
            {
                free(ctx->direct_data);
                ctx->direct_data = NULL;
                ctx->direct_len = 0;
                ctx->direct_mode = false;
                ctx->direct_pending = false;
            }
        }
        break;
    }

    case UART_STATE_ERROR:
    {
        DEBUG_PRINT("[UART][ERROR] Communication error detected.\n");
        ctx->state = UART_STATE_IDLE;
        if (ctx->direct_mode && ctx->direct_data)
        {
            free(ctx->direct_data);
            ctx->direct_data = NULL;
            ctx->direct_len = 0;
            ctx->direct_mode = false;
            ctx->direct_pending = false;
        }
        break;
    }

    default:
    {
        DEBUG_PRINT("[UART][STATE] Invalid state %d. Reset to IDLE.\n", ctx->state);
        ctx->state = UART_STATE_IDLE;
        break;
    }
    }
}
