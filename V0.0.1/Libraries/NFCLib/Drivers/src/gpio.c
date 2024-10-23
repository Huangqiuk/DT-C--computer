/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

#include <stdint.h>
#include <driver_config.h>
#include "common.h"

void gpio_Init( void )
{
	
	GPIO_InitType GPIO_InitStructure;
	
	// GPIO clock enable
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	// PN7150_VEN
	GPIO_InitStructure.Pin			  = GPIO_PIN_11;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull	  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode	  = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	// PN7150_IRQ
	GPIO_InitStructure.Pin			  = GPIO_PIN_12;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull	  = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode	  = GPIO_Mode_Input;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}





