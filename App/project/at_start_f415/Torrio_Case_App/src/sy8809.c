/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "sy8809.h"
#include "sy8809_table.h"
#include "init_pinout.h"
#include "i2c1.h"
#include "task_scheduler.h"
#include "timer2.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define NUM_CHECK_REGS ARRAY_SIZE(check_regs)
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/

typedef struct
{
    const uint8_t (*tbl)[2];
    int table_id; /* 對應 sy8809_current_table 的 enum */
} sy8809_table_t;

/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static Sy8809_HardwareSettings_t user_hardware_settings = {0};
static const sy8809_table_t table_map[] = {
    {sy8809_reg_table3_list, SY8809_REG_TABLE_3},
    {sy8809_reg_table4_list, SY8809_REG_TABLE_4},
    {sy8809_reg_table6_list, SY8809_REG_TABLE_6},
    {sy8809_reg_tableA_list, SY8809_REG_TABLE_A},
    {sy8809_reg_tableCEI_list, SY8809_REG_TABLE_CEI},
    {sy8809_reg_tableDFJ_list, SY8809_REG_TABLE_DFJ},
    {sy8809_reg_table5H_list, SY8809_REG_TABLE_5H},
    {sy8809_reg_tableG_list, SY8809_REG_TABLE_G},
};

static const setting_8809_table_list check_regs[] = {
    SY8809_0x22,
    SY8809_0x26,
    SY8809_0x27,
    SY8809_0x36,
};

static const uint8_t check_reg_addrs[NUM_CHECK_REGS] = {
    SY8809_REG_0x22,
    SY8809_REG_0x26,
    SY8809_REG_0x27,
    SY8809_REG_0x36,
};

static uint8_t sy8809_current_table = SY8809_REG_UNKNOWN;

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
static void DetectCurrentTable(void);
static bool match_table(const uint8_t tbl[][2], const uint8_t *read_values);
static void SettingRegTable5H(void);
static void ConfigBudDetectResistPin(confirm_state enable);
static void StartChipModeCheck(void);
static void ReadVbatProcess(void);
static void ReadNtcProcess(void);
static void StartWorkTask(void);

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
        // todo: sy8809 setup fail need reset function.
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
    exint_init_struct.line_select = EXINT_LINE_0;
    exint_init_struct.line_polarity = EXINT_TRIGGER_FALLING_EDGE;
    exint_init(&exint_init_struct);
    nvic_irq_enable(EXINT0_IRQn, 1, 0);
}

void Sy8809_ReadIrqState(void)
{
    charge_irq_flag = true;
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
        setting_8809_table_list idx = check_regs[i];
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
        I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, check_reg_addrs[i], sy8809_reg_rx_buff);
        read_values[i] = sy8809_reg_rx_buff[0];
    }

    printf("8809 read: 22:%02X 26:%02X 27:%02X 36:%02X\n",
           read_values[0],
           read_values[1],
           read_values[2],
           read_values[3]);

    sy8809_current_table = SY8809_REG_UNKNOWN;

    for (uint8_t t = 0; t < ARRAY_SIZE(table_map); ++t)
    {
        if (match_table(table_map[t].tbl, read_values))
        {
            sy8809_current_table = table_map[t].table_id;
            break;
        }
    }

    printf("judge table:%d\n", sy8809_current_table);
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

static void SettingRegTable5H(void)
{
    if (sy8809_current_table != SY8809_REG_TABLE_5H)
    {
        sy8809_current_table = SY8809_REG_TABLE_5H;
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

static void StartChipModeCheck(void)
{
    printf("%d sy8809 check chip\n", Timer2_GetTick());
    uint8_t sy8809_reg_rx_buff[1] = {0};
    I2c1_ReadReg(SY8809_I2C_SLAVE_ADDRESS, SY8809_REG_0x15, sy8809_reg_rx_buff);
    if ((sy8809_reg_rx_buff[0] & REG_BIT(0)) == SET_REG_BIT(0))
    {
        printf("check 0x15 is start woking\n");
        DetectCurrentTable();

        // todo: need check USB connect state
        // if ((sy8809_current_table == SY8809_REG_TABLE_4) ||
        //     (sy8809_current_table == SY8809_REG_UNKNOWN) ||
        //     (USB_State == USB_PLUG))

        if ((sy8809_current_table == SY8809_REG_TABLE_4) ||
            (sy8809_current_table == SY8809_REG_UNKNOWN))
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
}

uint8_t temp_test_count = 0;
static void StartWorkTask(void)
{
    if (temp_test_count > 100)
    {
        temp_test_count = 0;
        printf("%d [%s]\n", Timer2_GetTick(), __func__);
    }
    temp_test_count++;

    if (charge_irq_flag == true)
    {
        charge_irq_flag = false;
        printf("charge_irq_flag detected \n");
    }

    if (TaskScheduler_AddTask(StartWorkTask, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add sy8809 working task fail\n");
    }
}