
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
// Delay interval for battery status update task in milliseconds.
// This task runs every 120 seconds to read and update battery level.
#define BATTERY_TASK_UPDATE_INTERVAL_MS 120000U
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    BATTERY_UNKNOWN_LEVEL = 0xFFU,
    BATTERY_CRITICAL_LEVEL = 9U,
    BATTERY_LOW_LEVEL = 26U,
    BATTERY_MEDIUM_LEVEL = 66U,
    BATTERY_HIGH_LEVEL = 100U
} Battery_Level_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Battery_UpdateBatteryStatus(uint16_t vbat_voltage);
uint8_t Battery_GetBatteryPercent(void);
void Battery_UpdateStatusTask(void);
uint16_t Battery_GetBatteryVoltage(void);
void Battery_BudsCtxInit(void);
