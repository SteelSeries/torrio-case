/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "Commands.h"
#include "custom_hid_class.h"
#include "usb.h"
#include "version.h"
#include "task_scheduler.h"
#include "sy8809_xsense.h"
#include "app_fw_update.h"
#include "file_system.h"
#include "sy8809.h"
#include "system_state_manager.h"
#include "uart_interface.h"
#include <stdio.h>
#include <string.h>
#include "uart_comm_manager.h"
#include "lighting.h"
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))
#define WRITE_SERIAL_NUMBER_KEY 0xAA551133U

// ---------------------------------------------------------------------------
// Bud Debug Command payload length definitions
// ---------------------------------------------------------------------------
//
// The total UART buffer size available for transmission is 32 bytes,
// which is defined by CMD_MAX_DATA_LEN.
//
// The RTK communication protocol requires 7 bytes of overhead
// (header + length + checksum, etc.), leaving 25 bytes available
// for user payload data.
//
// Therefore:
//   BUD_DEBUG_PAYLOAD_MAX = CMD_MAX_DATA_LEN (32) - 7 = 25
//   BUD_DEBUG_PAYLOAD_MIN = 1 (must have at least 1 byte of data)
// ---------------------------------------------------------------------------
#define BUD_DEBUG_PAYLOAD_MAX (CMD_MAX_DATA_LEN - 7)
#define BUD_DEBUG_PAYLOAD_MIN 1
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
// define how command handlers look like
typedef Command_Status_t (*Command_Handler_t)(const uint8_t command[USB_RECEIVE_LEN]);

typedef struct
{
    uint8_t op;
    Command_Handler_t read;
    Command_Handler_t write;
} cmd_handler_t;
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static Command_Status_t HandleNoop(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t ReadVersion(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t RecoveryAndReset(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t DebugCommand(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t Sy8809DebugRegReadCommand(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t Sy8809DebugRegWriteCommand(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t Sy8809DebugXsenserReadCommand(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t EraseFile(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t WriteFile(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t Crc32File(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t GetSerialNumber(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t SetSerialNumber(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t ReadColorSpinAndMoldel(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t WriteColorSpinAndMoldel(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t FactoryReadBatteryAndNtc(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t FactorySetBatteryChargeStatus(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t GetBatteryStatus(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t HandleLedDebugCommand(const uint8_t command[USB_RECEIVE_LEN]);
static void HandleLightingDebugCommand(uint8_t command,uint8_t r,uint8_t g,uint8_t b);
static Command_Status_t FactoryDebugReadBuds(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t HandleFactoryEnterCommand(const uint8_t command[USB_RECEIVE_LEN]);

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[USB_RECEIVE_LEN] = {0};
// clang-format off
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP,                   .read = HandleNoop,                     .write = HandleNoop},
        // mcu control
        {.op = RESET_OP,                .read = HandleNoop,                     .write = RecoveryAndReset},

        // file/firmware update
        {.op = ERASE_FILE_OP,           .read = HandleNoop,                     .write = EraseFile},
        {.op = FILE_ACCESS_OP,          .read = HandleNoop,                     .write = WriteFile},
        {.op = FILE_CRC32_OP,           .read = Crc32File,                      .write = HandleNoop},

        // info
        {.op = VERSION_OP,              .read = ReadVersion,                    .write = HandleNoop},

        // debug
        {.op = DEBUG_CUSTOM_OP,         .read = HandleNoop,                     .write = DebugCommand},
        {.op = DEBUG_SY8809_OP,         .read = Sy8809DebugRegReadCommand,      .write = Sy8809DebugRegWriteCommand},
        {.op = DEBUG_SY8809_XSENSE_OP,  .read = Sy8809DebugXsenserReadCommand,  .write = HandleNoop},
        {.op = DEBUG_LEDRGB_OP,         .read = HandleNoop,                     .write = HandleLedDebugCommand},

        // factory
        {.op = FAC_SERIAL_OP, .read = GetSerialNumber, .write = SetSerialNumber},
        {.op = FAC_MODEL_COLOR_SPIN_OP, .read = ReadColorSpinAndMoldel, .write = WriteColorSpinAndMoldel},
        {.op = FAC_GET_BATTERY_AND_NTC, .read = FactoryReadBatteryAndNtc, .write = HandleNoop},
        {.op = FAC_SET_CHARGE_STATUS, .read = HandleNoop, .write = FactorySetBatteryChargeStatus},
        {.op = FAC_READ_BUDS_DEBUG, .read = FactoryDebugReadBuds, .write = HandleNoop},
        {.op = FAC_ENTER_MODE, .read = HandleFactoryEnterCommand, .write = HandleNoop},

        // Case/Buds
        {.op = GET_BATTERY_INFO, .read = GetBatteryStatus, .write = HandleNoop},
};
static Command_GetFactoryLighting_t fac_lighting_mode = COMMAND_FACTORY_NONE;
static Command_GetFactoryStatus_t fac_mode = COMMAND_FACTORY_NON_ENTER;

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Commands_HandleUsbCommand(const uint8_t *in, size_t in_len)
{
    Command_Status_t status = COMMAND_STATUS_ERROR_NO_HANDLER;

    memcpy(buffer, in, in_len);

    uint8_t command = (buffer[0]);
    uint8_t op = command & (~COMMAND_READ_FLAG);
    bool is_read = (command & COMMAND_READ_FLAG) == COMMAND_READ_FLAG;
    for (int i = 0; i < NUM_COMMANDS; ++i)
    {
        if (handler_table[i].op == op)
        {
            if (is_read)
            {
                status = handler_table[i].read(buffer);
                break;
            }
            else
            {
                status = handler_table[i].write(buffer);
                break;
            }
        }
    }
    if (status != COMMAND_STATUS_SUCCESS)
    {
        uint8_t buff[USBD_CUSTOM_OUT_MAXPACKET_SIZE] = {0};
        memset(buff, 0, sizeof(buff));
        snprintf((char *)buff, sizeof(buff), "USB COMMAND ERROR CMD:%02X\n", command);
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
}

Command_GetFactoryLighting_t Commands_HandleLightingMode(void)
{
  return fac_lighting_mode;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/

static Command_Status_t HandleNoop(const uint8_t command[USB_RECEIVE_LEN])
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadVersion(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t uab_send_buff[61] = {0x00};
    uint8_t version_format_buff[12] = {0};
    UART_CommContext_t *ctx = NULL;

    uab_send_buff[0] = VERSION_OP | COMMAND_READ_FLAG;
    // Case version 1~12
    Version_GetArteryVersion(version_format_buff, sizeof(version_format_buff));
    memcpy(&uab_send_buff[1], version_format_buff, 8);
    memset(version_format_buff, 0, sizeof(version_format_buff));

    // Left bud version 13~24
    ctx = UartCommManager_GetLeftBudContext();
    Version_GetStrVersion(ctx->Version_Headset_Partion, version_format_buff, sizeof(version_format_buff));
    memcpy(&uab_send_buff[13], version_format_buff, 8);
    memset(version_format_buff, 0, sizeof(version_format_buff));

    // Left bud DSP2 version 37~48
    Version_GetStrVersion(ctx->dsp2_version, version_format_buff, sizeof(version_format_buff));
    memcpy(&uab_send_buff[37], version_format_buff, 8);
    memset(version_format_buff, 0, sizeof(version_format_buff));

    // Right bud version 25~36
    ctx = UartCommManager_GetRightBudContext();
    Version_GetStrVersion(ctx->Version_Headset_Partion, version_format_buff, sizeof(version_format_buff));
    memcpy(&uab_send_buff[25], version_format_buff, 8);
    memset(version_format_buff, 0, sizeof(version_format_buff));

    // Right bud DSP2 version 49~60
    Version_GetStrVersion(ctx->dsp2_version, version_format_buff, sizeof(version_format_buff));
    memcpy(&uab_send_buff[49], version_format_buff, 8);
    memset(version_format_buff, 0, sizeof(version_format_buff));

    custom_hid_class_send_report(&otg_core_struct.dev, uab_send_buff, sizeof(uab_send_buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t RecoveryAndReset(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < RECOVERY_MODE_NUM_MODES)
    {
        if (command[1] == RECOVERY_MODE_BOOTLOADER)
        {
            if (command[2] == COMMAND_TARGET_CASE)
            {
                AppFwUpdata_SetResetFlag(true);
                AppFwUpdata_SetCurrentMode(BOOTLOADER_MODE);
            }
            else if (command[2] == COMMAND_TARGET_LEFT_BUD)
            {
                uint8_t payload[] = {RESET_OP, command[1]};
                UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, RESET_OP, payload, sizeof(payload), 10000);
            }
            else if (command[2] == COMMAND_TARGET_RIGHT_BUD)
            {
                uint8_t payload[] = {RESET_OP, command[1]};
                UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, RESET_OP, payload, sizeof(payload), 10000);
            }
        }
        else if (command[1] == RECOVERY_MODE_APPLICATION)
        {
            if (command[2] == COMMAND_TARGET_CASE)
            {
                AppFwUpdata_SetResetFlag(true);
                AppFwUpdata_SetCurrentMode(NORMAL_MODE);
            }
            else if (command[2] == COMMAND_TARGET_LEFT_BUD)
            {
                uint8_t payload[] = {RESET_OP, command[1]};
                UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, RESET_OP, payload, sizeof(payload), 10000);
            }
            else if (command[2] == COMMAND_TARGET_RIGHT_BUD)
            {
                uint8_t payload[] = {RESET_OP, command[1]};
                UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, RESET_OP, payload, sizeof(payload), 10000);
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t DebugCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    switch (command[1])
    {
    case 0x01:
    {
        HandleLightingDebugCommand(command[2], command[3], command[4], command[5]);
        break;
    }

    case 0x02:
    {
        break;
    }

    default:
        break;
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Sy8809DebugRegReadCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t reg_ret_buff[1] = {0};
    Sy8809_DebugRegRead(command[1], reg_ret_buff);
    uint8_t buff[] = {DEBUG_SY8809_OP | COMMAND_READ_FLAG, reg_ret_buff[0]};
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Sy8809DebugRegWriteCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t buff[] = {DEBUG_SY8809_OP, Sy8809_DebugRegWrite(command[1], command[2])};
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Sy8809DebugXsenserReadCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    if ((command[1] >= SY8809_XSENSE_NTC) && (command[1] <= SY8809_XSENSE_VBIN))
    {
        Sy8809Xsense_XsenseRead_t Pending_temp = {0};
        Pending_temp.is_command_read = true;
        Pending_temp.Pending = (Sy8809Xsense_OutputItem_t)command[1];
        Sy8809Xsense_SetPendingXsense(Pending_temp);
        if (TaskScheduler_AddTask(Sy8809Xsense_TrigXsenseConv, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
        {
            DEBUG_PRINT("add sy8809 trig xsense conv task fail\n");
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t EraseFile(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < COMMAND_TARGET_NUM_MODES)
    {
        if (command[1] == COMMAND_TARGET_CASE)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_CmdEraseHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update erase task fail\n");
            }
        }
        else if (command[1] == COMMAND_TARGET_LEFT_BUD)
        {
            DEBUG_PRINT("set left bud erase\n");
            uint8_t payload[] = {ERASE_FILE_OP, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, ERASE_FILE_OP, payload, sizeof(payload), 3000);
        }
        else if (command[1] == COMMAND_TARGET_RIGHT_BUD)
        {
            DEBUG_PRINT("set right bud erase\n");
            uint8_t payload[] = {ERASE_FILE_OP, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, ERASE_FILE_OP, payload, sizeof(payload), 3000);
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t WriteFile(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < COMMAND_TARGET_NUM_MODES)
    {
        AppFwUpdata_UsbReceiveData((uint8_t *)command, USB_RECEIVE_LEN);
        if (command[1] == COMMAND_TARGET_CASE)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_CmdWriteFlashHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update write task fail\n");
            }
        }
        else if (command[1] == COMMAND_TARGET_LEFT_BUD)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_LeftBudWriteFlashTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update write task fail\n");
            }
        }
        else if (command[1] == COMMAND_TARGET_RIGHT_BUD)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_RightBudWriteFlashTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update write task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Crc32File(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < COMMAND_TARGET_NUM_MODES)
    {
        if (command[1] == COMMAND_TARGET_CASE)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_CmdCrcCheckHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add CRC check task fail\n");
            }
        }
        else if (command[1] == COMMAND_TARGET_LEFT_BUD)
        {
            uint8_t payload[] = {FILE_CRC32_OP | COMMAND_READ_FLAG, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, FILE_CRC32_OP | COMMAND_READ_FLAG, payload, sizeof(payload), 3000);
        }
        else if (command[1] == COMMAND_TARGET_RIGHT_BUD)
        {
            uint8_t payload[] = {FILE_CRC32_OP | COMMAND_READ_FLAG, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, FILE_CRC32_OP | COMMAND_READ_FLAG, payload, sizeof(payload), 3000);
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t GetSerialNumber(const uint8_t command[USB_RECEIVE_LEN])
{
    FileSystem_UserData_t *data = (FileSystem_UserData_t *)FileSystem_GetUserData();
    uint8_t buff[59] = {0x00};
    buff[0] = FAC_SERIAL_OP | COMMAND_READ_FLAG;
    memcpy(&buff[1], data->serial_number, sizeof(data->serial_number));

    UART_CommContext_t *ctx = NULL;
    ctx = UartCommManager_GetLeftBudContext();
    memcpy(&buff[21], ctx->serial_number_buffer, sizeof(ctx->serial_number_buffer));

    ctx = UartCommManager_GetRightBudContext();
    memcpy(&buff[40], ctx->serial_number_buffer, sizeof(ctx->serial_number_buffer));

    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t SetSerialNumber(const uint8_t command[USB_RECEIVE_LEN])
{
    uint32_t key = ((uint32_t)command[2] << 24) |
                   ((uint32_t)command[3] << 16) |
                   ((uint32_t)command[4] << 8) |
                   ((uint32_t)command[5]);
    bool is_ret = false;

    if (key != WRITE_SERIAL_NUMBER_KEY)
    {
        is_ret = true;
    }
    else
    {
        Command_Target_t target = (Command_Target_t)command[1];
        if (target > COMMAND_TARGET_RIGHT_BUD)
        {
            is_ret = true;
        }
        else
        {

            switch (target)
            {
            case COMMAND_TARGET_CASE:
            {
                uint8_t new_serial[18] = {0};
                memcpy(new_serial, &command[6], sizeof(new_serial));
                FileSystem_UpdateSerialNumber(new_serial);
                break;
            }

            case COMMAND_TARGET_LEFT_BUD:
            {
                // TODO: UART communication to lift bud write serial number.
                break;
            }

            case COMMAND_TARGET_RIGHT_BUD:
            {
                // TODO: UART communication to lift bud write serial number.
                break;
            }
            }
        }
    }

    if (is_ret == true)
    {
        uint8_t buff[2] = {0x00};
        buff[0] = FAC_SERIAL_OP;
        buff[1] = FLASH_WRITE_ERRORS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadColorSpinAndMoldel(const uint8_t command[USB_RECEIVE_LEN])
{
    Command_Target_t target = (Command_Target_t)command[1];
    FileSystem_UserData_t *data = (FileSystem_UserData_t *)FileSystem_GetUserData();

    uint8_t buff[4] = {0x00};
    buff[0] = FAC_MODEL_COLOR_SPIN_OP | COMMAND_READ_FLAG;
    buff[1] = (uint8_t)target;

    if (target > COMMAND_TARGET_RIGHT_BUD)
    {
        buff[2] = FLASH_WRITE_ERRORS;
        buff[3] = FLASH_WRITE_ERRORS;
    }
    else
    {
        switch (target)
        {
        case COMMAND_TARGET_CASE:
        {
            buff[2] = data->model;
            buff[3] = data->color;
            break;
        }

        case COMMAND_TARGET_LEFT_BUD:
        {
            UART_CommContext_t *ctx = NULL;
            ctx = UartCommManager_GetLeftBudContext();
            buff[2] = ctx->mode_type;
            buff[3] = ctx->Color_Spin;
            break;
        }

        case COMMAND_TARGET_RIGHT_BUD:
        {
            UART_CommContext_t *ctx = NULL;
            ctx = UartCommManager_GetRightBudContext();
            buff[2] = ctx->mode_type;
            buff[3] = ctx->Color_Spin;
            break;
        }
        }
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t WriteColorSpinAndMoldel(const uint8_t command[USB_RECEIVE_LEN])
{
    Command_Target_t target = (Command_Target_t)command[1];
    if (target > COMMAND_TARGET_RIGHT_BUD)
    {
        uint8_t buff[2] = {0x00};
        buff[0] = FAC_MODEL_COLOR_SPIN_OP;
        buff[1] = FLASH_WRITE_ERRORS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
    else
    {

        switch (target)
        {
        case COMMAND_TARGET_CASE:
        {
            FileSystem_UpdateColorSpinAndModel(command[2], command[3]);
            break;
        }

        case COMMAND_TARGET_LEFT_BUD:
        {
            // TODO: UART communication to lift bud write color spin and model.
            break;
        }

        case COMMAND_TARGET_RIGHT_BUD:
        {
            // TODO: UART communication to lift bud write color spin and model.
            break;
        }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t FactoryReadBatteryAndNtc(const uint8_t command[USB_RECEIVE_LEN])
{
    if (TaskScheduler_AddTask(SystemStateManager_ReadBatteryAndNtcHandle, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        DEBUG_PRINT("add read battery and NTC task fail\n");
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t FactorySetBatteryChargeStatus(const uint8_t command[USB_RECEIVE_LEN])
{
    Command_Target_t target = (Command_Target_t)command[1];
    if (target > COMMAND_TARGET_RIGHT_BUD)
    {
        uint8_t buff[2] = {0x00};
        buff[0] = FAC_SET_CHARGE_STATUS;
        buff[1] = FLASH_WRITE_ERRORS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
    else
    {

        switch (target)
        {
        case COMMAND_TARGET_CASE:
        {
            Sy8809_ChargeStatusSet((Sy8809_ChargeControl_t)command[2]);
            break;
        }

        case COMMAND_TARGET_LEFT_BUD:
        {
            uint8_t payload[] = {FAC_SET_CHARGE_STATUS, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_LEFT, FAC_SET_CHARGE_STATUS, payload, sizeof(payload), 10000);
            break;
        }

        case COMMAND_TARGET_RIGHT_BUD:
        {

            uint8_t payload[] = {FAC_SET_CHARGE_STATUS, command[2]};
            UartInterface_SendBudCommand(UART_INTERFACE_BUD_RIGHT, FAC_SET_CHARGE_STATUS, payload, sizeof(payload), 10000);
            break;
        }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t FactoryDebugReadBuds(const uint8_t command[USB_RECEIVE_LEN])
{
    Command_Target_t target = (Command_Target_t)command[1];
    if ((target != COMMAND_TARGET_RIGHT_BUD) && (target != COMMAND_TARGET_LEFT_BUD))
    {
        uint8_t buff[2] = {0x00};
        buff[0] = FAC_READ_BUDS_DEBUG | COMMAND_READ_FLAG;
        buff[1] = FLASH_WRITE_ERRORS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
        return COMMAND_STATUS_SUCCESS;
    }

    if ((command[2] < BUD_DEBUG_PAYLOAD_MIN) || (command[2] > BUD_DEBUG_PAYLOAD_MAX))
    {
        uint8_t buff[2] = {0};
        buff[0] = FAC_READ_BUDS_DEBUG | COMMAND_READ_FLAG;
        buff[1] = FLASH_WRITE_ERRORS;
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
        return COMMAND_STATUS_SUCCESS;
    }

    UartCommandQueue_Command_t cmd;
    uint8_t length = command[2];
    memcpy(cmd.data, &command[3], length);
    cmd.length = length;
    cmd.command_id = FAC_READ_BUDS_DEBUG | COMMAND_READ_FLAG;
    cmd.timeout_ms = 10000;

    switch (target)
    {
    case COMMAND_TARGET_LEFT_BUD:
    {
        UartInterface_SendQueue(UART_INTERFACE_BUD_LEFT, &cmd);
        break;
    }

    case COMMAND_TARGET_RIGHT_BUD:
    {
        UartInterface_SendQueue(UART_INTERFACE_BUD_RIGHT, &cmd);
        break;
    }
    }

    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t GetBatteryStatus(const uint8_t command[USB_RECEIVE_LEN])
{
    if (TaskScheduler_AddTask(SystemStateManager_GetBatteryStatusHandle, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        DEBUG_PRINT("add read battery status task fail\n");
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t HandleLedDebugCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t buff[1] = {0};
    switch (command[1])
    {
    case COMMAND_TARGET_CASE:
    {
        if(fac_mode == COMMAND_FACTORY_MODE)
        {
            fac_lighting_mode = COMMAND_FACTORY_LED_ON_OFF;
            Lighting_Handler(LIGHTING_STABLE,command[2], command[3], command[4]);
        }
        break;
    }
    case COMMAND_TARGET_LEFT_BUD:
    {
        break;
    }
    case COMMAND_TARGET_RIGHT_BUD:
    {
        break;
    }
    default:
        break;
    }
    buff[0] = DEBUG_LEDRGB_OP;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static void HandleLightingDebugCommand(uint8_t command, uint8_t r, uint8_t g, uint8_t b)
{
    Lighting_Change_Flag = LIGHTING_CHANGE_TRUE;
    Lighting_Handler(command, r, g, b);
}

static Command_Status_t HandleFactoryEnterCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    uint32_t fac_key = ((uint32_t)command[1] << 16) |
                       ((uint32_t)command[2] << 8)  |
                       ((uint32_t)command[3]);

    if (fac_key == FAC_ENTER_KEY)
    {
        switch (command[4])
        {
        case COMMAND_TARGET_CASE:
        {
            fac_lighting_mode = COMMAND_FACTORY_MODE_LIGHTING;
            fac_mode = COMMAND_FACTORY_MODE;
            break;
        }
        case COMMAND_TARGET_LEFT_BUD:
        {
            break;                   
        }
        case COMMAND_TARGET_RIGHT_BUD:
        {
            break;                   
        }
        default:
            break;
        }
    }
    return COMMAND_STATUS_SUCCESS;
}
