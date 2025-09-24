/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "i2c1.h"
#include "sy8809.h"
#include <string.h>
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
static I2c1_HardwareSettings_t user_hardware_settings = {0};
static i2c_handle_type hi2cx = {0};
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
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/