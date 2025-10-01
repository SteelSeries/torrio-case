/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "i2c1.h"
#include "sy8809.h"
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

void i2c_lowlevel_init(i2c_handle_type *hi2c)
{
    gpio_init_type gpio_initstructure;

    if (hi2c->i2cx == user_hardware_settings.i2c1_port)
    {
        /* i2c periph clock enable */
        crm_periph_clock_enable(user_hardware_settings.i2c1_crm_clk, TRUE);
        crm_periph_clock_enable(user_hardware_settings.i2c1_scl_gpio_crm_clk, TRUE);
        crm_periph_clock_enable(user_hardware_settings.i2c1_sda_gpio_crm_clk, TRUE);

        /* gpio configuration */
        gpio_initstructure.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
        gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
        gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
        gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;

        /* configure i2c pins: scl */
        gpio_initstructure.gpio_pins = user_hardware_settings.i2c1_scl_gpio_pin;
        gpio_init(user_hardware_settings.i2c1_scl_gpio_port, &gpio_initstructure);

        /* configure i2c pins: sda */
        gpio_initstructure.gpio_pins = user_hardware_settings.i2c1_sda_gpio_pin;
        gpio_init(user_hardware_settings.i2c1_sda_gpio_port, &gpio_initstructure);

        i2c_init(hi2c->i2cx, I2C_FSMODE_DUTY_2_1, user_hardware_settings.i2c1_speed);

        i2c_own_address1_set(hi2c->i2cx, I2C_ADDRESS_MODE_7BIT, SY8809_I2C_SLAVE_ADDRESS);
    }
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