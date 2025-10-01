
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
//  0x0800 0000 - 0x0800 37FF : Bootloader(16K)
//  0x0800 3800 - 0x0800 3801 : Flash 2(color spin)
//  0x0800 3802 - 0x0800 3802 : Flash 2(shipping flag)
//  0x0800 3803 - 0x0800 3803 : Flash 2(factory charge flag)
//  0x0800 3804 - 0x0800 3BFF : Flash 2(null)
//  0x0800 3C00 - 0x0800 3FFF : Flash 1(SN)
//  0x0800 4000 - 0x0801 FFFF : APP CODE
#define APP_FLASH_SIZE (0x20000)
#define APP_FLASH_START (0x4000)
  
#define APP_FLASH_START_ADDRESS (FLASH_BASE + APP_FLASH_START)
#define ERASE_FLASH_END_ADDRESS (FLASH_BASE + APP_FLASH_SIZE)
#define APP_CRC_FLASH_START_ADDRESS (ERASE_FLASH_END_ADDRESS - 4)
  
#define BOOTPATCH_FLASH_START_ADDRESS   0x08000000
#define BOOTPATCH_FLASH_END_ADDRESS     0x08003FFF
#define BUFFER_LEN                      1012
#define bootloader_len                  1024
#define READ_FLASH_BUFFER_LEN           64
#define LAST_CRC_INDES                  332
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
bool Bootloader_CheckBackDoor(void);
bool Bootloader_CheckAppCodeComplete(void);
void Bootloader_JumpToApp(void);
error_status Bootloader_FlashErase(void);
error_status Bootloader_FlashWrite(const uint8_t *in, size_t in_len);
error_status Bootloader_CmdCrcCheckHandler(uint8_t * buff);
error_status Bootloader_CommandHandleReadFlash(uint8_t *buff , const uint8_t *in);
