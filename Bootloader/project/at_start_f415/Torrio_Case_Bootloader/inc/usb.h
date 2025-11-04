
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "usb_conf.h"
#include "usb_core.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define USB_DET_PIN GPIO_PINS_3                // PC3
#define USB_DET_GPIO GPIOC                     // PC3
#define USB_DET_CRM_CLK CRM_GPIOC_PERIPH_CLOCK // PC3
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    USB_UNPLUG = 0,
    USB_PLUG,
    USB_UNKNOW,
} Usb_DetectConnectState_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern otg_core_type otg_core_struct;
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
extern void Usb_Clock48mSelect(usb_clk48_s clk_s);
extern void Usb_GpioConfig(void);
extern void Usb_LowPowerWakeupConfig(void);
extern void usb_delay_ms(uint32_t ms);
extern void usb_delay_us(uint32_t us);
Usb_DetectConnectState_t Usb_GetUsbDetectState(void);
