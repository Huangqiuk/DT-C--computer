/********************************************************************************
  * 文 件 名: led.c
  * 版 本 号: 初版            
  *******************************************************************************/
  
/********************************* 头文件************************************/
#include "common.h"
#include "delay.h"
#include "led.h"

/******************************** 宏定义*********************************/

/*********************************************************************
* 函   数  名: LED_Init
* 函数入参: void
* 函数出参: 无
* 返   回  值: void 
* 功能描述: Led灯初始化
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
* 函   数  名: LED_Reversal
* 函数入参: uint32 param
* 函数出参: 无
* 返   回  值: void 
* 功能描述: Led灯反转
***********
* 修改历史:
*   1.修改作者: zlb
*     修改日期: 2019/4/3
*     修改描述: 新函数 		   
**********************************************************************/

void LED_Reversal(void)
{
    static uint8 i;
	// led灯反转
	if((i++) % 2)
	{	
		GPIO_SetBits(GPIOA, GPIO_PIN_0);
		 
	}
	else
	{									
		GPIO_ResetBits(GPIOA, GPIO_PIN_0);				// led灯亮 GPIO_Pin_13
	}
}



