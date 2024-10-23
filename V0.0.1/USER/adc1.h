#ifndef __ADC1_H__
#define __ADC1_H__

#include "n32l40x.h"
#include "delay.h"


#define GET_ADC_VALUE_FREQUENCY             10          // 获取ADC值次数

#define ADC1_COUNT_TIME                     20           // 计数周期：20ms
#define ADC1_COUNT_LED_KEEP                 5000        // LED保持时间：5s

#define REFERENCE_VOLTAGE                   3.3         // 参考电压：3.3V
#define NUM_OF_DIGITS_MAX                   4096        // 12位AD采样最大数字量：4096

#define STM32_ADC_IN0_MC74                  ADC_Channel_10
#define STM32_ADC_IN1_VCC_CON0              ADC_Channel_11
#define STM32_ADC_IN2_VCC_CON1              ADC_Channel_12
#define STM32_ADC_IN3_12V_CON0              ADC_Channel_13
#define STM32_ADC_IN4_12V_CON1              ADC_Channel_0
#define STM32_ADC_IN5_5V_CON0               ADC_Channel_1
#define STM32_ADC_IN6_5V_CON1               ADC_Channel_2
#define STM32_ADC_IN7_3V3_CON0              ADC_Channel_3
#define STM32_ADC_IN8_3V3_CON1              ADC_Channel_4

void ADC1_Init(void);
uint16_t Get_ADC1_Value(uint8_t channel);
uint16_t Get_ADC1_Average(uint8_t ch, uint8_t times);
void ADC1_Process(void);


#endif
