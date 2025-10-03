
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include <stdint.h>
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
//  AT32F415RCT7-7 flash full size 256K
//  0x0800 0000 - 0x0800 37FF : Bootloader code(14K)
//  0x0800 3800 - 0x0800 3800 : Flash 1(user data)(model)
//  0x0800 3801 - 0x0800 3801 : Flash 1(user data)(color)
//  0x0800 3802 - 0x0800 3802 : Flash 1(user data)(shipping flag)
//  0x0800 3803 - 0x0800 3803 : Flash 1(user data)(dual Image Copy Flag)
//  0x0800 3804 - 0x0800 3C16 : Flash 18(user data)(SN)
//  0x0800 3C16 - 0x0800 3FFF : Flash (user data)(Not used yet)
//  0x0800 4000 - 0x0801 1FFF : App code(56K)
//  0x0801 2000 - 0x0801 FFFF : Dual img code(56K)

#define FLASH_FULL_SIZE (128U * 1024U) // 128KB
// #define FLASH_FULL_SIZE  (256U * 1024U)   // 256KB
#define SECTOR_SIZE 0x400U /* 1KB 128K flash*/
// #define SECTOR_SIZE 0x800U /* 2KB 256K flash*/

#define SECTOR_COUNT (FLASH_FULL_SIZE / SECTOR_SIZE)
#define UPDATE_DATA_LEN (1012)
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
void AppFwUpdate_CmdCrcCheckHandler(void);
void AppFwUpdata_SetResetFlag(bool state);
bool AppFwUpdata_GetResetFlag(void);
void AppFwUpdata_SetCurrentMode(uint8_t mode);
void AppFwUpdata_UsbReceiveData(uint8_t *data, uint16_t len);
