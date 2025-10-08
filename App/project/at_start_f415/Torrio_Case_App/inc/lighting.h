
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
#define LIGHTING_CHANGE_FALSE                 0x00U
#define LIGHTING_CHANGE_TRUE                  0x01U
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
  LED_ON,
  LED_OFF
} LED_STATE;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern uint8_t LIGHTING_CHANGE_FLAG;
extern uint8_t LIGHTING_MODE;
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Lighting_HandlerTask(void);
void Lighting_LEDNonPWMSetting(uint8_t rgb, confirm_state state);
