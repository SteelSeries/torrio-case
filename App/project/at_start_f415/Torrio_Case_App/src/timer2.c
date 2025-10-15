/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer2.h"

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
void Timer2_Init(void)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    crm_clocks_freq_get(&crm_clocks_freq_struct);

    crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
    tmr_32_bit_function_enable(TMR2, TRUE);

    /*
     * Determine the actual timer clock frequency.
     *
     * Timer2 is typically connected to APB1. According to the timer clock rules:
     * - If APB1 prescaler == 1 → Timer clock = APB1 clock
     * - If APB1 prescaler > 1  → Timer clock = APB1 clock × 2
     */
    uint32_t pclk1 = crm_clocks_freq_struct.apb1_freq;
    uint32_t tmr_clk;

    if (CRM->cfg_bit.apb1div > 0) // prescaler != 1
    {
        tmr_clk = pclk1 * 2;
    }
    else
    {
        tmr_clk = pclk1;
    }

    /*
     * Timer2 configuration:
     *
     * Goal: Configure Timer2 to tick every 100 microseconds (100 us)
     * → Timer frequency = 10,000 Hz
     *
     * Prescaler = (Timer_Clock / Desired_Timer_Frequency) - 1
     *           = (tmr_clk / 10,000) - 1
     *
     * Auto-reload value (ARR): 0xFFFFFFFF (32-bit max value)
     * → Timer counts from 0 to 4,294,967,295
     * → Max time = 0xFFFFFFFF × 100us ≈ 429,496.7 seconds ≈ 4.97 days
     */

    uint32_t prescaler = (tmr_clk / 10000) - 1;
    tmr_base_init(TMR2, 0xFFFFFFFF, prescaler);

    tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);

    /* tmr2 enable counter */
    tmr_counter_enable(TMR2, TRUE);
}

uint32_t Timer2_GetTick(void)
{
    return tmr_counter_value_get(TMR2);
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/