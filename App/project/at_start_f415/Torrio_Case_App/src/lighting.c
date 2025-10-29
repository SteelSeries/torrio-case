/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "lighting.h"
#include "timer3.h"
#include <stdio.h>
#include "task_scheduler.h"
#include <string.h>
#include "usb.h"
#include "qi.h"
#include "lid.h"
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
uint8_t Lighting_Change_Flag = LIGHTING_CHANGE_FALSE;
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static uint16_t breath_val = 0;
static uint16_t breath_rising = LIGHTING_BRIGHT_MAX;
static uint16_t breath_falling = LIGHTING_BRIGHT_MAX * 2;
static uint16_t breath_interval = 3;

static uint16_t light_max = 0;
static uint16_t light_min = LIGHTING_BRIGHT_MAX;
static uint16_t light_on = 1;

static uint16_t illum_val = 0;
static uint16_t illum_rising = LIGHTING_BRIGHT_MAX; //illum_rising - illum_val = led rising time
static uint16_t illum_hold_bright = LIGHTING_BRIGHT_MAX + LIGHTING_HOLD_TIME; //illum_hold_bright - illum_val = led hold bright time
static uint16_t illum_falling = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_HOLD_TIME; //illum_falling - illum_val = led falling time
static uint16_t illum_hold_dark = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_HOLD_TIME * 2; //illum_hold_dark - illum_val = led hold dark time

static uint16_t r_en, g_en, b_en;

static Lighting_Breath_State_t breath_complete_flag = LIGHTING_BREATH_COMPLETE;
static uint16_t lid_pre_state;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void IllumHandler(void);
static void BreathHandler(void);
static void BreathQuickHandler(void);
static void AlertHandler(void);
static void StableHandler(void);
static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB);
static Lighting_HardwareSettings_t user_hardware_settings = {0};
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lighting_HandleTask(void)
{
    TaskScheduler_RemoveTask(IllumHandler);
    TaskScheduler_RemoveTask(BreathHandler);
    TaskScheduler_RemoveTask(BreathQuickHandler);
    TaskScheduler_RemoveTask(AlertHandler);
    TaskScheduler_RemoveTask(StableHandler);
    if (Lid_GetState() != lid_pre_state)
    {
      breath_complete_flag = LIGHTING_BREATH_NON_COMPLETE;
    }
    
    if ((Lid_GetState() == LID_OPEN) && breath_complete_flag == LIGHTING_BREATH_NON_COMPLETE)
    {
        lid_pre_state = LID_OPEN;
        Lighting_Handler(LIGHTING_BREATH_QUICKLY, 0, 1, 0);
    }
    else if ((Lid_GetState() == LID_CLOSE) && breath_complete_flag == LIGHTING_BREATH_NON_COMPLETE)
    {
        lid_pre_state = LID_CLOSE;
        Lighting_Handler(LIGHTING_BREATH_QUICKLY, 1, 1, 0);
    }
    else if (Usb_GetUsbDetectState() == USB_PLUG)
    {
        Lighting_Handler(LIGHTING_BREATH, 0, 0, 1);
    }
    else if (Qi_GetDetectState() == QI_DETECT)
    {
        Lighting_Handler(LIGHTING_BREATH, 1, 0, 0);
    }
    else
    {
        PwmHandler(LIGHTING_LED_OFF, LIGHTING_LED_OFF, LIGHTING_LED_OFF);
    }
}

Lighting_Breath_State_t Lighting_LidOffHandle(void)
{
  if ((Lid_GetState() == LID_CLOSE) && breath_complete_flag == LIGHTING_BREATH_NON_COMPLETE)
  {
    if ((Lid_GetState() == LID_CLOSE) && breath_complete_flag == LIGHTING_BREATH_COMPLETE)
    {
        return LIGHTING_BREATH_COMPLETE;//when lid close and breath complete enter to standby mode
    }
  }
  return LIGHTING_BREATH_NON_COMPLETE;
}

void Lighting_Handler(uint16_t LightingMode, uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    if(Lighting_Change_Flag == LIGHTING_CHANGE_TRUE)
    {
        breath_val = 0;
        illum_val = 0;
        TaskScheduler_RemoveTask(IllumHandler);
        TaskScheduler_RemoveTask(BreathHandler);
        TaskScheduler_RemoveTask(BreathQuickHandler);
        TaskScheduler_RemoveTask(AlertHandler);
        TaskScheduler_RemoveTask(StableHandler);
        Lighting_Change_Flag = LIGHTING_CHANGE_FALSE;
    }

    r_en = PwmR;
    g_en = PwmG;
    b_en = PwmB;

    switch (LightingMode)
    {
        case LIGHTING_ILLUM:
        {
          if(TaskScheduler_AddTask(IllumHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              DEBUG_PRINT("add IllumHandler fail\n");
          }
          break;
        }
        case LIGHTING_BREATH:
        {
          if(TaskScheduler_AddTask(BreathHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              DEBUG_PRINT("add BreathHandler fail\n");
          }
          break;
        }
        case LIGHTING_BREATH_QUICKLY:
        {
          if(TaskScheduler_AddTask(BreathQuickHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              DEBUG_PRINT("add BreathQuickHandler fail\n");
          }
          break;
        }
        case LIGHTING_BLINK:
        {
          if(TaskScheduler_AddTask(AlertHandler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              DEBUG_PRINT("add AlertHandler fail\n");
          }
          break;
        }
        case LIGHTING_STABLE:
        {
          if(TaskScheduler_AddTask(StableHandler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              DEBUG_PRINT("add StableHandler fail\n");
          }
          break;
        }
        default:
        {
            break;
        }
    }   
    
}

void Lighting_LEDOnOffSetting(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    bool R,G,B;
    Lighting_Change_Flag = LIGHTING_CHANGE_TRUE;
    if(Lighting_Change_Flag == LIGHTING_CHANGE_TRUE)
    {
        breath_val = 0;
        illum_val = 0;
        TaskScheduler_RemoveTask(IllumHandler);
        TaskScheduler_RemoveTask(BreathHandler);
        TaskScheduler_RemoveTask(BreathQuickHandler);
        TaskScheduler_RemoveTask(AlertHandler);
        TaskScheduler_RemoveTask(StableHandler);
        Lighting_Change_Flag = LIGHTING_CHANGE_FALSE;
    }
    R = (PwmR == 1)? true:false;
    G = (PwmG == 1)? true:false;  
    B = (PwmB == 1)? true:false;
  PwmHandler(R*LIGHTING_BRIGHT_MAX, G*LIGHTING_BRIGHT_MAX, B*LIGHTING_BRIGHT_MAX);
}

void Lighting_GpioConfigHardware(const Lighting_HardwareSettings_t *hardware_settings)
{
    memcpy(&user_hardware_settings, hardware_settings, sizeof(Lighting_HardwareSettings_t));
    /*===========Timer3 PIN================*/
    gpio_init_type gpio_init_struct;

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

    gpio_init_struct.gpio_pins = user_hardware_settings.lighting_r_gpio_pin;
    gpio_init(user_hardware_settings.lighting_r_gpio_port, &gpio_init_struct);

    gpio_init_struct.gpio_pins = user_hardware_settings.lighting_g_gpio_pin;
    gpio_init(user_hardware_settings.lighting_g_gpio_port, &gpio_init_struct);

    gpio_init_struct.gpio_pins = user_hardware_settings.lighting_b_gpio_pin;
    gpio_init(user_hardware_settings.lighting_b_gpio_port, &gpio_init_struct);

    crm_periph_clock_enable(user_hardware_settings.lighting_r_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.lighting_g_gpio_crm_clk, TRUE);
    crm_periph_clock_enable(user_hardware_settings.lighting_b_gpio_crm_clk, TRUE);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void IllumHandler(void)
{
    static uint16_t illum_reg;
    illum_val += breath_interval;
    if(illum_val == illum_hold_dark)
    {
      illum_val = 0;
      illum_reg = illum_val;
    }
    else if((illum_val >= illum_falling) && (illum_val < illum_hold_dark))
    {
      illum_reg = light_max;
    }
    else if((illum_val >= illum_hold_bright) && (illum_val < illum_falling))
    {
      illum_reg = illum_falling - illum_val;
    }
    else if((illum_val >= illum_rising) && (illum_val < illum_hold_bright))
    {
      illum_reg = light_min;
    }
    else if(illum_val < illum_rising)
    {
      illum_reg = illum_val;
    }
    PwmHandler(illum_reg * r_en, illum_reg * g_en, illum_reg * b_en);
    if(TaskScheduler_AddTask(IllumHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add IllumHandler fail\n");
    }
}

static void BreathHandler(void)
{
    static uint16_t breath_reg;
    breath_val += breath_interval;
    if(TaskScheduler_AddTask(BreathHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add BreathHandler fail\n");
    } 
    if(breath_val < breath_rising)
    {
      breath_reg = breath_val;
    }
    else if((breath_val >= breath_rising) && (breath_val < breath_falling))
    {
      breath_reg = breath_falling - breath_val;
    }
    else
    {
      breath_val = light_max;
      breath_reg = breath_val;
    }
    PwmHandler(breath_reg * r_en, breath_reg * g_en, breath_reg * b_en);
}

static void BreathQuickHandler(void)
{
    static uint16_t breath_reg;
    breath_val += breath_interval;
    if(TaskScheduler_AddTask(BreathQuickHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add BreathQuickHandler fail\n");
    }
    if(breath_val < breath_rising)
    {
      breath_reg = breath_val;
    }
    else if((breath_val >= breath_rising) && (breath_val < breath_falling))
    {
      breath_reg = breath_falling - breath_val;
    }
    else
    {
      breath_val = light_max;
      breath_reg = breath_val;
      TaskScheduler_RemoveTask(BreathQuickHandler);
      breath_complete_flag = LIGHTING_BREATH_COMPLETE;
    }
    PwmHandler(breath_reg * r_en, breath_reg * g_en, breath_reg * b_en);
}

static void AlertHandler(void)
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
    PwmHandler(alert_reg * r_en, alert_reg * g_en, alert_reg * b_en);
    if(TaskScheduler_AddTask(AlertHandler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add AlertHandler fail\n");
    }
}

static void StableHandler(void)
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
    PwmHandler(stable_reg * r_en, stable_reg * g_en, stable_reg * b_en);
    if(TaskScheduler_AddTask(StableHandler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        DEBUG_PRINT("add StableHandler fail\n");
    }
}

static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, PwmR);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, PwmG);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_3, PwmB);
}
