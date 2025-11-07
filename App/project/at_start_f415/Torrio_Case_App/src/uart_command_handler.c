/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_command_handler.h"
#include "uart_command_queue.h"
#include "Commands.h"
#include "custom_hid_class.h"
#include "usb.h"
#include <string.h>
#include "battery.h"
#include "file_system.h"
#include "uart_driver.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))

/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
// define how command handlers look like
typedef Command_Status_t (*Uart_Command_Handler_t)(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
typedef Command_Status_t (*Uart_Command_Timeout_Handler_t)(UART_CommContext_t *ctx);

typedef struct
{
    uint8_t op;
    Uart_Command_Handler_t read;
    Uart_Command_Handler_t write;
    Uart_Command_Timeout_Handler_t timeout;
} cmd_handler_t;

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static Command_Status_t HandleNoop(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t TimeoutHandleNoop(UART_CommContext_t *ctx);
static Command_Status_t ReadVersion(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t FactoryDebugReadBuds(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t FactorySetBatteryChargeStatus(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t EraseFile(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t WriteFile(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t Crc32File(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t RecoveryAndReset(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t ReadBudsButtonAndMode(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t ReadBudsColorAndMode(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t ReadBudsSerialNumber(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);
static Command_Status_t ReadBudsBatteryStatus(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet);

static Command_Status_t EraseFileTimeoutHandle(UART_CommContext_t *ctx);
static Command_Status_t WriteFileTimeoutHandle(UART_CommContext_t *ctx);
static Command_Status_t Crc32FileTimeoutHandle(UART_CommContext_t *ctx);

/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[CMD_MAX_DATA_LEN] = {0};
// clang-format off
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP,                           .read = HandleNoop,                         .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        // info
        {.op = BUD_CMD_FW_VERSION,              .read = HandleNoop,                         .write = ReadVersion,                       .timeout = TimeoutHandleNoop},
        // factory
        {.op = FAC_READ_BUDS_DEBUG,             .read = FactoryDebugReadBuds,               .write = HandleNoop,                        .timeout = TimeoutHandleNoop},

        // mcu control
        {.op = RESET_OP,                        .read = HandleNoop,                         .write = RecoveryAndReset,                  .timeout = TimeoutHandleNoop},

        // file/firmware update
        {.op = ERASE_FILE_OP,                   .read = HandleNoop,                         .write = EraseFile,                         .timeout = EraseFileTimeoutHandle},
        {.op = FILE_ACCESS_OP,                  .read = HandleNoop,                         .write = WriteFile,                         .timeout = WriteFileTimeoutHandle},
        {.op = FILE_CRC32_OP,                   .read = Crc32File,                          .write = HandleNoop,                        .timeout = Crc32FileTimeoutHandle},

        // internal
        {.op = BUD_CMD_PREVENT_SLEEP,           .read = HandleNoop,                         .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_BUTTON_AND_MODE,         .read = ReadBudsButtonAndMode,              .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_DEEP_POWER_OFF,          .read = HandleNoop,                         .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_MODEL_AND_COLOR,         .read = ReadBudsColorAndMode,               .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_SERIAL_NUMBER,           .read = HandleNoop,                         .write = ReadBudsSerialNumber,              .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_BATTERY_STATE,           .read = ReadBudsBatteryStatus,              .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
        {.op = BUD_CMD_CHARGE_SETING,           .read = FactorySetBatteryChargeStatus,      .write = HandleNoop,                        .timeout = TimeoutHandleNoop},
    };
// clang-format on

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void UartCommandsHandle_CommandsHandle(UART_CommContext_t *ctx, UartProtocol_Packet_t rx_packet)
{
    Command_Status_t status = COMMAND_STATUS_ERROR_NO_HANDLER;

    memcpy(buffer, rx_packet.payload, rx_packet.payload_len);

    uint8_t command = ctx->command_id;
    uint8_t op = command & (~COMMAND_READ_FLAG);
    bool is_read = (command & COMMAND_READ_FLAG) == COMMAND_READ_FLAG;
    for (int i = 0; i < NUM_COMMANDS; ++i)
    {
        if (handler_table[i].op == op)
        {
            if (is_read)
            {
                status = handler_table[i].read(buffer, ctx, rx_packet);
                break;
            }
            else
            {
                status = handler_table[i].write(buffer, ctx, rx_packet);
                break;
            }
        }
    }
    if (status != COMMAND_STATUS_SUCCESS)
    {
        DEBUG_PRINT("UNKNOW COMMAND :%02X\n", command);
    }
}

void UartCommandsHandle_CommandsHandleTimeout(UART_CommContext_t *ctx)
{
    Command_Status_t status = COMMAND_STATUS_ERROR_NO_HANDLER;

    uint8_t command = ctx->command_id;
    uint8_t op = command & (~COMMAND_READ_FLAG);
    for (int i = 0; i < NUM_COMMANDS; ++i)
    {
        if (handler_table[i].op == op)
        {
            status = handler_table[i].timeout(ctx);
            break;
        }
    }

    if (status != COMMAND_STATUS_SUCCESS)
    {
        DEBUG_PRINT("UNKNOW COMMAND :%02X\n", command);
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static Command_Status_t HandleNoop(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t TimeoutHandleNoop(UART_CommContext_t *ctx)
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadVersion(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    DEBUG_PRINT("version data len :%d\n", packet.payload_len);
    UartInterface_Port_t target;
    if (ctx->side == UART_BUD_LEFT)
    {
        target = UART_INTERFACE_BUD_LEFT;
    }
    else if (ctx->side == UART_BUD_RIGHT)
    {
        target = UART_INTERFACE_BUD_RIGHT;
    }
    else
    {
        return COMMAND_STATUS_SUCCESS;
    }

    if (packet.payload_len == 9)
    {

        ctx->Version_Headset_Partion[0] = command[1] & 0x0F;
        ctx->Version_Headset_Partion[1] = command[0];
        ctx->Version_Headset_Partion[2] = command[1] & 0xF0;

        DEBUG_PRINT("MCU version %02X %02X %02X\n",
                    ctx->Version_Headset_Partion[0],
                    ctx->Version_Headset_Partion[1],
                    ctx->Version_Headset_Partion[2]);

        ctx->dsp2_version[0] = 0x00;
        ctx->dsp2_version[1] = command[5];
        ctx->dsp2_version[2] = command[6];

        DEBUG_PRINT("DSP2 version %02X %02X %02X\n",
                    ctx->dsp2_version[0],
                    ctx->dsp2_version[1],
                    ctx->dsp2_version[2]);

        ctx->Color_Spin = command[7];

        ctx->mode = (Uart_BudsWorkMode_t)command[4];
        if (ctx->mode == UART_BUDS_WORK_MODE_APP)
        {
            uint8_t payload[] = {BUD_CMD_FW_VERSION, 1};
            UartInterface_SendBudCommand(target, BUD_CMD_FW_VERSION, payload, sizeof(payload), 1000);
        }
    }
    else if (packet.payload_len == 6)
    {
        memcpy(ctx->anc_version_buffer, command, sizeof(ctx->anc_version_buffer));
    }

    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t FactoryDebugReadBuds(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    uint8_t buff[CMD_MAX_DATA_LEN] = {0x00};
    buff[0] = FAC_READ_BUDS_DEBUG | COMMAND_READ_FLAG;
    memcpy(&buff[1], ctx->rx_buffer, CMD_MAX_DATA_LEN);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t FactorySetBatteryChargeStatus(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t EraseFile(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    uint8_t buff[2] = {0x00};
    buff[0] = ERASE_FILE_OP;
    buff[1] = FLASH_OPERATION_SUCCESS;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t WriteFile(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    uint8_t buff[2] = {0x00};
    buff[0] = FILE_ACCESS_OP;
    buff[1] = FLASH_OPERATION_SUCCESS;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Crc32File(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    uint8_t txBuf[10] = {0x00};
    txBuf[0] = FILE_CRC32_OP | COMMAND_READ_FLAG;
    txBuf[1] = FLASH_OPERATION_SUCCESS;
    custom_hid_class_send_report(&otg_core_struct.dev, txBuf, sizeof(txBuf));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t RecoveryAndReset(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t EraseFileTimeoutHandle(UART_CommContext_t *ctx)
{
    uint8_t buff[2] = {0x00};
    buff[0] = ERASE_FILE_OP;
    buff[1] = FLASH_WRITE_ERRORS;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t WriteFileTimeoutHandle(UART_CommContext_t *ctx)
{
    uint8_t buff[2] = {0x00};
    buff[0] = FILE_ACCESS_OP;
    buff[1] = FLASH_WRITE_ERRORS;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Crc32FileTimeoutHandle(UART_CommContext_t *ctx)
{
    uint8_t txBuf[10] = {0x00};
    txBuf[0] = FILE_CRC32_OP | COMMAND_READ_FLAG;
    txBuf[1] = FLASH_WRITE_ERRORS;
    custom_hid_class_send_report(&otg_core_struct.dev, txBuf, sizeof(txBuf));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadBudsButtonAndMode(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    UartInterface_Port_t target;
    ctx->mode = (Uart_BudsWorkMode_t)command[0];
    ctx->button_io_state[0] = (Uart_BudsButtonIoState_t)command[1];
    ctx->button_io_state[1] = (Uart_BudsButtonIoState_t)command[2];
    ctx->button_io_state[2] = (Uart_BudsButtonIoState_t)command[3];

    if (ctx->side == UART_BUD_LEFT)
    {
        target = UART_INTERFACE_BUD_LEFT;
    }
    else if (ctx->side == UART_BUD_RIGHT)
    {
        target = UART_INTERFACE_BUD_RIGHT;
    }
    else
    {
        return COMMAND_STATUS_SUCCESS;
    }

    switch (ctx->mode)
    {
    case UART_BUDS_WORK_MODE_APP:
    {
        {
            uint8_t payload[] = {BUD_CMD_FW_VERSION, 0};
            UartInterface_SendBudCommand(target, BUD_CMD_FW_VERSION, payload, sizeof(payload), 1000);
        }
        {
            uint8_t payload[] = {BUD_CMD_MODEL_AND_COLOR | COMMAND_READ_FLAG};
            UartInterface_SendBudCommand(target, BUD_CMD_MODEL_AND_COLOR | COMMAND_READ_FLAG, payload, sizeof(payload), 100);
        }
        {
            uint8_t payload[] = {BUD_CMD_SERIAL_NUMBER, 1};
            UartInterface_SendBudCommand(target, BUD_CMD_SERIAL_NUMBER, payload, sizeof(payload), 1000);
        }
        {
            uint8_t payload[] = {BUD_CMD_SERIAL_NUMBER, 2};
            UartInterface_SendBudCommand(target, BUD_CMD_SERIAL_NUMBER, payload, sizeof(payload), 1000);
        }
        {
            uint8_t payload[] = {BUD_CMD_BUD_STATE | COMMAND_READ_FLAG};
            UartInterface_SendBudCommand(target, BUD_CMD_BUD_STATE | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
        }
        {
            uint8_t payload[] = {BUD_CMD_BATTERY_STATE | COMMAND_READ_FLAG};
            UartInterface_SendBudCommand(target, BUD_CMD_BATTERY_STATE | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
        }
        UartDrive_SendDeepPowerOffToPair(target);
        break;
    }

    case UART_BUDS_WORK_MODE_BOOTLOADER:
    {
        {
            uint8_t payload[] = {BUD_CMD_FW_VERSION, 0};
            UartInterface_SendBudCommand(target, BUD_CMD_FW_VERSION, payload, sizeof(payload), 1000);
        }
        break;
    }
    }

    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadBudsColorAndMode(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    ctx->mode_type = command[0];
    ctx->Color_Spin = command[1];
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadBudsSerialNumber(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    if (packet.payload_len == 11)
    {
        memcpy(ctx->serial_number_buffer, command, 10);
    }
    else if (packet.payload_len == 10)
    {
        memcpy(&ctx->serial_number_buffer[10], command, 9);
    }

    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadBudsBatteryStatus(const uint8_t command[CMD_MAX_DATA_LEN], UART_CommContext_t *ctx, UartProtocol_Packet_t packet)
{
    UartInterface_Port_t target;
    if (ctx->side == UART_BUD_LEFT)
    {
        target = UART_INTERFACE_BUD_LEFT;
        DEBUG_PRINT("[ReadBudsBatteryStatus] Side: LEFT\n");
    }
    else if (ctx->side == UART_BUD_RIGHT)
    {
        target = UART_INTERFACE_BUD_RIGHT;
        DEBUG_PRINT("[ReadBudsBatteryStatus] Side: RIGHT\n");
    }
    else
    {
        DEBUG_PRINT("[ReadBudsBatteryStatus] Side: UNKNOWN (%d)\n", ctx->side);
        return COMMAND_STATUS_SUCCESS;
    }

    ctx->ntc = ((uint16_t)command[0] << 8) | command[1];
    ctx->vbat = ((uint16_t)command[3] << 8) | command[4];
    ctx->battery_level = command[5];

    DEBUG_PRINT("NTC: %u (0x%04X)\n", ctx->ntc, ctx->ntc);
    DEBUG_PRINT("VBAT: %u (0x%04X)\n", ctx->vbat, ctx->vbat);
    DEBUG_PRINT("Battery Level: %u%%\n", ctx->battery_level);

    if (FileSystem_GetUserData()->presetChargeState == PRESET_CHARGE_ACTIVE)
    {
        if (ctx->vbat >= BUDS_PRESET_CHARGE_STOP_VOLTAGE)
        {
            if (target == UART_INTERFACE_BUD_LEFT)
            {
                Battery_GetPresetChargeState()->left_bud_charge_status = BATTERY_PRESET_CHARGE_DONE;
            }
            else if (target == UART_INTERFACE_BUD_RIGHT)
            {
                Battery_GetPresetChargeState()->right_bud_charge_status = BATTERY_PRESET_CHARGE_DONE;
            }
            uint8_t payload[] = {BUD_CMD_CHARGE_SETING | COMMAND_READ_FLAG, 0x00};
            UartInterface_SendBudCommand(target, BUD_CMD_CHARGE_SETING | COMMAND_READ_FLAG, payload, sizeof(payload), 1000);
            DEBUG_PRINT("Stop charge command sent to target %d.\n", target);
        }
    }

    return COMMAND_STATUS_SUCCESS;
}
