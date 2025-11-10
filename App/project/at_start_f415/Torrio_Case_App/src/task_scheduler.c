/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "task_scheduler.h"
#include "timer2.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define MAX_TASKS 20                                       // Maximum number of tasks
#define SEC_TO_TICKS(sec) ((sec) * 1000000 / TIME_BASE_US) // Convert seconds to ticks

// UINT32_MAX is the maximum value for a 32-bit unsigned integer (4294967295)
// TIME_BASE_US represents the time base in microseconds per tick (here, 100 us)
//
// The conversion macro MS_TO_TICKS(interval_ms) calculates:
//   (interval_ms * 1000) / TIME_BASE_US
// To avoid overflow, interval_ms * 1000 must not exceed UINT32_MAX * TIME_BASE_US
//
// Therefore, the maximum safe interval in milliseconds is:
//   (UINT32_MAX / 1000) * TIME_BASE_US
//
// This ensures the converted tick count fits within a 32-bit unsigned integer,
// preventing overflow and incorrect scheduling behavior.
#define MAX_INTERVAL_MS ((UINT32_MAX / 1000) * TIME_BASE_US)
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/

typedef struct
{
    void (*taskFunc)(void);
    uint32_t interval;
    uint32_t lastRun;
    TaskScheduler_RunMode_t runMode; // use typedef enum here
    TaskScheduler_StartMode_t startMode;
} Task;
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static Task taskList[MAX_TASKS];
static uint8_t numTasks = 0;
static volatile void (*temp_taskFunc)(void);
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void TaskScheduler_Run(void)
{
    for (uint8_t i = 0; i < numTasks; /* no i++ here */)
    {
        if ((Timer2_GetTick() - taskList[i].lastRun) >= taskList[i].interval)
        {
            temp_taskFunc = taskList[i].taskFunc;
            taskList[i].lastRun = Timer2_GetTick();

            if (taskList[i].runMode == TASK_RUN_ONCE)
            {
                // Remove task
                for (uint32_t j = i; j < numTasks - 1; j++)
                {
                    taskList[j] = taskList[j + 1];
                }
                numTasks--;
            }
            temp_taskFunc();
            continue;
        }
        i++;
    }
}

TaskScheduler_TaskStatus_t TaskScheduler_AddTask(void (*func)(void),
                                                 uint32_t interval_ticks,
                                                 TaskScheduler_RunMode_t runMode,
                                                 TaskScheduler_StartMode_t startMode)
{
    if (interval_ticks > MAX_INTERVAL_MS)
    {
        return TASK_INVALID_INTERVAL;
    }

    if (numTasks >= MAX_TASKS)
    {
        return TASK_LIST_FULL;
    }

    for (uint32_t i = 0; i < numTasks; i++)
    {
        if (taskList[i].taskFunc == func)
        {
            return TASK_ALREADY_EXISTS;
        }
    }

    uint32_t now = Timer2_GetTick();

    taskList[numTasks].taskFunc = func;
    taskList[numTasks].interval = MS_TO_TICKS(interval_ticks);
    taskList[numTasks].runMode = runMode;

    if (startMode == TASK_START_IMMEDIATE)
    {
        taskList[numTasks].lastRun = now - MS_TO_TICKS(interval_ticks);
    }
    else if (startMode == TASK_START_DELAYED)
    {
        taskList[numTasks].lastRun = now;
    }
    numTasks++;

    return TASK_OK;
}

TaskScheduler_TaskStatus_t TaskScheduler_RemoveTask(void (*func)(void))
{
    for (uint8_t i = 0; i < numTasks; i++)
    {
        if (taskList[i].taskFunc == func)
        {
            // Shift remaining tasks forward to fill the gap
            for (uint8_t j = i; j < numTasks - 1; j++)
            {
                taskList[j] = taskList[j + 1];
            }
            numTasks--;
            return TASK_OK;
        }
    }
    return TASK_REMOVE_NOT_FOUND;
}

uint32_t TaskScheduler_GetTimeUntilNextTask(void)
{
    uint32_t minDelta = -1;
    uint32_t now = Timer2_GetTick();

    for (uint32_t i = 0; i < numTasks; i++)
    {
        uint32_t delta = taskList[i].interval - (now - taskList[i].lastRun);
        if (delta <= 0)
        {
            return 0;
        }
        if (minDelta == -1 || delta < minDelta)
        {
            minDelta = delta;
        }
    }
    return minDelta;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/