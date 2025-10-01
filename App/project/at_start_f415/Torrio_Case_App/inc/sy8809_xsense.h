
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    SY8809_XSENSE_NULL,
    SY8809_XSENSE_NTC,
    SY8809_XSENSE_IBAT,
    SY8809_XSENSE_IVOR,
    SY8809_XSENSE_IVOL,
    SY8809_XSENSE_IVIN,
    SY8809_XSENSE_VBAT,
    SY8809_XSENSE_VBIN
} Sy8809Xsense_OutputItem_t;

typedef enum
{
    XSENSE_GAIN_1 = 0,
    XSENSE_GAIN_2 = 1,
    XSENSE_GAIN_4 = 2,
    XSENSE_GAIN_8 = 3,
} Sy8809Xsense_XsenseConfig_XsenseGain_t;

typedef enum
{
    XSENSE_CH_IVOL = 0x00,
    XSENSE_CH_IVOR = 0x01,
    XSENSE_CH_RSVD2 = 0x02,
    XSENSE_CH_IBAT = 0x03,
    XSENSE_CH_NTC = 0x04,
    XSENSE_CH_VIN_DIV8 = 0x05,
    XSENSE_CH_VBAT_DIV4 = 0x06,
    XSENSE_CH_IVIN = 0x07,
    XSENSE_CH_PMID_DIV8 = 0x08,
    XSENSE_CH_DISABLED = 0x0F,
} Sy8809Xsense_XsenseConfig_ChSel_t;

typedef enum
{
    ENXSENSE_DISABLED = 0x00,
    ENXSENSE_ENABLED = 0x40,
} Sy8809Xsense_XsenseConfig_EnXsense_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void Sy8809Xsense_TrigXsenseConv(void);
void Sy8809Xsense_SetPendingXsense(Sy8809Xsense_OutputItem_t Pending);
void Sy8809Xsense_ReadXsenseProcess(void);
