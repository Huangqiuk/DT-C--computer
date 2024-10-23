#include "Source_Voltage.h"
#include "Source_Adc.h"
#include "Source_Relay.h"


/***************************************************************
 * 函 数 名：MC74HC4051ADR2G_GPIO_Config
 * 函数入参：None
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：MC74HC4051ADR2G芯片引脚配置
***************************************************************/
void MC74HC4051ADR2G_GPIO_Config(void)
{
     GPIO_InitType GPIO_InitStructure;
    // Enable GPIO clocks
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA|RCC_APB2_PERIPH_GPIOB, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15 ;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_12mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Down;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);


}

/***************************************************************
 * 函 数 名：MC74HC4051ADR2G_Select_Channel
 * 函数入参：channel 通道号：1-48
 * 函数出参：None
 * 返 回 值：None
 * 功能描述：MC74HC4051ADR2G选择通道
***************************************************************/
void MC74HC4051ADR2G_Select_Channel(uint8_t channel)
{	
    /* 使能通道选择 */
	if(channel>5)
		return;
	
   	SW_EN_SELECT();
    switch(channel)
    {
        case 0:
            SW_A_LOW_LEVEL();
            SW_B_LOW_LEVEL();
            SW_C_LOW_LEVEL();
            break;
        case 1:
            SW_A_HIGH_LEVEL();
            SW_B_LOW_LEVEL();
            SW_C_LOW_LEVEL();
            break;
        case 2:
            SW_A_LOW_LEVEL();
            SW_B_HIGH_LEVEL();
            SW_C_LOW_LEVEL();
            break;
        case 3:
            SW_A_HIGH_LEVEL();
            SW_B_HIGH_LEVEL();
            SW_C_LOW_LEVEL();
            break;
        case 4:
            SW_A_LOW_LEVEL();
            SW_B_LOW_LEVEL();
            SW_C_HIGH_LEVEL();
            break;
        case 5:
            SW_A_HIGH_LEVEL();
            SW_B_LOW_LEVEL();
            SW_C_HIGH_LEVEL();
            break;
        case 6:
            SW_A_LOW_LEVEL();
            SW_B_HIGH_LEVEL();
            SW_C_HIGH_LEVEL();
            break;
        case 7:
            SW_A_HIGH_LEVEL();
            SW_B_HIGH_LEVEL();
            SW_C_HIGH_LEVEL();
            break;

  }
}
//读取通道电压值
//输入channel范围：0-5
//主要：0-2通道最大可测80V，3-5最大可测6V
float PucsSource_VolRead(uint8_t channel)
{
	  float voltage=0;
	  uint16_t adc=0;
      uint32_t i=0;	   
	  MC74HC4051ADR2G_Select_Channel( channel);	  
	  Delayms(20) ;      	
	  adc=Get_AdcValue(ADC_ANS);
	  SW_EN_DISSELECT();
      if(channel<=2)
	  {		  
	     voltage=adc/4096.0*3.0*23.26;
	     return voltage;
	  }
	  else
	  {
	      voltage=adc/4096.0*3.0*2.0;
	      return voltage;	  
	  }
	
}	


