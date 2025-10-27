/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "lid.h"
#include "task_scheduler.h"
#include "system_state_manager.h"
#include "task_scheduler.h"
#include "usb.h"
#include "lighting.h"
#include <string.h>

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
static Lid_HardwareSettings_t user_hardware_settings = {0};
static Lid_State_t lid_state = LID_UNKNOW;
static Lid_State_t pre_lid_state = LID_UNKNOW;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lid_GpioConfigHardware(const Lid_HardwareSettings_t *hardware_settings)
{
  gpio_init_type gpio_init_struct;
  memcpy(&user_hardware_settings, hardware_settings, sizeof(Lid_HardwareSettings_t));

  crm_periph_clock_enable(user_hardware_settings.lid_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = user_hardware_settings.lid_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(user_hardware_settings.lid_gpio_port, &gpio_init_struct);
  lid_state = (Lid_State_t)gpio_input_data_bit_read(user_hardware_settings.lid_gpio_port, user_hardware_settings.lid_gpio_pin);
  pre_lid_state = lid_state;

// This section is only used during the Scala EV (Engineering Verification) phase,
// where the circuit board included a MOSFET switch for opening the lid.
// In later hardware versions, this feature was removed, so this code is no longer needed.
#ifdef SCALA_BOARD
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_2;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOC, &gpio_init_struct);
  gpio_bits_reset(GPIOC, GPIO_PINS_2);
#endif
}

Lid_State_t Lid_GetState(void)
{
  return pre_lid_state;
}

void Lid_StatusCheckTask(void)
{
  static bool is_debounce_check = false;

  lid_state = (Lid_State_t)gpio_input_data_bit_read(user_hardware_settings.lid_gpio_port, user_hardware_settings.lid_gpio_pin);

  if (is_debounce_check == false)
  {
    if (lid_state != pre_lid_state)
    {
      is_debounce_check = true;
    }
  }
  else
  {
    if (lid_state != pre_lid_state)
    {
      pre_lid_state = lid_state;
      DEBUG_PRINT("Lid state changed to: %s\n", lid_state == LID_OPEN ? "OPEN" : "CLOSED");
      if (pre_lid_state == LID_CLOSE)
      {
        if ((Usb_FirstSetupUsbState() == USB_UNPLUG) && (Lighting_LidOffHandle() == LIGHTING_BREATH_COMPLETE))
        {
          if (TaskScheduler_AddTask(SystemStateManager_EnterStandbyModeCheck, 10, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
          {
            DEBUG_PRINT("add enter standby task fail\n");
          }
        }
      }
    }
    is_debounce_check = false;
  }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/