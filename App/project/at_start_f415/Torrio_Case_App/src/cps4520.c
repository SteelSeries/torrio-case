/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "cps4520.h"
#include "cps4520_table.h"
#include "i2c_comm.h"
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
static Cps4520_IntRegState_t cps4520_init_reg_flag = CPS4520_NON_INT;
static const uint8_t cps4520_reg_init_list[CPS4520_REG_TABLE_LEN][2] = {
    {CPS4520_REG_0x13, CPS4520_VERSION},
    {CPS4520_REG_0x15, CPS4520_MFG_CODE_B1},
    {CPS4520_REG_0x14, CPS4520_MFG_CODE_B2}};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/

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
      if(pre_cps4520_state == CPS4520_NON_DETECT)
      {
        cps4520_init_reg_flag = CPS4520_NON_INT;
      }
    }
    is_debounce_check = false;
  }
}

void Cps4520_RegCheckTask(void)
{
    static uint8_t cps4520_reg_rx_buff[3] = {0};

    if(cps4520_init_reg_flag == CPS4520_INT_COMPELETE)
    {
      for (uint8_t i = 0; i < CPS4520_REG_TABLE_LEN; i++)
      {
          I2cComm_ReadReg(CPS4520_I2C_SLAVE_ADDRESS, 
                          cps4520_reg_init_list[i][0], 
                          & cps4520_reg_rx_buff[i]);
      }
      if(cps4520_reg_rx_buff[0] == cps4520_reg_init_list[0][1] && cps4520_reg_rx_buff[1] == cps4520_reg_init_list[1][1] && cps4520_reg_rx_buff[2] == cps4520_reg_init_list[2][1])
      {
        cps4520_init_reg_flag = CPS4520_INT_CORRECT;
        for (uint8_t i = 0; i < CPS4520_REG_TABLE_LEN; i++)
        {
            DEBUG_PRINT("CPS4520_REG: %02X\n", cps4520_reg_rx_buff[i]);
        }
      }
    }
}

Cps4520_DetectConnectState_t Cps4520_GetDetectState(void)
{
  return pre_cps4520_state;
}

void Cps4520_SettingRegTableInit(void)
{
    if(cps4520_init_reg_flag == CPS4520_NON_INT)
    {
      for (uint8_t i = 0; i < CPS4520_REG_TABLE_LEN; i++)
      {
          I2cComm_WriteReg(
                          CPS4520_I2C_SLAVE_ADDRESS,
                          cps4520_reg_init_list[i][0],
                          cps4520_reg_init_list[i][1]);
      }
      cps4520_init_reg_flag = CPS4520_INT_COMPELETE;
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
