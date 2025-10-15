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
    gpio_initstructure.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_pins = GPIO_PINS_9 | GPIO_PINS_8;
    gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOB, &gpio_initstructure);
    /*====================================*/

    /* get system clock */
    crm_clocks_freq_get(&crm_clocks_freq_struct);

    crm_periph_clock_enable(CRM_TMR4_PERIPH_CLOCK, TRUE);

    /*
     * Determine actual timer clock frequency.
     * Timer4 is on APB1. Per timer clock rules:
     * - If APB1 prescaler == 1 → TMR_CLK = APB1
     * - If APB1 prescaler > 1  → TMR_CLK = APB1 × 2
     */
    uint32_t pclk1 = crm_clocks_freq_struct.apb1_freq;
    uint32_t tmr_clk;

    if (CRM->cfg_bit.apb1div > 0)
    {
        tmr_clk = pclk1 * 2;
    }
    else
    {
        tmr_clk = pclk1;
    }

    /*
     * Goal: Generate an update event (overflow) every 1ms → 1 kHz
     *
     * Strategy:
     * - Set prescaler to divide TMR_CLK to 10 kHz → 1 tick = 100 us
     * - Set auto-reload (ARR) = 10 (so 10 ticks = 1 ms)
     *
     * Timer frequency after prescaler = tmr_clk / (PSC + 1)
     * Target: 10 kHz → PSC = (tmr_clk / 10000) - 1
     * ARR = 10 - 1 = 9
     */
    uint16_t prescaler = (tmr_clk / 10000) - 1;
    uint16_t arr = 10 - 1; // 1 ms interval

    tmr_base_init(TMR4, arr, prescaler);
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