
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"
#include "uart_command_queue.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define BUD_IO_NUM 3U // has 3 button on each ear
#define BUDS_UNKNOW_STATE 0xFFU
#define BUDS_VERSTION_DATA_LEN 3U
#define BUDS_ANC_VERSTION_DATA_LEN 5U
#define BUDS_SN_DATA_LEN 19U
/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/
typedef enum
{
    UART_STATE_IDLE,
    UART_STATE_SENDING,
    UART_STATE_WAITING_RESPONSE,
    UART_STATE_PROCESSING,
    UART_STATE_TIMEOUT,
    UART_STATE_ERROR
} Uart_State_t;

typedef enum
{
    UART_BUD_LEFT = 0,
    UART_BUD_RIGHT,
    UART_BUD_UNKNOWN
} Uart_BudSide_t;

typedef enum
{
    UART_BUDS_CONNT_UNKNOW = 0,
    UART_BUDS_CONNT_DISCONNECT,
    UART_BUDS_CONNT_CONNECT
} Uart_BudsConnectState_t;

typedef enum
{
    UART_BUDS_IO_DISCONNECT = 0,
    UART_BUDS_IO_CONNECT,
    UART_BUDS_IO_UNKNOW,
} Uart_BudsIoState_t;

typedef enum
{
    UART_BUDS_WORK_MODE_APP = 0xAA,
    UART_BUDS_WORK_MODE_BOOTLOADER = 0xBB,
    UART_BUDS_WORK_MODE_FACTORY = 0xFA,
    UART_BUDS_WORK_MODE_UNKNOW = BUDS_UNKNOW_STATE,
} Uart_BudsWorkMode_t;

typedef enum
{
    UART_BUDS_BUTTON_IO_LOW = 0,
    UART_BUDS_BUTTON_IO_HIGH,
    UART_BUDS_BUTTON_IO_UNKNOW = BUDS_UNKNOW_STATE,
} Uart_BudsButtonIoState_t;

typedef struct
{
    usart_type *uart;
    UartCommandQueue_Queue_t cmd_queue;
    Uart_State_t state;
    uint32_t timeout_tick;
    uint16_t current_timeout_ms;
    uint8_t tx_buffer[CMD_MAX_DATA_LEN];
    uint16_t tx_len;
    uint8_t rx_buffer[CMD_MAX_DATA_LEN];
    uint16_t rx_index;
    uint16_t expected_len; // Expected total length of the incoming packet
    bool sync_detected;    // Indicates whether the CMD_SYNC_BYTE (start byte) has been detected
    bool packet_ready;     // Set to true when a complete packet has been received in the interrupt
    uint8_t retry_count;
    uint8_t tx_seqn;
    Uart_BudSide_t side; // Indicates which bud this context belongs to (Left or Right)
    uint8_t command_id;
    bool direct_mode;
    bool direct_pending;
    uint8_t *direct_data;
    uint16_t direct_len;
    uint16_t direct_event_id;
    uint32_t direct_timeout_ms;

    Uart_BudsConnectState_t Connect;
    Uart_BudsIoState_t detect_state;
    Uart_BudsIoState_t detect_state_pre;
    uint16_t detect_debounce;

    Uart_BudsWorkMode_t mode;
    Uart_BudsButtonIoState_t button_io_state[BUD_IO_NUM];

    uint8_t Color_Spin;
    uint8_t mode_type;
    uint8_t Version_Headset_Partion[BUDS_VERSTION_DATA_LEN];
    uint8_t dsp2_version[BUDS_VERSTION_DATA_LEN];
    uint8_t anc_version_buffer[BUDS_ANC_VERSTION_DATA_LEN];
    uint8_t serial_number_buffer[BUDS_SN_DATA_LEN];

    uint8_t battery_level;
    uint16_t vbat;
    uint16_t ntc;
} UART_CommContext_t;
/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
void UartCommManager_Init(void);
void UartCommManager_DisconnectReinit(UART_CommContext_t *ctx);
void UartCommManager_RunningTask(void);
UART_CommContext_t *UartCommManager_GetLeftBudContext(void);
UART_CommContext_t *UartCommManager_GetRightBudContext(void);
