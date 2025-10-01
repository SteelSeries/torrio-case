
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

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *usb_detect_gpio_port;
    uint32_t usb_detect_gpio_pin;
    crm_periph_clock_type usb_detect_gpio_crm_clk;

    gpio_type *usb_otg_pin_sof_gpio_port;
    uint32_t usb_otg_pin_sof_gpio_pin;
    crm_periph_clock_type usb_otg_pin_sof_gpio_crm_clk;

    gpio_type *usb_otg_pin_vbus_gpio_port;
    uint32_t usb_otg_pin_vbus_gpio_pin;
    crm_periph_clock_type usb_otg_pin_vbus_gpio_crm_clk;
} Usb_HardwareSettings_t;

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
void Usb_Clock48mSelect(usb_clk48_s clk_s);
void Usb_LowPowerWakeupConfig(void);
void Usb_ReadyStateSet(usbd_event_type usb_state);
usbd_event_type Usb_ReadyStateGet(void);
void Usb_GpioConfigHardware(const Usb_HardwareSettings_t *hardware_settings);
Usb_DetectConnectState_t Usb_GetUsbDetectState(void);
