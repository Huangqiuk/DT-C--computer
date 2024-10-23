#ifndef _LED_H_
#define _LED_H_

#include "common.h"
#include "pwmled.h"

// 灯亮度
#define LED_PWMBULLBUFF    						100
#define LED_PWMBUFF    							100
#define LED_PWMBUFF0                           	0  



// LED 刷新显示结构体
typedef struct
{
	BOOL   Enblink;
	BOOL   Enblink2;
}LEDCOMMON_CB;


extern LEDCOMMON_CB ledCommonCB;


// LED 全亮灭控制
void LED_AllOnOff(uint32 data);

// LED 亮灭开关
void LED_SetLedOnOff(LED_NAME seg,  BOOL onoff);

// 骑行状态显示
void LED_RidingStateShow(BOOL  refreshAll);

// 电量灯控制
void  LED_BatLedSwitching(BOOL enable);

// 档位灯个数闪烁
void  LED_AssistLedBlink(uint32 param);

// 蓝牙升级状态
void  LED_UpradeStateShow(uint32 param);


// 错误状态
void LED_ErrocodeStateShow(BOOL  refreshAll);


#endif




