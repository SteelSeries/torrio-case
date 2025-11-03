/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "qi.h"
#include "usb.h"
#include "task_scheduler.h"
#include "system_state_manager.h"
#include "Commands.h"
#include <string.h>
#include "custom_hid_class.h"

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
static Qi_HardwareSettings_t user_hardware_settings = {0};
static Qi_DetectConnectState_t qi_state = QI_UNKNOW;
static Qi_DetectConnectState_t pre_qi_state = QI_UNKNOW;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Qi_GpioConfigHardware(const Qi_HardwareSettings_t *hardware_settings)
{
  gpio_init_type gpio_init_struct;
  memcpy(&user_hardware_settings, hardware_settings, sizeof(Qi_HardwareSettings_t));

  crm_periph_clock_enable(user_hardware_settings.qi_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = user_hardware_settings.qi_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(user_hardware_settings.qi_gpio_port, &gpio_init_struct);
}

void Qi_StatusCheckTask(void)
{
  static bool is_debounce_check = false;
  qi_state = (Qi_DetectConnectState_t)gpio_input_data_bit_read(user_hardware_settings.qi_gpio_port, user_hardware_settings.qi_gpio_pin);

  if (is_debounce_check == false)
  {
    if (qi_state != pre_qi_state)
    {
      is_debounce_check = true;
    }
  }
  else
  {
    if (qi_state != pre_qi_state)
    {
      pre_qi_state = qi_state;
      DEBUG_PRINT("Qi state changed to: %s\n", qi_state == QI_DETECT ? "DETECT" : "NON_DETECT");
    }
    is_debounce_check = false;
  }
}

Qi_DetectConnectState_t Qi_GetDetectState(void)
{
  return pre_qi_state;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
