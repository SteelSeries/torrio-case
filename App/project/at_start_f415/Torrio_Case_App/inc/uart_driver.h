
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "uart_comm_manager.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef struct
{
    gpio_type *left_bud_uart_tx_gpio_port;
    uint32_t left_bud_uart_tx_gpio_pin;
    crm_periph_clock_type left_bud_uart_tx_gpio_crm_clk;

    gpio_type *left_bud_uart_rx_gpio_port;
    uint32_t left_bud_uart_rx_gpio_pin;
    crm_periph_clock_type left_bud_uart_rx_gpio_crm_clk;

    gpio_type *right_bud_uart_tx_gpio_port;
    uint32_t right_bud_uart_tx_gpio_pin;
    crm_periph_clock_type right_bud_uart_tx_gpio_crm_clk;

    gpio_type *right_bud_uart_rx_gpio_port;
    uint32_t right_bud_uart_rx_gpio_pin;
    crm_periph_clock_type right_bud_uart_rx_gpio_crm_clk;

    usart_type *left_bud_uart;
    usart_type *right_bud_uart;

} UartDrive_HardwareSettings_t;

typedef enum
{
    UART_ONEWIRE_SEND_MODE = 0,   // Send mode: TX = UART, RX = floating (analog)
    UART_ONEWIRE_RECEIVE_MODE = 1 // Receive mode: TX = floating (analog), RX = UART
} UART_OneWireMode_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartDrive_GpioConfigHardware(const UartDrive_HardwareSettings_t *hardware_settings);
void UartDrive_SetOneWireMode(UART_CommContext_t *ctx, UART_OneWireMode_t mode);
void UartDrive_SendData(UART_CommContext_t *ctx);
void UartDrive_RxIrqHandler(UART_CommContext_t *ctx, uint8_t data);
