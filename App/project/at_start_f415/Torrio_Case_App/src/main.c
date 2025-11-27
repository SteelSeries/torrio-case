/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "custom_hid_class.h"
#include "custom_hid_desc.h"
#include "usb.h"
#include "cps4520.h"
#include "Commands.h"
#include "timer2.h"
#include "timer3.h"
#include "timer5.h"
#include "task_scheduler.h"
#include "lighting.h"
#include "sy8809.h"
#include "power_control.h"
#include "init_pinout.h"
#include "adc.h"
#include "timer4.h"
#include "app_fw_update.h"
#include "file_system.h"
#include "system_clock.h"
#include "lid.h"
#include "i2c1.h"
#include "i2c2.h"
#include "wdt.h"
#include "button.h"
#include "uart_comm_manager.h"
#include "system_state_manager.h"
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

  InitPinout_Init();

  SystemClock_ClockConfigSwitch();

  at32_board_init();

  PowerControl_Init();

  Wdt_Init();

  // ==============================debug message==============================
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  FileSystem_UserData_t *data = (FileSystem_UserData_t *)FileSystem_GetUserData();
#ifdef __ICCARM__
  DEBUG_PRINT("\n\n\nAPP Start!!! IAR build\n");
#else
  DEBUG_PRINT("\n\n\nAPP Start!!! GCC build\n");
#endif
  print_clock("SCLK", crm_clocks_freq_struct.sclk_freq);
  print_clock("AHB", crm_clocks_freq_struct.ahb_freq);
  print_clock("APB2", crm_clocks_freq_struct.apb2_freq);
  print_clock("APB1", crm_clocks_freq_struct.apb1_freq);
  print_clock("ADC", crm_clocks_freq_struct.adc_freq);

  DEBUG_PRINT("===== User Data Debug Info =====\n");
  DEBUG_PRINT("Model              : %02X\n", data->model);
  DEBUG_PRINT("Color              : %02X\n", data->color);
  DEBUG_PRINT("Shipping Flag      : %02X\n", data->shipping_flag);
  DEBUG_PRINT("Dual Image CopyFlg : %02X\n", data->dual_image_copy_flag);
  DEBUG_PRINT("Preset Charge State: 0x%02X\n", data->presetChargeState);

  DEBUG_PRINT("Serial Number      : ");
  for (uint8_t i = 0; i < sizeof(data->serial_number); i++)
  {
    DEBUG_PRINT("%02X ", data->serial_number[i]);
  }
  DEBUG_PRINT("\n");

  DEBUG_PRINT("================================\n");
  // ==============================debug message==============================
  Wdt_Enable();

  FileSystem_CheckImageCopyFlag();

  if (Usb_GetUsbDetectState() == USB_PLUG)
  {
    DEBUG_PRINT("system setup USB detect\n");
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
  }
  else
  {
    Timer5_Init();
  }

  Timer2_Init();

  Timer3_Init();

  Timer4_Init();

  Adc_Init();

  UartCommManager_Init();

  if(data->shipping_flag != SHIPPING_MODE_CLEAR)
  {
    DEBUG_PRINT("Enter Shipping Mode at main\r\n");
    if (TaskScheduler_AddTask(SystemStateManager_EnterShippingMode, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
      DEBUG_PRINT("add SystemStateManager_EnterShippingMode fail\n");
    }
  }

  if (TaskScheduler_AddTask(Sy8809_InitTask, 100, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
     DEBUG_PRINT("add sy8809 task fail\n");
  }

  if (TaskScheduler_AddTask(Lid_StatusCheckTask, 10, TASK_RUN_FOREVER, TASK_START_DELAYED) != TASK_OK)
  {
    DEBUG_PRINT("add lid check task fail\n");
  }

  if (TaskScheduler_AddTask(Button_StatusCheckTask, 10, TASK_RUN_FOREVER, TASK_START_DELAYED) != TASK_OK)
  {
    printf("add button check task fail\n");
  }

  if (TaskScheduler_AddTask(Usb_StatusCheckTask, 50, TASK_RUN_FOREVER, TASK_START_DELAYED) != TASK_OK)
  {
    DEBUG_PRINT("add USB check task fail\n");
  }

  if (TaskScheduler_AddTask(Cps4520_DetectStatusCheckTask, 10, TASK_RUN_FOREVER, TASK_START_DELAYED) != TASK_OK)
  {
    DEBUG_PRINT("add Qi check task fail\n");
  }

  if (TaskScheduler_AddTask(Lighting_HandleTask, 5, TASK_RUN_FOREVER, TASK_START_DELAYED) != TASK_OK)
  {
    DEBUG_PRINT("add lighting handle task fail\n");
  }

  DEBUG_PRINT("main loop start\n");
  while (1)
  {
    TaskScheduler_Run();

    Wdt_CountReset();

    if (Usb_FirstSetupUsbState() == USB_UNPLUG)
    {
      sleepTime = TaskScheduler_GetTimeUntilNextTask();
      if (sleepTime > 0)
      {
        // DEBUG_PRINT("setting sleep time:%d\n", sleepTime);
        Timer5_StartOneShot(sleepTime);
        PowerControl_EnterSleep();
        // DEBUG_PRINT("system wakeup\n");
      }
    }

    if (AppFwUpdata_GetResetFlag())
    {
      DEBUG_PRINT("system reset\n");
      AppFwUpdata_SetResetFlag(false);
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
  unsigned long mhz_frac = (hz_ul % 1000000UL) / 1000UL;

  (void)mhz_int;
  (void)mhz_frac;
  DEBUG_PRINT("%-12s: %lu Hz (%lu.%03lu MHz)\n", name, hz_ul, mhz_int, mhz_frac);
}
