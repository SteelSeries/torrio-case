/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "Bootloader.h"
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
typedef void (*pFunction)(void);
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
bool crc_used_flag = false;
uint32_t bootloader_len = 0x400;
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/

 /*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
bool check_backdoor(void)
{
    uint8_t i = 0;
    while(gpio_input_data_bit_read(USER_BUTTON_PORT , USER_BUTTON_PIN) == SET)
    {
      delay_ms(10);
      i++;
      if(i>10)
      {
        gCurrentMode = BOOTLOADER_MODE;
        return true;
      }
    }
    return false;
}
uint16_t FLASH_ReadHalfWord(uint32_t faddr)
{
  return *(vu8*)faddr;
}

/**
 * @brief Read the data of the specified length from the specified address
 * @param ReadAddr starting address
 * @param pBuffer data pointer
 * @param NumToRead half-word (16-bit) number
*/
void FLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead)
{
  for(uint16_t i=0 ; i<NumToRead ; i++)
  {
    pBuffer[i]=FLASH_ReadHalfWord(ReadAddr); // Read 1 byte
    ReadAddr += 1; //shift 1 byte
  }
}
void JumpToApp(void)
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
bool CheckAppCodeComplete(void)
{
    uint8_t FLASH_ReadCRC[4] = {0};
    uint8_t null_count = 0;
    FLASH_Read(ERASE_FLASH_END_ADDRESS - 4, FLASH_ReadCRC, 4);
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
uint32_t crc32_compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
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
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/