/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "usb.h"
#include "task_scheduler.h"
#include "system_state_manager.h"
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
otg_core_type otg_core_struct;

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static usbd_event_type usb_ready = USBD_NOP_EVENT;
static Usb_HardwareSettings_t user_hardware_settings = {0};
static Usb_DetectConnectState_t usb_detect_state = USB_UNKNOW;
static Usb_DetectConnectState_t pre_usb_detect_state = USB_UNKNOW;
static Usb_DetectConnectState_t FirstSetupUsbState = USB_UNKNOW;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Usb_GpioConfigHardware(const Usb_HardwareSettings_t *hardware_settings)
{
  gpio_init_type gpio_init_struct;

  memcpy(&user_hardware_settings, hardware_settings, sizeof(Usb_HardwareSettings_t));

  crm_periph_clock_enable(user_hardware_settings.usb_detect_gpio_crm_clk, TRUE);
  crm_periph_clock_enable(user_hardware_settings.usb_otg_pin_sof_gpio_crm_clk, TRUE);
  crm_periph_clock_enable(user_hardware_settings.usb_otg_pin_vbus_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;

#ifdef USB_SOF_OUTPUT_ENABLE
  crm_periph_clock_enable(user_hardware_settings.usb_otg_pin_sof_gpio_crm_clk, TRUE);
  gpio_init_struct.gpio_pins = user_hardware_settings.usb_otg_pin_sof_gpio_pin;
  gpio_init(user_hardware_settings.usb_otg_pin_sof_gpio_port, &gpio_init_struct);
#endif

  /* otgfs use vbus pin */
#ifndef USB_VBUS_IGNORE
  gpio_init_struct.gpio_pins = user_hardware_settings.usb_otg_pin_vbus_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init(user_hardware_settings.usb_otg_pin_vbus_gpio_port, &gpio_init_struct);
#endif

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pins = user_hardware_settings.usb_detect_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init(user_hardware_settings.usb_detect_gpio_port, &gpio_init_struct);
  usb_detect_state = (Usb_DetectConnectState_t)gpio_input_data_bit_read(user_hardware_settings.usb_detect_gpio_port, user_hardware_settings.usb_detect_gpio_pin);
  pre_usb_detect_state = usb_detect_state;
  FirstSetupUsbState = usb_detect_state;
}

Usb_DetectConnectState_t Usb_GetUsbDetectState(void)
{
  return pre_usb_detect_state;
}

Usb_DetectConnectState_t Usb_FirstSetupUsbState(void)
{
  return FirstSetupUsbState;
}

void Usb_StatusCheckTask(void)
{
  static bool is_debounce_check = false;
  usb_detect_state = (Usb_DetectConnectState_t)gpio_input_data_bit_read(user_hardware_settings.usb_detect_gpio_port, user_hardware_settings.usb_detect_gpio_pin);

  if (is_debounce_check == false)
  {
    if (usb_detect_state != pre_usb_detect_state)
    {
      is_debounce_check = true;
    }
  }
  else
  {
    if (usb_detect_state != pre_usb_detect_state)
    {
      pre_usb_detect_state = usb_detect_state;
      DEBUG_PRINT("USB state changed to: %s\n", usb_detect_state == USB_PLUG ? "PLUG" : "UNPLUG");
      if (pre_usb_detect_state == USB_UNPLUG)
      {
        if (TaskScheduler_AddTask(SystemStateManager_SystemResetCheck, 10, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
        {
          DEBUG_PRINT("add system reset task fail\n");
        }
      }
    }
    is_debounce_check = false;
  }
}

void Usb_ReadyStateSet(usbd_event_type usb_state)
{
  usb_ready = usb_state;
  DEBUG_PRINT("USB event:%d\n", usb_ready);
}

usbd_event_type Usb_ReadyStateGet(void)
{
  return usb_ready;
}

void usb_delay_ms(uint32_t ms)
{
  /* user can define self delay function */
  delay_ms(ms);
}

void usb_delay_us(uint32_t us)
{
  delay_us(us);
}

void Usb_Clock48mSelect(usb_clk48_s clk_s)
{
  crm_clocks_freq_type clocks_struct;

  crm_clocks_freq_get(&clocks_struct);
  switch (clocks_struct.sclk_freq)
  {
  /* 48MHz */
  case 48000000:
    crm_usb_clock_div_set(CRM_USB_DIV_1);
    break;

  /* 72MHz */
  case 72000000:
    crm_usb_clock_div_set(CRM_USB_DIV_1_5);
    break;

  /* 96MHz */
  case 96000000:
    crm_usb_clock_div_set(CRM_USB_DIV_2);
    break;

  /* 120MHz */
  case 120000000:
    crm_usb_clock_div_set(CRM_USB_DIV_2_5);
    break;

  /* 144MHz */
  case 144000000:
    crm_usb_clock_div_set(CRM_USB_DIV_3);
    break;

  default:
    break;
  }
}

#ifdef USB_LOW_POWER_WAKUP
void Usb_LowPowerWakeupConfig(void)
{
  exint_init_type exint_init_struct;

  exint_default_para_init(&exint_init_struct);

  exint_init_struct.line_enable = TRUE;
  exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
  exint_init_struct.line_select = OTG_WKUP_EXINT_LINE;
  exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
  exint_init(&exint_init_struct);

  nvic_irq_enable(OTG_WKUP_IRQ, 0, 0);
}

/**
 * @brief  this function handles otgfs wakup interrupt.
 * @param  none
 * @retval none
 */
void OTG_WKUP_HANDLER(void)
{
  exint_flag_clear(OTG_WKUP_EXINT_LINE);
}

#endif
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/