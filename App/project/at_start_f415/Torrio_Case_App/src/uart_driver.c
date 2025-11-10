/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_driver.h"
#include "uart_protocol.h"
#include "task_scheduler.h"
#include "usb.h"
#include "uart_command_handler.h"
#include "Commands.h"
#include "lid.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define BUDS_UART_BAUD_RATE 921600

// Each task runs every 10 ms
// To achieve 50 ms debounce time:
// 50 ms / 10 ms = 5 task cycles
#define BUD_INCASE_DEBOUNCE_TIMER 5

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
static UART_CommContext_t *user_left_bud_ctx = NULL;
static UART_CommContext_t *user_right_bud_ctx = NULL;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void InitOneWriteSend(const UartHardwareConfig_t *config);
static void InitOneWriteReceive(const UartHardwareConfig_t *config);
static void CheckBudConnection(UART_CommContext_t *ctx);
static void SendInitCommand(UartInterface_Port_t target);
static void SendDeepPowerOffToPair(UartInterface_Port_t target);
static void SendLidStateToPair(UartInterface_Port_t target);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void UartDrive_BudsCtxInit(void)
{
    user_left_bud_ctx = UartCommManager_GetLeftBudContext();
    user_right_bud_ctx = UartCommManager_GetRightBudContext();
}

void UartDrive_GpioConfigHardware(const UartDrive_HardwareSettings_t *hardware_settings)
{
    memcpy(&user_hardware_settings, hardware_settings, sizeof(UartDrive_HardwareSettings_t));

    crm_periph_clock_enable(user_hardware_settings.left_bud_uart_tx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.left_bud_uart_rx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.right_bud_uart_tx_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.right_bud_uart_rx_gpio_crm_clk, TRUE);

    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);

    nvic_irq_enable(USART2_IRQn, 0, 0);
    nvic_irq_enable(USART3_IRQn, 0, 0);

    gpio_init_type gpio_init_struct;

    /* Initialize structure to default values */
    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;

    /* ---------- Configure left TX pin as analog (Input mode) ---------- */
    gpio_init_struct.gpio_pins = user_hardware_settings.left_bud_uart_tx_gpio_pin;
    gpio_init(user_hardware_settings.left_bud_uart_tx_gpio_port, &gpio_init_struct);

    /* ---------- Configure left RX pin as UART (Input mode) ---------- */
    gpio_init_struct.gpio_pins = user_hardware_settings.left_bud_uart_rx_gpio_pin;
    gpio_init(user_hardware_settings.left_bud_uart_rx_gpio_port, &gpio_init_struct);

    /* ---------- Configure right TX pin as analog (Input mode) ---------- */
    gpio_init_struct.gpio_pins = user_hardware_settings.right_bud_uart_tx_gpio_pin;
    gpio_init(user_hardware_settings.right_bud_uart_tx_gpio_port, &gpio_init_struct);

    /* ---------- Configure right RX pin as UART (Input mode) ---------- */
    gpio_init_struct.gpio_pins = user_hardware_settings.right_bud_uart_rx_gpio_pin;
    gpio_init(user_hardware_settings.right_bud_uart_rx_gpio_port, &gpio_init_struct);
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
    if (ctx == NULL || ctx->uart == NULL)
    {
        return;
    }

    usart_type *uart = ctx->uart;
    const uint8_t *send_buf;
    uint16_t send_len;

    if (ctx->direct_mode)
    {
        send_buf = ctx->direct_data;
        send_len = ctx->direct_len;
        DEBUG_PRINT("[UART][SEND] Direct send %d bytes.\n", send_len);
    }
    else
    {
        send_buf = ctx->tx_buffer;
        send_len = ctx->tx_len;
        DEBUG_PRINT("[UART][SEND] Queue send %d bytes.\n", send_len);
    }

    for (uint16_t i = 0; i < send_len; i++)
    {
        while (usart_flag_get(uart, USART_TDBE_FLAG) == RESET)
            ;
        usart_data_transmit(uart, send_buf[i]);
        while (usart_flag_get(uart, USART_TDC_FLAG) == RESET)
            ;
    }

    DEBUG_PRINT("[UART][SEND] Completed %d bytes.\n", send_len);
}

void UartDrive_RxIrqHandler(UART_CommContext_t *ctx, uint8_t data)
{
    if (!ctx->sync_detected)
    {
        if (data == CMD_SYNC_BYTE)
        {
            ctx->rx_index = 0;
            ctx->rx_buffer[ctx->rx_index++] = data;
            ctx->sync_detected = true;
            ctx->expected_len = 0;
        }
        return;
    }

    ctx->rx_buffer[ctx->rx_index++] = data;

    if (ctx->rx_index == 4)
    {
        uint16_t len_field = (uint16_t)ctx->rx_buffer[2] | ((uint16_t)ctx->rx_buffer[3] << 8);
        ctx->expected_len = len_field + 5; // total = LEN + SYNC(1)+SEQ(1)+CHK(1)+LEN(2)
    }

    if (ctx->expected_len > 0 && ctx->rx_index >= ctx->expected_len)
    {
        ctx->packet_ready = true;
        ctx->sync_detected = false;
    }

    if (ctx->rx_index >= sizeof(ctx->rx_buffer))
    {
        ctx->rx_index = 0;
        ctx->sync_detected = false;
    }
}

void UartDrive_BudsConnectCheckTask(void)
{
    CheckBudConnection(user_left_bud_ctx);
    CheckBudConnection(user_right_bud_ctx);

    if (TaskScheduler_AddTask(UartDrive_BudsConnectCheckTask, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add buds connect check task fail\n");
    }
}

void UartDrive_SendDeepPowerOffToPair(UartInterface_Port_t target)
{
    SendDeepPowerOffToPair(target);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/

static void InitOneWriteSend(const UartHardwareConfig_t *config)
{
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    usart_enable(config->uart, FALSE);
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
    usart_interrupt_enable(config->uart, USART_RDBF_INT, FALSE);
    usart_enable(config->uart, TRUE);
}

static void InitOneWriteReceive(const UartHardwareConfig_t *config)
{
    gpio_init_type gpio_init_struct;

    /* Initialize structure to default values */
    gpio_default_para_init(&gpio_init_struct);
    usart_enable(config->uart, FALSE);

    /* ---------- Configure TX pin as analog (Input mode) ---------- */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = config->tx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(config->tx_port, &gpio_init_struct);

    /* ---------- Configure RX pin as UART (Input mode) ---------- */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = config->rx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(config->rx_port, &gpio_init_struct);

    /* ---------- Configure UART ---------- */
    usart_init(config->uart, BUDS_UART_BAUD_RATE, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_parity_selection_config(config->uart, USART_PARITY_NONE);
    usart_transmitter_enable(config->uart, FALSE);
    usart_receiver_enable(config->uart, TRUE);
    usart_interrupt_enable(config->uart, USART_RDBF_INT, TRUE);
    usart_enable(config->uart, TRUE);
}

static void CheckBudConnection(UART_CommContext_t *ctx)
{
    gpio_type *port;
    uint32_t pin;
    UartInterface_Port_t side;

    if (ctx->uart == user_hardware_settings.left_bud_uart)
    {
        port = user_hardware_settings.left_bud_uart_tx_gpio_port;
        pin = user_hardware_settings.left_bud_uart_tx_gpio_pin;
        side = UART_INTERFACE_BUD_LEFT;
    }
    else if (ctx->uart == user_hardware_settings.right_bud_uart)
    {
        port = user_hardware_settings.right_bud_uart_tx_gpio_port;
        pin = user_hardware_settings.right_bud_uart_tx_gpio_pin;
        side = UART_INTERFACE_BUD_RIGHT;
    }
    else
    {
        return;
    }

    Uart_BudsIoState_t pin_state = (Uart_BudsIoState_t)gpio_input_data_bit_read(port, pin);

    // Handle first-time check after power-on:
    // If state is UNKNOWN, read the current GPIO level directly
    // and set the initial connection state without waiting for debounce.
    if (ctx->detect_state == UART_BUDS_IO_UNKNOW)
    {
        ctx->detect_state = pin_state;
        ctx->detect_state_pre = pin_state;

        if (pin_state == UART_BUDS_IO_DISCONNECT)
        {
            ctx->Connect = UART_BUDS_CONNT_DISCONNECT;
            DEBUG_PRINT("%s bud initial state: disconnected\n",
                        (ctx->uart == user_hardware_settings.left_bud_uart) ? "left" : "right");
        }
        else
        {
            ctx->Connect = UART_BUDS_CONNT_CONNECT;
            SendInitCommand(side);
            DEBUG_PRINT("%s bud initial state: connected\n",
                        (ctx->uart == user_hardware_settings.left_bud_uart) ? "left" : "right");
        }

        // no need to debounce at first detection
        return;
    }

    if (ctx->detect_state != pin_state)
    {
        ctx->detect_state = pin_state;
        ctx->detect_debounce = BUD_INCASE_DEBOUNCE_TIMER;
    }

    if (ctx->detect_debounce > 0)
    {
        ctx->detect_debounce--;
        if (ctx->detect_debounce == 0)
        {
            if (ctx->detect_state != ctx->detect_state_pre)
            {

                if (ctx->uart == user_hardware_settings.left_bud_uart)
                {
                    DEBUG_PRINT("left bud ");
                }
                else if (ctx->uart == user_hardware_settings.right_bud_uart)
                {
                    DEBUG_PRINT("right bud ");
                }

                if (ctx->detect_state == UART_BUDS_IO_DISCONNECT)
                {
                    ctx->Connect = UART_BUDS_CONNT_DISCONNECT;
                    UartCommManager_DisconnectReinit(ctx);
                    SendLidStateToPair(side);
                    DEBUG_PRINT("disconnected\n");
                }
                else
                {
                    ctx->Connect = UART_BUDS_CONNT_CONNECT;
                    SendInitCommand(side);
                    DEBUG_PRINT("connected\n");
                }

                ctx->detect_state_pre = ctx->detect_state;
            }
        }
    }
}

static void SendInitCommand(UartInterface_Port_t target)
{
    if (Usb_GetUsbDetectState() == USB_PLUG)
    {
        {
            uint8_t payload[] = {BUD_CMD_PREVENT_SLEEP | COMMAND_READ_FLAG};
            UartInterface_SendBudCommand(target, BUD_CMD_PREVENT_SLEEP | COMMAND_READ_FLAG, payload, sizeof(payload), 100);
        }
        {
            uint8_t payload[] = {BUD_CMD_BUTTON_AND_MODE | COMMAND_READ_FLAG};
            UartInterface_SendBudCommand(target, BUD_CMD_BUTTON_AND_MODE | COMMAND_READ_FLAG, payload, sizeof(payload), 100);
        }
    }
    else
    {
        SendDeepPowerOffToPair(target);
    }
}

static void SendDeepPowerOffToPair(UartInterface_Port_t target)
{
    uint8_t payload[] = {BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG};
    UartInterface_SendBudCommand(target, BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);

    if (target == UART_INTERFACE_BUD_LEFT &&
        user_right_bud_ctx->Connect == UART_BUDS_CONNT_CONNECT)
    {
        uint8_t payload[] = {BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG};
        UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
    }
    else if (target == UART_INTERFACE_BUD_RIGHT &&
             user_left_bud_ctx->Connect == UART_BUDS_CONNT_CONNECT)
    {
        uint8_t payload[] = {BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG};
        UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, BUD_CMD_DEEP_POWER_OFF | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
    }
}

static void SendLidStateToPair(UartInterface_Port_t target)
{
    if (target == UART_INTERFACE_BUD_LEFT &&
        user_right_bud_ctx->Connect == UART_BUDS_CONNT_CONNECT)
    {
        uint8_t payload[] = {BUD_CMD_SYNC_CASE_LID_STATE | COMMAND_READ_FLAG, (uint8_t)Lid_GetState()};
        UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, BUD_CMD_SYNC_CASE_LID_STATE | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
    }
    else if (target == UART_INTERFACE_BUD_RIGHT &&
             user_left_bud_ctx->Connect == UART_BUDS_CONNT_CONNECT)
    {
        uint8_t payload[] = {BUD_CMD_SYNC_CASE_LID_STATE | COMMAND_READ_FLAG, (uint8_t)Lid_GetState()};
        UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, BUD_CMD_SYNC_CASE_LID_STATE | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
    }
}
