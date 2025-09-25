
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
} Sy8809_Reg_Table_t;

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
} setting_8809_table_list;

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
    // SY8809_REG_TABLE_E,
    // SY8809_REG_TABLE_F,
    // SY8809_REG_TABLE_H,
    // SY8809_REG_TABLE_I,
    // SY8809_REG_TABLE_J
} SY8809_REG_TABLE;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/
const uint8_t sy8809_reg_table3_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41}, // IINDPM = 0.5A, VDPPM = 4.7V
    {SY8809_REG_0x21, 0x06}, // Vfloat = 4.35V
    {SY8809_REG_0x22, 0x09}, // ICC = 480mA
    {SY8809_REG_0x23, 0x20}, // ITC = 60mA, IEND = 20mA
    {SY8809_REG_0x24, 0x00}, // VSYS_MIN = 3.35V
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00}, // Disable VOX loading detect
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87}, // VPMID = 4.8V, BAT_UVLO = 3.0V, lowBAT_Set = 3.5V
    {SY8809_REG_0x31, 0x00}, // default xsense mode setting need delay 100ms
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F}, // IOFF_Base = 3mA, TD_L2H = 100ms, TD_H2L = 50ms
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10}, // default no JETA    JETA : 0x10    suspend need sleep
    {SY8809_REG_0x37, 0x00}, // 809 internal HW protect setting -> FW no use
    {SY8809_REG_0x44, 0x00}, // default
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_table4_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x02},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x60},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x00},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x20}};

const uint8_t sy8809_reg_table5H_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x02},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_table6_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x02},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x60},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x20}};

const uint8_t sy8809_reg_tableA_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x1C},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_tableB_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x09},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_tableCEI_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x0C},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_tableDFJ_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x06},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

const uint8_t sy8809_reg_tableG_list[SY8809_REG_TABLE_LEN][2] = {
    {SY8809_REG_0x20, 0x41},
    {SY8809_REG_0x21, 0x06},
    {SY8809_REG_0x22, 0x17},
    {SY8809_REG_0x23, 0x20},
    {SY8809_REG_0x24, 0x00},
    {SY8809_REG_0x25, 0xE0},
    {SY8809_REG_0x26, 0x00},
    {SY8809_REG_0x2E, 0x00},
    {SY8809_REG_0x2F, 0x00},
    {SY8809_REG_0x30, 0x87},
    {SY8809_REG_0x31, 0x00},
    {SY8809_REG_0x32, 0x00},
    {SY8809_REG_0x33, 0x1F},
    {SY8809_REG_0x34, 0x10},
    {SY8809_REG_0x35, 0x03},
    {SY8809_REG_0x36, 0x10},
    {SY8809_REG_0x37, 0x00},
    {SY8809_REG_0x44, 0x00},
    {SY8809_REG_0x27, 0x27}};

// const uint16_t battery_voltage_table[21] = {3510, 3590, 3610, 3630, 3650,
//                                             3670, 3690, 3710, 3740, 3760,
//                                             3800, 3840, 3880, 3920, 3980,
//                                             4030, 4080, 4140, 4200, 4250,
//                                             CASE_MAX_VBAT};
/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
