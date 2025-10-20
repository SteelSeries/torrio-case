/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "power_control.h"
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
void PowerControl_Init(void)
{
    /* enable pwc clock */
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

    if (pwc_flag_get(PWC_STANDBY_FLAG) != RESET)
    {
        /* wakeup from standby */
        pwc_flag_clear(PWC_STANDBY_FLAG);
    }

    if (pwc_flag_get(PWC_WAKEUP_FLAG) != RESET)
    {
        /* wakeup event occurs */
        pwc_flag_clear(PWC_WAKEUP_FLAG);
    }
}

void PowerControl_EnterSleep(void)
{
    __IO uint32_t index = 0;
    __IO uint32_t systick_index = 0;

    /* save systick register configuration */
    systick_index = SysTick->CTRL;
    systick_index &= ~((uint32_t)0xFFFFFFFE);

    /* disable systick */
    SysTick->CTRL &= (uint32_t)0xFFFFFFFE;

    /* enter sleep mode */
    pwc_sleep_mode_enter(PWC_SLEEP_ENTER_WFI);

    /* restore systick register configuration */
    {
        uint32_t tmp = SysTick->CTRL;
        tmp |= systick_index;
        SysTick->CTRL = tmp;
    }
}

void PowerControl_EnterStandby(void)
{
    pwc_wakeup_pin_enable(PWC_WAKEUP_PIN_1, TRUE);
    pwc_standby_mode_enter();
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/