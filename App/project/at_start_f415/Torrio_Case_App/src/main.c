/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "at32f415_clock.h"
#include "custom_hid_class.h"
#include "custom_hid_desc.h"
#include "usb.h"
#include "Commands.h"
#include "timer2.h"
#include "timer5.h"
#include "task_scheduler.h"
#include "lighting.h"
#include "sy8809.h"
#include "power_control.h"
#include "init_pinout.h"
#include "adc.h"
#include "timer4.h"
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
static uint32_t sleepTime;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void print_clock(const char *name, uint32_t hz);
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
int main(void)
{
  crm_clocks_freq_type crm_clocks_freq_struct = {0};

  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();

  at32_board_init();

  crm_clocks_freq_get(&crm_clocks_freq_struct);

  printf("APP Start!!!\n");
  print_clock("SCLK", crm_clocks_freq_struct.sclk_freq);
  print_clock("AHB", crm_clocks_freq_struct.ahb_freq);
  print_clock("APB2", crm_clocks_freq_struct.apb2_freq);
  print_clock("APB1", crm_clocks_freq_struct.apb1_freq);
  print_clock("ADC", crm_clocks_freq_struct.adc_freq);

  InitPinout_Init();

#ifdef USB_LOW_POWER_WAKUP
  Usb_LowPowerWakeupConfig();
#endif

  /* enable otgfs clock */
  crm_periph_clock_enable(OTG_CLOCK, TRUE);

  /* select usb 48m clcok source */
  Usb_Clock48mSelect(USB_CLK_HEXT);

  /* enable otgfs irq */
  nvic_irq_enable(OTG_IRQ, 0, 0);

  /* init usb */
  usbd_init(&otg_core_struct,
            USB_FULL_SPEED_CORE_ID,
            USB_ID,
            &custom_hid_class_handler,
            &custom_hid_desc_handler);

  Timer2_Init();

  Timer5_Init();

  Timer4_Init();

  Adc_Init();

  PowerControl_Init();

  if (TaskScheduler_AddTask(Sy8809_InitTask, 100, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
    printf("add sy8809 task fail\n");
  }

  printf("main loop start\n");
  while (1)
  {
    TaskScheduler_Run();

    if (Usb_ReadyStateGet() != USBD_RESET_EVENT)
    {
      sleepTime = TaskScheduler_GetTimeUntilNextTask();
      if (sleepTime > 0)
      {
        // printf("setting sleep time:%d\n", sleepTime);
        Timer5_StartOneShot(sleepTime);
        PowerControl_EnterSleep();
        // printf("system wakeup\n");
      }
    }

    if (SS_RESET_FLAG)
    {
      printf("system reset\n");
      SS_RESET_FLAG = false;
      delay_ms(500);
      usbd_disconnect(&otg_core_struct.dev);
      delay_ms(500);
      nvic_system_reset();
    }
  }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void print_clock(const char *name, uint32_t hz)
{
  unsigned long hz_ul = (unsigned long)hz;
  unsigned long mhz_int = hz_ul / 1000000UL;
  unsigned long mhz_frac = (hz_ul % 1000000UL) / 1000UL; /* 三位數小數 (kHz) */

  /* %-12s 讓名稱對齊 */
  printf("%-12s: %lu Hz (%lu.%03lu MHz)\n", name, hz_ul, mhz_int, mhz_frac);
}
