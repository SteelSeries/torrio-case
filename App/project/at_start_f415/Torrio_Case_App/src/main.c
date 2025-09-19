/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f415_board.h"
#include "at32f415_clock.h"
#include "custom_hid_class.h"
#include "custom_hid_desc.h"
#include "usb.h"

/** @addtogroup AT32F415_periph_examples
  * @{
  */

/** @addtogroup 415_USB_device_custom_hid USB_device_custom_hid
  * @{
  */

/* usb global struct define */
#if defined ( __ICCARM__ ) /* iar compiler */
  #pragma data_alignment=4
#endif
ALIGNED_HEAD uint8_t report_buf[USBD_CUSTOM_IN_MAXPACKET_SIZE] ALIGNED_TAIL;

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();

  at32_board_init();

  /* usb gpio config */
  Usb_GpioConfig();

#ifdef USB_LOW_POWER_WAKUP
  Usb_LowPowerWakeupConfig();
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

  while(1)
  {
    if(at32_button_press() == USER_BUTTON)
    {
      report_buf[0] = HID_REPORT_ID_5;
      report_buf[1] = (~report_buf[1]) & 0x1;
      custom_hid_class_send_report(&otg_core_struct.dev, report_buf, USBD_CUSTOM_IN_MAXPACKET_SIZE);
    }
  }
}






/**
  * @}
  */

/**
  * @}
  */
