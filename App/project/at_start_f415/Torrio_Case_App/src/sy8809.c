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
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static Sy8809_HardwareSettings_t user_hardware_settings = {0};

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static flag_status GetSdaState(void);

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
