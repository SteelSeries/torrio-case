/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "init_pinout.h"
#include "pinout.h"
#include "lid.h"
#include "i2c1.h"
#include "usb.h"
#include "qi.h"
#include "sy8809.h"
#include "adc.h"
#include "lighting.h"
#include "button.h"
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
static const Lid_HardwareSettings_t lid_config =
    {
        .lid_gpio_port = HALL_OUT_GPIO,
        .lid_gpio_pin = HALL_OUT_PIN,
        .lid_gpio_crm_clk = HALL_OUT_CRM_CLK,
};

static const Button_HardwareSettings_t button_config =
    {
        .button_gpio_port = BUTTON_GPIO,
        .button_gpio_pin = BUTTON_PIN,
        .button_gpio_crm_clk = BUTTON_CRM_CLK,
};

static const Qi_HardwareSettings_t qi_config =
    {
        .qi_gpio_port = QI_CHARGE_DETECT_GPIO,
        .qi_gpio_pin = QI_CHARGE_DETECT_PIN,
        .qi_gpio_crm_clk = QI_CHARGE_DETECT_CRM_CLK,
};

static const I2c1_HardwareSettings_t i2c1_config =
    {
        .i2c1_sda_gpio_port = I2C1_SDA_GPIO_PORT,
        .i2c1_sda_gpio_pin = I2C1_SDA_PIN,
        .i2c1_sda_gpio_crm_clk = I2C1_SDA_GPIO_CLK,

        .i2c1_scl_gpio_port = I2C1_SCL_GPIO_PORT,
        .i2c1_scl_gpio_pin = I2C1_SCL_PIN,
        .i2c1_scl_gpio_crm_clk = I2C1_SCL_GPIO_CLK,

        .i2c1_port = I2C1_PORT,
        .i2c1_crm_clk = I2C1_CLK,

        .i2c1_speed = I2C1_SPEED,
};

static const Sy8809_HardwareSettings_t sy8809_config =
    {
        .sy8809_sda_gpio_port = I2C1_SDA_GPIO_PORT,
        .sy8809_sda_gpio_pin = I2C1_SDA_PIN,
        .sy8809_sda_gpio_crm_clk = I2C1_SDA_GPIO_CLK,

        .busd_detect_resist_gpio_port = BUD_DETECT_RESIST_SWITCH_GPIO,
        .busd_detect_resist_gpio_pin = BUD_DETECT_RESIST_SWITCH_PIN,
        .busd_detect_resist_gpio_crm_clk = BUD_DETECT_RESIST_SWITCH_CRM_CLK,

        .sy8809_irq_gpio_port = CHARGER_IRQ_GPIO,
        .sy8809_irq_gpio_pin = CHARGER_IRQ_PIN,
        .sy8809_irq_gpio_crm_clk = CHARGER_IRQ_CRM_CLK,
};

static const Usb_HardwareSettings_t usb_config =
    {
        .usb_detect_gpio_port = USB_DET_GPIO,
        .usb_detect_gpio_pin = USB_DET_PIN,
        .usb_detect_gpio_crm_clk = USB_DET_CRM_CLK,

        .usb_otg_pin_sof_gpio_port = BUD_DETECT_RESIST_SWITCH_GPIO,
        .usb_otg_pin_sof_gpio_pin = BUD_DETECT_RESIST_SWITCH_PIN,
        .usb_otg_pin_sof_gpio_crm_clk = BUD_DETECT_RESIST_SWITCH_CRM_CLK,

        .usb_otg_pin_vbus_gpio_port = CHARGER_IRQ_GPIO,
        .usb_otg_pin_vbus_gpio_pin = CHARGER_IRQ_PIN,
        .usb_otg_pin_vbus_gpio_crm_clk = CHARGER_IRQ_CRM_CLK,
};

static const Adc_HardwareSettings_t adc_config =
    {
        .adc_gpio_port = ADC_GPIO,
        .adc_gpio_pin = ADC_PIN,
        .adc_gpio_crm_clk = ADC_CRM_CLK,
};

static const Lighting_HardwareSettings_t pwm_config =
    {
        .lighting_r_gpio_port = PWM_R_GPIO_PORT,
        .lighting_r_gpio_pin = PWM_R_PIN,
        .lighting_r_gpio_crm_clk = PWM_R_GPIO_CLK,

        .lighting_g_gpio_port = PWM_G_GPIO_PORT,
        .lighting_g_gpio_pin = PWM_G_PIN,
        .lighting_g_gpio_crm_clk = PWM_G_GPIO_CLK,

        .lighting_b_gpio_port = PWM_B_GPIO_PORT,
        .lighting_b_gpio_pin = PWM_B_PIN,
        .lighting_b_gpio_crm_clk = PWM_B_GPIO_CLK,
};

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void InitPinout_Init(void)
{
    Lid_GpioConfigHardware(&lid_config);
    Sy8809_GpioConfigHardware(&sy8809_config);
    Usb_GpioConfigHardware(&usb_config);
    Adc_GpioConfigHardware(&adc_config);
    Lighting_GpioConfigHardware(&pwm_config);
	Button_GpioConfigHardware(&button_config);
    Qi_GpioConfigHardware(&qi_config);
}

void InitPinout_I2c1Init(void)
{
    I2c1_GpioConfigHardware(&i2c1_config);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/