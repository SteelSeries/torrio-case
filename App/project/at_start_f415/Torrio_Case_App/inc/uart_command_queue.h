
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define CMD_QUEUE_SIZE 10
#define CMD_MAX_DATA_LEN 32 // 可根據 command 最大長度調整
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    uint8_t data[CMD_MAX_DATA_LEN];
    uint8_t length;
    uint8_t command_id;
} UartCommand_t;

typedef struct
{
    UartCommand_t queue[CMD_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} UartCommandQueue_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartCommandQueue_Init(UartCommandQueue_t *q);
bool UartCommandQueue_Enqueue(UartCommandQueue_t *q, const UartCommand_t *cmd);
bool UartCommandQueue_Dequeue(UartCommandQueue_t *q, UartCommand_t *cmd);
bool UartCommandQueue_IsEmpty(const UartCommandQueue_t *q);
bool UartCommandQueue_IsFull(const UartCommandQueue_t *q);