/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer3.h"
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
static Pwm_HardwareSettings_t user_hardware_settings = {0};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Pwm_GpioConfigHardware(const Pwm_HardwareSettings_t *hardware_settings)
{
    tmr_output_config_type tmr_oc_init_structure;

    memcpy(&user_hardware_settings, hardware_settings, sizeof(Pwm_HardwareSettings_t));

    uint16_t ch1_val = 333;
    uint16_t ch2_val = 249;
    uint16_t ch3_val = 166;
    uint16_t div_value = 0;
    /*===========Timer3 PIN================*/
    gpio_init_type gpio_init_struct;

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

    gpio_init_struct.gpio_pins = user_hardware_settings.pwm_r_gpio_pin;
    gpio_init(user_hardware_settings.pwm_r_gpio_port, &gpio_init_struct);

    gpio_init_struct.gpio_pins = user_hardware_settings.pwm_g_gpio_pin;
    gpio_init(user_hardware_settings.pwm_g_gpio_port, &gpio_init_struct);

    gpio_init_struct.gpio_pins = user_hardware_settings.pwm_b_gpio_pin;
    gpio_init(user_hardware_settings.pwm_b_gpio_port, &gpio_init_struct);
    /*====================================*/

    crm_periph_clock_enable(user_hardware_settings.pwm_r_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.pwm_g_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.pwm_b_gpio_crm_clk, TRUE);

    /* compute the div value */
    div_value = (uint16_t)(system_core_clock / 24000000) - 1;

    // Initialize Timer3 for time base configuration
    // 
    // - The system core clock is set to HICK_VALUE, which is defined as 8,000,000 Hz.
    //   This means the timer will use this clock frequency for its operation.
    // 
    // - The div_value is calculated as: (system_core_clock / 24000000) - 1
    //   This sets the timer's frequency to 24 MHz.
    // 
    // - Timer3 is configured with an auto-reload value (ARR) of 665.
    //   This means the timer will count from 0 up to 665 before rolling over.
    // 
    // - Final configuration:
    //     - Timer counts up
    //     - The timer will generate an interrupt or event when it reaches the ARR value.
    tmr_base_init(TMR3, 665, div_value);
    tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR3, TMR_CLOCK_DIV1);

    tmr_output_default_para_init(&tmr_oc_init_structure);
    tmr_oc_init_structure.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    tmr_oc_init_structure.oc_idle_state = FALSE;
    tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    tmr_oc_init_structure.oc_output_state = TRUE;
    tmr_output_channel_config(TMR3, TMR_SELECT_CHANNEL_1, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, ch1_val);
    tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_1, TRUE);

    tmr_output_channel_config(TMR3, TMR_SELECT_CHANNEL_2, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, ch2_val);
    tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_2, TRUE);

    tmr_output_channel_config(TMR3, TMR_SELECT_CHANNEL_3, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_3, ch3_val);
    tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_3, TRUE);

    tmr_period_buffer_enable(TMR3, TRUE);

    /* tmr enable counter */
    tmr_counter_enable(TMR3, TRUE);
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/