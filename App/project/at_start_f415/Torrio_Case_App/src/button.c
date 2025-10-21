/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "button.h"
#include "usb.h"
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
static Button_HardwareSettings_t user_hardware_settings = {0};
static Button_State_t button_state = BUTTON_UNKNOW;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Button_GpioConfigHardware(const Button_HardwareSettings_t *hardware_settings)
{
  gpio_init_type gpio_init_struct;
  memcpy(&user_hardware_settings, hardware_settings, sizeof(Button_HardwareSettings_t));

  crm_periph_clock_enable(user_hardware_settings.button_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = user_hardware_settings.button_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(user_hardware_settings.button_gpio_port, &gpio_init_struct);
}

Button_State_t Button_GetState(void)
{
  return button_state;
}

void Button_StatusCheckTask(void)
{
  static bool is_debounce_check = false;

  button_state = (Button_State_t)gpio_input_data_bit_read(user_hardware_settings.button_gpio_port, user_hardware_settings.button_gpio_pin);

  if (is_debounce_check == false)
  {
      is_debounce_check = true;
  }
  else
  {
    if (button_state == BUTTON_PRESS)//TDB action on button press
    {
        printf("Button state is PRESS\n");
        uint8_t buff[2] = {0x00};
        buff[0] = BUTTON_PRESS;
        buff[1] = COMMAND_STATUS_SUCCESS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
    is_debounce_check = false;
  }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
