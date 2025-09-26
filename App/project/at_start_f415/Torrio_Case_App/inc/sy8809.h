
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define SY8809_I2C_SLAVE_ADDRESS 0x0C

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
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Sy8809_InitTask(void);
void Sy8809_GpioConfigHardware(const Sy8809_HardwareSettings_t *hardware_settings);
void Sy8809_ReadIrqState(void);
