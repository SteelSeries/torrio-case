/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "bootloader.h"
#include "command.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define IN_MAXPACKET_SIZE 1024
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
typedef void (*pFunction)(void);
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint32_t i;
static uint16_t gFW_BinLen = 0;
static uint16_t gFW_FirstBinLen = 0;
static uint8_t Read_flash_data[READ_FLASH_BUFFER_LEN];
static uint32_t Read_flash_address = 0;
static uint8_t Read_flashAdrss_tab[4] = {0};
static bool crc_used_flag = false;
static uint32_t sum_crc32;
static uint8_t crc_data[BUFFER_LEN];
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static uint16_t ReadFlashHalfWord(uint32_t faddr);
static void ReadFlash(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);
static uint32_t Crc32Compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
error_status Bootloader_FlashErase(void)
{
    flash_unlock();
    flash_flag_clear(FLASH_PRGMERR_FLAG);
    for (i = (ERASE_FLASH_END_ADDRESS - 1024); i >= APP_FLASH_START_ADDRESS; i -= 1024)
    {
        flash_status_type status = flash_sector_erase(i);
        if (status != FLASH_OPERATE_DONE)
        {
            return ERROR;
        }
    }
    flash_lock();
    return SUCCESS;
}
error_status Bootloader_FlashWrite(const uint8_t *in, size_t in_len)
{
    uint8_t buffer[IN_MAXPACKET_SIZE];
    uint32_t FW_UPDATE_Buffer[(BUFFER_LEN / 4)];
    uint32_t FW_Updateing_destAdrss;
    memcpy(buffer, in, in_len);
    gFW_BinLen = buffer[3] | (buffer[4] << 8);

    if (gFW_BinLen != gFW_FirstBinLen)
    {
        gFW_FirstBinLen = gFW_BinLen;
    }
    flash_unlock();
    flash_status_type status = flash_operation_wait_for(ERASE_TIMEOUT);
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
    for (i = 0; i < (BUFFER_LEN / 4); ++i)
    {
        FW_UPDATE_Buffer[i] = 0;
        FW_UPDATE_Buffer[i] = buffer[k + 9];
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 10] << 8);
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 11] << 16);
        FW_UPDATE_Buffer[i] |= (uint32_t)(buffer[k + 12] << 24);
        k += 4;
    }

    FW_Updateing_destAdrss = (uint32_t)(buffer[5]);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[6] << 8);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[7] << 16);
    FW_Updateing_destAdrss |= (uint32_t)(buffer[8] << 24);
    FW_Updateing_destAdrss += APP_FLASH_START_ADDRESS;
    for (i = 0; i < (BUFFER_LEN / 4); ++i)
    {
        if (flash_word_program(FW_Updateing_destAdrss, FW_UPDATE_Buffer[i]) != FLASH_OPERATE_DONE)
        {
            return ERROR;
        }
        if (FW_UPDATE_Buffer[i] != *(uint32_t *)FW_Updateing_destAdrss)
        {
            return ERROR;
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
            crc_data[i] = buffer[i + 9]; // include CRC of binary last 4 bytes
        }
        sum_crc32 = Crc32Compute(crc_data, LAST_CRC_INDES - 4, &sum_crc32); // No need last 4 bytes for CRC
    }
    else
    {
        for (i = 0; i < BUFFER_LEN; ++i)
        {
            crc_data[i] = buffer[i + 9];
        }
        sum_crc32 = Crc32Compute(crc_data, BUFFER_LEN, &sum_crc32);
    }
    flash_lock();
    return SUCCESS;
}
error_status Bootloader_CmdCrcCheckHandler(uint8_t *buff)
{
    if (crc_used_flag == false)
    {
        ReadFlash(APP_CRC_FLASH_START_ADDRESS, Read_flash_data, 4);

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
    return SUCCESS;
}
error_status Bootloader_CommandHandleReadFlash(uint8_t *buff, const uint8_t *in)
{
    uint8_t command[IN_MAXPACKET_SIZE];
    memcpy(command, in, IN_MAXPACKET_SIZE);
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

    ReadFlash(Read_flash_address, Read_flash_data, Read_Flash_len);

    buff[0] = WRITE_READ_FLASH_BLOCK;

    for (int i = 0; i < Read_Flash_len; ++i)
    {
        buff[i + 2] = Read_flash_data[i];
    }
    return SUCCESS;
}
bool Bootloader_CheckBackDoor(void)
{
    uint8_t i = 0;
    while (gpio_input_data_bit_read(USER_BUTTON_PORT, USER_BUTTON_PIN) == SET)
    {
        delay_ms(10);
        i++;
        if (i > 10)
        {
            gCurrentMode = BOOTLOADER_MODE;
            return true;
        }
    }
    return false;
}
void Bootloader_JumpToApp(void)
{
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    /* Test if user code is programmed starting from USBD_DFU_APP_DEFAULT_ADD address */
    if (((*(__IO uint32_t *)APP_FLASH_START_ADDRESS) & 0x2FFE0000) == 0x20000000)
    {
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t *)(APP_FLASH_START_ADDRESS + 4);
        JumpToApplication = (pFunction)JumpAddress;

        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)APP_FLASH_START_ADDRESS);
        JumpToApplication();
    }
}
bool Bootloader_CheckAppCodeComplete(void)
{
    uint8_t FLASH_ReadCRC[4] = {0};
    uint8_t null_count = 0;
    ReadFlash(ERASE_FLASH_END_ADDRESS - 4, FLASH_ReadCRC, 4);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (FLASH_ReadCRC[i] == 0Xff)
        {
            null_count++;
        }
    }

    if (null_count == 4)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/

static uint16_t ReadFlashHalfWord(uint32_t faddr)
{
    return *(vu8 *)faddr;
}

/**
 * @brief Read the data of the specified length from the specified address
 * @param ReadAddr starting address
 * @param pBuffer data pointer
 * @param NumToRead half-word (16-bit) number
 */
static void ReadFlash(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead)
{
    for (uint16_t i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = ReadFlashHalfWord(ReadAddr); // Read 1 byte
        ReadAddr += 1;                            // shift 1 byte
    }
}
static uint32_t Crc32Compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
{
    uint32_t crc;

    crc_used_flag = true;

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
