
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "sy8809_table.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define SY8809_I2C_SLAVE_ADDRESS 0x0C

// Bit mask for ST_CHG_STAT[1:0] in register 0x12
// These two bits indicate the battery charging status:
// 00 - Not charging (VIN < VIN_UVLO)
// 01 - Trickle charging
// 10 - Constant current charging
// 11 - Charging complete
#define ST8809_ST_CHG_STAT_MASK    (REG_BIT(1) | REG_BIT(0))  // 0x03

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *sy8809_sda_gpio_port;
    uint32_t sy8809_sda_gpio_pin;
    crm_periph_clock_type sy8809_sda_gpio_crm_clk;

    gpio_type *busd_detect_resist_gpio_port;
    uint32_t busd_detect_resist_gpio_pin;
    crm_periph_clock_type busd_detect_resist_gpio_crm_clk;

    gpio_type *sy8809_irq_gpio_port;
    uint32_t sy8809_irq_gpio_pin;
    crm_periph_clock_type sy8809_irq_gpio_crm_clk;

} Sy8809_HardwareSettings_t;

typedef enum
{
    SY8809_CHARGE_STOP = 0,
    SY8809_CHARGE_STAR
} Sy8809_ChargeControl_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Sy8809_InitTask(void);
void Sy8809_GpioConfigHardware(const Sy8809_HardwareSettings_t *hardware_settings);
void Sy8809_ReadIrqState(void);
const Sy8809_ChargeStatus_t *Sy8809_GetChargeIcStatusInfo(void);
i2c_status_type Sy8809_DebugRegWrite(const uint8_t reg, const uint8_t value);
void Sy8809_DebugRegRead(const uint8_t reg, uint8_t *buff);
void Sy8809_StartWorkTask(void);
void Sy8809_ChargeStatusSet(Sy8809_ChargeControl_t status);
