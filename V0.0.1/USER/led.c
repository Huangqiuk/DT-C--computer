/********************************************************************************
  * �� �� ��: led.c
  * �� �� ��: ����            
  *******************************************************************************/
  
/********************************* ͷ�ļ�************************************/
#include "common.h"
#include "delay.h"
#include "led.h"

/******************************** �궨��*********************************/

/*********************************************************************
* ��   ��  ��: LED_Init
* �������: void
* ��������: ��
* ��   ��  ֵ: void 
* ��������: Led�Ƴ�ʼ��
***********	   
**********************************************************************/
void LED_Init(void)
{
	
	GPIO_InitType GPIO_InitStructure;
    // Enable GPIO clocks
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_8mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Down;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
* ��   ��  ��: LED_Reversal
* �������: uint32 param
* ��������: ��
* ��   ��  ֵ: void 
* ��������: Led�Ʒ�ת
***********
* �޸���ʷ:
*   1.�޸�����: zlb
*     �޸�����: 2019/4/3
*     �޸�����: �º��� 		   
**********************************************************************/

void LED_Reversal(void)
{
    static uint8 i;
	// led�Ʒ�ת
	if((i++) % 2)
	{	
		GPIO_SetBits(GPIOA, GPIO_PIN_0);
		 
	}
	else
	{									
		GPIO_ResetBits(GPIOA, GPIO_PIN_0);				// led���� GPIO_Pin_13
	}
}



