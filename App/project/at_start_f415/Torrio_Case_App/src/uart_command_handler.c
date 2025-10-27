/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_command_handler.h"
#include "uart_command_queue.h"
#include "Commands.h"
#include "custom_hid_class.h"
#include "usb.h"
#include <string.h>

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))

/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
// define how command handlers look like
typedef Command_Status_t (*Uart_Command_Handler_t)(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx);

typedef struct
{
    uint8_t op;
    Uart_Command_Handler_t read;
    Uart_Command_Handler_t write;
} cmd_handler_t;

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static Command_Status_t HandleNoop(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx);
static Command_Status_t ReadVersion(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx);
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[CMD_MAX_DATA_LEN] = {0};
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP, .read = HandleNoop, .write = HandleNoop},
        // info
        {.op = VERSION_OP, .read = ReadVersion, .write = HandleNoop},
};

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void UartCommandsHandle_CommandsHandle(UART_CommContext_t *ctx, UartProtocol_Packet_t rx_packet)
{
    Command_Status_t status = COMMAND_STATUS_ERROR_NO_HANDLER;

    memcpy(buffer, &rx_packet.payload[2], rx_packet.payload_len);

    uint8_t command = (buffer[0]);
    uint8_t op = command & (~COMMAND_READ_FLAG);
    bool is_read = (command & COMMAND_READ_FLAG) == COMMAND_READ_FLAG;
    for (int i = 0; i < NUM_COMMANDS; ++i)
    {
        if (handler_table[i].op == op)
        {
            if (is_read)
            {
                status = handler_table[i].read(buffer, ctx);
                break;
            }
            else
            {
                status = handler_table[i].write(buffer, ctx);
                break;
            }
        }
    }
    if (status != COMMAND_STATUS_SUCCESS)
    {
        DEBUG_PRINT("USB COMMAND ERROR CMD:%02X\n", command);
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static Command_Status_t HandleNoop(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx)
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadVersion(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx)
{
    uint8_t buff[13] = {0x00};
    buff[0] = VERSION_OP | COMMAND_READ_FLAG;

    DEBUG_PRINT("[CMD][%s][ReadVersion] Called.\n",
                (ctx->side == UART_BUD_LEFT) ? "LEFT" : (ctx->side == UART_BUD_RIGHT) ? "RIGHT"
                                                                                      : "UNKNOWN");

    DEBUG_PRINT("[CMD][%s][ReadVersion] Received command ID: 0x%02X\n",
                (ctx->side == UART_BUD_LEFT) ? "LEFT" : (ctx->side == UART_BUD_RIGHT) ? "RIGHT"
                                                                                      : "UNKNOWN",
                command[0]);

    DEBUG_PRINT("[CMD][%s][ReadVersion] Sending %d bytes via USB HID: ",
                (ctx->side == UART_BUD_LEFT) ? "LEFT" : (ctx->side == UART_BUD_RIGHT) ? "RIGHT"
                                                                                      : "UNKNOWN",
                (int)sizeof(command));
    for (uint8_t i = 0; i < sizeof(buff); i++)
    {
        DEBUG_PRINT("%02X ", command[i]);
    }
    DEBUG_PRINT("\n");

    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));

    DEBUG_PRINT("[CMD][%s][ReadVersion] Command sent successfully.\n",
                (ctx->side == UART_BUD_LEFT) ? "LEFT" : (ctx->side == UART_BUD_RIGHT) ? "RIGHT"
                                                                                      : "UNKNOWN");

    return COMMAND_STATUS_SUCCESS;
}
