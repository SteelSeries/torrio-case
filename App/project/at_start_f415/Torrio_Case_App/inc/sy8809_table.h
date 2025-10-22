
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define SY8809_REG_TABLE_LEN 19

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    SY8809_REG_0x10 = 0x10,
    SY8809_REG_0x11,
    SY8809_REG_0x12,
    SY8809_REG_0x13,
    SY8809_REG_0x14,
    SY8809_REG_0x15,
    SY8809_REG_0x16,
    SY8809_REG_0x17,
    SY8809_REG_0x20 = 0x20,
    SY8809_REG_0x21,
    SY8809_REG_0x22,
    SY8809_REG_0x23,
    SY8809_REG_0x24,
    SY8809_REG_0x25,
    SY8809_REG_0x26,
    SY8809_REG_0x27,
    SY8809_REG_0x2E = 0x2E,
    SY8809_REG_0x2F,
    SY8809_REG_0x30 = 0x30,
    SY8809_REG_0x31,
    SY8809_REG_0x32,
    SY8809_REG_0x33,
    SY8809_REG_0x34,
    SY8809_REG_0x35,
    SY8809_REG_0x36,
    SY8809_REG_0x37,
    SY8809_REG_0x40 = 0x40,
    SY8809_REG_0x41,
    SY8809_REG_0x42,
    SY8809_REG_0x43,
    SY8809_REG_0x44
} Sy8809_Reg_t;

typedef enum
{
    SY8809_0x20,
    SY8809_0x21,
    SY8809_0x22,
    SY8809_0x23,
    SY8809_0x24,
    SY8809_0x25,
    SY8809_0x26,
    SY8809_0x2E,
    SY8809_0x2F,
    SY8809_0x30,
    SY8809_0x31,
    SY8809_0x32,
    SY8809_0x33,
    SY8809_0x34,
    SY8809_0x35,
    SY8809_0x36,
    SY8809_0x37,
    SY8809_0x44,
    SY8809_0x27
} Sy8809_SettingTableList_t;

typedef enum
{
    SY8809_REG_UNKNOWN = 0,
    SY8809_REG_TABLE_3 = 3,
    SY8809_REG_TABLE_4,
    SY8809_REG_TABLE_5H,
    SY8809_REG_TABLE_6,
    SY8809_REG_TABLE_A,
    SY8809_REG_TABLE_B,
    SY8809_REG_TABLE_CEI,
    SY8809_REG_TABLE_DFJ,
    SY8809_REG_TABLE_G
} Sy8809_Table_t;

typedef enum
{
    SY8809_NTC_LEVEL_20_TO_45 = 0x00,
    SY8809_NTC_LEVEL_10_TO_20 = 0x01,
    SY8809_NTC_LEVEL_0_TO_10 = 0x03,
    SY8809_NTC_LEVEL_45_TO_60 = 0x04,
    SY8809_NTC_LEVEL_MINUS_10_TO_0 = 0x0B,
    SY8809_NTC_LEVEL_OVER_60 = 0x0C,
    SY8809_NTC_LEVEL_BELOW_MINUS_10 = 0x0F,
    SY8809_NTC_LEVEL_UNKNOW = 0xFF
} Sy8809_NtcLevel_t;

typedef enum
{
    SY8809_BUD_CHARGE_STATE_UNKNOW = 0,
    SY8809_BUD_CHARGE_STATE_CHARGING,
    SY8809_BUD_CHARGE_STATE_COMPLETE,
    SY8809_BUD_CHARGE_STATE_TABLE4_COMPLETE
} Sy8809_BudsChargeStatus_t;

typedef enum
{
    SY8809_CASE_CHARGE_STATUS_NO_CHARGING = 0x00,   // No charging (VIN < VIN_UVLO)
    SY8809_CASE_CHARGE_STATUS_TRICKLE = 0x01,       // Trickle charging
    SY8809_CASE_CHARGE_STATUS_CONSTANT_CURR = 0x02, // Constant current charging
    SY8809_CASE_CHARGE_STATUS_CHARGE_DONE = 0x03,   // Charging complete (battery full)
    SY8809_CASE_CHARGE_STATUS_UNKNOW = 0xFF
} Sy8809_CaseChargeStatus_t;

typedef struct
{
    uint8_t reg_0x10;
    uint8_t reg_0x11;
    uint8_t reg_0x12;
    uint8_t reg_0x13;
    uint8_t reg_0x14;
    uint8_t reg_0x15;
    uint8_t reg_0x16;
    uint8_t reg_0x17;
    uint8_t reg_0x22;
} Sy8809_RegStateCheck_t;

typedef struct
{
    Sy8809_NtcLevel_t ntc_level;
    Sy8809_Table_t current_table;
    Sy8809_BudsChargeStatus_t left_bud_charge_status;
    Sy8809_BudsChargeStatus_t right_bud_charge_status;
    Sy8809_CaseChargeStatus_t case_charge_status;
    Sy8809_RegStateCheck_t check_reg_state;
} Sy8809_ChargeStatus_t;

typedef struct
{
    const uint8_t (*tbl)[2];
    Sy8809_Table_t table_id;
} Sy8809_TableMap_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
