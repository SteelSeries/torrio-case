/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "system_clock.h"
#include "usb.h"
#include "lid.h"

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
static void SclkHext144mConfig(void);
static void SclkHext8mConfig(void);
static void SclkPll24mConfig(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void SystemClock_ClockConfigSwitch(void)
{
    if (Usb_GetUsbDetectState() != USB_PLUG)
    {
        if (Lid_GetState() != LID_OPEN)
        {
            SclkHext8mConfig();
        }
        else
        {
            SclkPll24mConfig();
        }
    }
    else
    {
        SclkHext144mConfig();
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
/**
 * @brief  system clock config program
 * @note   the system clock is configured as follow:
 *         system clock (sclk)   = hext / 2 * pll_mult
 *         system clock source   = pll (hext)
 *         - hext                = HEXT_VALUE
 *         - sclk                = 144000000
 *         - ahbdiv              = 1
 *         - ahbclk              = 144000000
 *         - apb2div             = 2
 *         - apb2clk             = 72000000
 *         - apb1div             = 2
 *         - apb1clk             = 72000000
 *         - pll_mult            = 36
 *         - flash_wtcyc         = 4 cycle
 * @param  none
 * @retval none
 */
static void SclkHext144mConfig(void)
{
    crm_reset();

    /* config flash psr register */
    flash_psr_set(FLASH_WAIT_CYCLE_4);

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);

    /* wait till hext is ready */
    while (crm_hext_stable_wait() == ERROR)
    {
    }

    /* config pll clock resource */
    crm_pll_config(CRM_PLL_SOURCE_HEXT_DIV, CRM_PLL_MULT_36);

    /* enable pll */
    crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);

    /* wait till pll is ready */
    while (crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
    {
    }

    /* config ahbclk */
    crm_ahb_div_set(CRM_AHB_DIV_1);

    /* config apb2clk, the maximum frequency of APB1/APB2 clock is 75 MHz  */
    crm_apb2_div_set(CRM_APB2_DIV_2);

    /* config apb1clk, the maximum frequency of APB1/APB2 clock is 75 MHz  */
    crm_apb1_div_set(CRM_APB1_DIV_2);

    /* enable auto step mode */
    crm_auto_step_mode_enable(TRUE);

    /* select pll as system clock source */
    crm_sysclk_switch(CRM_SCLK_PLL);

    /* wait till pll is used as system clock source */
    while (crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
    {
    }

    /* disable auto step mode */
    crm_auto_step_mode_enable(FALSE);

    /* update system_core_clock global variable */
    system_core_clock_update();
    delay_init();
}

/**
 * @brief  system clock config program
 * @note   the system clock is configured as follow:
 *         system clock (sclk)   = (hext)
 *         system clock source   = hext
 *         - hext                = HEXT_VALUE
 *         - sclk                = 8000000
 *         - ahbdiv              = 4
 *         - ahbclk              = 2000000
 *         - apb1div             = 1
 *         - apb1clk             = 2000000
 *         - apb2div             = 1
 *         - apb2clk             = 2000000
 *         - flash_wtcyc         = 0 cycle
 * @param  none
 * @retval none
 */
static void SclkHext8mConfig(void)
{
    crm_reset();
    flash_psr_set(FLASH_WAIT_CYCLE_0);

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
    while (crm_hext_stable_wait() == ERROR)
    {
    }

    crm_ahb_div_set(CRM_AHB_DIV_4);
    crm_apb2_div_set(CRM_APB2_DIV_1);
    crm_apb1_div_set(CRM_APB1_DIV_1);

    crm_sysclk_switch(CRM_SCLK_HEXT);
    while (crm_sysclk_switch_status_get() != CRM_SCLK_HEXT)
    {
    }

    system_core_clock_update();
    delay_init();
}

/**
 * @brief  system clock config program
 * @note   the system clock is configured as follow:
 *         system clock (sclk)   = hext * pll_mult
 *         system clock source   = HEXT_VALUE
 *         - hext                = HEXT_VALUE
 *         - sclk                = 24000000
 *         - ahbdiv              = 1
 *         - ahbclk              = 24000000
 *         - apb1div             = 1
 *         - apb1clk             = 24000000
 *         - apb2div             = 1
 *         - apb2clk             = 24000000
 *         - pll_mult            = 3
 *         - flash_wtcyc         = 0 cycle
 * @param  none
 * @retval none
 */
static void SclkPll24mConfig(void)
{
    crm_reset();
    flash_psr_set(FLASH_WAIT_CYCLE_0);

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
    while (crm_hext_stable_wait() == ERROR)
    {
    }

    crm_pll_config(CRM_PLL_SOURCE_HEXT, CRM_PLL_MULT_3);
    crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
    while (crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
    {
    }

    crm_ahb_div_set(CRM_AHB_DIV_1);
    crm_apb1_div_set(CRM_APB1_DIV_1);
    crm_apb2_div_set(CRM_APB2_DIV_1);

    crm_sysclk_switch(CRM_SCLK_PLL);
    while (crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
    {
    }

    system_core_clock_update();
    delay_init();
}
