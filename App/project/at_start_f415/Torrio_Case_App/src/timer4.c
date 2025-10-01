/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer4.h"
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
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Timer4_Init(void)
{
    tmr_output_config_type tmr_oc_init_structure;
    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    /*===========DEBUG PIN================*/
    gpio_init_type gpio_initstructure;
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    gpio_default_para_init(&gpio_initstructure);
    gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
    gpio_initstructure.gpio_pins = GPIO_PINS_9;
    gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOB, &gpio_initstructure);
    /*====================================*/

    /* get system clock */
    crm_clocks_freq_get(&crm_clocks_freq_struct);

    crm_periph_clock_enable(CRM_TMR4_PERIPH_CLOCK, TRUE);

    // System clock is 144 MHz
    // Prescaler value is 9 → actual division is (9 + 1) = 10
    // Timer clock after prescaler: 144,000,000 / 10 = 14,400,000 Hz
    // Auto-reload value:
    // (144,000,000 / 10) / 1000 = 14,400 → ARR = 14,400 - 1 = 14399
    // Timer will generate an update event every 14,400 counts → 1ms interval (1000 Hz)
    tmr_base_init(TMR4, 9, (crm_clocks_freq_struct.sclk_freq / 10000 - 1));
    tmr_cnt_dir_set(TMR4, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR4, TMR_CLOCK_DIV1);

    tmr_output_default_para_init(&tmr_oc_init_structure);
    tmr_oc_init_structure.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
    tmr_oc_init_structure.oc_output_state = TRUE;
    tmr_oc_init_structure.oc_idle_state = FALSE;

    tmr_output_channel_config(TMR4, TMR_SELECT_CHANNEL_4, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR4, TMR_SELECT_CHANNEL_4, 5);
}

void Timer4_AdcTrigStart(void)
{
    /* TMR4 enable counter */

    tmr_counter_enable(TMR4, TRUE);
    tmr_channel_enable(TMR4, TMR_SELECT_CHANNEL_4, TRUE);
    tmr_output_enable(TMR4, TRUE);
}

void Timer4_AdcTrigStop(void)
{
    /* TMR4 Disable counter */

    tmr_counter_enable(TMR4, FALSE);
    tmr_channel_enable(TMR4, TMR_SELECT_CHANNEL_4, FALSE);
    tmr_output_enable(TMR4, FALSE);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/