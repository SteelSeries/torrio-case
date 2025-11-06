
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define USER_DATA_FLASH_SIZE (0x400U)
#define USER_DATA_FLASH_START (0x3800)
#define USER_DATA_START_ADDRESS (FLASH_BASE + USER_DATA_FLASH_START)
#define USER_DATA_END_ADDRESS (FLASH_BASE + USER_DATA_FLASH_START + USER_DATA_FLASH_SIZE)
#define CASE_SN_DATA_LEN 19U
//  AT32F415RBT7-7 flash full size 128K
//  AT32F415RCT7-7 flash full size 256K
//  0x0800 0000 - 0x0800 37FF : Bootloader code(14K)
//  0x0800 3800 - 0x0800 3800 : Flash 1(user data)(model)
//  0x0800 3801 - 0x0800 3801 : Flash 1(user data)(color)
//  0x0800 3802 - 0x0800 3802 : Flash 1(user data)(shipping flag)
//  0x0800 3803 - 0x0800 3803 : Flash 1(user data)(dual Image Copy Flag)
//  0x0800 3804 - 0x0800 3817 : Flash 19(user data)(SN)
//  0x0800 3818 - 0x0800 3818 : Flash 1(user data)(presetChargeState)
//  0x0800 3819 - 0x0800 3FFF : Flash (user data)(Not used yet)
//  0x0800 4000 - 0x0801 1FFF : App code(56K)
//  0x0801 2000 - 0x0801 FFFF : Dual img code(56K)

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    DUAL_IMAGE_FLAG_NONE = 0xFF,    /* Default state after erase (no copy request) */
    DUAL_IMAGE_FLAG_REQUEST = 0xA5, /* App requests Bootloader to copy Dual Image */
    DUAL_IMAGE_FLAG_INPROG = 0x55,  /* (Optional) Copy operation is in progress */
    DUAL_IMAGE_FLAG_ERROR = 0x00    /* (Optional) Copy failed */
} FileSystem_DualImageFlag_t;

typedef struct __attribute__((packed))
{
    uint8_t model;                           // 0x08003800
    uint8_t color;                           // 0x08003801
    uint8_t shipping_flag;                   // 0x08003802
    uint8_t dual_image_copy_flag;            // 0x08003803
    uint8_t serial_number[CASE_SN_DATA_LEN]; // 0x08003804 ~ 0x08003817
    uint8_t presetChargeState;               // 0x08003818
    uint8_t reserved;
} FileSystem_UserData_t;

typedef enum
{
    UPDATE_FIELD_NONE = 0,
    UPDATE_FIELD_MODEL = 0x01,
    UPDATE_FIELD_COLOR = 0x02,
    UPDATE_FIELD_SHIPPING_FLAG = 0x04,
    UPDATE_FIELD_DUAL_IMAGE_FLAG = 0x08,
    UPDATE_FIELD_SERIAL_NUMBER = 0x10,
    UPDATE_FIELD_PRESET_CHARGE = 0x20,
} FileSystem_UserDataUpdateField_t;

typedef enum
{
    PRESET_CHARGE_ACTIVE = 0xAA,
    PRESET_CHARGE_EXIT = 0xEE
} FileSystem_PresetChargeMode_t;

typedef struct
{
    uint32_t field_mask;
    uint8_t model;
    uint8_t color;
    uint8_t shipping_flag;
    uint8_t dual_image_copy_flag;
    uint8_t serial_number[CASE_SN_DATA_LEN];
    uint8_t presetChargeState;
    uint8_t reserved;
} FileSystem_UserDataUpdate_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
const FileSystem_UserData_t *FileSystem_GetUserData(void);
void FileSystem_UpdateSerialNumber(const uint8_t *new_serial);
void FileSystem_UpdateColorSpinAndModel(const uint8_t model_value, const uint8_t color_value);
void FileSystem_MarkDualImageReadyToMigrate(void);
void FileSystem_CheckImageCopyFlag(void);
void FileSystem_MarkPresetChargeActive(FileSystem_PresetChargeMode_t state);
