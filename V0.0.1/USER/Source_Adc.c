#include "Source_Adc.h"


ADC_CHL adc[MAX_CHL_NUM]={
                           {GPIO_PIN_0,GPIOA,ADC_CH_1},{GPIO_PIN_1,GPIOA,ADC_CH_2},{GPIO_PIN_2,GPIOA,ADC_CH_3},{GPIO_PIN_3,GPIOA,ADC_CH_4}
                         };


// 硬件ADC0初始化
void Source_ADC_HwInit(void)
{
	GPIO_InitType GPIO_InitStructure;
	ADC_InitType ADC_InitStructure;

    // Enable GPIO clocks
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    // Enable ADC clocks
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = adc[0].Pin | adc[1].Pin|adc[2].Pin|adc[3].Pin;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(adc[0].GPIOx, &GPIO_InitStructure);

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

	ADC_ConfigRegularChannel(ADC, adc[0].ADC_Channel, 1, ADC_SAMP_TIME_55CYCLES5);
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


//获取用途ADC的值
uint16_t Get_AdcValue(ADC_PURPOSE adcPurpose)
{
  uint32_t adcVar=0;	
  uint32_t value[10]={0};
  uint32_t MIN;
  uint32_t MAX;	
  

  ADC_ClearFlag(ADC, ADC_FLAG_ENDC);	
  ADC_ConfigRegularChannel(ADC, adc[adcPurpose].ADC_Channel, 1, ADC_SAMP_TIME_55CYCLES5);
  
  for(uint8_t i=0;i<10;++i)
  {
	  ADC_EnableSoftwareStartConv(ADC, ENABLE);
	  while(!ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC))	;
	  value[i]=ADC_GetDat(ADC);	
	  ADC_EnableSoftwareStartConv(ADC, DISABLE);
	  ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
  }	 
  MIN= value[0];
  MAX= value[0];	  
  for(uint8_t i=0;i<10;++i)
  {
	if(value[i]<MIN)
	{
		MIN=value[i];
	}
	if(value[i]>MAX)
	{
		MAX=value[i];
	}
  } 	  
  adcVar=(value[0]+value[1]+value[2]+value[3]+value[4]+value[5]+value[6]+value[7]+value[8]+value[9]-MIN-MAX)/8;	  
  return (uint16_t) adcVar;
}










