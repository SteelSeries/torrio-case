
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define LIGHTING_CHANGE_FALSE                 0x00U
#define LIGHTING_CHANGE_TRUE                  0x01U
#define LIGHTING_BRIGHT_MAX                   PWM_SET_LEVEL
#define LIGHTING_BRIGHT_MIN                   0
#define LIGHTING_HOLD_TIME                    300
#define LIGHTING_BREATH_HOLD_TIME             200
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
  LIGHTING_LED_OFF,
  LIGHTING_STABLE,
  LIGHTING_BREATH,
  LIGHTING_BREATH_QUICKLY,
  LIGHTING_BREATH_QUICKLY_ONCE,
  LIGHTING_BLINK,
  LIGHTING_ILLUM
} LED_MODE;

typedef struct
{
    gpio_type *lighting_r_gpio_port;
    uint32_t lighting_r_gpio_pin;
    crm_periph_clock_type lighting_r_gpio_crm_clk;

    gpio_type *lighting_g_gpio_port;
    uint32_t lighting_g_gpio_pin;
    crm_periph_clock_type lighting_g_gpio_crm_clk;

    gpio_type *lighting_b_gpio_port;
    uint32_t lighting_b_gpio_pin;
    crm_periph_clock_type lighting_b_gpio_crm_clk;

} Lighting_HardwareSettings_t;

typedef enum
{
    LIGHTING_LID_COMPLETE = 0,
    LIGHTING_LID_NON_COMPLETE,
    LIGHTING_LID_UNKNOW
} Lighting_Lid_State_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern uint8_t Lighting_Change_Flag;
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Lighting_GpioConfigHardware(const Lighting_HardwareSettings_t *hardware_settings);
void Lighting_HandleTask(void);
void Lighting_Handler(uint16_t LightingMode, uint16_t PwmR, uint16_t PwmG, uint16_t PwmB);
Lighting_Lid_State_t Lighting_LidOffHandle(void);
