#include "common.h"
#include "adc1.h"
#include "led.h"
#include "timer.h"



/***************************************************************
 * 函 数 名：Get_ADC1_Value
 * 函数入参：uint8_t channel -> 通道号
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：获取ADC1值
***************************************************************/
uint16_t Get_ADC1_Value(uint8_t channel)
{
  	/* 设置指定ADC的规则组通道，一个序列，采样时间 */
	ADC_ConfigRegularChannel(ADC, channel, 1, ADC_SAMP_TIME_55CYCLES5);	  	  
	ADC_EnableSoftwareStartConv(ADC, ENABLE);
    while(!ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC));
	return ADC_GetDat(ADC);	
	ADC_EnableSoftwareStartConv(ADC, DISABLE);
	ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
}

/***************************************************************
 * 函 数 名：Get_ADC1_Average
 * 函数入参：uint8_t channel -> 通道号
             uint8_t times -> 读取次数
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：获取ADC1平均值
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
 * 函 数 名：ADC1_DEVICE_Config
 * 函数入参：None
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：ADC1外设初始化
***************************************************************/
static void ADC1_DEVICE_Config(void)
{
//    ADC_InitTypeDef     ADC_InitStructure;
//    
//    /* 使能ADC1时钟 */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//    
//    /* 设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M */
//    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
//    
//    /* 复位ADC1 */
//    ADC_DeInit(ADC1);
//    
//    /* 独立模式 */
//    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
//    /* 使用单通道模式 */
//    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
//    /* 禁止连续转换模式 */
//    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//    /* 不使用外部触发源用于常规通道转换 */
//    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//    /* 数据右对齐方式 */
//    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//    /* ADC通道转换个数：1 */
//    ADC_InitStructure.ADC_NbrOfChannel = 1;
//    /* 配置ADC1参数 */
//    ADC_Init(ADC1, &ADC_InitStructure);
//    
//    /* 使能ADC1 */
//    ADC_Cmd(ADC1, ENABLE);
//    
//    /* 使能ADC1复位校准 */
//    ADC_ResetCalibration(ADC1);
//    /* 等待复位校准结束 */
//	while(ADC_GetResetCalibrationStatus(ADC1));
//    
//	/* 开启ADC1校准 */
//	ADC_StartCalibration(ADC1);
//    /* 等待校准结束 */
//    while(ADC_GetCalibrationStatus(ADC1));

//    /* 开始转换 */
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
    // ADC的输入时钟不得超过64MHz
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
    // 建议每次上电执行一次校准
    // Start ADC1 calibration
    ADC_StartCalibration(ADC);
    // Check the end of ADC1 calibration
    while (ADC_GetCalibrationStatus(ADC));
    
	// 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
}



/***************************************************************
 * 函 数 名：ADC1_Init
 * 函数入参：None
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：ADC1初始化
***************************************************************/
void ADC1_Init(void)
{       
    /* 配置ADC1外设 */
    ADC1_DEVICE_Config();
}

/***************************************************************
 * 函 数 名：Get_ADC_Actual_Value
 * 函数入参：adc_value -> 累计的ADC值
             resistance_ratio -> 分压电阻值
 * 函数出参：None
 * 返 回 值：ADC实际电压值
 * 功能描述：获取ADC实际值
***************************************************************/
float Get_ADC_Actual_Value(uint32_t adc_value, uint8_t resistance)
{
    uint32_t adc_average_value = 0;
    float adc_conversion_value = 0;
    
    /* 将获取到的多次ADC值进行取平均值 */
    adc_average_value = adc_value / GET_ADC_VALUE_FREQUENCY;
    /* 计算ADC转换值 */
    adc_conversion_value = (float)adc_average_value * (REFERENCE_VOLTAGE/NUM_OF_DIGITS_MAX);
    
    /* 返回实际电压值 */
    return adc_conversion_value * resistance;
}
