#ifndef __WATCH_H_
#define __WATCH_H_

// 看门狗使能开关
#define WDT_ENABLE              0

#define WDT_RESET_TIME 			3000U		// 看门狗复位时间，单位:ms

// 函数声明
void WDT_Init(void);
void WDT_Clear(void);

#endif
