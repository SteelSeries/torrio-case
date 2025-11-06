/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "cps4520.h"
#include "cps4520_table.h"
#include "usb.h"
#include "i2c2.h"
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
static Cps4520_HardwareSettings_t user_hardware_settings = {0};
static Cps4520_DetectConnectState_t cps4520_state = CPS4520_UNKNOW;
static Cps4520_DetectConnectState_t pre_cps4520_state = CPS4520_UNKNOW;
static bool cps4520_init_flag = false;
static const uint8_t cps4520_reg_init_list[CPS4520_REG_TABLE_LEN][2] = {
    {CPS4520_REG_0x13, 0x21},
    {CPS4520_REG_0x15, 0x01},
    {CPS4520_REG_0x14, 0x8A}};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void SettingRegTableInit(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Cps4520_GpioConfigHardware(const Cps4520_HardwareSettings_t *hardware_settings)
{
  gpio_init_type gpio_init_struct;
  exint_init_type exint_init_struct;

  memcpy(&user_hardware_settings, hardware_settings, sizeof(Cps4520_HardwareSettings_t));

  crm_periph_clock_enable(user_hardware_settings.cps4520_detect_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = user_hardware_settings.cps4520_detect_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(user_hardware_settings.cps4520_detect_gpio_port, &gpio_init_struct);

  crm_periph_clock_enable(user_hardware_settings.cps4520_int_gpio_crm_clk, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = user_hardware_settings.cps4520_int_gpio_pin;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(user_hardware_settings.cps4520_int_gpio_port, &gpio_init_struct);

  gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOB, GPIO_PINS_SOURCE5);

  exint_default_para_init(&exint_init_struct);
  exint_init_struct.line_enable = TRUE;
  exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
  exint_init_struct.line_select = EXINT_LINE_5;
  exint_init_struct.line_polarity = EXINT_TRIGGER_FALLING_EDGE;
  exint_init(&exint_init_struct);
  nvic_irq_enable(EXINT9_5_IRQn, 1, 0);
  exint_flag_clear(EXINT_LINE_5);
}

void Cps4520_DetectStatusCheckTask(void)
{
  static bool is_debounce_check = false;
  cps4520_state = (Cps4520_DetectConnectState_t)gpio_input_data_bit_read(user_hardware_settings.cps4520_detect_gpio_port, user_hardware_settings.cps4520_detect_gpio_pin);

  if (is_debounce_check == false)
  {
    if (cps4520_state != pre_cps4520_state)
    {
      is_debounce_check = true;
    }
  }
  else
  {
    if (cps4520_state != pre_cps4520_state)
    {
      pre_cps4520_state = cps4520_state;
      DEBUG_PRINT("Qi state changed to: %s\n", cps4520_state == CPS4520_DETECT ? "DETECT" : "NON_DETECT");
    }
    is_debounce_check = false;
  }
}

void Cps4520_InitReg(void)
{
  static uint8_t cps4520_reg13_rx_buff[1] = {0};
  static uint8_t cps4520_reg14_rx_buff[1] = {0};
  static uint8_t cps4520_reg15_rx_buff[1] = {0};
  if(cps4520_init_flag == false)
  {
    SettingRegTableInit();
    I2c2_ReadReg(CPS4520_I2C_SLAVE_ADDRESS, 0x13, cps4520_reg13_rx_buff);
    DEBUG_PRINT("CPS4520_REG_0x13: %02X\n", cps4520_reg13_rx_buff[0]);
    I2c2_ReadReg(CPS4520_I2C_SLAVE_ADDRESS, 0x15, cps4520_reg15_rx_buff);
    DEBUG_PRINT("CPS4520_REG_0x15: %02X\n", cps4520_reg15_rx_buff[0]);
    I2c2_ReadReg(CPS4520_I2C_SLAVE_ADDRESS, 0x14, cps4520_reg14_rx_buff);
    DEBUG_PRINT("CPS4520_REG_0x14: %02X\n", cps4520_reg14_rx_buff[0]);
    if(cps4520_reg15_rx_buff[0] == 0x01 && cps4520_reg14_rx_buff[0] == 0x8A && cps4520_reg13_rx_buff[0] == 0x21)
    {
      cps4520_init_flag = true;
      DEBUG_PRINT("CPS4520 Init\n");
    }
  }
}

Cps4520_DetectConnectState_t Cps4520_GetDetectState(void)
{
  return pre_cps4520_state;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void SettingRegTableInit(void)
{
    DEBUG_PRINT("table init\n");

    for (uint8_t i = 0; i < CPS4520_REG_TABLE_LEN; i++)
    {
        I2c2_WriteReg(CPS4520_I2C_SLAVE_ADDRESS,
                      cps4520_reg_init_list[i][0],
                      cps4520_reg_init_list[i][1]);
    }
}
