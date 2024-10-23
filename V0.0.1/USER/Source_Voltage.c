#include "Source_Voltage.h"
#include "Source_Adc.h"
#include "Source_Relay.h"


/***************************************************************
 * �� �� ����MC74HC4051ADR2G_GPIO_Config
 * ������Σ�None
 * �������Σ�None
 * �� �� ֵ��None
 * ����������MC74HC4051ADR2GоƬ��������
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
 * �� �� ����MC74HC4051ADR2G_Select_Channel
 * ������Σ�channel ͨ���ţ�1-48
 * �������Σ�None
 * �� �� ֵ��None
 * ����������MC74HC4051ADR2Gѡ��ͨ��
***************************************************************/
void MC74HC4051ADR2G_Select_Channel(uint8_t channel)
{	
    /* ʹ��ͨ��ѡ�� */
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
//��ȡͨ����ѹֵ
//����channel��Χ��0-5
//��Ҫ��0-2ͨ�����ɲ�80V��3-5���ɲ�6V
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


