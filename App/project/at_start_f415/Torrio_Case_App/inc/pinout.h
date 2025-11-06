
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define ADC_PIN             GPIO_PINS_0                 // PC0
#define ADC_GPIO            GPIOC                       // PC0
#define ADC_CRM_CLK         CRM_GPIOC_PERIPH_CLOCK      // PC0

#define USB_DET_PIN         GPIO_PINS_3                 // PC3
#define USB_DET_GPIO        GPIOC                       // PC3
#define USB_DET_CRM_CLK     CRM_GPIOC_PERIPH_CLOCK      // PC3

#define CHARGER_IRQ_PIN     GPIO_PINS_4                 // PA4
#define CHARGER_IRQ_GPIO    GPIOA                       // PA4
#define CHARGER_IRQ_CRM_CLK CRM_GPIOA_PERIPH_CLOCK      // PA4

#define HALL_OUT_PIN        GPIO_PINS_1                 // PB1
#define HALL_OUT_GPIO       GPIOB                       // PB1
#define HALL_OUT_CRM_CLK    CRM_GPIOB_PERIPH_CLOCK      // PB1

#define BUTTON_PIN          GPIO_PINS_15                // PC15
#define BUTTON_GPIO         GPIOC                       // PC15
#define BUTTON_CRM_CLK      CRM_GPIOC_PERIPH_CLOCK      // PC15

#define UART2_L_TX_PIN      GPIO_PINS_2                 // PA2
#define UART2_L_TX_GPIO     GPIOA                       // PA2
#define UART2_L_TX_CRM_CLK  CRM_GPIOA_PERIPH_CLOCK      // PA2

#define UART2_L_RX_PIN      GPIO_PINS_3                 // PA3
#define UART2_L_RX_GPIO     GPIOA                       // PA3
#define UART2_L_RX_CRM_CLK  CRM_GPIOA_PERIPH_CLOCK      // PA3

#define LEFT_UART           USART2

#define UART3_R_TX_PIN      GPIO_PINS_10                // PB10
#define UART3_R_TX_GPIO     GPIOB                       // PB10
#define UART3_R_TX_CRM_CLK  CRM_GPIOB_PERIPH_CLOCK      // PB10

#define UART3_R_RX_PIN      GPIO_PINS_11                // PB11
#define UART3_R_RX_GPIO     GPIOB                       // PB11
#define UART3_R_RX_CRM_CLK  CRM_GPIOB_PERIPH_CLOCK      // PB11

#define RIGHT_UART           USART3

#define BUD_DETECT_RESIST_SWITCH_PIN        GPIO_PINS_1                 // PC1
#define BUD_DETECT_RESIST_SWITCH_GPIO       GPIOC                       // PC1
#define BUD_DETECT_RESIST_SWITCH_CRM_CLK    CRM_GPIOC_PERIPH_CLOCK      // PC1

#define OTG_SOF_PIN        GPIO_PINS_8                 // PA8
#define OTG_SOF_GPIO       GPIOA                       // PA8
#define OTG_SOF_CRM_CLK    CRM_GPIOA_PERIPH_CLOCK      // PA8

#define OTG_VBUS_PIN        GPIO_PINS_9                 // PA9
#define OTG_VBUS_GPIO       GPIOA                       // PA9
#define OTG_VBUS_CRM_CLK    CRM_GPIOA_PERIPH_CLOCK      // PA9

// NOTE: This configuration was required in early versions of Scala's circuit design,
// but has since been removed in later versions. It is currently used for experimental purposes only.
// Please disable this feature in future versions.
#ifdef SCALA_BOARD
#define OPEN_LID_MOS_SWITCH_PIN             GPIO_PINS_2                 // PC2
#define OPEN_LID_MOS_SWITCH_GPIO            GPIOC                       // PC2
#define OPEN_LID_MOS_SWITCH_CRM_CLK         CRM_GPIOC_PERIPH_CLOCK      // PC2
#endif

#define CPS4520_CHARGE_DETECT_PIN           GPIO_PINS_13                // PB13
#define CPS4520_CHARGE_DETECT_GPIO          GPIOB                       // PB13
#define CPS4520_CHARGE_DETECT_CRM_CLK       CRM_GPIOB_PERIPH_CLOCK      // PB13

#define CPS4520_CHARGE_INT_PIN              GPIO_PINS_5                 // PB5
#define CPS4520_CHARGE_INT_GPIO             GPIOB                       // PB5
#define CPS4520_CHARGE_INT_CRM_CLK          CRM_GPIOB_PERIPH_CLOCK      // PB5

#define I2C1_SCL_PIN                        GPIO_PINS_6                 // PB6
#define I2C1_SCL_GPIO_PORT                  GPIOB                       // PB6
#define I2C1_SCL_GPIO_CLK                   CRM_GPIOB_PERIPH_CLOCK      // PB6

#define I2C1_SDA_PIN                        GPIO_PINS_7                 // PB7
#define I2C1_SDA_GPIO_PORT                  GPIOB                       // PB7
#define I2C1_SDA_GPIO_CLK                   CRM_GPIOB_PERIPH_CLOCK      // PB7

#define I2C2_SCL_PIN                        GPIO_PINS_6                 // PF6
#define I2C2_SCL_GPIO_PORT                  GPIOF                       // PF6
#define I2C2_SCL_GPIO_CLK                   CRM_GPIOF_PERIPH_CLOCK      // PF6

#define I2C2_SDA_PIN                        GPIO_PINS_7                 // PF7
#define I2C2_SDA_GPIO_PORT                  GPIOF                       // PF7
#define I2C2_SDA_GPIO_CLK                   CRM_GPIOF_PERIPH_CLOCK      // PF7

#define PWM_R_PIN                           GPIO_PINS_6                 // PA6
#define PWM_R_GPIO_PORT                     GPIOA                       // PA6
#define PWM_R_GPIO_CLK                      CRM_TMR3_PERIPH_CLOCK      // PA6

#define PWM_G_PIN                           GPIO_PINS_7                 // PA7
#define PWM_G_GPIO_PORT                     GPIOA                       // PA7
#define PWM_G_GPIO_CLK                      CRM_TMR3_PERIPH_CLOCK      // PA7

#define PWM_B_PIN                           GPIO_PINS_0                 // PB0
#define PWM_B_GPIO_PORT                     GPIOB                       // PB0
#define PWM_B_GPIO_CLK                      CRM_TMR3_PERIPH_CLOCK      // PB0

#define I2C1_PORT                           I2C1
#define I2C1_CLK                            CRM_I2C1_PERIPH_CLOCK

#define I2C1_SPEED                          100000

#define I2C2_PORT                           I2C2
#define I2C2_CLK                            CRM_I2C2_PERIPH_CLOCK

#define I2C2_SPEED                          100000

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
