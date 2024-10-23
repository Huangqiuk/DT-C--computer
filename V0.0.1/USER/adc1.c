#include "common.h"
#include "adc1.h"
#include "led.h"
#include "timer.h"



/***************************************************************
 * �� �� ����Get_ADC1_Value
 * ������Σ�uint8_t channel -> ͨ����
 * �������Σ�None
 * �� �� ֵ��None
 * ������������ȡADC1ֵ
***************************************************************/
uint16_t Get_ADC1_Value(uint8_t channel)
{
  	/* ����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ�� */
	ADC_ConfigRegularChannel(ADC, channel, 1, ADC_SAMP_TIME_55CYCLES5);	  	  
	ADC_EnableSoftwareStartConv(ADC, ENABLE);
    while(!ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC));
	return ADC_GetDat(ADC);	
	ADC_EnableSoftwareStartConv(ADC, DISABLE);
	ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
}

/***************************************************************
 * �� �� ����Get_ADC1_Average
 * ������Σ�uint8_t channel -> ͨ����
             uint8_t times -> ��ȡ����
 * �������Σ�None
 * �� �� ֵ��None
 * ������������ȡADC1ƽ��ֵ
***************************************************************/
uint16_t Get_ADC1_Average(uint8_t channel, uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t t;
    
	for(t = 0; t < times; t++)
	{
		temp_val += Get_ADC1_Value(channel);
		Delayms(2);
	}
    
	return temp_val/times;
}

/***************************************************************
 * �� �� ����ADC1_DEVICE_Config
 * ������Σ�None
 * �������Σ�None
 * �� �� ֵ��None
 * ����������ADC1�����ʼ��
***************************************************************/
static void ADC1_DEVICE_Config(void)
{
//    ADC_InitTypeDef     ADC_InitStructure;
//    
//    /* ʹ��ADC1ʱ�� */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//    
//    /* ����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M */
//    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
//    
//    /* ��λADC1 */
//    ADC_DeInit(ADC1);
//    
//    /* ����ģʽ */
//    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
//    /* ʹ�õ�ͨ��ģʽ */
//    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
//    /* ��ֹ����ת��ģʽ */
//    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//    /* ��ʹ���ⲿ����Դ���ڳ���ͨ��ת�� */
//    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//    /* �����Ҷ��뷽ʽ */
//    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//    /* ADCͨ��ת��������1 */
//    ADC_InitStructure.ADC_NbrOfChannel = 1;
//    /* ����ADC1���� */
//    ADC_Init(ADC1, &ADC_InitStructure);
//    
//    /* ʹ��ADC1 */
//    ADC_Cmd(ADC1, ENABLE);
//    
//    /* ʹ��ADC1��λУ׼ */
//    ADC_ResetCalibration(ADC1);
//    /* �ȴ���λУ׼���� */
//	while(ADC_GetResetCalibrationStatus(ADC1));
//    
//	/* ����ADC1У׼ */
//	ADC_StartCalibration(ADC1);
//    /* �ȴ�У׼���� */
//    while(ADC_GetCalibrationStatus(ADC1));

//    /* ��ʼת�� */
//    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    GPIO_InitType GPIO_InitStructure;
	ADC_InitType ADC_InitStructure;

    // Enable GPIO clocks
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    // Enable ADC clocks
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_4;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // RCC_ADCHCLK_DIVx
    // ADC������ʱ�Ӳ��ó���64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
	RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

	// ADC configuration
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

	ADC_ConfigRegularChannel(ADC, ADC_CH_5, 1, ADC_SAMP_TIME_55CYCLES5);
    // Enable ADC
    ADC_Enable(ADC, ENABLE);
    // Check ADC Ready
    while(ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);
    // ����ÿ���ϵ�ִ��һ��У׼
    // Start ADC1 calibration
    ADC_StartCalibration(ADC);
    // Check the end of ADC1 calibration
    while (ADC_GetCalibrationStatus(ADC));
    
	// ����ADCת��
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}



/***************************************************************
 * �� �� ����ADC1_Init
 * ������Σ�None
 * �������Σ�None
 * �� �� ֵ��None
 * ����������ADC1��ʼ��
***************************************************************/
void ADC1_Init(void)
{       
    /* ����ADC1���� */
    ADC1_DEVICE_Config();
}

/***************************************************************
 * �� �� ����Get_ADC_Actual_Value
 * ������Σ�adc_value -> �ۼƵ�ADCֵ
             resistance_ratio -> ��ѹ����ֵ
 * �������Σ�None
 * �� �� ֵ��ADCʵ�ʵ�ѹֵ
 * ������������ȡADCʵ��ֵ
***************************************************************/
float Get_ADC_Actual_Value(uint32_t adc_value, uint8_t resistance)
{
    uint32_t adc_average_value = 0;
    float adc_conversion_value = 0;
    
    /* ����ȡ���Ķ��ADCֵ����ȡƽ��ֵ */
    adc_average_value = adc_value / GET_ADC_VALUE_FREQUENCY;
    /* ����ADCת��ֵ */
    adc_conversion_value = (float)adc_average_value * (REFERENCE_VOLTAGE/NUM_OF_DIGITS_MAX);
    
    /* ����ʵ�ʵ�ѹֵ */
    return adc_conversion_value * resistance;
}
