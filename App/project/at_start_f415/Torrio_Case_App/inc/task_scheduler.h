
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    TASK_RUN_FOREVER = 0, // Task will persist indefinitely and will not be removed after execution
    TASK_RUN_ONCE,        // Task will be executed only once and then automatically removed
} TaskScheduler_RunMode_t;

typedef enum
{
    TASK_START_DELAYED,  // Wait interval before first run
    TASK_START_IMMEDIATE // Run as soon as idle
} TaskScheduler_StartMode_t;

typedef enum
{
    TASK_OK = 0,           // Task added/removed successfully
    TASK_LIST_FULL,        // Task list is full
    TASK_ALREADY_EXISTS,   // Task already exists, not added again
    TASK_REMOVE_NOT_FOUND, // Task not found in the list
} TaskScheduler_TaskStatus_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void TaskScheduler_Run(void);
TaskScheduler_TaskStatus_t TaskScheduler_RemoveTask(void (*func)(void));
TaskScheduler_TaskStatus_t TaskScheduler_AddTask(void (*func)(void),
                                                 uint16_t interval_ticks,
                                                 TaskScheduler_RunMode_t runMode,
                                                 TaskScheduler_StartMode_t startMode);
uint32_t TaskScheduler_GetTimeUntilNextTask(void);
