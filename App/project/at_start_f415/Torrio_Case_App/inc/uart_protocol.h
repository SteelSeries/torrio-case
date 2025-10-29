
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "uart_command_queue.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define CMD_SYNC_BYTE 0xAA

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    CMD_ONE_WIRE_UART_ACK = 0x3000,
    CMD_ONE_WIRE_UART_DATA,
} UartProtocol_CommandId_t;

typedef struct
{
    uint16_t event_id;
    uint8_t tx_seq;
    uint8_t payload[CMD_MAX_DATA_LEN];
    uint16_t payload_len;
} UartProtocol_Packet_t;

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
bool UartProtocol_PackCommand(uint16_t event_id, uint8_t *tx_seq,
                              const uint8_t *payload_ptr, uint16_t payload_len,
                              uint8_t *out_buf, uint16_t *out_len);
bool UARTProtocol_UnpackCommand(const uint8_t *in_buf, uint16_t in_len, UartProtocol_Packet_t *out_packet);
