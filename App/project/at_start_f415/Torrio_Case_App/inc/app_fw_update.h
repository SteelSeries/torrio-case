
#pragma once
/*
==========================================================
        Dual Bank Firmware Update Process
==========================================================

Step 1: APP mode - Receive new firmware into Bank 1 (Dual Image)
----------------------------------------------------------------
- Firmware is downloaded via USB (or other interface) and stored
  in Bank 1:
      0x08012000 - 0x0801FFFF (Dual Image region, 56KB)
- After full reception, compute CRC on the received image.
- Only if CRC check passes (image integrity verified):
    - Set dual_image_copy_flag = DUAL_IMAGE_FLAG_REQUEST (0xA5)
      to indicate that Bootloader should copy this image to Bank 0
      on next reset.

Step 2: Trigger Bootloader via USB Command
------------------------------------------
- APP receives a USB command "01 Reset" to trigger firmware update.
- System executes NVIC_SystemReset() to reset the MCU.
- After reset, MCU will enter Bootloader (at 0x08000000).

Step 3: Bootloader checks dual_image_copy_flag
-----------------------------------------------
- On startup, Bootloader reads dual_image_copy_flag at 0x08003803.
- If flag == DUAL_IMAGE_FLAG_REQUEST (0xA5):
    - Bootloader will start copying Bank 1 (Dual Image) to Bank 0 (App).

Step 4: Copy Dual Image (Bank 1) to App Bank (Bank 0)
------------------------------------------------------
- Define source: Bank 1 start address = 0x08012000
- Define destination: Bank 0 start address = 0x08004000
- Define size: 56KB
- Bootloader iterates over Bank 1 by sectors/pages:
    1. Read a chunk (sector/page) from Dual Image.
    2. Erase corresponding page in App bank if needed.
    3. Write chunk to App bank using flash write routines.
    4. Verify each write by comparing written data or CRC.

Step 5: Jump to App after copy completes
-----------------------------------------
- After the entire Dual Image is copied:
    - MCU jumps to App starting address (0x08004000) to execute new firmware.

Step 6: APP clears dual_image_copy_flag
---------------------------------------
- At the very beginning of App execution:
    1. APP reads dual_image_copy_flag at 0x08003803.
    2. If flag == DUAL_IMAGE_FLAG_REQUEST (0xA5):
        - APP sets flag = DUAL_IMAGE_FLAG_NONE (0xFF)
          to indicate update is complete.
- Rationale:
    - Reaching App implies Bootloader has successfully copied
      the new firmware from Bank 1 to Bank 0.
    - Clearing the flag ensures next reset will not trigger another copy.
==========================================================
*/

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
