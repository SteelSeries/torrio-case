/**
 **************************************************************************
 * @file     at32f415_int.c
 * @brief    main interrupt service routines.
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

/* includes ------------------------------------------------------------------*/
#include "at32f415_int.h"
#include "usbd_int.h"
#include "usb.h"
#include "sy8809.h"
#include "cps4520.h"
#include "timer4.h"
#include "task_scheduler.h"
#include "sy8809_xsense.h"
#include "uart_comm_manager.h"
#include "uart_driver.h"

static UART_CommContext_t *user_left_bud_ctx = NULL;
static UART_CommContext_t *user_right_bud_ctx = NULL;

/** @addtogroup AT32F415_periph_examples
 * @{
 */

/** @addtogroup 415_USB_device_custom_hid
 * @{
 */

void Interrupt_BudsCtxInit(void)
{
  user_left_bud_ctx = UartCommManager_GetLeftBudContext();
  user_right_bud_ctx = UartCommManager_GetRightBudContext();
}

void NMI_Handler(void)
{
}

/**
 * @brief  this function handles hard fault exception.
 * @param  none
 * @retval none
 */
void HardFault_Handler(void)
{
  /* go to infinite loop when hard fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  this function handles memory manage exception.
 * @param  none
 * @retval none
 */
void MemManage_Handler(void)
{
  /* go to infinite loop when memory manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  this function handles bus fault exception.
 * @param  none
 * @retval none
 */
void BusFault_Handler(void)
{
  /* go to infinite loop when bus fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  this function handles usage fault exception.
 * @param  none
 * @retval none
 */
void UsageFault_Handler(void)
{
  /* go to infinite loop when usage fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  this function handles svcall exception.
 * @param  none
 * @retval none
 */
void SVC_Handler(void)
{
}

/**
 * @brief  this function handles debug monitor exception.
 * @param  none
 * @retval none
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  this function handles pendsv_handler exception.
 * @param  none
 * @retval none
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  this function handles systick handler.
 * @param  none
 * @retval none
 */
void SysTick_Handler(void)
{
}

/**
 * @brief  this function handles otgfs interrupt.
 * @param  none
 * @retval none
 */
void OTG_IRQ_HANDLER(void)
{
  usbd_irq_handler(&otg_core_struct);
}

/**
 * @brief  this function handles the timer interrupt request.
 * @param  none
 * @retval none
 */
void TMR5_GLOBAL_IRQHandler(void)
{
  if (tmr_interrupt_flag_get(TMR5, TMR_OVF_FLAG) != RESET)
  {
    /* clear timer 5 ovf flag */
    tmr_flag_clear(TMR5, TMR_OVF_FLAG);
  }
}

/**
 * @brief  exint0 interrupt handler
 * @param  none
 * @retval none
 */
void EXINT4_IRQHandler(void)
{
  if (exint_interrupt_flag_get(EXINT_LINE_4) != RESET)
  {
    Sy8809_ReadIrqState();
    exint_flag_clear(EXINT_LINE_4);
  }
}

/**
 * @brief  exint9_5 interrupt handler
 * @param  none
 * @retval none
 */
void EXINT9_5_IRQHandler(void)
{
  if (exint_interrupt_flag_get(EXINT_LINE_5) != RESET)
  {
    Cps4520_InitReg();
    DEBUG_PRINT("CPS4520 INT IRQ\n");
    exint_flag_clear(EXINT_LINE_5);
  }
}

/**
 * @brief  this function handles adc1 handler.
 * @param  none
 * @retval none
 */
void ADC1_IRQHandler(void)
{
}

/**
 * @brief  this function handles dma1_channel1 handler.
 * @param  none
 * @retval none
 */
void DMA1_Channel1_IRQHandler(void)
{
  if (dma_interrupt_flag_get(DMA1_FDT1_FLAG) != RESET)
  {
    dma_flag_clear(DMA1_FDT1_FLAG);
    Timer4_AdcTrigStop();

    if (TaskScheduler_AddTask(Sy8809Xsense_ReadXsenseProcess, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
      DEBUG_PRINT("add ReadXsenseProcess task fail\n");
    }
  }
}

/**
 * @brief  this function handles usart2 handler.
 * @param  none
 * @retval none
 */
void USART2_IRQHandler(void)
{
  if (usart_interrupt_flag_get(USART2, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */
    uint8_t data = usart_data_receive(USART2);
    UartDrive_RxIrqHandler(user_left_bud_ctx, data);
  }
}

/**
 * @brief  this function handles usart3 handler.
 * @param  none
 * @retval none
 */
void USART3_IRQHandler(void)
{
  if (usart_interrupt_flag_get(USART3, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */
    uint8_t data = usart_data_receive(USART3);
    UartDrive_RxIrqHandler(user_right_bud_ctx, data);
  }
}

/**
 * @}
 */

/**
 * @}
 */
