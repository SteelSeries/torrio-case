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
#include <stdio.h>
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))
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
static Command_Status_t Command_HandleNoop(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t CommandVersion_ReadVersion(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t CommandRecovery_Reset(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t handle_debug_command(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t handle_sy8809_debug_read_command(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t handle_sy8809_debug_write_command(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t CommandHcfs_EraseFile(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t CommandHcfs_WriteFile(const uint8_t command[USB_RECEIVE_LEN]);
static Command_Status_t CommandHcfs_Crc32File(const uint8_t command[USB_RECEIVE_LEN]);

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[USB_RECEIVE_LEN] = {0};
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP, .read = Command_HandleNoop, .write = Command_HandleNoop},
        // mcu control
        {.op = RESET_OP, .read = Command_HandleNoop, .write = CommandRecovery_Reset},
        // file/firmware update
        {.op = ERASE_FILE_OP, .read = Command_HandleNoop, .write = CommandHcfs_EraseFile},
        {.op = FILE_ACCESS_OP, .read = Command_HandleNoop, .write = CommandHcfs_WriteFile},
        {.op = FILE_CRC32_OP, .read = CommandHcfs_Crc32File, .write = Command_HandleNoop},
        // info
        {.op = VERSION_OP, .read = CommandVersion_ReadVersion, .write = Command_HandleNoop},
        // debug
        {.op = DEBUG_CUSTOM_OP, .read = Command_HandleNoop, .write = handle_debug_command},
        {.op = DEBUG_SY8809_OP, .read = handle_sy8809_debug_read_command, .write = handle_sy8809_debug_write_command},
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

static Command_Status_t Command_HandleNoop(const uint8_t command[USB_RECEIVE_LEN])
{
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandVersion_ReadVersion(const uint8_t command[USB_RECEIVE_LEN])
{
    uint8_t buff[13] = {0x00};
    uint8_t temp_buff[12] = {0};
    buff[0] = VERSION_OP | COMMAND_READ_FLAG;
    Version_GetArteryVersion(temp_buff, sizeof(temp_buff));
    memcpy(&buff[1], temp_buff, 8);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandRecovery_Reset(const uint8_t command[USB_RECEIVE_LEN])
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

static Command_Status_t handle_debug_command(const uint8_t command[USB_RECEIVE_LEN])
{
    switch (command[1])
    {
    case 0x01:
    {
        break;
    }

    default:
        break;
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t handle_sy8809_debug_read_command(const uint8_t command[USB_RECEIVE_LEN])
{
    if ((command[1] >= SY8809_XSENSE_NTC) && (command[1] <= SY8809_XSENSE_VBIN))
    {
        Sy8809Xsense_SetPendingXsense((Sy8809Xsense_OutputItem_t)command[1]);
        if (TaskScheduler_AddTask(Sy8809Xsense_TrigXsenseConv, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
        {
            printf("add sy8809 trig xsense conv task fail\n");
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t handle_sy8809_debug_write_command(const uint8_t command[USB_RECEIVE_LEN])
{
    // Todo: sy8809 write function.
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandHcfs_EraseFile(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < FILE_ID_NUM_MODES)
    {
        if (command[1] == FILE_ID_LOCAL)
        {
            if (TaskScheduler_AddTask(AppFwUpdate_CmdEraseHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                printf("add fw update erase task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandHcfs_WriteFile(const uint8_t command[USB_RECEIVE_LEN])
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
                printf("add fw update write task fail\n");
            }
        }
    }
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandHcfs_Crc32File(const uint8_t command[USB_RECEIVE_LEN])
{
    if (command[1] < FILE_ID_NUM_MODES)
    {
        if (command[1] == FILE_ID_LOCAL)
        {

            if (TaskScheduler_AddTask(AppFwUpdate_CmdCrcCheckHandler, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                printf("add CRC check task fail\n");
            }
            // uint8_t buff[10] = {0x00};
            // AppFwUpdate_CmdCrcCheckHandler(buff);
            // custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
        }
    }
    return COMMAND_STATUS_SUCCESS;
}
