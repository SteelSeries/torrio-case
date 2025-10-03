/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "file_system.h"
#include "Bootloader.h"
#include <string.h>

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static const FileSystem_UserData_t *user_data = (const FileSystem_UserData_t *)USER_DATA_START_ADDRESS;

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static error_status EraseFlashRegion(uint32_t start_addr, uint32_t end_addr);
static error_status WriteFlashProcess(uint32_t address, const uint8_t *data, size_t in_len);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void FileSystem_MarkDualImageReadyToMigrate(void)
{
    FileSystem_UserData_t user_data_ram = {0};
    memcpy(&user_data_ram, (const void *)USER_DATA_START_ADDRESS, sizeof(FileSystem_UserData_t));
    user_data_ram.dual_image_copy_flag = (uint8_t)DUAL_IMAGE_FLAG_REQUEST;
    EraseFlashRegion(USER_DATA_START_ADDRESS, USER_DATA_END_ADDRESS);
    WriteFlashProcess(USER_DATA_START_ADDRESS, (uint8_t *)&user_data_ram, sizeof(FileSystem_UserData_t));
}

const FileSystem_UserData_t *FileSystem_GetUserData(void)
{
    return user_data;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static error_status EraseFlashRegion(uint32_t start_addr, uint32_t end_addr)
{
    flash_status_type status = FLASH_OPERATE_DONE;
    uint32_t sector_start = (start_addr - FLASH_BASE) / SECTOR_SIZE;
    uint32_t sector_end = (end_addr - FLASH_BASE + SECTOR_SIZE - 1) / SECTOR_SIZE;
    uint32_t sector;

    if (start_addr < FLASH_BASE ||
        end_addr <= start_addr ||
        end_addr > (FLASH_BASE + FLASH_FULL_SIZE))
    {
        return ERROR;
    }

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

    for (sector = sector_start; sector < sector_end; sector++)
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

static error_status WriteFlashProcess(uint32_t address, const uint8_t *data, size_t in_len)
{
    uint32_t i;
    uint8_t buffer[SECTOR_SIZE] = {0x00};
    flash_status_type status = FLASH_OPERATE_DONE;

    if (in_len > SECTOR_SIZE)
    {
        return ERROR;
    }

    memcpy(buffer, data, in_len);

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

    uint32_t FW_Updateing_destAdrss = address;

    for (i = 0; i < in_len; i++)
    {
        if (flash_byte_program(FW_Updateing_destAdrss, buffer[i]) != FLASH_OPERATE_DONE)
        {
            flash_lock();
            return ERROR;
        }

        if (buffer[i] != *(uint8_t *)FW_Updateing_destAdrss)
        {
            flash_lock();
            return ERROR;
        }
        FW_Updateing_destAdrss += 1;
    }
    flash_lock();
    return SUCCESS;
}