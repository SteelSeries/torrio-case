
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

typedef enum
{
    BATTERY_PRESET_CHARGE_ACTIVE = 0x00,
    BATTERY_PRESET_CHARGE_DONE,
} Battery_PresetChargeState_t;

typedef struct
{
    Battery_PresetChargeState_t case_charge_status;
    Battery_PresetChargeState_t left_bud_charge_status;
    Battery_PresetChargeState_t right_bud_charge_status;
} Battery_PresetChargeData_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
Battery_PresetChargeData_t *Battery_GetPresetChargeState(void);
void Battery_UpdateBatteryStatus(uint16_t vbat_voltage);
uint8_t Battery_GetBatteryPercent(void);
void Battery_UpdateStatusTask(void);
uint16_t Battery_GetBatteryVoltage(void);
void Battery_BudsCtxInit(void);
