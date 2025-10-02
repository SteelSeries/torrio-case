
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define APP_FLASH_SIZE (0xE000)
#define APP_FLASH_START (0x4000)
#define APP_FLASH_START_ADDRESS (FLASH_BASE + APP_FLASH_START)
#define APP_FLASH_END_ADDRESS (FLASH_BASE + APP_FLASH_START + APP_FLASH_SIZE)

#define DUAL_IMG_FLASH_SIZE (0xE000)
#define DUAL_IMG_FLASH_START (0x12000)
#define DUAL_IMG_START_ADDRESS (FLASH_BASE + DUAL_IMG_FLASH_START)
#define DUAL_IMG_END_ADDRESS (FLASH_BASE + DUAL_IMG_FLASH_START + DUAL_IMG_FLASH_SIZE)

//  AT32F415RBT7-7 flash full size 128K
//  0x0800 0000 - 0x0800 37FF : Bootloader code(14K)
//  0x0800 3800 - 0x0800 3801 : Flash 2(user data)(color spin)
//  0x0800 3802 - 0x0800 3802 : Flash 1(user data)(shipping flag)
//  0x0800 3803 - 0x0800 3803 : Flash 1(user data)(factory charge flag)
//  0x0800 3804 - 0x0800 3BFF : Flash 2(user data)(null)
//  0x0800 3C00 - 0x0800 3FFF : Flash 1(user data)(SN)
//  0x0800 4000 - 0x0801 1FFF : App code(56K)
//  0x0801 2000 - 0x0801 FFFF : Dual img code(56K)

// #define SECTOR_SIZE 0x800U /* 2KB  256K flash*/
#define SECTOR_SIZE 0x400U /* 2KB 128K flash*/
#define BUFFER_LEN (20)
#define USB_UPDATE_BUFFER_LEN (1024)
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void AppFwUpdate_CmdEraseHandler(void);
void AppFwUpdate_CmdWriteFlashHandler(void);
error_status AppFwUpdate_CmdCrcCheckHandler(uint8_t *buff);
void AppFwUpdata_SetResetFlag(bool state);
bool AppFwUpdata_GetResetFlag(void);
void AppFwUpdata_SetCurrentMode(uint8_t mode);
void AppFwUpdata_UsbReceiveData(uint8_t *data, uint16_t len);
