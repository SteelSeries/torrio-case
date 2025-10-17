/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "sy8809.h"
#include "init_pinout.h"
#include "i2c1.h"
#include "task_scheduler.h"
#include "timer2.h"
#include "usb.h"
#include "lid.h"
#include "system_state_manager.h"
#include "custom_hid_class.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define NUM_CHECK_REGS ARRAY_SIZE(current_table_check_list)
#define SY8809_REG_0x16_NTC_MASK 0x0FU // Mask for bits [3:0] indicating NTC level

/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static const uint8_t sy8809_reg_table3_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_table4_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_table5H_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_table6_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_tableA_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_tableB_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_tableCEI_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_tableDFJ_list[SY8809_REG_TABLE_LEN][2] = {
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

static const uint8_t sy8809_reg_tableG_list[SY8809_REG_TABLE_LEN][2] = {
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

static Sy8809_HardwareSettings_t user_hardware_settings = {0};
static const Sy8809_TableMap_t table_map[] = {
    {sy8809_reg_table3_list, SY8809_REG_TABLE_3},
    {sy8809_reg_table4_list, SY8809_REG_TABLE_4},
    {sy8809_reg_table6_list, SY8809_REG_TABLE_6},
    {sy8809_reg_tableA_list, SY8809_REG_TABLE_A},
    {sy8809_reg_tableCEI_list, SY8809_REG_TABLE_CEI},
    {sy8809_reg_tableDFJ_list, SY8809_REG_TABLE_DFJ},
    {sy8809_reg_table5H_list, SY8809_REG_TABLE_5H},
    {sy8809_reg_tableG_list, SY8809_REG_TABLE_G},
};

static const Sy8809_SettingTableList_t current_table_check_list[] = {
    SY8809_0x22,
    SY8809_0x26,
    SY8809_0x27,
    SY8809_0x36,
};

static const uint8_t current_table_check_addrs[NUM_CHECK_REGS] = {
    SY8809_REG_0x22,
    SY8809_REG_0x26,
    SY8809_REG_0x27,
    SY8809_REG_0x36,
};

static Sy8809_ChargeStatus_t ChargeIcStatusInfo = {
    .check_reg_state = {0},
    .left_bud_charge_status = SY8809_BUD_CHARGE_STATE_UNKNOW,
    .right_bud_charge_status = SY8809_BUD_CHARGE_STATE_UNKNOW,
    .current_table = SY8809_REG_UNKNOWN,
    .ntc_level = SY8809_NTC_LEVEL_UNKNOW,
};
/*
 * Flag set by the Charge IC IRQ interrupt.
 *
 * When the external interrupt (IRQ pin) from the Charge IC is triggered,
 * this flag will be set to true. The main loop should check this flag
 * periodically to determine when it needs to read and process the
 * updated charging status from the Charge IC.
 */
static volatile bool charge_irq_flag = false;

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static flag_status GetSdaState(void);
static bool match_table(const uint8_t tbl[][2], const uint8_t *read_values);
static void DetectCurrentTable(void);
static void ConfigBudDetectResistPin(confirm_state enable);
static void StartChipModeCheck(void);
static void ReadVbatProcess(void);
static void ReadNtcProcess(void);
static void StartWorkTask(void);
static void UpdateTableByPowerSource(void);
static void UpdateStatusRegisters(void);
static void CheckNtcOverTempe(void);
static void CheckBudsChargeStatus(void);
static void UsbModeApplyTable(void);
static void NormalModeApplyTable(void);
static void SettingRegTable5H(void);
static void SettingRegTable3(void);
static void SettingRegTable4(void);
static void SettingRegTableA(void);
static void SettingRegTableB(void);
static void SettingRegTableCEI(void);
static void SettingRegTableDFJ(void);
static void SettingRegTable6(void);
static void SettingRegTableG(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Sy8809_InitTask(void)
{
    printf("%d start Sy8809 init\n", Timer2_GetTick());

    if (GetSdaState() == SET)
    {
        printf("sy8809 SDA state High power on comple\n");
        InitPinout_I2c1Init();
        if (TaskScheduler_AddTask(StartChipModeCheck, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
        {
            printf("add sy8809 check chip task fail\n");
        }
    }
    else
    {
        printf("check SDA IO is low entery retry\n");
        if (TaskScheduler_AddTask(Sy8809_InitTask, 100, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
        {
            printf("add retry sy8809 task fail\n");
        }
        // Todo: sy8809 setup fail need reset function.
    }
}

void Sy8809_GpioConfigHardware(const Sy8809_HardwareSettings_t *hardware_settings)
{
    gpio_init_type gpio_init_struct;
    exint_init_type exint_init_struct;

    memcpy(&user_hardware_settings, hardware_settings, sizeof(Sy8809_HardwareSettings_t));

    crm_periph_clock_enable(user_hardware_settings.sy8809_sda_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.busd_detect_resist_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.sy8809_irq_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;

    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = user_hardware_settings.busd_detect_resist_gpio_pin;
    gpio_init(user_hardware_settings.busd_detect_resist_gpio_port, &gpio_init_struct);
    gpio_bits_write(user_hardware_settings.busd_detect_resist_gpio_port, user_hardware_settings.busd_detect_resist_gpio_pin, FALSE);

    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = user_hardware_settings.sy8809_sda_gpio_pin;
    gpio_init(user_hardware_settings.sy8809_sda_gpio_port, &gpio_init_struct);

    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init_struct.gpio_pins = user_hardware_settings.sy8809_irq_gpio_pin;
    gpio_init(user_hardware_settings.sy8809_irq_gpio_port, &gpio_init_struct);

    gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOA, GPIO_PINS_SOURCE4);

    exint_default_para_init(&exint_init_struct);
    exint_init_struct.line_enable = TRUE;
    exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
    exint_init_struct.line_select = EXINT_LINE_4;
    exint_init_struct.line_polarity = EXINT_TRIGGER_FALLING_EDGE;
    exint_init(&exint_init_struct);
    nvic_irq_enable(EXINT4_IRQn, 1, 0);
}

void Sy8809_ReadIrqState(void)
{
    charge_irq_flag = true;
}

void Sy8809_SetChargeIcStatusInfo(Sy8809_ChargeStatus_t *ChargeIcStatusInfoTemp)
{
    memcpy(&ChargeIcStatusInfo, ChargeIcStatusInfoTemp, sizeof(Sy8809_ChargeStatus_t));
}

void Sy8809_GetChargeIcStatusInfo(Sy8809_ChargeStatus_t *ChargeIcStatusInfoTemp)
{
    memcpy(ChargeIcStatusInfoTemp, &ChargeIcStatusInfo, sizeof(Sy8809_ChargeStatus_t));
}

i2c_status_type Sy8809_DebugRegWrite(const uint8_t reg, const uint8_t value)
{
    return I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS, reg, value);
}

void Sy8809_DebugRegRead(const uint8_t reg, uint8_t *buff)
{
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, reg, buff);
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static flag_status GetSdaState(void)
{
    return gpio_input_data_bit_read(user_hardware_settings.sy8809_sda_gpio_port, user_hardware_settings.sy8809_sda_gpio_pin);
}

static bool match_table(const uint8_t tbl[][2], const uint8_t *read_values)
{
    for (size_t i = 0; i < NUM_CHECK_REGS; ++i)
    {
        Sy8809_SettingTableList_t idx = current_table_check_list[i];
        if (tbl[idx][1] != read_values[i])
        {
            return false;
        }
    }
    return true;
}

static void DetectCurrentTable(void)
{
    uint8_t sy8809_reg_rx_buff[1] = {0};
    uint8_t read_values[NUM_CHECK_REGS] = {0};
    for (uint8_t i = 0; i < NUM_CHECK_REGS; i++)
    {
        I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, current_table_check_addrs[i], sy8809_reg_rx_buff);
        read_values[i] = sy8809_reg_rx_buff[0];
    }

    printf("8809 read: 22:%02X 26:%02X 27:%02X 36:%02X\n",
           read_values[0],
           read_values[1],
           read_values[2],
           read_values[3]);

    ChargeIcStatusInfo.current_table = SY8809_REG_UNKNOWN;

    for (uint8_t t = 0; t < ARRAY_SIZE(table_map); ++t)
    {
        if (match_table(table_map[t].tbl, read_values))
        {
            ChargeIcStatusInfo.current_table = table_map[t].table_id;
            break;
        }
    }

    printf("judge table:%d\n", ChargeIcStatusInfo.current_table);
}

static void ConfigBudDetectResistPin(confirm_state enable)
{
    // if (first_start_state != WDT_WAKE_UP)
    // {
    //     if (gpio_output_data_bit_read(user_hardware_settings.busd_detect_resist_gpio_port, user_hardware_settings.busd_detect_resist_gpio_pin) != enable)
    //     {
    //         gpio_bits_write(user_hardware_settings.busd_detect_resist_gpio_port, user_hardware_settings.busd_detect_resist_gpio_pin, enable)
    //     }
    // }
    // else
    // {
    gpio_bits_write(user_hardware_settings.busd_detect_resist_gpio_port, user_hardware_settings.busd_detect_resist_gpio_pin, FALSE);
    // }
}

static void StartChipModeCheck(void)
{
    printf("%d sy8809 check chip\n", Timer2_GetTick());
    uint8_t sy8809_reg_rx_buff[1] = {0};
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x15, sy8809_reg_rx_buff);
    if ((sy8809_reg_rx_buff[0] & REG_BIT(0)) == SET_REG_BIT(0))
    {
        printf("check 0x15 is start woking\n");
        DetectCurrentTable();

        if ((ChargeIcStatusInfo.current_table == SY8809_REG_TABLE_4) ||
            (ChargeIcStatusInfo.current_table == SY8809_REG_UNKNOWN) ||
            (Usb_GetUsbDetectState() == USB_PLUG))
        {
            SettingRegTable5H();
            if (TaskScheduler_AddTask(ReadVbatProcess, 100, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add sy8809 delay 100ms read vbat task fail\n");
            }
        }
        else
        {
            if (TaskScheduler_AddTask(ReadVbatProcess, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                printf("add sy8809 read vbat task fail\n");
            }
        }
    }
    else
    {
        // // check 0x15 chip reg state bit 0 not is 1, so wait 2.5 Sec retry this reg.
        if (TaskScheduler_AddTask(StartChipModeCheck, 2500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
        {
            printf("add retry sy8809 check chip task fail\n");
        }
    }
}

static void ReadVbatProcess(void)
{
    printf("[%s]\n", __func__);
    // Todo: read vbat Process
    if (TaskScheduler_AddTask(ReadNtcProcess, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        printf("add sy8809 read vbat task fail\n");
    }
}
static void ReadNtcProcess(void)
{
    printf("[%s]\n", __func__);

    // Todo: read NTC Process
    if (TaskScheduler_AddTask(StartWorkTask, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
    {
        printf("add sy8809 read vbat task fail\n");
    }

    if (Lid_GetState() == LID_CLOSE)
    {
        if (Usb_FirstSetupUsbState() == USB_UNPLUG)
        {
            if (TaskScheduler_AddTask(SystemStateManager_EnterStandbyModeCheck, 10, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
            {
                printf("add enter standby task fail\n");
            }
        }
    }
}

static void StartWorkTask(void)
{
    if (charge_irq_flag == true)
    {
        printf("charge_irq_flag detected \n");
        charge_irq_flag = false;
        UpdateTableByPowerSource();
    }

    if (TaskScheduler_AddTask(StartWorkTask, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add sy8809 working task fail\n");
    }
}

static void UpdateTableByPowerSource(void)
{
    UpdateStatusRegisters();

    CheckNtcOverTempe();
    CheckBudsChargeStatus();

    if (Usb_GetUsbDetectState() == USB_PLUG)
    {
        printf("USB table check\n");
        UsbModeApplyTable();
    }
    // Todo: check Qi chatge whether connect.
    // else if (QI_Charge_state == QI_CONTACT)
    // {
    //     printf("Qi table check\n");
    //     sy8809_QiInCheckIccTable();
    // }
    else
    {
        printf("normal table check\n");
        NormalModeApplyTable();
    }
}

/**
 * @brief Reads SY8809 status registers after an interrupt
 *        and stores them into ChargeIcStatusInfo.check_reg_state.
 *
 * This function should be called immediately after the SY8809 IRQ
 * to capture the current interrupt and status register values.
 * It does not process or interpret the values;
 * other modules should handle state evaluation.
 */
static void UpdateStatusRegisters(void)
{
    uint8_t sy8809_reg_rx_buff[1] = {0};
    printf("start read 8809 int state\n");
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x12, sy8809_reg_rx_buff);
    ChargeIcStatusInfo.check_reg_state.reg_0x12 = sy8809_reg_rx_buff[0];
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x13, sy8809_reg_rx_buff);
    ChargeIcStatusInfo.check_reg_state.reg_0x13 = sy8809_reg_rx_buff[0];
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x15, sy8809_reg_rx_buff);
    ChargeIcStatusInfo.check_reg_state.reg_0x15 = sy8809_reg_rx_buff[0];
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x14, sy8809_reg_rx_buff);
    ChargeIcStatusInfo.check_reg_state.reg_0x14 = sy8809_reg_rx_buff[0];
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x16, sy8809_reg_rx_buff);
    ChargeIcStatusInfo.check_reg_state.reg_0x16 = sy8809_reg_rx_buff[0];

    uint8_t sy8809_debug_read_reg_table[] = {0x10, 0x11, 0x12, 0x13, 0x14,
                                             0x15, 0x16, 0x17, 0x20, 0x21,
                                             0x22, 0x23, 0x24, 0x25, 0x26,
                                             0x27, 0x30, 0x31, 0x32, 0x33,
                                             0x34, 0x35, 0x36, 0x37, 0x40,
                                             0x41, 0x42, 0x43, 0x44, 0x4F};
    printf("sy8809 debug ");
    for (size_t i = 0; i < sizeof(sy8809_debug_read_reg_table); i++)
    {
        I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, sy8809_debug_read_reg_table[i], sy8809_reg_rx_buff);
        printf("%02X:%02X ", sy8809_debug_read_reg_table[i], sy8809_reg_rx_buff[0]);
    }
    printf("\n");
}

static void CheckNtcOverTempe(void)
{
    ChargeIcStatusInfo.ntc_level = (Sy8809_NtcLevel_t)(ChargeIcStatusInfo.check_reg_state.reg_0x16 & SY8809_REG_0x16_NTC_MASK);

    if ((ChargeIcStatusInfo.ntc_level == SY8809_NTC_LEVEL_OVER_60) ||
        (ChargeIcStatusInfo.ntc_level == SY8809_NTC_LEVEL_BELOW_MINUS_10))
    {
        SettingRegTable6();
        if ((Lid_GetState() == LID_OPEN) &&
            (Usb_GetUsbDetectState() == USB_UNPLUG))
        {
            // Todo: over tempetrue system enter suspend state.
        }
    }
    else
    {
        if (ChargeIcStatusInfo.current_table == SY8809_REG_TABLE_6)
        {
            SettingRegTable5H();
        }
    }
}

static void CheckBudsChargeStatus(void)
{
    if (ChargeIcStatusInfo.current_table == SY8809_REG_TABLE_4)
    {
        ChargeIcStatusInfo.left_bud_charge_status = SY8809_BUD_CHARGE_STATE_TABLE4_COMPLETE;
        ChargeIcStatusInfo.right_bud_charge_status = SY8809_BUD_CHARGE_STATE_TABLE4_COMPLETE;
    }
    else
    {
        if ((ChargeIcStatusInfo.check_reg_state.reg_0x14 & REG_BIT(4)) == 0) // Check VOL loading    1 = Complete  0 = Charging
        {
            ChargeIcStatusInfo.left_bud_charge_status = SY8809_BUD_CHARGE_STATE_CHARGING;
        }
        else
        {
            ChargeIcStatusInfo.left_bud_charge_status = SY8809_BUD_CHARGE_STATE_COMPLETE;
        }

        if ((ChargeIcStatusInfo.check_reg_state.reg_0x14 & REG_BIT(5)) == 0) // Check VOR loading    1 = Complete  0 = Charging
        {
            ChargeIcStatusInfo.right_bud_charge_status = SY8809_BUD_CHARGE_STATE_CHARGING;
        }
        else
        {
            ChargeIcStatusInfo.right_bud_charge_status = SY8809_BUD_CHARGE_STATE_COMPLETE;
        }
    }

    printf("charge stage L:%d R:%d 0x14:%02X\n",
           ChargeIcStatusInfo.left_bud_charge_status,
           ChargeIcStatusInfo.right_bud_charge_status,
           ChargeIcStatusInfo.check_reg_state.reg_0x14);
}

static void UsbModeApplyTable(void)
{
    switch (ChargeIcStatusInfo.ntc_level)
    {

    case SY8809_NTC_LEVEL_0_TO_10:
    {
        SettingRegTableCEI();
        break;
    }

    case SY8809_NTC_LEVEL_45_TO_60:
    {
        SettingRegTableDFJ();
        break;
    }

    case SY8809_NTC_LEVEL_10_TO_20:
    case SY8809_NTC_LEVEL_20_TO_45:
    {

        if (((ChargeIcStatusInfo.check_reg_state.reg_0x14 & REG_BIT(4)) == 0) ||
            ((ChargeIcStatusInfo.check_reg_state.reg_0x14 & REG_BIT(5)) == 0))
        {
            SettingRegTableB();
        }
        else
        {
            SettingRegTableA();
        }
        break;
    }
    }
}

static void NormalModeApplyTable(void)
{
    // if ((NTC_over_tempe_alarm == false) && (Case_VBAT_level != BATTERY_LEVEL_POWEROFF))
    // {
    if (Usb_GetUsbDetectState() == USB_UNPLUG)
    // (first_start_state == WDT_WAKE_UP))
    {
        if (Lid_GetState() == LID_OPEN)
        {
            SettingRegTable3();
        }
        else
        {
            if ((ChargeIcStatusInfo.left_bud_charge_status == SY8809_BUD_CHARGE_STATE_CHARGING) || (ChargeIcStatusInfo.right_bud_charge_status == SY8809_BUD_CHARGE_STATE_CHARGING))
            {
                SettingRegTable3();
            }
            else if ((ChargeIcStatusInfo.left_bud_charge_status == SY8809_BUD_CHARGE_STATE_COMPLETE) && (ChargeIcStatusInfo.right_bud_charge_status == SY8809_BUD_CHARGE_STATE_COMPLETE))
            {
                SettingRegTable4();
            }
        }
    }
    else
    {
        SettingRegTable3();
    }
    // }
}

static void SettingRegTable5H(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_5H)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_5H;
        printf("table5H\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_table5H_list[i][0],
                          sy8809_reg_table5H_list[i][1]);
        }
    }
}

static void SettingRegTable3(void)
{
    if ((Usb_GetUsbDetectState() == USB_PLUG) ||
        (Lid_GetState() == LID_OPEN))
    // (first_start_state == NORMAL_START))
    {
        ConfigBudDetectResistPin(TRUE);
        printf("set table 3 PC1 to H\n");
    }

    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_3)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_3;
        printf("table3\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_table3_list[i][0],
                          sy8809_reg_table3_list[i][1]);
        }
    }
}

static void SettingRegTable4(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_4)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_4;
        printf("table4\n");

        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_table4_list[i][0],
                          sy8809_reg_table4_list[i][1]);
        }
        ConfigBudDetectResistPin(FALSE);
    }
}

static void SettingRegTableA(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_A)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_A;
        printf("tableA\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_tableA_list[i][0],
                          sy8809_reg_tableA_list[i][1]);
        }
    }
}

static void SettingRegTableB(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_B)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_B;
        printf("tableB\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_tableB_list[i][0],
                          sy8809_reg_tableB_list[i][1]);
        }
    }
}

static void SettingRegTableCEI(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_CEI)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_CEI;
        printf("tableCEI\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_tableCEI_list[i][0],
                          sy8809_reg_tableCEI_list[i][1]);
        }
    }
}

static void SettingRegTableDFJ(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_DFJ)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_DFJ;
        printf("tableDFJ\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_tableDFJ_list[i][0],
                          sy8809_reg_tableDFJ_list[i][1]);
        }
    }
}

static void SettingRegTable6(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_6)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_6;
        printf("table6\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_table6_list[i][0],
                          sy8809_reg_table6_list[i][1]);
        }
    }
}

static void SettingRegTableG(void)
{
    if (ChargeIcStatusInfo.current_table != SY8809_REG_TABLE_G)
    {
        ChargeIcStatusInfo.current_table = SY8809_REG_TABLE_G;
        printf("tableG\n");
        ConfigBudDetectResistPin(TRUE);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_tableG_list[i][0],
                          sy8809_reg_tableG_list[i][1]);
        }
    }
}
