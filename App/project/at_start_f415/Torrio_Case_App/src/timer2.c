/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "timer2.h"
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
void Timer2_Init(void)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    crm_clocks_freq_get(&crm_clocks_freq_struct);

    crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
    tmr_32_bit_function_enable(TMR2, TRUE);

    // Initialize Timer2 as a 100-microsecond system time base
    //
    // - Auto-reload value (ARR) is set to 0xFFFFFFFF for full 32-bit range.
    //   This means the timer will count from 0 up to 4,294,967,295 before rolling over.
    //
    // - Prescaler is calculated as: (AHB_Frequency / Desired_Timer_Frequency) - 1
    //   For AHB = 144 MHz and target = 10,000 Hz (i.e., one tick every 100 us):
    //     Prescaler = (144,000,000 / 10,000) - 1 = 14,400 - 1 = 14,399
    //
    // - Final configuration:
    //     - Timer counts up every 100 microseconds
    //     - Max measurable time = 0xFFFFFFFF × 100 us ≈ 429,496.7295 seconds ≈ 4.97 days
    tmr_base_init(TMR2, 0xFFFFFFFF, (crm_clocks_freq_struct.ahb_freq / 10000) - 1);

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