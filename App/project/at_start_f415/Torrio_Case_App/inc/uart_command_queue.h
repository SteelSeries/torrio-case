
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define CMD_QUEUE_SIZE 10
#define CMD_MAX_DATA_LEN 32
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    uint8_t data[CMD_MAX_DATA_LEN];
    uint8_t length;
    uint8_t command_id;
    uint16_t timeout_ms;
} UartCommandQueue_Command_t;

typedef struct
{
    UartCommandQueue_Command_t queue[CMD_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} UartCommandQueue_Queue_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartCommandQueue_Init(UartCommandQueue_Queue_t *q);
bool UartCommandQueue_Enqueue(UartCommandQueue_Queue_t *q, const UartCommandQueue_Command_t *cmd);
bool UartCommandQueue_Dequeue(UartCommandQueue_Queue_t *q, UartCommandQueue_Command_t *cmd);
bool UartCommandQueue_IsEmpty(const UartCommandQueue_Queue_t *q);
bool UartCommandQueue_IsFull(const UartCommandQueue_Queue_t *q);