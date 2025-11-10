
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *lid_gpio_port;
    uint32_t lid_gpio_pin;
    crm_periph_clock_type lid_gpio_crm_clk;
} Lid_HardwareSettings_t;

typedef enum
{
    LID_CLOSE = 0,
    LID_OPEN,
    LID_UNKNOW
} Lid_State_t;

typedef enum
{
    LID_USB_REPROT_OPEN = 0,
    LID_USB_REPROT_CLOSE,
    LID_USB_REPROT_UNKNOW
} Lid_Usb_Report_State_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Lid_GpioConfigHardware(const Lid_HardwareSettings_t *hardware_settings);
Lid_State_t Lid_GetState(void);
void Lid_StatusCheckTask(void);
void Lid_GetLidStatusHandle(void);
