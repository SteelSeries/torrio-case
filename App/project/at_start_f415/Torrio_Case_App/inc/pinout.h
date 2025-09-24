
#pragma once
/*************************************************************************************************
 *                                          INCLUDES                                             *
 *************************************************************************************************/
#include "at32f415_board.h"

/*************************************************************************************************
 *                                   GLOBAL MACRO DEFINITIONS                                    *
 *************************************************************************************************/
#define USB_DET_PIN         GPIO_PINS_3                 // PC3
#define USB_DET_GPIO        GPIOC                       // PC3
#define USB_DET_CRM_CLK     CRM_GPIOC_PERIPH_CLOCK      // PC3

#define CHARGER_IRQ_PIN     GPIO_PINS_4                 // PA4
#define CHARGER_IRQ_GPIO    GPIOA                       // PA4
#define CHARGER_IRQ_CRM_CLK CRM_GPIOA_PERIPH_CLOCK      // PA4

#define HALL_OUT_PIN        GPIO_PINS_1                 // PB1
#define HALL_OUT_GPIO       GPIOB                       // PB1
#define HALL_OUT_CRM_CLK    CRM_GPIOB_PERIPH_CLOCK      // PB1

#define UART2_L_TX_PIN      GPIO_PINS_2                 // PA2
#define UART2_L_TX_GPIO     GPIOA                       // PA2
#define UART2_L_TX_CRM_CLK  CRM_GPIOA_PERIPH_CLOCK      // PA2

#define UART2_L_RX_PIN      GPIO_PINS_3                 // PA3
#define UART2_L_RX_GPIO     GPIOA                       // PA3
#define UART2_L_RX_CRM_CLK  CRM_GPIOA_PERIPH_CLOCK      // PA3

#define UART3_R_TX_PIN      GPIO_PINS_10                // PB10
#define UART3_R_TX_GPIO     GPIOB                       // PB10
#define UART3_R_TX_CRM_CLK  CRM_GPIOB_PERIPH_CLOCK      // PB10

#define UART3_R_RX_PIN      GPIO_PINS_11                // PB11
#define UART3_R_RX_GPIO     GPIOB                       // PB11
#define UART3_R_RX_CRM_CLK  CRM_GPIOB_PERIPH_CLOCK      // PB11

#define LED_R_PIN           GPIO_PINS_6                 // PA6
#define LED_R_GPIO          GPIOA                       // PA6
#define LED_R_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK      // PA6

#define LED_G_PIN           GPIO_PINS_7                 // PA7
#define LED_G_GPIO          GPIOA                       // PA7
#define LED_G_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK      // PA7

#define LED_B_PIN           GPIO_PINS_0                 // PB0
#define LED_B_GPIO          GPIOB                       // PB0
#define LED_B_CRM_CLK       CRM_GPIOB_PERIPH_CLOCK      // PB0

#define BUD_DETECT_RESIST_SWITCH_PIN        GPIO_PINS_1                 // PC1
#define BUD_DETECT_RESIST_SWITCH_GPIO       GPIOC                       // PC1
#define BUD_DETECT_RESIST_SWITCH_CRM_CLK    CRM_GPIOC_PERIPH_CLOCK      // PC1

#define OPEN_LID_MOS_SWITCH_PIN             GPIO_PINS_2                 // PC2
#define OPEN_LID_MOS_SWITCH_GPIO            GPIOC                       // PC2
#define OPEN_LID_MOS_SWITCH_CRM_CLK         CRM_GPIOC_PERIPH_CLOCK      // PC2

#define QI_CHARGE_DETECT_PIN                GPIO_PINS_13                // PB13
#define QI_CHARGE_DETECT_GPIO               GPIOB                       // PB13
#define QI_CHARGE_DETECT_CRM_CLK            CRM_GPIOB_PERIPH_CLOCK      // PB13

/*************************************************************************************************
 *                                    GLOBAL TYPE DEFINITIONS                                    *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL VARIABLE DECLARATIONS                                 *
 *************************************************************************************************/

/*************************************************************************************************
 *                                  GLOBAL FUNCTION DECLARATIONS                                 *
 *************************************************************************************************/
