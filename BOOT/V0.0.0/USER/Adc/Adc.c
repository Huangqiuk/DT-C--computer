#include "Adc.h"


void Adc_Init(){
	
  GPIO_InitType GPIO_InitStructure;
	ADC_InitType ADC_InitStructure;
	//--------------------RCC----------------------
	/* RCC_ADCHCLK_DIV1*/
	RCC_Hclk_Config(RCC_SYSCLK_DIV1);
  /* Enable GPIOA clocks */
  RCC_APB_Peripheral_Clock_Enable(RCC_APB_PERIPH_IOPA);
  /* Enable ADC clocks */
	RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_ADC);
	RCC_ADC_Clock_Config(RCC_ADC_DIV2);//24M���
  /* enable ADC 1M clock */
  RCC_ADC_1M_Clock_Config(RCC_ADC1MCLK_DIV48);//1M���
	
	//------------------GPIOA.1 -- channle:0---------------------

  GPIO_Structure_Initialize(&GPIO_InitStructure);
    /* Configure PA1  as analog input*/
  GPIO_InitStructure.Pin       = GPIO_PIN_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
  GPIO_Peripheral_Initialize(GPIOA, &GPIO_InitStructure);	
	
    /*----------------------ADC configuration -----------------*/
    ADC_InitStructure.MultiChEn      = ENABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_REGULAR_SWSTRRCH;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = ADC_REGULAR_LEN_2;
    ADC_Initializes(&ADC_InitStructure);

    /* Enable ADC */
    ADC_ON();
    /* Check ADC Ready */
    while(ADC_Flag_Status_Get(ADC_RD_FLAG,ADC_FLAG_STR,ADC_FLAG_RDY) == RESET);
    while(ADC_Flag_Status_Get(ADC_RD_FLAG,ADC_FLAG_STR,ADC_FLAG_PD_RDY));	
}

uint16_t Get_Key_Adc(){

	ADC_Regular_Sequence_Multi_Config(ADC_Channel_00_PA1,0);
	
	//ADC_Regular_Sequence_Multi_Config(ADC_Channel_04_PA12,1);
	/* Configure channel sample time ----------------------*/
	ADC_Channel_Sample_Time_Config(ADC_SAMP_TIME_240CYCLES);
	/* enable software  conversion-------------------------*/
	ADC_Regular_Channels_Software_Conversion_Operation(ADC_EXTRTRIG_SWSTRRCH_ENABLE);
	
	/* wait end of conversion flag-------------------------*/
	while(ADC_Flag_Status_Get(ADC_RUN_FLAG ,ADC_FLAG_ENDC ,ADC_FLAG_RDY)==0){
		}
	ADC_Flag_Status_Clear(ADC_FLAG_ENDC);
	ADC_Flag_Status_Clear(ADC_FLAG_STR);
	
	return ADC_Regular_Group_Conversion_Data_Get(0);
}
//获取稳定的ADC
uint16_t Get_Stable_Key_Adc(){

	//连续采集12个键值，取中间4个取平均
	uint16_t ADC_Result[12] = {0};
	uint16_t adc_temp=0;
	uint32_t sum = 0;
	for(uint8_t i=0; i < 12; i++){
		ADC_Result[i] = Get_Key_Adc();
	}
		for(uint8_t i=0; i<12; i++){
		//printf("%d\t", ADC_Result[i]);
	}
		//printf("\r\n");
	for(uint8_t i=0; i<12-1; i++){
		for(uint8_t j=0; j<12-i-1; j++){
			if(ADC_Result[j] > ADC_Result[j+1]){
				adc_temp = ADC_Result[j];
				ADC_Result[j] = ADC_Result[j+1];
				ADC_Result[j+1] = adc_temp;
			}
		}
	}
	for(uint8_t i=0; i < 4; i++){
		sum += ADC_Result[4 +	i];
	}
		//printf("\r\n---------------------------------\r\n");
	return sum>>2;
}


