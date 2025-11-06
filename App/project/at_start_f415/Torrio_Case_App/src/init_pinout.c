/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "init_pinout.h"
#include "pinout.h"
#include "lid.h"
#include "i2c1.h"
#include "i2c2.h"
#include "usb.h"
#include "cps4520.h"
#include "sy8809.h"
#include "adc.h"
#include "lighting.h"
#include "button.h"
#include "uart_driver.h"
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

static const Cps4520_HardwareSettings_t CPS4520_config =
    {
        .cps4520_detect_gpio_port = CPS4520_CHARGE_DETECT_GPIO,
        .cps4520_detect_gpio_pin = CPS4520_CHARGE_DETECT_PIN,
        .cps4520_detect_gpio_crm_clk = CPS4520_CHARGE_DETECT_CRM_CLK,

        .cps4520_int_gpio_port = CPS4520_CHARGE_INT_GPIO,
        .cps4520_int_gpio_pin = CPS4520_CHARGE_INT_PIN,
        .cps4520_int_gpio_crm_clk = CPS4520_CHARGE_INT_CRM_CLK,
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

static const I2c2_HardwareSettings_t i2c2_config =
    {
        .i2c2_sda_gpio_port = I2C2_SDA_GPIO_PORT,
        .i2c2_sda_gpio_pin = I2C2_SDA_PIN,
        .i2c2_sda_gpio_crm_clk = I2C2_SDA_GPIO_CLK,

        .i2c2_scl_gpio_port = I2C2_SCL_GPIO_PORT,
        .i2c2_scl_gpio_pin = I2C2_SCL_PIN,
        .i2c2_scl_gpio_crm_clk = I2C2_SCL_GPIO_CLK,

        .i2c2_port = I2C2_PORT,
        .i2c2_crm_clk = I2C2_CLK,

        .i2c2_speed = I2C2_SPEED,
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

        .usb_otg_pin_sof_gpio_port = OTG_SOF_GPIO,
        .usb_otg_pin_sof_gpio_pin = OTG_SOF_PIN,
        .usb_otg_pin_sof_gpio_crm_clk = OTG_SOF_CRM_CLK,

        .usb_otg_pin_vbus_gpio_port = OTG_VBUS_GPIO,
        .usb_otg_pin_vbus_gpio_pin = OTG_VBUS_PIN,
        .usb_otg_pin_vbus_gpio_crm_clk = OTG_VBUS_CRM_CLK,
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

static const UartDrive_HardwareSettings_t buds_uart_config =
    {
        .left_bud_uart_tx_gpio_port = UART2_L_TX_GPIO,
        .left_bud_uart_tx_gpio_pin = UART2_L_TX_PIN,
        .left_bud_uart_tx_gpio_crm_clk = UART2_L_TX_CRM_CLK,

        .left_bud_uart_rx_gpio_port = UART2_L_RX_GPIO,
        .left_bud_uart_rx_gpio_pin = UART2_L_RX_PIN,
        .left_bud_uart_rx_gpio_crm_clk = UART2_L_RX_CRM_CLK,

        .right_bud_uart_tx_gpio_port = UART3_R_TX_GPIO,
        .right_bud_uart_tx_gpio_pin = UART3_R_TX_PIN,
        .right_bud_uart_tx_gpio_crm_clk = UART3_R_TX_CRM_CLK,

        .right_bud_uart_rx_gpio_port = UART3_R_RX_GPIO,
        .right_bud_uart_rx_gpio_pin = UART3_R_RX_PIN,
        .right_bud_uart_rx_gpio_crm_clk = UART3_R_RX_CRM_CLK,

        .left_bud_uart = LEFT_UART,
        .right_bud_uart = RIGHT_UART,
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
    Cps4520_GpioConfigHardware(&CPS4520_config);
    UartDrive_GpioConfigHardware(&buds_uart_config);

    /*===========DEBUG PIN================*/
    gpio_init_type gpio_initstructure;
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    gpio_default_para_init(&gpio_initstructure);
    gpio_initstructure.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_pins = GPIO_PINS_9 | GPIO_PINS_8;
    gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOB, &gpio_initstructure);
    /*====================================*/
}

void InitPinout_I2c1Init(void)
{
    I2c1_GpioConfigHardware(&i2c1_config);
}

void InitPinout_I2c2Init(void)
{
    I2c2_GpioConfigHardware(&i2c2_config);
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
