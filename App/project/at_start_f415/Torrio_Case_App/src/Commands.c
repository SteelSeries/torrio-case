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
#include <stdio.h>
#include <string.h>
#include "uart_command_queue.h"
#include "uart_comm_manager.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))
#define WRITE_SERIAL_NUMBER_KEY 0xAA551133U
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

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[USB_RECEIVE_LEN] = {0};
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP, .read = HandleNoop, .write = HandleNoop},
        // mcu control
        {.op = RESET_OP, .read = HandleNoop, .write = RecoveryAndReset},

        // file/firmware update
        {.op = ERASE_FILE_OP, .read = HandleNoop, .write = EraseFile},
        {.op = FILE_ACCESS_OP, .read = HandleNoop, .write = WriteFile},
        {.op = FILE_CRC32_OP, .read = Crc32File, .write = HandleNoop},

        // info
        {.op = VERSION_OP, .read = ReadVersion, .write = HandleNoop},

        // debug
        {.op = DEBUG_CUSTOM_OP, .read = HandleNoop, .write = DebugCommand},
        {.op = DEBUG_SY8809_OP, .read = Sy8809DebugRegReadCommand, .write = Sy8809DebugRegWriteCommand},
        {.op = DEBUG_SY8809_XSENSE_OP, .read = Sy8809DebugXsenserReadCommand, .write = HandleNoop},

        // factory
        {.op = FAC_SERIAL_OP, .read = GetSerialNumber, .write = SetSerialNumber},
        {.op = FAC_MODEL_COLOR_SPIN_OP, .read = ReadColorSpinAndMoldel, .write = WriteColorSpinAndMoldel},
        {.op = FAC_GET_BATTERY_AND_NTC, .read = FactoryReadBatteryAndNtc, .write = HandleNoop},
        {.op = FAC_SET_CHARGE_STATUS, .read = HandleNoop, .write = FactorySetBatteryChargeStatus},

        // Case/Buds
        {.op = GET_BATTERY_INFO, .read = GetBatteryStatus, .write = HandleNoop},
};

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

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/

static Command_Status_t HandleNoop(const uint8_t command[USB_RECEIVE_LEN])
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t ReadVersion(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t buff[13] = {0x00};
    uint8_t temp_buff[12] = {0};
    buff[0] = VERSION_OP | COMMAND_READ_FLAG;
    Version_GetArteryVersion(temp_buff, sizeof(temp_buff));
    memcpy(&buff[1], temp_buff, 8);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t RecoveryAndReset(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < RECOVERY_MODE_NUM_MODES)
    {
        if (command[1] == RECOVERY_MODE_BOOTLOADER)
        {
            if (command[2] == FILE_ID_LOCAL)
            {
                AppFwUpdata_SetResetFlag(true);
                AppFwUpdata_SetCurrentMode(BOOTLOADER_MODE);
            }
            else if (command[2] == FILE_ID_PERIPHERAL)
            {
                // Todo: switch to buds or dongle
            }
        }
        else if (command[1] == RECOVERY_MODE_APPLICATION)
        {
            if (command[2] == FILE_ID_LOCAL)
            {
                AppFwUpdata_SetResetFlag(true);
                AppFwUpdata_SetCurrentMode(NORMAL_MODE);
            }
            else if (command[2] == FILE_ID_PERIPHERAL)
            {
                // Todo: switch to buds or dongle
            }
        }
    }
    else
    {
        AppFwUpdata_SetResetFlag(true);
        AppFwUpdata_SetCurrentMode(NORMAL_MODE);
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t DebugCommand(const uint8_t command[USB_RECEIVE_LEN])
{
    switch (command[1])
    {
    case 0x01:
    {
        break;
    }

    case 0x02:
    {

        UART_CommContext_t *ctx = UartCommManager_GetLeftBudContext();
        UartCommand_t cmd;
        cmd.command_id = 0x01;
        memcpy(cmd.data, command, sizeof(cmd.data));
        cmd.length = sizeof(cmd.data);
        UartCommandQueue_Enqueue(&ctx->cmd_queue, &cmd);
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
    if (command[1] < FILE_ID_NUM_MODES)
    {
        if (command[1] == FILE_ID_LOCAL)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_CmdEraseHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update erase task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t WriteFile(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t Temp_buffer[USB_RECEIVE_LEN] = {0};
    if (command[1] < FILE_ID_NUM_MODES)
    {
        if (command[1] == FILE_ID_LOCAL)
        {
            memcpy(Temp_buffer, command, USB_RECEIVE_LEN);
            AppFwUpdata_UsbReceiveData(Temp_buffer, USB_RECEIVE_LEN);
            if (TaskScheduler_AddTask(AppFwUpdate_CmdWriteFlashHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add fw update write task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t Crc32File(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < FILE_ID_NUM_MODES)
    {
        if (command[1] == FILE_ID_LOCAL)
        {

            if (TaskScheduler_AddTask(AppFwUpdate_CmdCrcCheckHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                DEBUG_PRINT("add CRC check task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t GetSerialNumber(const uint8_t command[USB_RECEIVE_LEN])
{
    FileSystem_UserData_t *data = (FileSystem_UserData_t *)FileSystem_GetUserData();
    uint8_t buff[58] = {0x00};
    buff[0] = FAC_SERIAL_OP | COMMAND_READ_FLAG;
    memcpy(&buff[1], data->serial_number, sizeof(data->serial_number));
    // TODO: need get buds serial number.
    // left 20~38, right 39~57.
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
            // TODO: UART communication to lift bud setting charge status.
            break;
        }

        case COMMAND_TARGET_RIGHT_BUD:
        {
            // TODO: UART communication to lift bud setting charge status.
            break;
        }
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
