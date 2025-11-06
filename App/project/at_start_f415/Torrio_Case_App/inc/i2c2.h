
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *i2c2_sda_gpio_port;
    gpio_type *i2c2_scl_gpio_port;

    uint32_t i2c2_sda_gpio_pin;
    uint32_t i2c2_scl_gpio_pin;

    crm_periph_clock_type i2c2_sda_gpio_crm_clk;
    crm_periph_clock_type i2c2_scl_gpio_crm_clk;

    i2c_type *i2c2_port;
    crm_periph_clock_type i2c2_crm_clk;
    uint32_t i2c2_speed;

} I2c2_HardwareSettings_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern i2c_handle_type hi2c2x;

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void I2c2_GpioConfigHardware(const I2c2_HardwareSettings_t *hardware_settings);
I2c2_HardwareSettings_t const *I2c2_setting(void);
i2c_status_type I2c2_ReadReg(uint16_t address, uint8_t reg, uint8_t *i2c_rx_buff);
i2c_status_type I2c2_WriteReg(uint16_t address, uint8_t reg, uint8_t data);
