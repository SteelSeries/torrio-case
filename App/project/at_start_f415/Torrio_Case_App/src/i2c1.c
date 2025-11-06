/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "i2c1.h"
#include "sy8809.h"
#include "pinout.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define I2C_TIMEOUT 0xFFFFFFFF

/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static I2c1_HardwareSettings_t user_hardware_settings = {0};
i2c_handle_type hi2cx = {0};
static const I2c1_HardwareSettings_t i2c1_config =
    {
        .i2c1_sda_gpio_port = I2C1_SDA_GPIO_PORT,
        .i2c1_sda_gpio_pin = I2C1_SDA_PIN,
        .i2c1_sda_gpio_crm_clk = I2C1_SDA_GPIO_CLK,

        .i2c1_scl_gpio_port = I2C1_SCL_GPIO_PORT,
        .i2c1_scl_gpio_pin = I2C1_SCL_PIN,
        .i2c1_scl_gpio_crm_clk = I2C1_SCL_GPIO_CLK,

        .i2c1_port = I2C1_PORT,
        .i2c1_crm_clk = I2C1_CLK,

        .i2c1_speed = I2C1_SPEED,
};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void I2c1_GpioConfigHardware(const I2c1_HardwareSettings_t *hardware_settings)
{
    memcpy(&user_hardware_settings, hardware_settings, sizeof(I2c1_HardwareSettings_t));
    hi2cx.i2cx = user_hardware_settings.i2c1_port;
    i2c_config(&hi2cx);
}

I2c1_HardwareSettings_t const *I2c1_setting(void)
{
    return &i2c1_config;
}

i2c_status_type I2c1_ReadReg(uint16_t address, uint8_t reg, uint8_t *i2c_rx_buff)
{
    i2c_status_type i2c_status;
    uint8_t reg_tx_buff[] = {reg};
    /* start the request reception process */
    if ((i2c_status = i2c_master_transmit(&hi2cx, address, reg_tx_buff, sizeof(reg_tx_buff), I2C_TIMEOUT)) != I2C_OK)
    {
        return i2c_status;
    }

    /* start the request reception process */
    if ((i2c_status = i2c_master_receive(&hi2cx, address, i2c_rx_buff, sizeof(i2c_rx_buff), I2C_TIMEOUT)) != I2C_OK)
    {
        return i2c_status;
    }

    return i2c_status;
}

i2c_status_type I2c1_WriteReg(uint16_t address, uint8_t reg, uint8_t data)
{
    i2c_status_type i2c_status;
    uint8_t reg_tx_buff[] = {reg, data};
    /* start the request reception process */
    if ((i2c_status = i2c_master_transmit(&hi2cx, address, reg_tx_buff, sizeof(reg_tx_buff), I2C_TIMEOUT)) != I2C_OK)
    {
        return i2c_status;
    }
    return i2c_status;
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/