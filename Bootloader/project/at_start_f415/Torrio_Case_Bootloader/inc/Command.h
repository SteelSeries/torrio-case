
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

#define RESET_DEVICE                 0x01U
#define RESET_COMMAND                0x00U
#define ERASE_THE_FLASH              0x02U
#define WRITE_FLASH_BLOCK            0x03U
#define READ_FLASH_BLOCK             0x03U
#define CRC_FLASH_CHECK              0x04U
// firmware info
#define GET_FIRMWARE_VERSION         0x10U

#define NORMAL_MODE                     (0x01U)
#define BOOTLOADER_MODE                 (0x10U)

#define FLASH_OPERATION_SUCCESS         (0x00U)
#define FLASH_SIZE_INVALID              (0x01U)
#define FLASH_OFFSET_INVALID            (0x02U)
#define FLASH_WRITE_ERRORS              (0x03U)
   
#define BUFFER_LEN                      1012
#define READ_FLASH_BUFFER_LEN           64
#define LAST_CRC_INDES                  332

#define NO_OP                       0x00U // command module

#define COMMAND_READ_FLAG           0x80U
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    COMMAND_STATUS_SUCCESS = 0,
    COMMAND_STATUS_ERROR_INTERNAL,
    COMMAND_STATUS_ERROR_BAD_PARAM,
    COMMAND_STATUS_ERROR_NO_HANDLER,
} Command_Status_t;

typedef enum
{
    RECOVERY_MODE_APPLICATION = 0,
    RECOVERY_MODE_BOOTLOADER,
    RECOVERY_MODE_NUM_MODES
} Recovery_Mode_t;

typedef enum
{
    FILE_ID_LOCAL = 0,
    FILE_ID_PERIPHERAL,
    FILE_ID_NUM_MODES
} File_Id_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern uint8_t gCurrentMode;

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Commands_HandleUsbCommand(const uint8_t * in, size_t in_len);