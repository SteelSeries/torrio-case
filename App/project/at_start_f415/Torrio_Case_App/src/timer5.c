/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer5.h"
#include "at32f415_clock.h"

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
void Timer5_Init(void)
{
    /* enable tmr 5 clock */
    crm_periph_clock_enable(CRM_TMR5_PERIPH_CLOCK, TRUE);

    /* config the counting direction */
    tmr_cnt_dir_set(TMR5, TMR_COUNT_UP);

    /* config the clock divider value */
    tmr_clock_source_div_set(TMR5, TMR_CLOCK_DIV1);

    /* enable tmr 5 interrupt */
    tmr_interrupt_enable(TMR5, TMR_OVF_INT, TRUE);

    /* config tmr 5 nvic */
    nvic_irq_enable(TMR5_GLOBAL_IRQn, 0, 0);

    /* enable tmr 2 */
    tmr_counter_enable(TMR5, TRUE);
}

void Timer5_StartOneShot(uint32_t delay_Tick)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    crm_clocks_freq_get(&crm_clocks_freq_struct);

    if (delay_Tick >= 65535)
    {
        delay_Tick = 65535;
    }
    /*
     * Prescaler calculation:
     *   Timer frequency = 144,000,000 / (14400) = 10,000 Hz → 100 us per tick
     *
     * Timer period = delay_Tick - 1
     *   Because the timer counts from 0 to ARR (inclusive),
     *   if we want a delay of N ticks, we set ARR = N - 1
     */
    tmr_base_init(TMR5, delay_Tick - 1, (crm_clocks_freq_struct.sclk_freq / 10000 - 1));
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/