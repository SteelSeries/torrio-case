/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer5.h"

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
    nvic_irq_enable(TMR5_GLOBAL_IRQn, 1, 0);

    /* enable tmr 5 */
    tmr_counter_enable(TMR5, TRUE);
}

void Timer5_StartOneShot(uint32_t delay_Tick)
{
    // Structure to store system clock frequencies
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    crm_clocks_freq_get(&crm_clocks_freq_struct);

    // Get APB1 peripheral clock
    uint32_t pclk1 = crm_clocks_freq_struct.apb1_freq;
    uint32_t tmr_clk;

    /*
     * Determine actual timer clock (TMR5_CLK) from APB1
     *
     * Rule:
     *   - If APB1 prescaler == 1 → Timer clock = APB1 clock
     *   - If APB1 prescaler > 1  → Timer clock = APB1 clock × 2
     *
     * Notes:
     *   - At 8 MHz or 24 MHz, APB1 prescaler = 1 → timer clock = APB1
     *   - At 144 MHz, APB1 prescaler > 1 (e.g., /4) → timer clock = APB1 × 2
     */
    if (CRM->cfg_bit.apb1div > 0) // Prescaler not equal to 1
    {
        tmr_clk = pclk1 * 2;
    }
    else
    {
        tmr_clk = pclk1;
    }

    // Cap delay_Tick to max 16-bit value (ARR is 16-bit)
    if (delay_Tick >= 65535)
    {
        delay_Tick = 65535;
    }
    else if (delay_Tick == 1)
    {
        delay_Tick = 2;
    }

    /*
     * Timer tick frequency = tmr_clk / (prescaler + 1)
     *
     * We want 1 tick = 100 us → 10,000 ticks per second
     * So: prescaler = (tmr_clk / 10,000) - 1
     *
     * Example:
     * --------------------------------------------------------------
     * APB1 = 8 MHz   → tmr_clk = 8 MHz   → prescaler = (8M / 10k) - 1 = 799
     * APB1 = 24 MHz  → tmr_clk = 24 MHz  → prescaler = (24M / 10k) - 1 = 2399
     * APB1 = 72 MHz (from 144 MHz / 2)   → tmr_clk = 144 MHz → prescaler = 14399
     */
    uint16_t prescaler = (tmr_clk / 10000) - 1;

    // Set up timer: ARR = delay_Tick - 1, PSC = prescaler
    tmr_base_init(TMR5, delay_Tick - 1, prescaler);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/