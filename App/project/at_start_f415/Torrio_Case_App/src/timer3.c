/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer3.h"
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
void Timer3_Init(void)
{
    tmr_output_config_type tmr_oc_init_structure;

    uint16_t ch1_val = 0U;
    uint16_t ch2_val = 0U;
    uint16_t ch3_val = 0U;
    uint16_t div_value = 0;

    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    crm_clocks_freq_get(&crm_clocks_freq_struct);

    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);

    /* compute the div value */
    if(system_core_clock > (TIMER3_PWM_FREQUENCY * PWM_SET_LEVEL)) 
    {
        div_value = (uint16_t)(system_core_clock / (TIMER3_PWM_FREQUENCY * PWM_SET_LEVEL) - 1U);
    } else 
    {
        div_value = 0U;
    }

    // Initialize Timer3 for time base configuration
    // 
    // - System core clock configurations:
    //   * 144 MHz -> div_value = 17  (Timer clock = 8 MHz)
    //   *  24 MHz -> div_value = 2   (Timer clock = 8 MHz)
    //   *   8 MHz -> div_value = 0   (Timer clock = 8 MHz)
    //
    // - PWM Configuration:
    //   * Resolution: 10-bit (ARR = 1023, counts from 0 to 1023)
    //   * Target PWM frequency: 7812.5 Hz (exactly 8MHz/1024)
    //   * Required timer clock = PWM frequency * Resolution = 7812.5 * 1024 = 8 MHz
    //
    // - Timer operation:
    //   * Counts up from 0 to 1023
    //   * PWM resolution: 0.098% (1/1024)
    //   * Consistent 7812.5 Hz across all system clock speeds

    tmr_base_init(TMR3, PWM_SET_LEVEL, div_value);
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