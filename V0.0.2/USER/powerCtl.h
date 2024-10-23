#ifndef __POWER_CTL_H__
#define __POWER_CTL_H__	 

#include "n32l40x_gpio.h"
#include "common.h"
#include "delay.h"

#define BOOST_OFF()					  GPIO_SetBits(GPIOB, GPIO_PIN_9)
#define BOOST_ON()			     	GPIO_ResetBits(GPIOB, GPIO_PIN_9)
#define BUCK_OFF()				  	GPIO_SetBits(GPIOB, GPIO_PIN_8)
#define BUCK_ON()		     			GPIO_ResetBits(GPIOB, GPIO_PIN_8)
#define VOUT_SW0_ON()					GPIO_SetBits(GPIOB, GPIO_PIN_13)
#define VOUT_SW0_OFF()			  GPIO_ResetBits(GPIOB, GPIO_PIN_13)
#define VOUT_SW1_ON()					GPIO_SetBits(GPIOB, GPIO_PIN_14)
#define VOUT_SW1_OFF()			  GPIO_ResetBits(GPIOB, GPIO_PIN_14)
#define VOUT_EN_ON()					GPIO_SetBits(GPIOB, GPIO_PIN_12)
#define VOUT_EN_OFF()			    GPIO_ResetBits(GPIOB, GPIO_PIN_12)

#define A_H()               GPIO_SetBits(GPIOC, GPIO_PIN_13)
#define A_L()               GPIO_ResetBits(GPIOC, GPIO_PIN_13)
#define B_H()               GPIO_SetBits(GPIOA, GPIO_PIN_0)
#define B_L()               GPIO_ResetBits(GPIOA, GPIO_PIN_0)
#define C_H()               GPIO_SetBits(GPIOA, GPIO_PIN_1)
#define C_L()               GPIO_ResetBits(GPIOA, GPIO_PIN_1)
#define G2A_H()             GPIO_SetBits(GPIOA, GPIO_PIN_2)
#define G2A_L()             GPIO_ResetBits(GPIOA, GPIO_PIN_2)
#define G2B_H()             GPIO_SetBits(GPIOA, GPIO_PIN_3)
#define G2B_L()             GPIO_ResetBits(GPIOA, GPIO_PIN_3)
#define G1_H()              GPIO_SetBits(GPIOA, GPIO_PIN_4)
#define G1_L()              GPIO_ResetBits(GPIOA, GPIO_PIN_4)

/* 函数声明 ---------------------------------------------------------------------------------------------------------*/
// 引脚初始化
void Power_Init(void);

// 获取电源板输入电压
uint16_t Get_Vin_Voltage(void);

// 获取升压后的电压
uint32_t Get_Boost_Voltage(void);

// 获取降压后的电压
uint16_t Get_Buck_Voltage(void);

// 获取仪表供电电流
uint16_t Get_Vout_Current(void);	

// ADC引脚初始化
void Power_ADC(void);

#endif

