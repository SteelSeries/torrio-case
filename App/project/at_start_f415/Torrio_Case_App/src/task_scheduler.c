/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "task_scheduler.h"
#include "timer2.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define MAX_TASKS 10                                       // Maximum number of tasks
#define TIME_BASE_US 100U                                  // Tick duration in microseconds (100us per tick)
#define MS_TO_TICKS(ms) ((ms) * 1000 / TIME_BASE_US)       // Convert milliseconds to ticks
#define SEC_TO_TICKS(sec) ((sec) * 1000000 / TIME_BASE_US) // Convert seconds to ticks
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
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void TaskScheduler_Run(void)
{
    uint32_t now = Timer2_GetTick();

    for (uint8_t i = 0; i < numTasks; /* no i++ here */)
    {
        if ((now - taskList[i].lastRun) >= taskList[i].interval)
        {
            taskList[i].lastRun = now;
            taskList[i].taskFunc();

            if (taskList[i].runMode == TASK_RUN_ONCE)
            {
                // Remove task
                for (uint32_t j = i; j < numTasks - 1; j++)
                {
                    taskList[j] = taskList[j + 1];
                }
                numTasks--;
                continue;
            }
        }
        i++;
    }
}

TaskScheduler_TaskStatus_t TaskScheduler_AddTask(void (*func)(void),
                                                 uint16_t interval_ticks,
                                                 TaskScheduler_RunMode_t runMode,
                                                 TaskScheduler_StartMode_t startMode)
{
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
    int32_t minDelta = -1;
    uint32_t now = Timer2_GetTick();

    for (uint32_t i = 0; i < numTasks; i++)
    {
        int32_t delta = taskList[i].interval - (now - taskList[i].lastRun);
        if (delta <= 0)
        {
            return 0; // 任務已到期，立即執行
        }
        if (minDelta == -1 || delta < minDelta)
        {
            minDelta = delta;
        }
    }
    return minDelta; // 剩餘時間（ticks 單位）
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/