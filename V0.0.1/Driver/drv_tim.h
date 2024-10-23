//-----------------------------------------------------------------------------
// drv_tim.h
//-----------------------------------------------------------------------------
#ifndef __DRV_TIM_H_
#define __DRV_TIM_H_

#include "type.h"

//-----------------------------------------------------------------------------
// Systick for delay 
//-----------------------------------------------------------------------------
void Systick_init(void);
void HwSleepMs(u32 nms);
void HwSleepUs(u32 nus);
#endif

