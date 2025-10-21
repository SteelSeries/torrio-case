/*************************************************************************************************
 *                                         INCLUDES                                              *
 *************************************************************************************************/
#include "adc.h"
#include <string.h>
/*************************************************************************************************
 *                                  LOCAL MACRO DEFINITIONS                                      *
 *************************************************************************************************/
// Number of ADC samples per read for averaging
#define ADC_DMA_BUFFER_SIZE 8
#define ADC_REF_VOLTAGE_MV 3300
#define ADC_RESOLUTION 4095
/*************************************************************************************************
 *                                  LOCAL TYPE DEFINITIONS                                       *
 *************************************************************************************************/
/*************************************************************************************************
 *                                GLOBAL VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
/*************************************************************************************************
 *                                STATIC VARIABLE DEFINITIONS                                    *
 *************************************************************************************************/
static Adc_HardwareSettings_t user_hardware_settings = {0};
static __IO uint16_t adc1_ordinary_valuetab[ADC_DMA_BUFFER_SIZE] = {0};

/*************************************************************************************************
 *                                STATIC FUNCTION DECLARATIONS                                   *
 *************************************************************************************************/
static void DmaConfig(void);
static void AdcConfig(void);
static uint16_t GetAvgRawValueHandle(void);

/*************************************************************************************************
 *                                GLOBAL FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
void Adc_GpioConfigHardware(const Adc_HardwareSettings_t *hardware_settings)
{
    gpio_init_type gpio_init_struct;
    memcpy(&user_hardware_settings, hardware_settings, sizeof(Adc_HardwareSettings_t));

    crm_periph_clock_enable(user_hardware_settings.adc_gpio_crm_clk, TRUE);

    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = user_hardware_settings.adc_gpio_pin;
    gpio_init(user_hardware_settings.adc_gpio_port, &gpio_init_struct);
}

void Adc_Init(void)
{
    DmaConfig();
    AdcConfig();
}

void Adc_GetAvgRawAndVoltage(uint16_t *adc_raw, uint16_t *voltage_mv)
{
    uint16_t adc_value = GetAvgRawValueHandle();

    if (adc_raw != NULL)
    {
        *adc_raw = adc_value;
    }

    if (voltage_mv != NULL)
    {
        *voltage_mv = (uint16_t)(((uint32_t)adc_value * ADC_REF_VOLTAGE_MV) / ADC_RESOLUTION);
    }
}

/*************************************************************************************************
 *                                STATIC FUNCTION DEFINITIONS                                    *
 *************************************************************************************************/
static uint16_t GetAvgRawValueHandle(void)
{
    DEBUG_PRINT("[%s] ", __func__);
    uint32_t Adc_Avg_Value_temp = 0;

    for (uint8_t sample_times_index = 0; sample_times_index < ADC_DMA_BUFFER_SIZE; sample_times_index++)
    {
        DEBUG_PRINT("%d ", adc1_ordinary_valuetab[sample_times_index]);
        Adc_Avg_Value_temp += adc1_ordinary_valuetab[sample_times_index];
    }
    DEBUG_PRINT("\n");
    Adc_Avg_Value_temp = Adc_Avg_Value_temp >> 3;
    return Adc_Avg_Value_temp;
}

static void DmaConfig(void)
{
    dma_init_type dma_init_struct;
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);

    nvic_irq_enable(DMA1_Channel1_IRQn, 0, 0);

    dma_reset(DMA1_CHANNEL1);

    dma_default_para_init(&dma_init_struct);

    dma_init_struct.buffer_size = ADC_DMA_BUFFER_SIZE;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t)adc1_ordinary_valuetab;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init_struct.memory_inc_enable = TRUE;

    dma_init_struct.peripheral_base_addr = (uint32_t)&(ADC1->odt);
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
    dma_init_struct.peripheral_inc_enable = FALSE;

    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = TRUE;

    dma_init(DMA1_CHANNEL1, &dma_init_struct);

    dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, TRUE);

    /* enable DMA after ADC activation */
    dma_channel_enable(DMA1_CHANNEL1, TRUE);
}

static void AdcConfig(void)
{
    adc_base_config_type adc_base_struct;
    crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);
    adc_reset(ADC1);


    crm_adc_clock_div_set(CRM_ADC_DIV_4);

    
    adc_base_default_para_init(&adc_base_struct);
    /* ADC1 config */
    adc_base_struct.sequence_mode = FALSE;
    adc_base_struct.repeat_mode = FALSE;
    adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
    adc_base_struct.ordinary_channel_length = 1;
    adc_base_config(ADC1, &adc_base_struct);

    adc_ordinary_channel_set(ADC1, ADC_CHANNEL_10, 1, ADC_SAMPLETIME_28_5);
    adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_TMR4CH4, TRUE);
    adc_dma_mode_enable(ADC1, TRUE);

    adc_enable(ADC1, TRUE);

    /* ADC calibration */
    adc_calibration_init(ADC1);
    while (adc_calibration_init_status_get(ADC1))
        ;
    adc_calibration_start(ADC1);
    while (adc_calibration_status_get(ADC1))
        ;
}
