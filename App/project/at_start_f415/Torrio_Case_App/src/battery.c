/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "battery.h"
#include "sy8809.h"
#include "sy8809_xsense.h"
#include "usb.h"
#include "task_scheduler.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define CASE_MAX_VBAT 4340
#define CASE_MIN_VBAT 3500
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static const uint16_t battery_voltage_table[] = {3510, 3590, 3610, 3630, 3650,
                                                 3670, 3690, 3710, 3740, 3760,
                                                 3800, 3840, 3880, 3920, 3980,
                                                 4030, 4080, 4140, 4200, 4250,
                                                 CASE_MAX_VBAT};
static uint8_t pre_Case_VBAT_percent = BATTERY_UNKNOWN_LEVEL;
static uint16_t adc_convert_to_voltage = 0;

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Battery_UpdateStatusTask(void)
{
    Sy8809Xsense_XsenseRead_t Pending_temp = {0};
    Pending_temp.is_command_read = false;
    Pending_temp.Pending = SY8809_XSENSE_VBAT;
    Sy8809Xsense_SetPendingXsense(Pending_temp);
    if (TaskScheduler_AddTask(Sy8809Xsense_TrigXsenseConv, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        DEBUG_PRINT("add sy8809 trig xsense conv task fail\n");
    }
    if (TaskScheduler_AddTask(Battery_UpdateStatusTask, BATTERY_TASK_UPDATE_INTERVAL_MS, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add battery status update task fail\n");
    }
}

uint8_t Battery_GetBatteryPercent(void)
{
    return pre_Case_VBAT_percent;
}

uint16_t Battery_GetBatteryVoltage(void)
{
    return adc_convert_to_voltage;
}

void Battery_UpdateBatteryStatus(uint16_t vbat_voltage)
{
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();
    uint16_t Case_VBAT_percent = 0;
    adc_convert_to_voltage = vbat_voltage * 4;
    DEBUG_PRINT("battery calculate start\n");
    DEBUG_PRINT("Voltage:%d\n", adc_convert_to_voltage);
    if ((charge_status->case_charge_status == SY8809_CASE_CHARGE_STATUS_CHARGE_DONE) &&
        (Usb_GetUsbDetectState() == USB_PLUG))
    // ((Usb_GetUsbDetectState() == USB_PLUG) || (QI_Charge_state == QI_CONTACT)))
    // todo: check Qi connect status
    {
        Case_VBAT_percent = 100;
        pre_Case_VBAT_percent = Case_VBAT_percent;
    }
    else
    {
        if (adc_convert_to_voltage <= CASE_MIN_VBAT)
        {
            Case_VBAT_percent = 0;
        }
        else if (adc_convert_to_voltage >= CASE_MAX_VBAT)
        {
            Case_VBAT_percent = 100;
        }
        else
        {
            for (uint8_t i = 0; i < (sizeof(battery_voltage_table) / sizeof(battery_voltage_table[0]) - 1); i++)
            {
                if (adc_convert_to_voltage >= battery_voltage_table[i] && adc_convert_to_voltage <= battery_voltage_table[i + 1])
                {
                    uint16_t diff = battery_voltage_table[i + 1] - battery_voltage_table[i];
                    uint16_t offset = adc_convert_to_voltage - battery_voltage_table[i];
                    uint16_t percent_in_block = (offset * 100) / diff; // 0~100%
                    percent_in_block = (percent_in_block * 5) / 100;   // scale to 0~5%
                    Case_VBAT_percent = percent_in_block + (i * 5);
                    break;
                }
            }
        }

        if (Case_VBAT_percent == 0)
        {
            Case_VBAT_percent = 1;
        }

        // Handle first-time initialization case
        if (pre_Case_VBAT_percent == BATTERY_UNKNOWN_LEVEL)
        {
            // On first boot, allow pre_Case_VBAT_percent to be set regardless of USB state
            pre_Case_VBAT_percent = Case_VBAT_percent;
        }
        else if (Usb_GetUsbDetectState() == USB_PLUG)
        {
            // Battery should only increase while charging
            // Only update if the new percentage is greater and within a valid change range
            if ((Case_VBAT_percent > pre_Case_VBAT_percent) &&
                ((Case_VBAT_percent - pre_Case_VBAT_percent) < 127))
            {
                pre_Case_VBAT_percent = Case_VBAT_percent;
            }
        }
        else
        {
            // Battery should only decrease while discharging
            // Only update if the new percentage is lower and within a valid change range
            if ((pre_Case_VBAT_percent > Case_VBAT_percent) &&
                ((pre_Case_VBAT_percent - Case_VBAT_percent) < 127))
            {
                pre_Case_VBAT_percent = Case_VBAT_percent;
            }
        }
    }
    DEBUG_PRINT("percent:%d\n", pre_Case_VBAT_percent);
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/