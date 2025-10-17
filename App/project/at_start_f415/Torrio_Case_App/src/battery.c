/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "battery.h"
#include "sy8809.h"
#include "usb.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define CASE_MAX_VBAT 4340
#define CASE_MIN_VBAT 3500
#define SY8809_0X12_CASE_BATT_CHARGE_COMPLETE 0x03
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
static uint8_t pre_Case_VBAT_percent = 0;

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
uint8_t Battery_GetBatteryPercent(void)
{
    return pre_Case_VBAT_percent;
}

void Battery_UpdateBatteryStatus(uint16_t vbat_voltage)
{
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();
    uint16_t Case_VBAT_percent = 0;
    uint16_t ADC_convert_to_Voltage = vbat_voltage * 4;

    if (((charge_status->check_reg_state.reg_0x12 & SY8809_0X12_CASE_BATT_CHARGE_COMPLETE) == SY8809_0X12_CASE_BATT_CHARGE_COMPLETE) &&
        (Usb_GetUsbDetectState() == USB_PLUG))
    // ((Usb_GetUsbDetectState() == USB_PLUG) || (QI_Charge_state == QI_CONTACT)))
    // todo: check Qi connect status
    {
        Case_VBAT_percent = 100;
        pre_Case_VBAT_percent = Case_VBAT_percent;
    }
    else
    {
        if (ADC_convert_to_Voltage <= CASE_MIN_VBAT)
        {
            Case_VBAT_percent = 0;
        }
        else if (ADC_convert_to_Voltage >= CASE_MAX_VBAT)
        {
            Case_VBAT_percent = 100;
        }
        else
        {
            for (uint8_t i = 0; i < (sizeof(battery_voltage_table) / sizeof(battery_voltage_table[0]) - 1); i++)
            {
                if (ADC_convert_to_Voltage >= battery_voltage_table[i] && ADC_convert_to_Voltage <= battery_voltage_table[i + 1])
                {
                    uint16_t diff = battery_voltage_table[i + 1] - battery_voltage_table[i];
                    uint16_t offset = ADC_convert_to_Voltage - battery_voltage_table[i];
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

        if (Usb_GetUsbDetectState() == USB_PLUG)
        {
            // The battery always increases when charging
            if ((Case_VBAT_percent > pre_Case_VBAT_percent) && ((Case_VBAT_percent - pre_Case_VBAT_percent) < 127))
            {
                pre_Case_VBAT_percent = Case_VBAT_percent;
            }
        }
        else
        {
            // The battery will always decrease when not charging.
            if ((pre_Case_VBAT_percent > Case_VBAT_percent) && ((pre_Case_VBAT_percent - Case_VBAT_percent) < 127))
            {
                pre_Case_VBAT_percent = Case_VBAT_percent;
            }
        }
    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/