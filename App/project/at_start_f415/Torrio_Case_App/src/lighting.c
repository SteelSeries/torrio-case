/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "lighting.h"
#include "timer3.h"
#include <stdio.h>
#include "task_scheduler.h"

/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
T_LED_VAR LED_RED;
T_LED_VAR LED_GREEN;
T_LED_VAR LED_BLUE;
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint8_t pwm_flag = 1;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void SetPwmDuty(uint8_t color)
{
    tmr_output_config_type tmr_oc_init_structure;
    tmr_output_default_para_init(&tmr_oc_init_structure);
    tmr_oc_init_structure.oc_output_state = TRUE;
    tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
}
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lighting_InitLedStruct(T_LED_VAR *led)
{
    led->change_flag = true;
    led->pwm_enable_flag = true;
    led->breath_timer = 0;
    led->breath_dir = LED_DIR_BRIGHT;
    led->breath_speed = 0;
    led->PWM_duty = LED_OFF;
    led->timer_count = 0;
    led->blink_count = 0;
}
void Lighting_HandlerTask(void)
{
    printf("run Lighting_HandlerTask\n");
    if(TaskScheduler_AddTask(Lighting_HandlerTask, 1, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add lighting task fail\n");
    }
}

void Lighting_LEDNonPWMSetting(uint8_t rgb, confirm_state state)
{
    pwm_flag = 0;
    confirm_state enable = (confirm_state)!state;
    
    gpio_init_type gpio_initstructure;
    gpio_initstructure.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
    
    switch (rgb)
    {
        case LED_R:
            gpio_initstructure.gpio_pins = LED_R_PIN;
            gpio_init(GPIOA, &gpio_initstructure);
            gpio_bits_write(GPIOA, LED_R_PIN, enable);
        break;
        case LED_G:
            gpio_initstructure.gpio_pins = LED_G_PIN;
            gpio_init(GPIOA, &gpio_initstructure);
            gpio_bits_write(GPIOA, LED_G_PIN, enable);
        break;
        case LED_B:
            gpio_initstructure.gpio_pins = LED_B_PIN;
            gpio_init(GPIOB, &gpio_initstructure);
            gpio_bits_write(GPIOB, LED_B_PIN, enable);
        break;
    }
}
void Lighting_QueueHandler(void)
{
    //TaskScheduler_RemoveTask
}
void Lighting_BreathHandler(void)
{
    // breath handler
    if(pwm_flag == 0)
    {
        // re-init timer3 for pwm
        Timer3_Init();
        pwm_flag = 1;
    }

    if(TaskScheduler_AddTask(Lighting_BreathHandler, 1, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add lighting task fail\n");
    }
}
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/