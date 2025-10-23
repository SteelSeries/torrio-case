/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_comm_manager.h"
#include "task_scheduler.h"
#include "timer2.h"
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
static UART_CommContext_t bud_left_ctx;
static UART_CommContext_t bud_right_ctx;
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
}

void UartCommManager_RunningTask(void)
{
    CommTask(&bud_left_ctx);
    CommTask(&bud_right_ctx);

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
    UartCommandQueue_Init(&ctx->cmd_queue);
    ctx->state = UART_STATE_IDLE;
    ctx->timeout_tick = 0;
    ctx->retry_count = 0;
    memset(ctx->tx_buffer, 0, sizeof(ctx->tx_buffer));
    memset(ctx->rx_buffer, 0, sizeof(ctx->rx_buffer));
}

static void CommTask(UART_CommContext_t *ctx)
{
    switch (ctx->state)
    {
    case UART_STATE_IDLE:
    {
        if (!UartCommandQueue_IsEmpty(&ctx->cmd_queue))
        {
            UartCommand_t cmd;
            if (UartCommandQueue_Dequeue(&ctx->cmd_queue, &cmd))
            {
                DEBUG_PRINT("[UART][IDLE] Dequeued command -> ID: 0x%02X, Len: %d\n",
                            cmd.command_id, cmd.length);

                DEBUG_PRINT("[UART][IDLE] Data: ");
                for (uint8_t i = 0; i < cmd.length; i++)
                {
                    DEBUG_PRINT("0x%02X ", cmd.data[i]);
                }
                DEBUG_PRINT("\r\n");

                // UART_PackCommand(&cmd, ctx->tx_buffer);
                // UART_Send(ctx->usart, ctx->tx_buffer, cmd.length);

                ctx->timeout_tick = Timer2_GetTick() + 100;
                ctx->state = UART_STATE_WAITING_RESPONSE;
                DEBUG_PRINT("[UART][STATE] -> WAITING_RESPONSE (timeout @ %lu)\n", ctx->timeout_tick);
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

        // if (UART_ReceiveAvailable(&ctx->uart))
        // {
        //     UART_Receive(&ctx->uart, ctx->rx_buffer);
        //     if (UART_ParseResponse(ctx->rx_buffer))
        //     {
        //         ctx->state = UART_STATE_IDLE;
        //         ctx->retry_count = 0;
        //     }
        //     else
        //     {
        //         ctx->state = UART_STATE_ERROR;
        //     }
        // }
        // else if (Timer2_GetTick() > ctx->timeout_tick)
        if (Timer2_GetTick() > ctx->timeout_tick)
        {
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
            // UART_Send(&ctx->uart, ctx->tx_buffer, /* len */);
            ctx->timeout_tick = Timer2_GetTick() + 100;
            ctx->state = UART_STATE_WAITING_RESPONSE;
            DEBUG_PRINT("[UART][STATE] -> WAITING_RESPONSE (retry)\n");
        }
        else
        {
            DEBUG_PRINT("[UART][TIMEOUT] Max retry reached. Reset to IDLE.\n");
            ctx->state = UART_STATE_IDLE; // 或錯誤處理
        }
        break;
    }

    case UART_STATE_ERROR:
    {
        DEBUG_PRINT("[UART][ERROR] Communication error detected.\n");
        ctx->state = UART_STATE_IDLE;
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
