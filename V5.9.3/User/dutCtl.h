#ifndef __DUT_CTL_H__
#define __DUT_CTL_H__
#include "common.h"

#define Relay_ON()  		gpio_bit_set(GPIOE, GPIO_PIN_11)
#define Relay_OFF()  		gpio_bit_reset(GPIOE, GPIO_PIN_11)
#define DutVol_12()			gpio_bit_reset(GPIOE, GPIO_PIN_12)
#define DutVol_24()			gpio_bit_set(GPIOE, GPIO_PIN_12)

void DUTCTRL_Init(void);            // �̵��������ʼ��
void DUTCTRL_PowerOnOff(uint32 param);
void DUTCTRL_BusInit(void);         // ͨѶЭ���ʼ��

#endif

