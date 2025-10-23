/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "uart_protocol.h"
#include "uart_command_queue.h"
#include <string.h>

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define CMD_SYNC_BYTE 0xAA
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
static uint8_t CalcChecksum(uint8_t *dataPtr, uint16_t len);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
bool UartProtocol_PackCommand(uint16_t event_id, uint8_t *tx_seq,
                              const uint8_t *payload_ptr, uint16_t payload_len,
                              uint8_t *out_buf, uint16_t *out_len)
{
    DEBUG_PRINT("UartProtocol_PackCommand called: event_id=0x%04X, tx_seq=%d, payload_len=%d, *out_len=%d\n",
                event_id, *tx_seq, payload_len, *out_len);

    uint16_t total_len = payload_len + 7;
    if (*out_len < total_len)
    {
        DEBUG_PRINT("Buffer too small: required=%d, available=%d\n", total_len, *out_len);
        return false;
    }

    uint16_t idx = 0;
    out_buf[idx++] = CMD_SYNC_BYTE;
    out_buf[idx++] = *tx_seq;
    out_buf[idx++] = (uint8_t)(payload_len + 2); // len + 2 for event_id
    out_buf[idx++] = (uint8_t)((payload_len + 2) >> 8);
    out_buf[idx++] = (uint8_t)(event_id & 0xFF);
    out_buf[idx++] = (uint8_t)((event_id >> 8) & 0xFF);

    memcpy(&out_buf[idx], payload_ptr, payload_len);
    idx += payload_len;

    out_buf[idx] = CalcChecksum(&out_buf[1], total_len - 2);

    *out_len = total_len;

    DEBUG_PRINT("Packed buffer: ");
    for (uint16_t i = 0; i < total_len; i++)
    {
        DEBUG_PRINT("%02X ", out_buf[i]);
    }
    DEBUG_PRINT("\n");

    (*tx_seq)++;
    DEBUG_PRINT("tx_seq incremented to %d\n", *tx_seq);

    return true;
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t CalcChecksum(uint8_t *dataPtr, uint16_t len)
{
    uint8_t check_sum;

    check_sum = 0;
    while (len)
    {
        check_sum += *dataPtr;
        dataPtr++;
        len--;
    }
    return (uint8_t)(0xff - check_sum + 1); //((~check_sum)+1);
}