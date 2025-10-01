/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "sy8809_xsense.h"
#include "timer4.h"
#include "task_scheduler.h"
#include "i2c1.h"
#include "sy8809.h"
#include "adc.h"
#include "custom_hid_class.h"
#include "usb.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define WAIT_8809_XSENSE_STABLE_TIMER 200U
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
void Sy8809Xsense_TrigXsenseConv(void)
{
    printf("[%s]start\n", __func__);

    ConfigXsenseOutput(pending_xsense);
    // After setting xSense, wait 200ms before starting ADC sampling.
    // This ensures that the ADC reads stable data from the xSense pin of SY8809.
    if (TaskScheduler_AddTask(Timer4_AdcTrigStart, WAIT_8809_XSENSE_STABLE_TIMER, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add adc trig task fail\n");
    }
}

void Sy8809Xsense_SetPendingXsense(Sy8809Xsense_OutputItem_t Pending)
{
    pending_xsense = Pending;
}

void Sy8809Xsense_ReadXsenseProcess(void)
{
    printf("[%s]\n", __func__);
    uint16_t adc_voltage_mv = 0;
    uint16_t adc_raw = 0;
    uint8_t usb_report_buff[4] = {0x00};
    Adc_GetAvgRawAndVoltage(&adc_voltage_mv, &adc_raw);

    switch (pending_xsense)
    {
    case SY8809_XSENSE_VBAT:
    {
        // Todo: check battery level and conversion to percentage.
        usb_report_buff[0] = (uint8_t)(adc_raw >> 8);
        usb_report_buff[1] = (uint8_t)adc_raw;
        usb_report_buff[2] = (uint8_t)(adc_voltage_mv >> 8);
        usb_report_buff[3] = (uint8_t)adc_voltage_mv;

        custom_hid_class_send_report(&otg_core_struct.dev, usb_report_buff, sizeof(usb_report_buff));
        break;
    }

    case SY8809_XSENSE_NTC:
    case SY8809_XSENSE_IBAT:
    case SY8809_XSENSE_IVOR:
    case SY8809_XSENSE_IVOL:
    case SY8809_XSENSE_IVIN:
    case SY8809_XSENSE_VBIN:
    {
        usb_report_buff[0] = (uint8_t)(adc_raw >> 8);
        usb_report_buff[1] = (uint8_t)adc_raw;
        usb_report_buff[2] = (uint8_t)(adc_voltage_mv >> 8);
        usb_report_buff[3] = (uint8_t)adc_voltage_mv;

        custom_hid_class_send_report(&otg_core_struct.dev, usb_report_buff, sizeof(usb_report_buff));
        break;
    }
    }

    if (TaskScheduler_AddTask(XsenseClear, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add sy8809 working task fail\n");
    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void XsenseClear(void)
{
    printf("[%s]\n", __func__);
    Sy8809_ChargeStatus_t charge_staus;
    Sy8809_GetChargeIcStatusInfo(&charge_staus);
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  ENXSENSE_DISABLED);

    switch (pending_xsense)
    {

    case SY8809_XSENSE_NTC:
    {
        I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                      SY8809_REG_0x36,
                      0x10);
        break;
    }

    case SY8809_XSENSE_VBAT:
    {
        I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                      SY8809_REG_0x22,
                      charge_staus.check_reg_state.reg_0x22);
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
    printf("[%s]\n", __func__);
    Sy8809_ChargeStatus_t charge_staus;
    uint8_t sy8809_reg_rx_buff[1] = {0};
    Sy8809_GetChargeIcStatusInfo(&charge_staus);

    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x15, sy8809_reg_rx_buff);
    charge_staus.check_reg_state.reg_0x15 = sy8809_reg_rx_buff[0];
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x12, sy8809_reg_rx_buff);
    charge_staus.check_reg_state.reg_0x12 = sy8809_reg_rx_buff[0];

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x36,
                  0x1C);

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_NTC));
    Sy8809_SetChargeIcStatusInfo(&charge_staus);
}

static void SetXsenseOutputIbat(void)
{
    printf("set read IBAT\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IBAT));
}

static void SetXsenseOutputIvor(void)
{
    printf("set read IVOR\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVOR));
}

static void SetXsenseOutputIvol(void)
{
    printf("set read IVOL\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVOL));
}

static void SetXsenseOutputIvin(void)
{
    printf("set read IVIN\n");
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_IVIN));
}

static void SetXsenseOutputVbat(void)
{
    printf("set read Vbat\n");
    Sy8809_ChargeStatus_t charge_staus;
    uint8_t sy8809_reg_rx_buff[1] = {0};

    Sy8809_GetChargeIcStatusInfo(&charge_staus);
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x12, sy8809_reg_rx_buff);
    charge_staus.check_reg_state.reg_0x12 = sy8809_reg_rx_buff[0];

    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x22, sy8809_reg_rx_buff);
    charge_staus.check_reg_state.reg_0x22 = sy8809_reg_rx_buff[0];

    // set xSense output vbat
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_VBAT_DIV4));

    // stop case battery charge
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x22,
                  0x00);
    Sy8809_SetChargeIcStatusInfo(&charge_staus);
}

static void SetXsenseOutputVbin(void)
{
    printf("set read VBIN\n");
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  XsenseConfigure(ENXSENSE_ENABLED, XSENSE_GAIN_1, XSENSE_CH_VIN_DIV8));
    ;
}

static void SetXsenseReset(void)
{
    printf("set xSense reset\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
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
