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
#include "battery.h"
#include "commands.h"
#include "battery.h"
#include "file_system.h"

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
static uint16_t breath_hold_bright = LIGHTING_BRIGHT_MAX + LIGHTING_BREATH_HOLD_TIME;
static uint16_t breath_falling = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_BREATH_HOLD_TIME;
static uint16_t breath_hold_dark = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_BREATH_HOLD_TIME * 2;
static uint16_t breath_interval = 4;
static uint16_t breath_quick_interval = 8;

static uint16_t light_max = 0;
static uint16_t light_min = LIGHTING_BRIGHT_MAX;
static uint16_t light_on = 100;//500ms
static uint16_t light_off = 100;//500ms

static uint16_t illum_val = 0;
static uint16_t illum_rising = LIGHTING_BRIGHT_MAX; //illum_rising - illum_val = led rising time
static uint16_t illum_hold_bright = LIGHTING_BRIGHT_MAX + LIGHTING_HOLD_TIME; //illum_hold_bright - illum_val = led hold bright time
static uint16_t illum_falling = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_HOLD_TIME; //illum_falling - illum_val = led falling time
static uint16_t illum_hold_dark = LIGHTING_BRIGHT_MAX * 2 + LIGHTING_HOLD_TIME * 2; //illum_hold_dark - illum_val = led hold dark time
static uint16_t illum_falling_interval = 42;
static uint16_t illum_rising_interval = 21;
static uint16_t illum_hold_interval = 2;

static uint16_t r_en, g_en, b_en;

static Lighting_Lid_State_t lid_complete_flag = LIGHTING_LID_COMPLETE;
static uint16_t lid_pre_state;
static uint8_t lid_compelete_count = 0;
/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void IllumHandler(void);
static void BreathHandler(void);
static void BreathQuickHandler(void);
static void BreathQuickOnceHandler(void);
static void AlertHandler(void);
static void StableHandler(void);
static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB);
static void BatteryChargingSetting(void);
static void BatteryLevelSetting(void);
static Lighting_HardwareSettings_t user_hardware_settings = {0};
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Lighting_HandleTask(void)
{
    if (Lid_GetState() != lid_pre_state)
    {
      lid_complete_flag = LIGHTING_LID_NON_COMPLETE;
      breath_val = 0;
      illum_val = 0;
    }

    if(Commands_HandleLightingMode() == COMMAND_FACTORY_LED_ON_OFF)
    {
        return;
    }
    else if(Commands_HandleLightingMode() == COMMAND_FACTORY_MODE_LIGHTING)
    {
        Lighting_Handler(LIGHTING_BLINK, 1, 0, 0);
    }
    else if ((Lid_GetState() == LID_OPEN) && lid_complete_flag == LIGHTING_LID_NON_COMPLETE)
    {
        lid_pre_state = LID_OPEN;
        lid_compelete_count = 0;
        BatteryLevelSetting();
    }
    else if ((Lid_GetState() == LID_CLOSE) && lid_complete_flag == LIGHTING_LID_NON_COMPLETE)
    {
        lid_pre_state = LID_CLOSE;
        BatteryLevelSetting();
    }
    else if (Usb_GetUsbDetectState() == USB_PLUG)
    {
        BatteryChargingSetting();
    }
    else if (Qi_GetDetectState() == QI_DETECT)
    {
        BatteryChargingSetting();
    }
    else
    {
        PwmHandler(LIGHTING_LED_OFF, LIGHTING_LED_OFF, LIGHTING_LED_OFF);
    }

    if (FileSystem_GetUserData()->presetChargeState == PRESET_CHARGE_ACTIVE)
    {
      Battery_PresetChargeData_t *preset = Battery_GetPresetChargeState();
      if ((preset->case_charge_status == BATTERY_PRESET_CHARGE_ACTIVE) &&
          (preset->left_bud_charge_status == BATTERY_PRESET_CHARGE_ACTIVE) &&
          (preset->right_bud_charge_status == BATTERY_PRESET_CHARGE_ACTIVE))
      {
        // todo: LED display preset charge completes lighting effect.
      }
    }
}

Lighting_Lid_State_t Lighting_LidOffHandle(void)
{
  if(lid_complete_flag == LIGHTING_LID_COMPLETE)
    lid_compelete_count++;
  if(lid_compelete_count >= 2)
  {
      return LIGHTING_LID_COMPLETE;//when lid close and breath complete enter to standby mode
  }
  return LIGHTING_LID_NON_COMPLETE;
}

void Lighting_Handler(uint16_t LightingMode, uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    if(Lighting_Change_Flag == LIGHTING_CHANGE_TRUE)
    {
        breath_val = 0;
        illum_val = 0;
        Lighting_Change_Flag = LIGHTING_CHANGE_FALSE;
        DEBUG_PRINT("LightingMode:%d\r\n", LightingMode);
    }

    r_en = PwmR;
    g_en = PwmG;
    b_en = PwmB;

    switch (LightingMode)
    {
        case LIGHTING_ILLUM:
        {
          IllumHandler();
          break;
        }
        case LIGHTING_BREATH:
        {
          BreathHandler();
          break;
        }
        case LIGHTING_BREATH_QUICKLY:
        {
          BreathQuickHandler();
          break;
        }
        case LIGHTING_BREATH_QUICKLY_ONCE:
        {
          BreathQuickOnceHandler();
          break;
        }
        case LIGHTING_BLINK:
        {
          AlertHandler();
          break;
        }
        case LIGHTING_STABLE:
        {
          StableHandler();
          break;
        }
        default:
        {
            break;
        }
    }   
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
    if(illum_val >= illum_hold_dark)
    {
      illum_val = 0;
      illum_reg = illum_val;
      lid_complete_flag = LIGHTING_LID_COMPLETE;
    }
    else if((illum_val >= illum_falling) && (illum_val < illum_hold_dark))
    {
      illum_reg = light_max;
      illum_val += illum_hold_interval;
    }
    else if((illum_val >= illum_hold_bright) && (illum_val < illum_falling))
    {
      illum_reg = illum_falling - illum_val;
      illum_val += illum_falling_interval;
    }
    else if((illum_val >= illum_rising) && (illum_val < illum_hold_bright))
    {
      illum_reg = light_min;
      illum_val += illum_hold_interval;
    }
    else if(illum_val < illum_rising)
    {
      illum_reg = illum_val;
      illum_val += illum_rising_interval;
    }
    PwmHandler(illum_reg * r_en, illum_reg * g_en, illum_reg * b_en);
}

static void BreathHandler(void)
{
    static uint16_t breath_reg;
    if(breath_val >= breath_hold_dark)
    {
      breath_val = 0;
      breath_reg = breath_val;
    }
    else if((breath_val >= breath_falling) && (breath_val < breath_hold_dark))
    {
      breath_reg = light_max;
      breath_val += breath_interval;
    }
    else if((breath_val >= breath_hold_bright) && (breath_val < breath_falling))
    {
      breath_reg = breath_falling - breath_val;
      breath_val += breath_interval;
    }
    else if((breath_val >= breath_rising) && (breath_val < breath_hold_bright))
    {
      breath_reg = light_min;
      breath_val += breath_interval;
    }
    else if(breath_val < breath_rising)
    {
      breath_reg = breath_val;
      breath_val += breath_interval;
    }
    PwmHandler(breath_reg * r_en, breath_reg * g_en, breath_reg * b_en);
}

static void BreathQuickHandler(void)
{
    static uint16_t breath_reg;
    if(breath_val >= breath_hold_dark)
    {
      breath_val = 0;
      breath_reg = breath_val;
    }
    else if((breath_val >= breath_falling) && (breath_val < breath_hold_dark))
    {
      breath_reg = light_max;
      breath_val += breath_quick_interval;
    }
    else if((breath_val >= breath_hold_bright) && (breath_val < breath_falling))
    {
      breath_reg = breath_falling - breath_val;
      breath_val += breath_quick_interval;
    }
    else if((breath_val >= breath_rising) && (breath_val < breath_hold_bright))
    {
      breath_reg = light_min;
      breath_val += breath_quick_interval;
    }
    else if(breath_val < breath_rising)
    {
      breath_reg = breath_val;
      breath_val += breath_quick_interval;
    }
    PwmHandler(breath_reg * r_en, breath_reg * g_en, breath_reg * b_en);
}

static void BreathQuickOnceHandler(void)
{
    static uint16_t breath_reg;
    if(breath_val >= breath_hold_dark)
    {
      breath_val = 0;
      breath_reg = breath_val;
      lid_complete_flag = LIGHTING_LID_COMPLETE;
    }
    else if((breath_val >= breath_falling) && (breath_val < breath_hold_dark))
    {
      breath_reg = light_max;
      breath_val += breath_quick_interval;
    }
    else if((breath_val >= breath_hold_bright) && (breath_val < breath_falling))
    {
      breath_reg = breath_falling - breath_val;
      breath_val += breath_quick_interval;
    }
    else if((breath_val >= breath_rising) && (breath_val < breath_hold_bright))
    {
      breath_reg = light_min;
      breath_val += breath_quick_interval;
    }
    else if(breath_val < breath_rising)
    {
      breath_reg = breath_val;
      breath_val += breath_quick_interval;
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
    else if((alert_val >= light_on) && (alert_val < (light_on + light_off)))
    {
      alert_val += 1;
      alert_reg = light_min;
    }
    else
    {
      alert_val = 0;
      alert_reg = light_min;
    }
    PwmHandler(alert_reg * r_en, alert_reg * g_en, alert_reg * b_en);
}

static void StableHandler(void)
{
    static uint16_t stable_reg;
    stable_reg = light_min;
    PwmHandler(stable_reg * r_en, stable_reg * g_en, stable_reg * b_en);
}

static void PwmHandler(uint16_t PwmR, uint16_t PwmG, uint16_t PwmB)
{
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, PwmR);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, PwmG);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_3, PwmB);
}

static void BatteryChargingSetting(void)
{
    if(Battery_GetBatteryPercent() <= BATTERY_LOW_LEVEL)
    {
      Lighting_Handler(LIGHTING_BREATH_QUICKLY, 1, 0, 0);
    }
    else if(Battery_GetBatteryPercent() <= BATTERY_MEDIUM_LEVEL)
    {
      Lighting_Handler(LIGHTING_BREATH, 1, 1, 0);
    }
    else if(Battery_GetBatteryPercent() < BATTERY_HIGH_LEVEL)
    {
      Lighting_Handler(LIGHTING_BREATH, 0, 1, 0);
    }
    else if(Battery_GetBatteryPercent() == BATTERY_HIGH_LEVEL)
    {
      Lighting_Handler(LIGHTING_STABLE, 0, 1, 0);
    }
    else
    {
      Lighting_Handler(LIGHTING_STABLE, 0, 0, 0);
    }
}

static void BatteryLevelSetting(void)
{
    if(Battery_GetBatteryPercent() <= BATTERY_CRITICAL_LEVEL)
    {
      Lighting_Handler(LIGHTING_BREATH_QUICKLY_ONCE, 1, 0, 0);
    }
    else if(Battery_GetBatteryPercent() <= BATTERY_LOW_LEVEL)
    {
      Lighting_Handler(LIGHTING_ILLUM, 1, 0, 0);
    }
    else if(Battery_GetBatteryPercent() <= BATTERY_MEDIUM_LEVEL)
    {
      Lighting_Handler(LIGHTING_ILLUM, 1, 1, 0);
    }
    else if(Battery_GetBatteryPercent() <= BATTERY_HIGH_LEVEL)
    {
      Lighting_Handler(LIGHTING_ILLUM, 0, 1, 0);
    }
    else
    {
      Lighting_Handler(LIGHTING_STABLE, 0, 0, 0);
    }
}
