#ifndef __SOURCE_VOLTAGE_H__
#define __SOURCE_VOLTAGE_H__

#include "common.h"
#include "timer.h"


#define SW_EN_SELECT()                      GPIO_WriteBit(GPIOB, GPIO_PIN_12, Bit_RESET) 
#define SW_EN_DISSELECT()                   GPIO_WriteBit(GPIOB, GPIO_PIN_12, Bit_SET)


#define SW_A_HIGH_LEVEL()                    GPIO_WriteBit(GPIOB, GPIO_PIN_15, Bit_SET) 
#define SW_A_LOW_LEVEL()                     GPIO_WriteBit(GPIOB, GPIO_PIN_15, Bit_RESET)  
#define SW_B_HIGH_LEVEL()                    GPIO_WriteBit(GPIOB, GPIO_PIN_14, Bit_SET)  
#define SW_B_LOW_LEVEL()                     GPIO_WriteBit(GPIOB, GPIO_PIN_14, Bit_RESET)  
#define SW_C_HIGH_LEVEL()                    GPIO_WriteBit(GPIOB, GPIO_PIN_13, Bit_SET)  
#define SW_C_LOW_LEVEL()                     GPIO_WriteBit(GPIOB, GPIO_PIN_13, Bit_RESET) 


// HC4051Í¨µÀ
typedef enum
{
	HC4051_INS0 =0,
	HC4051_INS1 =1,
	HC4051_INS2 =2,
	HC4051_INS3 =3,
	HC4051_INS4 =4,
	HC4051_INS5 =5  	
}HC4051_CHL;



void MC74HC4051ADR2G_GPIO_Config(void);

float PucsSource_VolRead(uint8_t channel);


#endif
