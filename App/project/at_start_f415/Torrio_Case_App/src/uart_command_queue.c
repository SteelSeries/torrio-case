/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_command_queue.h"
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

void UartCommandQueue_Init(UartCommandQueue_Queue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

bool UartCommandQueue_IsEmpty(const UartCommandQueue_Queue_t *q)
{
    return q->count == 0;
}

bool UartCommandQueue_IsFull(const UartCommandQueue_Queue_t *q)
{
    return q->count >= CMD_QUEUE_SIZE;
}

bool UartCommandQueue_Enqueue(UartCommandQueue_Queue_t *q, const UartCommandQueue_Command_t *cmd)
{
    if (UartCommandQueue_IsFull(q))
    {
        return false;
    }

    memcpy(&q->queue[q->tail], cmd, sizeof(UartCommandQueue_Command_t));
    q->tail = (q->tail + 1) % CMD_QUEUE_SIZE;
    q->count++;
    return true;
}

bool UartCommandQueue_Dequeue(UartCommandQueue_Queue_t *q, UartCommandQueue_Command_t *cmd)
{
    if (UartCommandQueue_IsEmpty(q))
    {
        return false;
    }

    memcpy(cmd, &q->queue[q->head], sizeof(UartCommandQueue_Command_t));
    q->head = (q->head + 1) % CMD_QUEUE_SIZE;
    q->count--;
    return true;
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/