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
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
uint8_t LIGHTING_CHANGE_FLAG = LIGHTING_CHANGE_FALSE;
uint8_t LIGHTING_MODE = LED_MODE_NORMAL;
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static bool pwm_flag = true;
static uint16_t breath_val = 0;
static uint16_t breath_falling = 665;
static uint16_t breath_rising = 1330;
static uint16_t breath_interval = 5;

static uint16_t light_max = 0;
static uint16_t light_min = 666;
static uint16_t light_on = 1;

static uint16_t illum_val = 0;
static uint16_t illum_falling = 665;
static uint16_t illum_hold_dark = 965;
static uint16_t illum_rising = 1630;
static uint16_t illum_hold_bright = 1930;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void lighting_illum_handler(void);
static void lighting_breath_handler(void);
static void lighting_breath_quick_handler(void);
static void lighting_alert_handler(void);
static void lighting_stable_handler(void);
static void lighting_pwm_handler(uint16_t pwm_R, uint16_t pwm_G, uint16_t pwm_B);
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lighting_HandlerTask(void)
{
    printf("run Lighting_HandlerTask\n");
    if(LIGHTING_CHANGE_FLAG == LIGHTING_CHANGE_TRUE)
    {
        breath_val = 0;
        illum_val = 0;
        TaskScheduler_RemoveTask(lighting_illum_handler);
        TaskScheduler_RemoveTask(lighting_breath_handler);
        TaskScheduler_RemoveTask(lighting_breath_quick_handler);
        TaskScheduler_RemoveTask(lighting_alert_handler);
        TaskScheduler_RemoveTask(lighting_stable_handler);
        LIGHTING_CHANGE_FLAG = LIGHTING_CHANGE_FALSE;
    }
    switch (LIGHTING_MODE)
    {
        case ILLUM:
            if(TaskScheduler_AddTask(lighting_illum_handler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add lighting_illum_handler_task fail\n");
            }
            break;
        case BREATH:
            if(TaskScheduler_AddTask(lighting_breath_handler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add lighting_breath_handler_task fail\n");
            }
            break;
        case BREATH_QUICKLY:
            if(TaskScheduler_AddTask(lighting_breath_quick_handler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add lighting_breath_quick_handler_task fail\n");
            }
            break;
        case BLINK:
            if(TaskScheduler_AddTask(lighting_alert_handler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add lighting_alert_handler_task fail\n");
            }
            break;
        case STABLE:
            if(TaskScheduler_AddTask(lighting_stable_handler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
            {
                printf("add lighting_stable_handler_task fail\n");
            }
            break;
        default:
            break;
    }   
    
}

void Lighting_LEDNonPWMSetting(uint8_t rgb, confirm_state state)
{
    pwm_flag = false;
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
/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void lighting_illum_handler(void)
{
  static uint16_t illum_reg;
    illum_val += breath_interval;
    if(illum_val == illum_hold_bright)
    {
      illum_val = 0;
      illum_reg = illum_val;
    }
    else if((illum_val >= illum_rising) && (illum_val < illum_hold_bright))
    {
      illum_reg = light_max;
    }
    else if((illum_val >= illum_hold_dark) && (illum_val < illum_rising))
    {
      illum_reg = illum_rising - illum_val;
    }
    else if((illum_val >= illum_falling) && (illum_val < illum_hold_dark))
    {
      illum_reg = light_min;
    }
    else if(illum_val < illum_falling)
    {
      illum_reg = illum_val;
    }
    lighting_pwm_handler(illum_reg, illum_reg, illum_reg);
    if(TaskScheduler_AddTask(lighting_illum_handler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add lighting task fail\n");
    }
}
static void lighting_breath_handler(void)
{
  static uint16_t breath_reg;
  breath_val += breath_interval;
  if(breath_val < breath_falling)
  {
    breath_reg = breath_val;
  }
  else if((breath_val >= breath_falling) && (breath_val < breath_rising))
  {
    breath_reg = breath_rising - breath_val;
  }
  else
  {
    breath_val = light_max;
    breath_reg = breath_val;
  }
  lighting_pwm_handler(breath_reg, breath_reg, breath_reg);
  if(TaskScheduler_AddTask(lighting_breath_handler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add lighting task fail\n");
  }
}
static void lighting_breath_quick_handler(void)
{
  static uint16_t breath_reg;
  breath_val += breath_interval;
  if(breath_val < breath_falling)
  {
    breath_reg = breath_val;
  }
  else if((breath_val >= breath_falling) && (breath_val < breath_rising))
  {
    breath_reg = breath_rising - breath_val;
  }
  else
  {
    breath_val = light_max;
    breath_reg = breath_val;
  }
  lighting_pwm_handler(breath_reg, breath_reg, breath_reg);
  if(TaskScheduler_AddTask(lighting_breath_quick_handler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add lighting task fail\n");
  }
}
static void lighting_alert_handler(void)
{
  static uint16_t alert_val;
  static uint16_t alert_reg;
  if(alert_val < light_on)
  {
    alert_val += 1;
    alert_reg = light_max;
  }
  else
  {
    alert_val = 0;
    alert_reg = light_min;
  }
  lighting_pwm_handler(alert_reg, alert_reg, alert_reg);
  if(TaskScheduler_AddTask(lighting_alert_handler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add lighting task fail\n");
  }
}
static void lighting_stable_handler(void)
{
  static uint16_t stable_val;
  static uint16_t stable_reg;
  if(stable_val < light_on)
  {
    stable_val += 1;
    stable_reg = light_max;
  }
  else
  {
    stable_val = 0;
    stable_reg = light_min;
  }
  lighting_pwm_handler(stable_reg, stable_reg, stable_reg);
  if(TaskScheduler_AddTask(lighting_stable_handler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add lighting task fail\n");
  }
}
static void lighting_pwm_handler(uint16_t pwm_R, uint16_t pwm_G, uint16_t pwm_B)
{
    if(pwm_flag == false)
    {
        // re-init timer3 for pwm
        Timer3_Init();
        pwm_flag = true;
    }
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, pwm_R);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, pwm_G);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_3, pwm_B);
}