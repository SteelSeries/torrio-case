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

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static flag_status GetSdaState(void);
static void DetectCurrentTable(void);
static bool match_table(const uint8_t tbl[][2], const uint8_t *read_values);
static void SettingRegTable5H(void);

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
        if (TaskScheduler_AddTask(Sy8809_StartChipModeCheck, 0, TASK_RUN_ONCE, TASK_START_IMMEDIATE) != TASK_OK)
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

void Sy8809_StartChipModeCheck(void)
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
            // sy8809_stage_wait_timer = SY8809_INIT_STAGE_WAIT_TIME;
            // sy8809_delay_init_state = SY8809_INIT_STAPE_3;
        }
        else
        {
            // sy8809_delay_init_state = SY8809_INIT_STAPE_4;
        }
        // todo:sy8809 init comple can start work, add sy8809 start working task.
    }
    else
    {
        // // check 0x15 chip reg state bit 0 not is 1, so wait 2.5 Sec retry this reg.
        if (TaskScheduler_AddTask(Sy8809_StartChipModeCheck, 2500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
        {
            printf("add retry sy8809 check chip task fail\n");
        }
    }
}

void Sy8809_GpioConfigHardware(const Sy8809_HardwareSettings_t *hardware_settings)
{
    gpio_init_type gpio_init_struct;
    memcpy(&user_hardware_settings, hardware_settings, sizeof(Sy8809_HardwareSettings_t));

    crm_periph_clock_enable(user_hardware_settings.sy8809_sda_gpio_crm_clk, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = user_hardware_settings.sy8809_sda_gpio_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init(user_hardware_settings.sy8809_sda_gpio_port, &gpio_init_struct);
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

static void SettingRegTable5H(void)
{
    if (sy8809_current_table != SY8809_REG_TABLE_5H)
    {
        sy8809_current_table = SY8809_REG_TABLE_5H;
        printf("table5H\n");
        // config_bud_detect_resist_pin(Bit_SET);
        for (uint8_t i = 0; i < SY8809_REG_TABLE_LEN; i++)
        {
            I2c1_WriteReg(SY8809_I2C_SLAVE_ADDRESS,
                          sy8809_reg_table5H_list[i][0],
                          sy8809_reg_table5H_list[i][1]);
        }
    }
}
