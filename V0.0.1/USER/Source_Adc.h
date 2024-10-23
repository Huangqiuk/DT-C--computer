#ifndef __SOURCE_ADC_H__
#define __SOURCE_ADC_H__

#include "common.h"
#include "timer.h"


#define MAX_CHL_NUM 4
//ADC结构体
typedef struct{
	uint16_t      Pin;
	GPIO_Module*  GPIOx;	
    uint8_t       ADC_Channel;
}ADC_CHL;


// adc用途
typedef enum
{
	ADC_ANS =0,
	ADC_CURRENT0,
	ADC_CURRENT1,
	ADC_OHM
}ADC_PURPOSE;



void Source_ADC_HwInit(void);
uint16_t Get_AdcValue(ADC_PURPOSE adcPurpose);



#endif

