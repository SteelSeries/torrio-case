/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "i2c1.h"
#include "sy8809.h"
#include "i2c2.h"
#include "cps4520.h"
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
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void i2c_lowlevel_init(i2c_handle_type *hi2c)
{
    gpio_init_type gpio_initstructure;
    const I2c1_HardwareSettings_t *i2c1_hardware_settings = I2c1_setting();
    const I2c2_HardwareSettings_t *i2c2_hardware_settings = I2c2_setting();

    if (hi2c->i2cx == i2c1_hardware_settings->i2c1_port)
    {
        /* i2c periph clock enable */
        crm_periph_clock_enable(i2c1_hardware_settings->i2c1_crm_clk, TRUE);
        crm_periph_clock_enable(i2c1_hardware_settings->i2c1_scl_gpio_crm_clk, TRUE);
        crm_periph_clock_enable(i2c1_hardware_settings->i2c1_sda_gpio_crm_clk, TRUE);

        /* gpio configuration */
        gpio_initstructure.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
        gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
        gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
        gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;

        /* configure i2c pins: scl */
        gpio_initstructure.gpio_pins = i2c1_hardware_settings->i2c1_scl_gpio_pin;
        gpio_init(i2c1_hardware_settings->i2c1_scl_gpio_port, &gpio_initstructure);

        /* configure i2c pins: sda */
        gpio_initstructure.gpio_pins = i2c1_hardware_settings->i2c1_sda_gpio_pin;
        gpio_init(i2c1_hardware_settings->i2c1_sda_gpio_port, &gpio_initstructure);

        i2c_init(hi2c->i2cx, I2C_FSMODE_DUTY_2_1, i2c1_hardware_settings->i2c1_speed);

        i2c_own_address1_set(hi2c->i2cx, I2C_ADDRESS_MODE_7BIT, SY8809_I2C_SLAVE_ADDRESS);
    }
    else if (hi2c->i2cx == i2c2_hardware_settings->i2c2_port)
    {
        /* i2c periph clock enable */
        crm_periph_clock_enable(i2c2_hardware_settings->i2c2_crm_clk, TRUE);
        crm_periph_clock_enable(i2c2_hardware_settings->i2c2_scl_gpio_crm_clk, TRUE);
        crm_periph_clock_enable(i2c2_hardware_settings->i2c2_sda_gpio_crm_clk, TRUE);
        gpio_pin_remap_config(I2C2_GMUX_0011, TRUE);

        /* gpio configuration */
        gpio_initstructure.gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN;
        gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
        gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
        gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;

        /* configure i2c pins: scl */
        gpio_initstructure.gpio_pins = i2c2_hardware_settings->i2c2_scl_gpio_pin;
        gpio_init(i2c2_hardware_settings->i2c2_scl_gpio_port, &gpio_initstructure);

        /* configure i2c pins: sda */
        gpio_initstructure.gpio_pins = i2c2_hardware_settings->i2c2_sda_gpio_pin;
        gpio_init(i2c2_hardware_settings->i2c2_sda_gpio_port, &gpio_initstructure);

        i2c_init(hi2c->i2cx, I2C_FSMODE_DUTY_2_1, i2c2_hardware_settings->i2c2_speed);

        i2c_own_address1_set(hi2c->i2cx, I2C_ADDRESS_MODE_7BIT, CPS4520_I2C_SLAVE_ADDRESS);
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/