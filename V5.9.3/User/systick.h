#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "common.h"

extern volatile uint32 SystickTimingCnt;

// ��ʼ��ϵͳ�δ�ʱ��
void SysTick_Init(uint32 ntime);
// ��ʱus����
void Systick_Delayus(uint32 nTime);
// ��ʱms����
void Systick_Delayms(uint32 nTime);

// ����ʱ
void Delayms(uint16 i);
#endif
