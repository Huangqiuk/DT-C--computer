#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "common.h"

extern volatile uint32 SystickTimingCnt;

// 初始化系统滴答时钟
void SysTick_Init(uint32 ntime);
// 延时us函数
void Systick_Delayus(uint32 nTime);
// 延时ms函数
void Systick_Delayms(uint32 nTime);

// 简单延时
void Delayms(uint16 i);
#endif
