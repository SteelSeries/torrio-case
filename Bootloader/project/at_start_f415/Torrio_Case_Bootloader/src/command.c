/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "Command.h"
#
#include "custom_hid_class.h"
#include "usb.h"
#include "version.h"
#include "Bootloader.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NUM_COMMANDS                 (sizeof(handler_table)/sizeof(handler_table[0]))
static uint8_t txBuf[1024] = {0};
static uint32_t sum_crc32;
static uint8_t crc_data[BUFFER_LEN];
static uint32_t i;
#define IN_MAXPACKET_SIZE           1024
#define OUT_MAXPACKET_SIZE          1024
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
typedef Command_Status_t (*Command_Handler_t)(const uint8_t command[OUT_MAXPACKET_SIZE]);

typedef struct
{
   uint8_t           op;
   Command_Handler_t read;
   Command_Handler_t write;
} cmd_handler_t;
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
__root __no_init uint8_t gCurrentMode @0x20000000;
bool SS_RESET_FLAG = false;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static Command_Status_t Command_HandleNoop(const uint8_t command[OUT_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleResetDevice(const uint8_t command[OUT_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleEraseFlash(const uint8_t command[OUT_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleWriteFlash(const uint8_t command[OUT_MAXPACKET_SIZE]);
static Command_Status_t Command_HandleReadFlash(const uint8_t command[OUT_MAXPACKET_SIZE]);
static Command_Status_t Command_CheckCRC32(const uint8_t command[OUT_MAXPACKET_SIZE]);
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t buffer[OUT_MAXPACKET_SIZE] = {0};
static uint8_t Read_flashAdrss_tab[4] = {0};
static uint32_t Read_flash_address = 0;
static uint8_t Read_flash_data[READ_FLASH_BUFFER_LEN];
static const cmd_handler_t handler_table[] =
{
     { .op = NO_OP,                     .read = Command_HandleNoop,                       .write = Command_HandleNoop },
     { .op = RESET_DEVICE,              .read = Command_HandleResetDevice,                       .write = Command_HandleResetDevice },
     { .op = ERASE_THE_FLASH,           .read = Command_HandleNoop,                       .write = Command_HandleEraseFlash },
     { .op = WRITE_FLASH_BLOCK,         .read = Command_HandleReadFlash,                  .write = Command_HandleWriteFlash },
     { .op = READ_FLASH_BLOCK,          .read = Command_HandleNoop,                       .write = Command_HandleNoop },
     { .op = CRC_FLASH_CHECK,           .read = Command_CheckCRC32,                       .write = Command_HandleNoop },
     { .op = GET_FIRMWARE_VERSION,      .read = Command_HandleNoop,                       .write = Command_HandleNoop }, 

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
        uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
        memset(buff, 0, sizeof(buff));
        snprintf((char*)buff, sizeof(buff), "USB COMMAND ERROR CMD:%02X\n", command);
        custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));

    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static Command_Status_t Command_HandleNoop(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleResetDevice(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    if (command[1] == RESET_COMMAND)
    {
        gCurrentMode = NORMAL_MODE;
        NVIC_SystemReset();
    }
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleEraseFlash(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    buff[0] = ERASE_THE_FLASH;
    buff[1] = FLASH_OPERATION_SUCCESS;
    flash_unlock();
    flash_flag_clear(FLASH_PRGMERR_FLAG);
    for (i = (ERASE_FLASH_END_ADDRESS - 1024); i >= APP_FLASH_START_ADDRESS; i -= 1024)
    {
        flash_status_type status = flash_sector_erase(i);
        if (status != FLASH_OPERATE_DONE)
        {
            buff[1] = FLASH_WRITE_ERRORS;
        break;
        }
    }
    flash_lock();
    custom_hid_class_send_report(&otg_core_struct.dev, buff, 64);
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleWriteFlash(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    txBuf[0] = WRITE_FLASH_BLOCK;
    uint32_t FW_UPDATE_Buffer[(BUFFER_LEN / 4)];
    uint32_t FW_Updateing_destAdrss;
    txBuf[1] = FLASH_OPERATION_SUCCESS;
    flash_unlock();
    flash_status_type status = flash_operation_wait_for(ERASE_TIMEOUT);
    if ((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
    {
        flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
        flash_lock();
        txBuf[1] = FLASH_WRITE_ERRORS;
    }
    else if (status == FLASH_OPERATE_TIMEOUT)
    {
        flash_lock();
        txBuf[1] = FLASH_WRITE_ERRORS;
    }

    uint32_t k = 0;
    for (i = 0; i < (BUFFER_LEN / 4); ++i)
    {
        FW_UPDATE_Buffer[i] = 0;
        FW_UPDATE_Buffer[i] = command[k + 9];
        FW_UPDATE_Buffer[i] |= (uint32_t)(command[k + 10] << 8);
        FW_UPDATE_Buffer[i] |= (uint32_t)(command[k + 11] << 16);
        FW_UPDATE_Buffer[i] |= (uint32_t)(command[k + 12] << 24);
        k += 4;
    }

    FW_Updateing_destAdrss = (uint32_t)(command[5]);
    FW_Updateing_destAdrss |= (uint32_t)(command[6] << 8);
    FW_Updateing_destAdrss |= (uint32_t)(command[7] << 16);
    FW_Updateing_destAdrss |= (uint32_t)(command[8] << 24);
    FW_Updateing_destAdrss += APP_FLASH_START_ADDRESS;
    for (i = 0; i < (BUFFER_LEN / 4); ++i)
    {
        if (flash_word_program(FW_Updateing_destAdrss, FW_UPDATE_Buffer[i]) != FLASH_OPERATE_DONE)
        {
            txBuf[1] = FLASH_WRITE_ERRORS;
        }
        if (FW_UPDATE_Buffer[i] != *(uint32_t *)FW_Updateing_destAdrss)
        {
            txBuf[1] = FLASH_WRITE_ERRORS;
        }
        FW_Updateing_destAdrss += 4;
        if (FW_Updateing_destAdrss >= ERASE_FLASH_END_ADDRESS) // if this address of last page to break the program.
        {
            break;
        }
    }

    if (FW_Updateing_destAdrss >= ERASE_FLASH_END_ADDRESS) // if this address of last page to break the program.
    {
        for (i = 0; i < LAST_CRC_INDES; ++i)
        {
            crc_data[i] = command[i + 9]; // include CRC of binary last 4 bytes
        }
        sum_crc32 = crc32_compute(crc_data, LAST_CRC_INDES - 4, &sum_crc32); // No need last 4 bytes for CRC
    }
    else
    {
        for (i = 0; i < BUFFER_LEN; ++i)
        {
            crc_data[i] = command[i + 9];
        }
        sum_crc32 = crc32_compute(crc_data, BUFFER_LEN, &sum_crc32);
    }
    flash_lock();
    custom_hid_class_send_report(&otg_core_struct.dev, txBuf, 64);
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_HandleReadFlash(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    //read the flash
    buff[0] = READ_FLASH_BLOCK;
    uint16_t Read_Flash_len = (command[4] << 8) + command[3];

    for (int i = 0; i < 4; ++i)
    {
        Read_flashAdrss_tab[i] = command[i + 5];
    }

    Read_flash_address = (uint32_t)(Read_flashAdrss_tab[0]);
    Read_flash_address |= (uint32_t)(Read_flashAdrss_tab[1] << 8);
    Read_flash_address |= (uint32_t)(Read_flashAdrss_tab[2] << 16);
    Read_flash_address |= (uint32_t)(Read_flashAdrss_tab[3] << 24);

    if ((Read_flash_address >= BOOTPATCH_FLASH_START_ADDRESS) && (Read_flash_address <= BOOTPATCH_FLASH_END_ADDRESS)) // 0x2004000 - 0x202FFFF = boot_patch (176K)
    {
        buff[1] = FLASH_WRITE_ERRORS;
    }

    FLASH_Read(Read_flash_address, Read_flash_data, Read_Flash_len);

    buff[0] = READ_FLASH_BLOCK;

    for (int i = 0; i < Read_Flash_len; ++i)
    {
        buff[i + 2] = Read_flash_data[i];
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, 64);
    return COMMAND_STATUS_SUCCESS;
}
static Command_Status_t Command_CheckCRC32(const uint8_t command[OUT_MAXPACKET_SIZE])
{
    //crc check
    uint8_t buff[OUT_MAXPACKET_SIZE] = {0};
    buff[0] = CRC_FLASH_CHECK+0x80;
    if (crc_used_flag == false)
    {
        FLASH_Read(APP_CRC_FLASH_START_ADDRESS, Read_flash_data, 4);

        for (int i = 0; i < 4; ++i)
        {
            crc_data[i] = Read_flash_data[i];
        }

        sum_crc32 = (uint32_t)Read_flash_data[0] + ((uint32_t)Read_flash_data[1] << 8) + ((uint32_t)Read_flash_data[2] << 16) + ((uint32_t)Read_flash_data[3] << 24);

        crc_data[LAST_CRC_INDES - 4] = Read_flash_data[0];
        crc_data[LAST_CRC_INDES - 3] = Read_flash_data[1];
        crc_data[LAST_CRC_INDES - 2] = Read_flash_data[2];
        crc_data[LAST_CRC_INDES - 1] = Read_flash_data[3];
    }

    buff[1] = FLASH_OPERATION_SUCCESS;
    buff[2] = crc_data[LAST_CRC_INDES - 4];
    buff[3] = crc_data[LAST_CRC_INDES - 3];
    buff[4] = crc_data[LAST_CRC_INDES - 2];
    buff[5] = crc_data[LAST_CRC_INDES - 1];
    buff[6] = sum_crc32;
    buff[7] = (sum_crc32 >> 8);
    buff[8] = (sum_crc32 >> 16);
    buff[9] = (sum_crc32 >> 24);

    // uint32_t check_flag = ((USB_DEVICE_PID-1)<<16);
    // check_flag |= USB_DEVICE_VID;
    // if(check_flag == *(uint32_t*)0x0801FFF8)
    //  gCurrentMode = 0xFF;
    custom_hid_class_send_report(&otg_core_struct.dev, buff, 64);
    return COMMAND_STATUS_SUCCESS;
}