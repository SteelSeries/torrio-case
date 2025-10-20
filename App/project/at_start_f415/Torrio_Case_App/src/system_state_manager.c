/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "system_state_manager.h"
#include "app_fw_update.h"
#include "power_control.h"
#include "custom_hid_class.h"
#include "usb.h"
#include "battery.h"
#include "Commands.h"
#include "sy8809.h"
#include "lid.h"
#include "task_scheduler.h"

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
void SystemStateManager_EnterStandbyModeCheck(void)
{
    printf("enter standby\n");
    PowerControl_EnterStandby();
}

void SystemStateManager_SystemResetCheck(void)
{
    AppFwUpdata_SetResetFlag(true);
}

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

void SystemStateManager_SystemStartWork(void)
{
    if (TaskScheduler_AddTask(Sy8809_StartWorkTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        printf("add sy8809 read vbat task fail\n");
    }

    if (Lid_GetState() == LID_CLOSE)
    {
        if (Usb_FirstSetupUsbState() == USB_UNPLUG)
        {
            if (TaskScheduler_AddTask(SystemStateManager_EnterStandbyModeCheck, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                printf("add enter standby task fail\n");
            }
        }
    }

    if (TaskScheduler_AddTask(Battery_UpdateStatusTask, BATTERY_TASK_UPDATE_INTERVAL_MS, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add battery status update task fail\n");
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/