/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "command.h"
#include "custom_hid_class.h"
#include "usb.h"
#include "bootloader.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS (sizeof(handler_table) / sizeof(handler_table[0]))
#define IN_MAXPACKET_SIZE 1024
#define OUT_MAXPACKET_SIZE 64
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
typedef Command_Status_t (*Command_Handler_t)(const uint8_t command[IN_MAXPACKET_SIZE]);

typedef struct
{
    uint8_t op;
    Command_Handler_t read;
    Command_Handler_t write;
} cmd_handler_t;
typedef enum
{
    RECOVERY_MODE_APPLICATION = 0,
    RECOVERY_MODE_BOOTLOADER,
    RECOVERY_MODE_NUM_MODES
} Recovery_Mode_t;
typedef enum
{
    FILE_ID_LOCAL = 0,
    FILE_ID_PERIPHERAL,
    FILE_ID_NUM_MODES

} File_Id_t;
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
__root __no_init uint8_t gCurrentMode @0x20000000;
bool SS_RESET_FLAG = false;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static Command_Status_t Command_HandleNoop(const uint8_t command[IN_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleResetDevice(const uint8_t command[IN_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleEraseFlash(const uint8_t command[IN_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleWriteFlash(const uint8_t command[IN_MAXPACKET_SIZE]);
static Command_Status_t Command_CheckCRC32(const uint8_t command[IN_MAXPACKET_SIZE]);
static Command_Status_t CommandVersion_ReadVersion(const uint8_t command[IN_MAXPACKET_SIZE]);
#ifndef DEBUG
static Command_Status_t Command_HandleReadFlash(const uint8_t command[IN_MAXPACKET_SIZE]);
#endif

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[IN_MAXPACKET_SIZE] = {0};
// clang-format off
static const cmd_handler_t handler_table[] =
    {
        {.op = NO_OP,                   .read = Command_HandleNoop,             .write = Command_HandleNoop},
        // mcu control
        {.op = RESET_DEVICE,            .read = Command_HandleNoop,             .write = Command_HandleResetDevice},
        // file/firmware update
        {.op = ERASE_THE_FLASH,         .read = Command_HandleNoop,             .write = Command_HandleEraseFlash},
#ifdef DEBUG
        {.op = WRITE_READ_FLASH_BLOCK,  .read = Command_HandleNoop,             .write = Command_HandleWriteFlash},
#else
        {.op = WRITE_READ_FLASH_BLOCK,  .read = Command_HandleReadFlash,        .write = Command_HandleWriteFlash},
#endif
        {.op = CRC_FLASH_CHECK,         .read = Command_CheckCRC32,             .write = Command_HandleNoop},
        // info
        {.op = GET_FIRMWARE_VERSION,    .read = CommandVersion_ReadVersion,     .write = Command_HandleNoop},
};
// clang-format on

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
        uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
        memset(buff, 0, sizeof(buff));
        snprintf((char *)buff, sizeof(buff), "USB COMMAND ERROR CMD:%02X\n", command);
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static Command_Status_t Command_HandleNoop(const uint8_t command[IN_MAXPACKET_SIZE])
{
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleResetDevice(const uint8_t command[IN_MAXPACKET_SIZE])
{
    if (command[1] < RECOVERY_MODE_NUM_MODES)
    {
        if (command[1] == RECOVERY_MODE_BOOTLOADER)
        {
            if (command[2] == FILE_ID_PERIPHERAL)
            {
                SS_RESET_FLAG = true;
                gCurrentMode = BOOTLOADER_MODE;
            }
        }
        else if (command[1] == RECOVERY_MODE_APPLICATION)
        {
            SS_RESET_FLAG = true;
            gCurrentMode = NORMAL_MODE;
        }
    }
    else
    {
        SS_RESET_FLAG = true;
        gCurrentMode = NORMAL_MODE;
    }
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleEraseFlash(const uint8_t command[IN_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    buff[0] = ERASE_THE_FLASH;
    buff[1] = FLASH_OPERATION_SUCCESS;
    if (Bootloader_FlashErase() != SUCCESS)
    {
        buff[1] = FLASH_WRITE_ERRORS;
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, OUT_MAXPACKET_SIZE);
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleWriteFlash(const uint8_t command[IN_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    buff[0] = WRITE_READ_FLASH_BLOCK;
    buff[1] = FLASH_OPERATION_SUCCESS;
    if (Bootloader_FlashWrite(command, IN_MAXPACKET_SIZE) != SUCCESS)
    {
        buff[1] = FLASH_WRITE_ERRORS;
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, OUT_MAXPACKET_SIZE);
    return COMMAND_STATUS_SUCCESS;
}

#ifndef DEBUG
static Command_Status_t Command_HandleReadFlash(const uint8_t command[IN_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    // read the flash
    buff[0] = WRITE_READ_FLASH_BLOCK | COMMAND_READ_FLAG;
    Bootloader_CommandHandleReadFlash(buff, command);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, OUT_MAXPACKET_SIZE);
    return COMMAND_STATUS_SUCCESS;
}
#endif

static Command_Status_t Command_CheckCRC32(const uint8_t command[IN_MAXPACKET_SIZE])
{
    // crc check
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    buff[0] = CRC_FLASH_CHECK | COMMAND_READ_FLAG;
    Bootloader_CmdCrcCheckHandler(buff);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, OUT_MAXPACKET_SIZE);
    return COMMAND_STATUS_SUCCESS;
}

static Command_Status_t CommandVersion_ReadVersion(const uint8_t command[IN_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0x00};
    uint8_t temp_buff[12] = {0};
    buff[0] = GET_FIRMWARE_VERSION | COMMAND_READ_FLAG;
    Version_GetArteryVersion(temp_buff, sizeof(temp_buff));
    memcpy(&buff[1], temp_buff, 8);
    custom_hid_class_send_report(&otg_core_struct.dev, buff, OUT_MAXPACKET_SIZE);
    return COMMAND_STATUS_SUCCESS;
}