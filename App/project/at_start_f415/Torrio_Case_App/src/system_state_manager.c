/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "system_state_manager.h"
#include "app_fw_update.h"
#include "power_control.h"
#include "custom_hid_class.h"
#include "usb.h"
#include "qi.h"
#include "battery.h"
#include "Commands.h"
#include "sy8809.h"
#include "lid.h"
#include "task_scheduler.h"
#include "uart_comm_manager.h"

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
static uint8_t ConvertCaseChargeStatusToCmd(Sy8809_CaseChargeStatus_t status);
static uint8_t ConvertBudChargeStatusToCmd(Sy8809_BudsChargeStatus_t status);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void SystemStateManager_EnterStandbyModeCheck(void)
{
    DEBUG_PRINT("enter standby\n");
    PowerControl_EnterStandby();
}

void SystemStateManager_SystemResetCheck(void)
{
    AppFwUpdata_SetResetFlag(true);
}

// This function is used by the factory to report the battery level as voltage and the status of the NTC.
void SystemStateManager_ReadBatteryAndNtcHandle(void)
{
    uint8_t buff[13] = {0x00};
    uint16_t case_battery_voltage = Battery_GetBatteryVoltage();
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();

    buff[0] = FAC_GET_BATTERY_AND_NTC | COMMAND_READ_FLAG;
    buff[1] = (uint8_t)case_battery_voltage & 0x00FF;
    buff[2] = (uint8_t)((case_battery_voltage >> 8) & 0x00FF);
    buff[3] = charge_status->check_reg_state.reg_0x16;
    // Todo: need add buds battery and NTC data.
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
}

// This function is used for GG engine, the reported battery level is a percentage.
void SystemStateManager_GetBatteryStatusHandle(void)
{
    uint8_t buff[7] = {0x00};
    uint8_t case_battery_level = Battery_GetBatteryPercent();
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();

    buff[0] = GET_BATTERY_INFO | COMMAND_READ_FLAG;
    buff[1] = case_battery_level;
    buff[2] = ConvertCaseChargeStatusToCmd(charge_status->case_charge_status);
    buff[4] = ConvertBudChargeStatusToCmd(charge_status->left_bud_charge_status);
    buff[6] = ConvertBudChargeStatusToCmd(charge_status->right_bud_charge_status);

    // Todo: need add buds battery level and charging status data.
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
}

void SystemStateManager_SystemStartWork(void)
{
    if (TaskScheduler_AddTask(Sy8809_StartWorkTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        DEBUG_PRINT("add sy8809 read vbat task fail\n");
    }

    if (Lid_GetState() == LID_CLOSE)
    {
        if ((Usb_FirstSetupUsbState() == USB_UNPLUG) && (Qi_GetDetectState() == QI_NON_DETECT))
        {
            if (TaskScheduler_AddTask(SystemStateManager_EnterStandbyModeCheck, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add enter standby task fail\n");
            }
        }
    }

    if (TaskScheduler_AddTask(Battery_UpdateStatusTask, BATTERY_TASK_UPDATE_INTERVAL_MS, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add battery status update task fail\n");
    }

    if (TaskScheduler_AddTask(UartCommManager_RunningTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        DEBUG_PRINT("add uart running task fail\n");
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t ConvertCaseChargeStatusToCmd(Sy8809_CaseChargeStatus_t status)
{
    switch (status)
    {
    case SY8809_CASE_CHARGE_STATUS_NO_CHARGING:
    case SY8809_CASE_CHARGE_STATUS_UNKNOW:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_UNPLUGGED;
    }

    case SY8809_CASE_CHARGE_STATUS_TRICKLE:
    case SY8809_CASE_CHARGE_STATUS_CONSTANT_CURR:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_CHARGE;
    }

    case SY8809_CASE_CHARGE_STATUS_CHARGE_DONE:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_COMPLETE;
    }

    default:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_UNPLUGGED;
    }
    }
}

static uint8_t ConvertBudChargeStatusToCmd(Sy8809_BudsChargeStatus_t status)
{
    switch (status)
    {
    case SY8809_BUD_CHARGE_STATE_CHARGING:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_CHARGE;
    }

    case SY8809_BUD_CHARGE_STATE_COMPLETE:
    case SY8809_BUD_CHARGE_STATE_TABLE4_COMPLETE:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_COMPLETE;
    }

    case SY8809_BUD_CHARGE_STATE_UNKNOW:
    default:
    {
        return (uint8_t)COMMAND_GET_BATTERY_STATUS_UNPLUGGED;
    }
    }
}
