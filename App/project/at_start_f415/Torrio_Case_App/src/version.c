/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "version.h"
#include <stdio.h>
#include <string.h>

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#ifndef VERSION_STR_MAX_LEN
#define VERSION_STR_MAX_LEN 9
#endif
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
// static bool format_as_bytes_dotted(uint32_t val, uint8_t *buf, size_t bufsz);
// static int write_dec_min2_uint8(uint8_t *dst, size_t dstsz, unsigned v);
static int write_hex2_uint8(uint8_t *dst, size_t dstsz, unsigned v);
static bool format_as_bytes_dotted_hex(uint32_t val, uint8_t *buf, size_t bufsz);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Version_GetArteryVersion(uint8_t * buff, uint8_t buff_len)
{
    format_as_bytes_dotted_hex(ARTERY_FW_VERSION, buff, buff_len);
}

void Version_GetStrVersion(uint8_t * data, uint8_t * buff, uint8_t buff_len)
{
    uint32_t ver_temp = 0;
    ver_temp = (uint32_t)data[2];
    ver_temp |= (uint32_t)(data[1] << 8);
    ver_temp |= (uint32_t)(data[0] << 16);
    format_as_bytes_dotted_hex(ver_temp, buff, buff_len);
 }
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static int write_hex2_uint8(uint8_t *dst, size_t dstsz, unsigned v)
{
    if (dst == NULL || dstsz < 2 || v > 0xFFU) return 0;
    const char hex[] = "0123456789ABCDEF";
    dst[0] = (uint8_t)hex[(v >> 4) & 0xF];
    dst[1] = (uint8_t)hex[v & 0xF];
    return 2;
}

static bool format_as_bytes_dotted_hex(uint32_t val, uint8_t *buf, size_t bufsz)
{
    if (buf == NULL || bufsz < VERSION_STR_MAX_LEN) return false;

    uint8_t b2 = (uint8_t)((val >> 16) & 0xFF);
    uint8_t b1 = (uint8_t)((val >> 8) & 0xFF);
    uint8_t b0 = (uint8_t)(val & 0xFF);

    uint8_t *p = buf;
    size_t rem = bufsz;

    int n = write_hex2_uint8(p, rem, b2);
    if (n == 0) return false;
    p += n; rem -= n;

    if (rem < 1) return false;
    *p++ = (uint8_t)'.'; rem--;

    n = write_hex2_uint8(p, rem, b1);
    if (n == 0) return false;
    p += n; rem -= n;

    if (rem < 1) return false;
    *p++ = (uint8_t)'.'; rem--;

    n = write_hex2_uint8(p, rem, b0);
    if (n == 0) return false;
    p += n; rem -= n;

    if (rem < 1) return false;
    *p = (uint8_t)'\0';
    return true;
}
