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
uint8_t Lighting_Change_Flag = LIGHTING_CHANGE_FALSE;
uint8_t Lighting_Mode = LED_MODE_NORMAL;
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
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
static void IllumHandler(void);
static void BreathHandler(void);
static void BreathQuickHandler(void);
static void AlertHandler(void);
static void StableHandler(void);
static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB);
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lighting_HandlerTask(void)
{
    printf("run Lighting_HandlerTask\n");
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
    switch (Lighting_Mode)
    {
        case ILLUM:
        {
          if(TaskScheduler_AddTask(IllumHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              printf("add IllumHandler fail\n");
          }
          break;
        }
        case BREATH:
        {
          if(TaskScheduler_AddTask(BreathHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              printf("add BreathHandler fail\n");
          }
          break;
        }
        case BREATH_QUICKLY:
        {
          if(TaskScheduler_AddTask(BreathQuickHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              printf("add BreathQuickHandler fail\n");
          }
          break;
        }
        case BLINK:
        {
          if(TaskScheduler_AddTask(AlertHandler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              printf("add AlertHandler fail\n");
          }
          break;
        }
        case STABLE:
        {
          if(TaskScheduler_AddTask(StableHandler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
          {
              printf("add StableHandler fail\n");
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
  R = (PwmR == 0)? true:false;
  G = (PwmG == 0)? true:false;  
  B = (PwmB == 0)? true:false;
  PwmHandler(R*666, G*666, B*666);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static void IllumHandler(void)
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
    PwmHandler(illum_reg, illum_reg, illum_reg);
    if(TaskScheduler_AddTask(IllumHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
    {
        printf("add IllumHandler fail\n");
    }
}

static void BreathHandler(void)
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
  PwmHandler(breath_reg, breath_reg, breath_reg);
  if(TaskScheduler_AddTask(BreathHandler, 10, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add BreathHandler fail\n");
  }
}

static void BreathQuickHandler(void)
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
  PwmHandler(breath_reg, breath_reg, breath_reg);
  if(TaskScheduler_AddTask(BreathQuickHandler, 5, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add BreathQuickHandler fail\n");
  }
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
  PwmHandler(alert_reg, alert_reg, alert_reg);
  if(TaskScheduler_AddTask(AlertHandler, 500, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add AlertHandler fail\n");
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
  PwmHandler(stable_reg, stable_reg, stable_reg);
  if(TaskScheduler_AddTask(StableHandler, 5000, TASK_RUN_ONCE, TASK_START_DELAYED) != TASK_OK)
  {
      printf("add StableHandler fail\n");
  }
}

static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, PwmR);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, PwmG);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_3, PwmB);
}
