/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "at32f415_clock.h"
#include "custom_hid_class.h"
#include "custom_hid_desc.h"
#include "usb.h"
#include "command.h"
#include "file_system.h"
#include "bootloader.h"
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
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
int main(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();

  at32_board_init();

  Bootloader_BackDoorHallGpioInit();

  Bootloader_UsbConnectGpioInit();

  DEBUG_PRINT("\n\n\nBootloader start!!!\n");

  if (FileSystem_CheckImageCopyFlag() == SUCCESS)
  {
    if (Bootloader_CheckBackDoor() == FALSE)
    {
      if (Bootloader_CheckAppCodeComplete())
      {
        if (gCurrentMode != BOOTLOADER_MODE)
        {
          DEBUG_PRINT("jump to app\n");
          Bootloader_JumpToApp();
        }
      }
    }
  }

  /* usb gpio config */
  Usb_GpioConfig();

#ifdef USB_LOW_POWER_WAKUP
  usb_low_power_wakeup_config();
#endif

  /* enable otgfs clock */
  crm_periph_clock_enable(OTG_CLOCK, TRUE);

  /* select usb 48m clcok source */
  Usb_Clock48mSelect(USB_CLK_HEXT);

  /* enable otgfs irq */
  nvic_irq_enable(OTG_IRQ, 0, 0);

  /* init usb */
  usbd_init(&otg_core_struct,
            USB_FULL_SPEED_CORE_ID,
            USB_ID,
            &custom_hid_class_handler,
            &custom_hid_desc_handler);

  while (1)
  {
    if (Usb_GetUsbDetectState() != USB_PLUG)
    {
      gCurrentMode = NORMAL_MODE;
      SS_RESET_FLAG = true;
    }

    if (SS_RESET_FLAG)
    {
      SS_RESET_FLAG = false;
      delay_ms(500);
      nvic_system_reset();
    }
  }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
