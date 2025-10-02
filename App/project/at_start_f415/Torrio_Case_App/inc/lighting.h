
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define LED_R_PIN GPIO_PINS_6                // PA6
#define LED_G_PIN GPIO_PINS_7                // PA7
#define LED_B_PIN GPIO_PINS_0                // PB0
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
  LED_R,
  LED_G,
  LED_B,
} LED_COLOR;

typedef enum
{
  LED_MODE_NORMAL,
  STABLE,
  BREATH,
  BREATH_QUICKLY,
  BLINK,
  ILLUM
} LED_MODE;

typedef enum
{
  LED_DIR_BRIGHT,
  LED_DIR_DARK,
  LED_DIR_NOCARE,
  LED_DIR_HOLD
} LED_BREATH_DIR;

typedef enum
{
  LED_DISPLAY_NOP,
  LED_DISPLAY_BATT_LAVEL,
  LED_DISPLAY_BATT_CHARGE_LAVEL,
  LED_DISPLAY_UPDATE_FW,
  LED_DISPLAY_PAIRING_BT,
  LED_DISPLAY_PAIRING_2_4G,
  LED_DISPLAY_LOW_BATTERY_ALERT,
  LED_DISPLAY_FACTORY_CHARGE_STATE
} LED_TABLE;

typedef enum
{
  LED_ON,
  LED_OFF
} LED_STATE;

typedef struct
{
  bool change_flag;
  bool pwm_enable_flag;
  uint16_t breath_timer;
  uint8_t breath_dir;
  uint16_t breath_speed;
  uint32_t PWM_duty;
  uint16_t timer_count;
  uint8_t blink_count;
} T_LED_VAR;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Lighting_HandlerTask(void);
void Lighting_LEDNonPWMSetting(uint8_t rgb, confirm_state state);
void Lighting_BreathHandler(void);
