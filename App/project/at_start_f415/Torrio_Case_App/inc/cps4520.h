
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define CPS4520_I2C_SLAVE_ADDRESS (0x30 << 1)  // 7-bit address shifted for R/W bit

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *cps4520_detect_gpio_port;
    uint32_t cps4520_detect_gpio_pin;
    crm_periph_clock_type cps4520_detect_gpio_crm_clk;

    gpio_type *cps4520_int_gpio_port;
    uint32_t cps4520_int_gpio_pin;
    crm_periph_clock_type cps4520_int_gpio_crm_clk;
} Cps4520_HardwareSettings_t;

typedef enum
{
    CPS4520_NON_DETECT = 0,
    CPS4520_DETECT,
    CPS4520_UNKNOW
} Cps4520_DetectConnectState_t;

typedef enum
{
    CPS4520_NON_INT = 0,
    CPS4520_INT_COMPELETE,
    CPS4520_INT_CORRECT
} Cps4520_IntRegState_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Cps4520_GpioConfigHardware(const Cps4520_HardwareSettings_t *hardware_settings);
void Cps4520_DetectStatusCheckTask(void);
void Cps4520_RegCheckTask(void);
Cps4520_DetectConnectState_t Cps4520_GetDetectState(void);
void Cps4520_SettingRegTableInit(void);

