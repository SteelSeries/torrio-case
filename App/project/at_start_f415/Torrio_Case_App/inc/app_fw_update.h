
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

#define USER_DATA_FLASH_SIZE (0x400U)
#define USER_DATA_FLASH_START (0x3800)
#define USER_DATA_START_ADDRESS (FLASH_BASE + USER_DATA_FLASH_START)
#define USER_DATA_END_ADDRESS (FLASH_BASE + USER_DATA_FLASH_START + USER_DATA_FLASH_SIZE)

//  AT32F415RBT7-7 flash full size 128K
//  AT32F415RCT7-7 flash full size 256K
//  0x0800 0000 - 0x0800 37FF : Bootloader code(14K)
//  0x0800 3800 - 0x0800 3801 : Flash 2(user data)(color spin)
//  0x0800 3802 - 0x0800 3802 : Flash 1(user data)(shipping flag)
//  0x0800 3803 - 0x0800 3803 : Flash 1(user data)(factory charge flag)
//  0x0800 3804 - 0x0800 3804 : Flash 1(user data)(dual Image Copy Flag)
//  0x0800 3805 - 0x0800 3BFF : Flash 2(user data)(null)
//  0x0800 3C00 - 0x0800 3FFF : Flash 1(user data)(SN)
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

typedef struct __attribute__((packed))
{
    uint8_t model;                // 0x08003800
    uint8_t color;                // 0x08003801
    uint8_t shipping_flag;        // 0x08003802
    uint8_t dual_image_copy_flag; // 0x08003803
    uint8_t serial_number[18];    // 0x08003804 ~ 0x08003C16
    uint8_t reserved;
} AppFwUpdate_UserData_t;
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
