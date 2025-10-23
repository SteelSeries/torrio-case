/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_driver.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define BUDS_UART_BAUD_RATE 921600

/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
typedef struct
{
    usart_type *uart;
    gpio_type *tx_port;
    uint32_t tx_pin;
    gpio_type *rx_port;
    uint32_t rx_pin;
} UartHardwareConfig_t;
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static UartDrive_HardwareSettings_t user_hardware_settings = {0};
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void InitOneWriteSend(const UartHardwareConfig_t *config);
static void InitOneWriteReceive(const UartHardwareConfig_t *config);
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void UartDrive_GpioConfigHardware(const UartDrive_HardwareSettings_t *hardware_settings)
{
    memcpy(&user_hardware_settings, hardware_settings, sizeof(UartDrive_HardwareSettings_t));

    crm_periph_clock_enable(user_hardware_settings.left_bud_uart_tx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.left_bud_uart_rx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.right_bud_uart_tx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.right_bud_uart_rx_gpio_crm_clk, TRUE);

    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
}

void UartDrive_SetOneWireMode(UART_CommContext_t *ctx, UART_OneWireMode_t mode)
{
    UartHardwareConfig_t config;

    if (ctx->uart == user_hardware_settings.left_bud_uart)
    {
        config.uart = user_hardware_settings.left_bud_uart;
        config.tx_port = user_hardware_settings.left_bud_uart_tx_gpio_port;
        config.tx_pin = user_hardware_settings.left_bud_uart_tx_gpio_pin;
        config.rx_port = user_hardware_settings.left_bud_uart_rx_gpio_port;
        config.rx_pin = user_hardware_settings.left_bud_uart_rx_gpio_pin;
    }
    else if (ctx->uart == user_hardware_settings.right_bud_uart)
    {
        config.uart = user_hardware_settings.right_bud_uart;
        config.tx_port = user_hardware_settings.right_bud_uart_tx_gpio_port;
        config.tx_pin = user_hardware_settings.right_bud_uart_tx_gpio_pin;
        config.rx_port = user_hardware_settings.right_bud_uart_rx_gpio_port;
        config.rx_pin = user_hardware_settings.right_bud_uart_rx_gpio_pin;
    }
    else
    {
        return;
    }

    if (mode == UART_ONEWIRE_SEND_MODE)
    {
        InitOneWriteSend(&config);
    }
    else if (mode == UART_ONEWIRE_RECEIVE_MODE)
    {
        InitOneWriteReceive(&config);
    }
}

void UartDrive_SendData(UART_CommContext_t *ctx)
{
    usart_type *uart;

    if (ctx->uart == user_hardware_settings.left_bud_uart)
    {
        uart = user_hardware_settings.left_bud_uart;
    }
    else if (ctx->uart == user_hardware_settings.right_bud_uart)
    {
        uart = user_hardware_settings.right_bud_uart;
    }

    for (uint8_t i = 0; i < ctx->tx_len; i++)
    {
        while (usart_flag_get(uart, USART_TDBE_FLAG) == RESET)
            ;
        usart_data_transmit(uart, ctx->tx_buffer[i]);
        while (usart_flag_get(uart, USART_TDC_FLAG) == RESET)
            ;
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/

static void InitOneWriteSend(const UartHardwareConfig_t *config)
{
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);

    /* Configure TX pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = config->tx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(config->tx_port, &gpio_init_struct);

    /* Configure RX pin as analog (floating) */
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = config->rx_pin;
    gpio_init(config->rx_port, &gpio_init_struct);

    /* Configure UART */
    usart_init(config->uart, BUDS_UART_BAUD_RATE, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_parity_selection_config(config->uart, USART_PARITY_NONE);
    usart_transmitter_enable(config->uart, TRUE);
    usart_receiver_enable(config->uart, FALSE);
    usart_enable(config->uart, TRUE);
}

static void InitOneWriteReceive(const UartHardwareConfig_t *config)
{
    gpio_init_type gpio_init_struct;

    /* Initialize structure to default values */
    gpio_default_para_init(&gpio_init_struct);

    /* ---------- Configure TX pin as analog (float) ---------- */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = config->tx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(config->tx_port, &gpio_init_struct);

    /* ---------- Configure RX pin as UART (MUX mode) ---------- */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = config->rx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(config->rx_port, &gpio_init_struct);

    /* ---------- Configure UART ---------- */
    usart_init(config->uart, BUDS_UART_BAUD_RATE, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_parity_selection_config(config->uart, USART_PARITY_NONE);
    usart_transmitter_enable(config->uart, FALSE);
    usart_receiver_enable(config->uart, TRUE);
    usart_enable(config->uart, TRUE);
}