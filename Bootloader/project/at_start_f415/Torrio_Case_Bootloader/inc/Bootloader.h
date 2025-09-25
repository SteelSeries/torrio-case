
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "at32f415_clock.h"
#include "usb_conf.h"
#include "usb_core.h"
#include "usbd_int.h"
#include "custom_hid_class.h"
#include "custom_hid_desc.h"
#include <stdbool.h>
#include "usb.h"
#include "Command.h"
/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
extern bool crc_used_flag;
extern uint32_t bootloader_len;
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
bool check_backdoor(void);
void JumpToApp(void);
bool CheckAppCodeComplete(void);
extern uint32_t crc32_compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc);
extern void FLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);
extern void FLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);
extern uint16_t FLASH_ReadHalfWord(uint32_t faddr);
extern bool CheckAppCodeComplete(void);
extern void JumpToApp(void);
