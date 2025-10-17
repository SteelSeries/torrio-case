#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"


/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
// firmware info
#define VERSION_OP                  0x10U // version

// MCU control
#define RESET_OP                    0x01U // recovery module

// For firmware/file update
#define ERASE_FILE_OP               0x02U 
#define FILE_ACCESS_OP              0x03U
#define FILE_CRC32_OP               0x04U

// debug
#define DEBUG_CUSTOM_OP             0x08U // each application implements it as they see fit
#define DEBUG_SY8809_OP             0x71U 
#define DEBUG_SY8809_XSENSE_OP      0x70U 

// internal - we can reuse those as USB APIs for Artery
#define INTERNAL_USB_SUSPEND_OP     0x0CU // USB entered suspend mode. Lower power usage
#define INTERNAL_USB_RESUME_OP      0x0DU // after suspension, resume from device received

// factory settings
#define FAC_SERIAL_OP               0x13 // factory settings
#define FAC_MODEL_COLOR_SPIN_OP     0x6D // factory settings


#define NO_OP                       0x00U // command module

#define NORMAL_MODE                 0x01U
#define BOOTLOADER_MODE             0x10U

#define COMMAND_READ_FLAG           0x80U

#define FLASH_WRITE_ERRORS          0x03U
#define FLASH_OPERATION_SUCCESS     0x00U

#define USB_RECEIVE_LEN             1024U
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
    COMMAND_TARGET_CASE = 0,
    COMMAND_TARGET_LEFT_BUD,
    COMMAND_TARGET_RIGHT_BUD
} Command_Target_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/


/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Commands_HandleUsbCommand(const uint8_t * in, size_t in_len);
