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
#define ERASE_FILE_OP               0x02U // HCFS
#define FILE_ACCESS_OP              0x03U // HCFS
#define FILE_CRC32_OP               0x04U // HCFS

// debug
#define DEBUG_CUSTOM_OP             0x08U // each application implements it as they see fit
#define DEBUG_SY8809_OP             0x09U 
#define DEBUG_LEDRGB_OP             0x72U
#define DEBUG_LIGHTING_OP           0x0BU

// internal - we can reuse those as USB APIs for Artery
#define INTERNAL_USB_SUSPEND_OP     0x0CU // USB entered suspend mode. Lower power usage
#define INTERNAL_USB_RESUME_OP      0x0DU // after suspension, resume from device received


#define NO_OP                       0x00U // command module

#define NORMAL_MODE                 0x01U
#define BOOTLOADER_MODE             0x10U

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

typedef enum
{
    AT32F415 = 0,
    Left_Earbud,
    Right_Earbud
} Chip_Select_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern bool SS_RESET_FLAG;
extern uint8_t gCurrentMode;

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Commands_HandleUsbCommand(const uint8_t * in, size_t in_len);
