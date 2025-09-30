/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "sy8809_xsense.h"
#include "timer4.h"
#include "task_scheduler.h"
#include "i2c1.h"
#include "sy8809.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define WAIT_8809_XSENSE_STABLE_TIMER 200U

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
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
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
                  0x44);
    Sy8809_SetChargeIcStatusInfo(&charge_staus);
}

static void SetXsenseOutputIbat(void)
{
    printf("set read IBAT\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x43);
}

static void SetXsenseOutputIvor(void)
{
    printf("set read IVOR\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x41);
}

static void SetXsenseOutputIvol(void)
{
    printf("set read IVOL\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x40);
}

static void SetXsenseOutputIvin(void)
{
    printf("set read IVIN\n");
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x47);
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
                  0x46);

    // stop case battery charge
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x22,
                  0x00);
    Sy8809_SetChargeIcStatusInfo(&charge_staus);

    // todo: check battery level and conversion to percentage.
}

static void SetXsenseOutputVbin(void)
{
    printf("set read VBIN\n");
    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x45);
}

static void SetXsenseReset(void)
{
    printf("set xSense reset\n");

    I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                  SY8809_REG_0x31,
                  0x00);
}