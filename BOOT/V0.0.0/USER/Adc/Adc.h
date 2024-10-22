#ifndef _ADC_H_
#define _ADC_H_

#include "main.h"

//ADC按键初始化
void Adc_Init();
//获取原始ADC
uint16_t Get_Key_Adc();
//获取滤波后的ADC
uint16_t Get_Stable_Key_Adc();


#endif//_ADC_H_
