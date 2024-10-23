#ifndef __POWER_CTL_H__
#define __POWER_CTL_H__	 

#include "common.h"

#define POWER_ON()						gpio_bit_set(GPIOB, GPIO_PIN_1)
#define POWER_OFF()						gpio_bit_reset(GPIOB, GPIO_PIN_1)

#define LCD_BL_ON()						gpio_bit_set(GPIOA, GPIO_PIN_8)
#define LCD_BL_OFF()					gpio_bit_reset(GPIOA, GPIO_PIN_8)

void POWER_Init(void);

		 				    
#endif

