/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "app_fw_update.h"
#include "Commands.h"
#include "custom_hid_class.h"
#include "usb.h"
#include "file_system.h"
#include "uart_interface.h"
#include "uart_protocol.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define NULL_FLASH_DATA 0xFFFFFFFF
#define WRITE_CMD_HEADER_SIZE 9U // Write command header size
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static __root __no_init uint8_t CurrentMode @0x20000000;
static bool reset_flag = false;
static uint32_t sum_crc32 = 0;
static uint8_t user_usb_receive_data[USB_RECEIVE_LEN] = {0};
static uint16_t gFW_BinLen = 0;
static uint16_t gFW_FirstBinLen = 0;
static uint8_t crc_data[UPDATE_DATA_LEN];

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static uint32_t Crc32Compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc);
static error_status EraseDualImageFlashProcess(void);
static error_status WriteDualImageFlashProcess(const uint8_t *in, size_t in_len);
static void ClearCrc32Calculate(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void AppFwUpdata_UsbReceiveData(uint8_t *data, uint16_t len)
{
    memcpy(user_usb_receive_data, data, len);
}

void AppFwUpdata_SetResetFlag(bool state)
{
    reset_flag = state;
}

bool AppFwUpdata_GetResetFlag(void)
{
    return reset_flag;
}

void AppFwUpdata_SetCurrentMode(uint8_t mode)
{
    CurrentMode = mode;
}

void AppFwUpdate_CmdEraseHandler(void)
{
    uint8_t buff[2] = {0x00};
    buff[0] = ERASE_FILE_OP;
    buff[1] = FLASH_OPERATION_SUCCESS;
    if (EraseDualImageFlashProcess() != SUCCESS)
    {
        buff[1] = FLASH_WRITE_ERRORS;
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
}

void AppFwUpdate_CmdWriteFlashHandler(void)
{
    uint8_t buff[2] = {0x00};
    buff[0] = FILE_ACCESS_OP;
    buff[1] = FLASH_OPERATION_SUCCESS;
    if (WriteDualImageFlashProcess(user_usb_receive_data, USB_RECEIVE_LEN) == ERROR)
    {
        buff[1] = FLASH_WRITE_ERRORS;
    }
    custom_hid_class_send_report(&otg_core_struct.dev, buff, sizeof(buff));
}

void AppFwUpdate_CmdCrcCheckHandler(void)
{
    uint8_t txBuf[10] = {0x00};
    txBuf[0] = FILE_CRC32_OP | COMMAND_READ_FLAG;
    txBuf[1] = FLASH_OPERATION_SUCCESS;
    txBuf[2] = crc_data[gFW_BinLen - 4];
    txBuf[3] = crc_data[gFW_BinLen - 3];
    txBuf[4] = crc_data[gFW_BinLen - 2];
    txBuf[5] = crc_data[gFW_BinLen - 1];
    txBuf[6] = sum_crc32;
    txBuf[7] = (sum_crc32 >> 8);
    txBuf[8] = (sum_crc32 >> 16);
    txBuf[9] = (sum_crc32 >> 24);
    for (uint8_t i = 2; i < 6; i++)
    {
        if (txBuf[i] != txBuf[i + 4])
        {
            DEBUG_PRINT("CRC check fail\n");
            EraseDualImageFlashProcess();
        }
    }
    FileSystem_MarkDualImageReadyToMigrate();
    custom_hid_class_send_report(&otg_core_struct.dev, txBuf, sizeof(txBuf));
}

// void AppFwUpdate_CmdCrcCheckHandler(void)
// {
//     uint8_t txBuf[10] = {0x00};
//     bool crc_check_flag = false;
//     txBuf[0] = FILE_CRC32_OP | COMMAND_READ_FLAG;
//     txBuf[1] = FLASH_OPERATION_SUCCESS;
//     txBuf[2] = crc_data[gFW_BinLen - 4];
//     txBuf[3] = crc_data[gFW_BinLen - 3];
//     txBuf[4] = crc_data[gFW_BinLen - 2];
//     txBuf[5] = crc_data[gFW_BinLen - 1];
//     txBuf[6] = sum_crc32;
//     txBuf[7] = (sum_crc32 >> 8);
//     txBuf[8] = (sum_crc32 >> 16);
//     txBuf[9] = (sum_crc32 >> 24);

//     for (uint8_t i = 2; i < 6; i++)
//     {
//         if (txBuf[i] != txBuf[i + 4])
//         {
//             DEBUG_PRINT("CRC check fail\n");
//             EraseDualImageFlashProcess();
//             crc_check_flag = true;
//             break;
//         }
//     }
//     if (crc_check_flag == false)
//     {
//         FileSystem_MarkDualImageReadyToMigrate();
//     }
//     custom_hid_class_send_report(&otg_core_struct.dev, txBuf, sizeof(txBuf));
// }

void AppFwUpdate_LeftBudWriteFlashTask(void)
{
    uint16_t BinLen = user_usb_receive_data[3] | (user_usb_receive_data[4] << 8);

    UartInterface_SendDirect(UART_INTERFACE_BUD_LEFT,
                             user_usb_receive_data,
                             BinLen + WRITE_CMD_HEADER_SIZE,
                             CMD_ONE_WIRE_UART_DATA,
                             5000,
                             FILE_ACCESS_OP);
}

void AppFwUpdate_RightBudWriteFlashTask(void)
{
    uint16_t BinLen = user_usb_receive_data[3] | (user_usb_receive_data[4] << 8);
    UartInterface_SendDirect(UART_INTERFACE_BUD_RIGHT,
                             user_usb_receive_data,
                             BinLen + WRITE_CMD_HEADER_SIZE,
                             CMD_ONE_WIRE_UART_DATA,
                             5000,
                             FILE_ACCESS_OP);
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static uint32_t Crc32Compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
{
    uint32_t crc;

    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

static error_status EraseDualImageFlashProcess(void)
{
    flash_status_type status = FLASH_OPERATE_DONE;
    uint32_t start_sector, end_sector;
    uint32_t sector;
    ClearCrc32Calculate();

    if ((DUAL_IMG_START_ADDRESS < FLASH_BASE) || (DUAL_IMG_START_ADDRESS % SECTOR_SIZE) ||
        (DUAL_IMG_END_ADDRESS > (FLASH_BASE + 128U * 1024U)) ||
        (DUAL_IMG_END_ADDRESS <= DUAL_IMG_START_ADDRESS))
    {
        return ERROR;
    }

    start_sector = ((DUAL_IMG_START_ADDRESS - FLASH_BASE) / SECTOR_SIZE);
    end_sector = ((DUAL_IMG_END_ADDRESS - FLASH_BASE) / SECTOR_SIZE);

    flash_unlock();

    status = flash_operation_wait_for(ERASE_TIMEOUT);
    if ((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
    {
        flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
    }
    else if (status == FLASH_OPERATE_TIMEOUT)
    {
        flash_lock();
        return ERROR;
    }

    for (sector = end_sector - 1; sector >= start_sector; sector--)
    {
        uint32_t addr = FLASH_BASE + sector * SECTOR_SIZE;

        status = flash_sector_erase(addr);
        if (status != FLASH_OPERATE_DONE)
        {
            flash_lock();
            return ERROR;
        }

        status = flash_operation_wait_for(ERASE_TIMEOUT);
        if ((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
        {
            flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
            flash_lock();
            return ERROR;
        }
        else if (status == FLASH_OPERATE_TIMEOUT)
        {
            flash_lock();
            return ERROR;
        }
    }
    /* Lock flash after operation */
    flash_lock();
    return SUCCESS;
}

static error_status WriteDualImageFlashProcess(const uint8_t *in, size_t in_len)
{
    uint32_t i;
    uint8_t buffer[USB_RECEIVE_LEN] = {0x00};
    uint32_t FW_UPDATE_Buffer[(UPDATE_DATA_LEN / 4)];
    uint32_t FW_Updateing_destAdrss;
    flash_status_type status = FLASH_OPERATE_DONE;

    memcpy(buffer, in, in_len);
    gFW_BinLen = buffer[3] | (buffer[4] << 8);

    if (gFW_BinLen != gFW_FirstBinLen)
    {
        gFW_FirstBinLen = gFW_BinLen;
    }

    flash_unlock();
    status = flash_operation_wait_for(ERASE_TIMEOUT);
    if ((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
    {
        flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
        flash_lock();
        return ERROR;
    }
    else if (status == FLASH_OPERATE_TIMEOUT)
    {
        flash_lock();
        return ERROR;
    }

    uint32_t k = 0;
    for (i = 0; i < (UPDATE_DATA_LEN / 4); ++i)
    {
        FW_UPDATE_Buffer[i] = 0;
        FW_UPDATE_Buffer[i] = buffer[k + WRITE_CMD_HEADER_SIZE];
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 10] << 8);
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 11] << 16);
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 12] << 24);
        k += 4;
    }

    FW_Updateing_destAdrss = (uint32_t)(buffer[5]);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[6] << 8);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[7] << 16);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[8] << 24);
    FW_Updateing_destAdrss += DUAL_IMG_START_ADDRESS;
    for (i = 0; i < (UPDATE_DATA_LEN / 4); ++i)
    {
        if (FW_UPDATE_Buffer[i] != NULL_FLASH_DATA)
        {
            if (flash_word_program(FW_Updateing_destAdrss, FW_UPDATE_Buffer[i]) != FLASH_OPERATE_DONE)
            {
                flash_lock();
                return ERROR;
            }
        }

        if (FW_UPDATE_Buffer[i] != *(uint32_t *)FW_Updateing_destAdrss)
        {
            flash_lock();
            return ERROR;
        }
        FW_Updateing_destAdrss += 4;
        if (FW_Updateing_destAdrss >= DUAL_IMG_END_ADDRESS) // if this address of last page to break the program.
        {
            break;
        }
    }

    if (FW_Updateing_destAdrss >= DUAL_IMG_END_ADDRESS) // if this address of last page to break the program.
    {
        for (i = 0; i < gFW_BinLen; ++i)
        {
            crc_data[i] = buffer[i + WRITE_CMD_HEADER_SIZE]; // include CRC of binary last 4 bytes
        }
        sum_crc32 = Crc32Compute(crc_data, gFW_BinLen - 4, &sum_crc32); // No need last 4 bytes for CRC
    }
    else
    {
        for (i = 0; i < UPDATE_DATA_LEN; ++i)
        {
            crc_data[i] = buffer[i + WRITE_CMD_HEADER_SIZE];
        }
        sum_crc32 = Crc32Compute(crc_data, UPDATE_DATA_LEN, &sum_crc32);
    }
    flash_lock();
    return SUCCESS;
}

static void ClearCrc32Calculate(void)
{
    sum_crc32 = 0;
    memset(crc_data, 0, sizeof(crc_data));
}
