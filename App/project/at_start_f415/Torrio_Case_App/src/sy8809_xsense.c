/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "sy8809_xsense.h"
#include "timer4.h"
#include "task_scheduler.h"
#include "i2c_comm.h"
#include "sy8809.h"
#include "adc.h"
#include "custom_hid_class.h"
#include "Commands.h"
#include "usb.h"
#include "battery.h"
#include "file_system.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define WAIT_8809_XSENSE_STABLE_TIMER 200U
/* To accurately read the VBAT voltage, charging must be paused first.
 * This is because the elevated voltage during charging can affect ADC accuracy.
 * After stopping the charging process, wait for about 5 seconds
 * to allow the voltage to settle to its true value.
 * Only then should the ADC measurement be performed.*/
#define WAIT_8809_XSENSE_VBAT_STABLE_TIMER 5000U
#define MASK_GAIN_XSENSE (0x3 << 4) // bits 5:4
#define SHIFT_GAIN_XSENSE 4
#define MASK_CH_SEL (0xF) // bits 3:0
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/**
 * @brief  Pending xSense command received from USB, to be handled by xsense task.
 *         Set by USB command handler, read by xsense task to configure SY8809.
 *
 * @note   Must be set before waking up xsense task.
 *
 * @warning This variable may be overwritten if multiple USB commands are received
 *          before the xsense task processes the previous one.
 */
static volatile Sy8809Xsense_OutputItem_t pending_xsense = SY8809_XSENSE_NULL;
static bool is_xsense_pending_command_read = false;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void ConfigXsenseOutput(Sy8809Xsense_OutputItem_t xsense);
static void SetXsenseOutputNtc(void);
static void SetXsenseOutputIbat(void);
static void SetXsenseOutputIvor(void);
static void SetXsenseOutputIvol(void);
static void SetXsenseOutputIvin(void);
static void SetXsenseOutputVbat(void);
static void SetXsenseOutputVbin(void);
static void SetXsenseReset(void);
static void XsenseClear(void);
static uint8_t XsenseConfigure(Sy8809Xsense_XsenseConfig_EnXsense_t en,
                               Sy8809Xsense_XsenseConfig_XsenseGain_t gain,
                               Sy8809Xsense_XsenseConfig_ChSel_t ch);
static uint8_t XsenseConfigureClear(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
Sy8809Xsense_OutputItem_t Sy8809Xsense_pending(void)
{
    return pending_xsense;
}

void Sy8809Xsense_TrigXsenseConv(void)
{

    uint16_t task_delay_time = 0;
    DEBUG_PRINT("[%s]start\n", __func__);

    ConfigXsenseOutput(pending_xsense);
    // After setting xSense, wait 200ms before starting ADC sampling.
    // This delay is sufficient in most cases for stable readings.
    // However, when measuring VBAT, a longer delay of 5 seconds is required
    // to allow the voltage to settle after stopping the charging process.

    if (pending_xsense == SY8809_XSENSE_VBAT)
    {
        task_delay_time = WAIT_8809_XSENSE_VBAT_STABLE_TIMER;
    }
    else
    {
        task_delay_time = WAIT_8809_XSENSE_STABLE_TIMER;
    }

    if (TaskScheduler_AddTask(Timer4_AdcTrigStart, task_delay_time, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add adc trig task fail\n");
    }
}

void Sy8809Xsense_FirstXsenseConvVbat(void)
{
    DEBUG_PRINT("[%s]start\n", __func__);

    pending_xsense = SY8809_XSENSE_VBAT;
    is_xsense_pending_command_read = false;
    ConfigXsenseOutput(pending_xsense);

    // Since the device is not charging in this state, it's sufficient to wait 200ms before triggering the ADC to sample VBAT
    if (TaskScheduler_AddTask(Timer4_AdcTrigStart, WAIT_8809_XSENSE_STABLE_TIMER, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add adc trig task fail\n");
    }
}

void Sy8809Xsense_SetPendingXsense(Sy8809Xsense_XsenseRead_t Pending_temp)
{
    pending_xsense = Pending_temp.Pending;
    is_xsense_pending_command_read = Pending_temp.is_command_read;
}

void Sy8809Xsense_ReadXsenseProcess(void)
{
    DEBUG_PRINT("[%s]\n", __func__);
    uint16_t adc_voltage_mv = 0;
    uint16_t adc_raw = 0;
    uint8_t usb_report_buff[5] = {0x00};
    Adc_GetAvgRawAndVoltage(&adc_raw, &adc_voltage_mv);

    switch (pending_xsense)
    {
    case SY8809_XSENSE_VBAT:
    {
        if (is_xsense_pending_command_read == true)
        {
            is_xsense_pending_command_read = false;
            usb_report_buff[0] = DEBUG_SY8809_XSENSE_OP | COMMAND_READ_FLAG;
            usb_report_buff[1] = (uint8_t)adc_raw;
            usb_report_buff[2] = (uint8_t)(adc_raw >> 8);
            usb_report_buff[3] = (uint8_t)adc_voltage_mv;
            usb_report_buff[4] = (uint8_t)(adc_voltage_mv >> 8);
            custom_hid_class_send_report(&otg_core_struct.dev, usb_report_buff, sizeof(usb_report_buff));
        }
        else
        {
            Battery_UpdateBatteryStatus(adc_voltage_mv);
        }
        break;
    }

    case SY8809_XSENSE_NTC:
    case SY8809_XSENSE_IBAT:
    case SY8809_XSENSE_IVOR:
    case SY8809_XSENSE_IVOL:
    case SY8809_XSENSE_IVIN:
    case SY8809_XSENSE_VBIN:
    {
        usb_report_buff[0] = DEBUG_SY8809_XSENSE_OP | COMMAND_READ_FLAG;
        usb_report_buff[1] = (uint8_t)adc_raw;
        usb_report_buff[2] = (uint8_t)(adc_raw >> 8);
        usb_report_buff[3] = (uint8_t)adc_voltage_mv;
        usb_report_buff[4] = (uint8_t)(adc_voltage_mv >> 8);
        custom_hid_class_send_report(&otg_core_struct.dev, usb_report_buff, sizeof(usb_report_buff));
        break;
    }
    }

    if (TaskScheduler_AddTask(XsenseClear, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add sy8809 working task fail\n");
    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void XsenseClear(void)
{
    DEBUG_PRINT("[%s]\n", __func__);
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  ENXSENSE_DISABLED);

    switch (pending_xsense)
    {

    case SY8809_XSENSE_NTC:
    {
        I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                      SY8809_REG_0x36,
                      0x10);
        break;
    }

    case SY8809_XSENSE_VBAT:
    {
        bool need_write = true;
        if (FileSystem_GetUserData()->presetChargeState == PRESET_CHARGE_ACTIVE &&
            Battery_GetPresetChargeState()->case_charge_status == BATTERY_PRESET_CHARGE_DONE)
        {
            need_write = false;
        }

        if (need_write)
        {
            I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
            				SY8809_REG_0x22,
                          	charge_status->check_reg_state.reg_0x22);

            DEBUG_PRINT("[VBAT] I2C write executed (preset=%02X, case_state=%d)\n",
                        FileSystem_GetUserData()->presetChargeState,
                        Battery_GetPresetChargeState()->case_charge_status);
        }
        else
        {
            DEBUG_PRINT("[VBAT] Skip I2C write (PresetCharge=ACTIVE, Case=CHARGE_DONE)\n");
        }

        break;
    }

        // case SY8809_XSENSE_IBAT:
        // case SY8809_XSENSE_IVOR:
        // case SY8809_XSENSE_IVOL:
        // case SY8809_XSENSE_IVIN:
        // case SY8809_XSENSE_VBIN:
    }
    pending_xsense = SY8809_XSENSE_NULL;
}

static void ConfigXsenseOutput(Sy8809Xsense_OutputItem_t xsense)
{
    switch (xsense)
    {
    case SY8809_XSENSE_NTC:
    {
        SetXsenseOutputNtc();
        break;
    }

    case SY8809_XSENSE_IBAT:
    {
        SetXsenseOutputIbat();
        break;
    }

    case SY8809_XSENSE_IVOR:
    {
        SetXsenseOutputIvor();
        break;
    }

    case SY8809_XSENSE_IVOL:
    {
        SetXsenseOutputIvol();
        break;
    }

    case SY8809_XSENSE_IVIN:
    {
        SetXsenseOutputIvin();
        break;
    }

    case SY8809_XSENSE_VBAT:
    {
        SetXsenseOutputVbat();
        break;
    }

    case SY8809_XSENSE_VBIN:
    {
        SetXsenseOutputVbin();
        break;
    }

    default:
    {
        SetXsenseReset();
        break;
    }
    }
}

static void SetXsenseOutputNtc(void)
{
    DEBUG_PRINT("[%s]\n", __func__);
    uint8_t sy8809_reg_rx_buff[1] = {0};
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();

    I2cComm_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x15, sy8809_reg_rx_buff);
    charge_status->check_reg_state.reg_0x15 = sy8809_reg_rx_buff[0];
    I2cComm_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x12, sy8809_reg_rx_buff);
    charge_status->check_reg_state.reg_0x12 = sy8809_reg_rx_buff[0];

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x36,
                  0x1C);

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_NTC));
}

static void SetXsenseOutputIbat(void)
{
    DEBUG_PRINT("set read IBAT\n");

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IBAT));
}

static void SetXsenseOutputIvor(void)
{
    DEBUG_PRINT("set read IVOR\n");

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVOR));
}

static void SetXsenseOutputIvol(void)
{
    DEBUG_PRINT("set read IVOL\n");

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVOL));
}

static void SetXsenseOutputIvin(void)
{
    DEBUG_PRINT("set read IVIN\n");
    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVIN));
}

static void SetXsenseOutputVbat(void)
{
    DEBUG_PRINT("set read Vbat\n");
    uint8_t sy8809_reg_rx_buff[1] = {0};
    Sy8809_ChargeStatus_t *charge_status = (Sy8809_ChargeStatus_t *)Sy8809_GetChargeIcStatusInfo();

    I2cComm_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x12, sy8809_reg_rx_buff);
    charge_status->check_reg_state.reg_0x12 = sy8809_reg_rx_buff[0];

    I2cComm_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x22, sy8809_reg_rx_buff);
    charge_status->check_reg_state.reg_0x22 = sy8809_reg_rx_buff[0];

    // set xSense output vbat
    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_VBAT_DIV4));

    // stop case battery charge
    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x22,
                  0x00);
}

static void SetXsenseOutputVbin(void)
{
    DEBUG_PRINT("set read VBIN\n");
    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_VIN_DIV8));
}

static void SetXsenseReset(void)
{
    DEBUG_PRINT("set xSense reset\n");

    I2cComm_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigureClear());
}

static uint8_t XsenseConfigure(Sy8809Xsense_XsenseConfig_EnXsense_t en,
                               Sy8809Xsense_XsenseConfig_XsenseGain_t gain,
                               Sy8809Xsense_XsenseConfig_ChSel_t ch)
{
    uint8_t reg_config = 0;
    if (en == ENXSENSE_ENABLED)
    {
        reg_config = (uint8_t)ENXSENSE_ENABLED;
    }
    reg_config |= ((uint8_t)gain << SHIFT_GAIN_XSENSE) & MASK_GAIN_XSENSE;
    reg_config |= ((uint8_t)ch & MASK_CH_SEL);
    return reg_config;
}

static uint8_t XsenseConfigureClear(void)
{
    return (uint8_t)ENXSENSE_DISABLED;
}
