#ifndef __PWM_LED_H__
#define __PWM_LED_H__

/********************************* 头文件************************************/
#include "common.h"

/********************************* 模块驱动层修改 ************************************/
// 软硬件PWM模式切换
#define LED_PWM_MODE							LED_HARD_PWM				// LED功能模式
#define LED_SOFT_PWM							0							// LED软件PWM模式
#define LED_HARD_PWM							1 							// LED硬件PWM模式

// 定义端口
#define LED_GPIO 								GPIOB

#define LED_RED_PIN 							GPIO_PIN_15
#define LED_YELLOW_PIN 							GPIO_PIN_13
#define LED_GREEN_PIN 							GPIO_PIN_12

// LED控制码
typedef enum
{
	LED_NAME_RED = 0,
	LED_NAME_YELLOW,
	LED_NAME_GREEN,

	LED_NAME_MAX
}LED_NAME;

// LED动作
#define LED_ACTION(GPIOx, PIN_x, ACTION)		(ACTION?GPIO_SetBits(GPIOx,PIN_x):GPIO_ResetBits(GPIOx,PIN_x))


// 红灯控制
#define	LED_RED_PWM(i)							do{\
													TIM_SetCmp4(TIM9, i);\
												}while(0)
	
// 黄灯控制
#define	LED_YELLOW_PWM(i)						do{\
													TIM_SetCmp2(TIM9, i);\
												}while(0)

// 绿灯控制
#define	LED_GREEN_PWM(i)						do{\
													TIM_SetCmp1(TIM9, i);\
												}while(0)

// LED触发函数执行间隔,单位为ms
// 硬件可调整范围为(TIMER_TIME(定时器精度)~100);
// 软件可调整范围为10，不可设置为其它值;
#define LED_COUNT_TIME							10ul		// 计数周期

// 传一个参数时执行：执行指定LED,1s周期，亮占50效果闪烁 LED_Set(x, 1000, 50, 100, 0, 0, 0)
// 传两个参数时执行：执行指定LED,指定周期,亮占50效果闪烁 LED_Set(x, y, 50, 100, 0, 0, 0)
// 传三个参数时执行：执行指定LED,指定周期,指定亮占整个周期的百分比闪烁 LED_Set(x, y, z, 100, 0, 0, 0)
// 传四个参数时执行：执行指定LED,指定周期,指定亮占整个周期的百分比，指定呼吸深度呼吸效果 LED_Set(x, y, z, 100, 0, 0, 0)
// 传五个参数时执行：执行指定LED,指定周期,指定亮占整个周期的百分比，指定呼吸深度呼吸效果 限制最高输出值 LED_Set(x, y, z, 100, 0, 0, 0)
// 传六个参数时执行: 预留
// 传七个参数时执行: LED_Set一致
#define LED_SET(...) GetMacro(__VA_ARGS__,LED_SET_ALL, LED_SET_ALL_LOOP_FOREVER, LED_SET_BREATH, LED_SET_HIGHLIMIT, LED_SET_FLASH_ACTIONTIMES, LED_SET_FLASH, LED_SET_ON_RATIO, LED_SET_ON, ...)(__VA_ARGS__)

// 接口的重载宏函数
#define LED_SET_ON(x)								LED_Set(x, 1000, 100, 100, 0, 0, 0, -1)

#define LED_SET_ON_RATIO(x,y)						LED_Set(x, 1000, y, 100, 0, 0, 0, -1)

#define LED_SET_FLASH(x,y,z)						LED_Set(x, y, z, 100, 0, 0, 0, -1)

#define LED_SET_FLASH_ACTIONTIMES(x,y,z,a)			LED_Set(x, y, z, 100, 0, 0, 0, a)

#define LED_SET_HIGHLIMIT(x,y,z,a,b)				LED_Set(x, y, z, b, 0, 0, 0, a)

#define LED_SET_BREATH(x,y,z,a,b,c)					LED_Set(x, y, z, 100, 0, 0, 0, a)

#define LED_SET_ALL_LOOP_FOREVER(x,y,z,a,b,c,d)		LED_Set(x, y, z, a, b, c, d, -1)

#define LED_SET_ALL(x,y,z,a,b,c,d,e)				LED_Set(x, y, z, a, b, c, d, e)

#define GetMacro(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME


/********************************* 模块通用部分禁止修改 ************************************/
// LED动作标识
typedef enum
{
	LED_ACTION_OFF = 0,					// 关闭
	LED_ACTION_ON						// 打开
}LED_ACTION_E;

// LED 占空比最大值
#define LED_PWM_OUT_MAX							100			// 占空比最大值

// LED动作队列
typedef struct{
	// 输出PWM
	volatile uint32 controlPwmOut;			// 控制输出PWM

	// 计数值	
	uint16 ledCounter;						// LED 主计数值
	uint16 breathUpCounter;					// LED 亮计数值 用于呼吸效果控制
	uint16 breathDownCounter;				// LED 灭计数值	用于呼吸效果控制

	// 待配置值
	uint16 cycleCountTimesSetValue;			// LED亮灭周期计时数
	uint16 onCycleCountTimesSetValue;		// 亮时间计数	
	uint8 highLimitPwmOut;					// LED亮时亮度比，0-100，默认100	
	uint8 lowLimitPwmOut;					// LED灭时亮度比，0-100，默认100
	uint16 breathUpCountTimesSetValue;		// LED亮计数值 用于呼吸效果控制
	uint16 breathDownCountTimesSetValue;	// LED灭计数值 用于呼吸效果控制
	int32  loopCount;						// 循环次数控制
	
	// 动作状态
	LED_ACTION_E action;
	LED_ACTION_E lastAction;
	
	void (*fun)(uint32 param);

	// LED硬件参数
	void* LED_GPIO_x;
	uint16 LED_PIN_x;
}LED_TYPE_QUEUE;

typedef struct{
	LED_TYPE_QUEUE typeQueue[LED_NAME_MAX];		// 每个LED有一个动作队列
}LED_CB;

extern LED_CB ledCB;

/********************************* 函数外部申明***************************/
// Led初始化
void LED_Inits(uint32 mainFre);	

// 设置LED工作模式
void LED_Set(LED_NAME ledName, uint32 ledCycleTime, uint8 ledOnDutyRatio, uint8 onBrightnessRatio, uint8 offBrightnessRatio, uint8 ledOnAscendRatio, uint8 ledOnDescendRatio,int32 actionTimes);

// 控制周期获取接口
uint8 LED_GetControlCycle(LED_NAME ledName);

// 功能测试接口
void LED_CloseAllLeds(void);

void RunLed_Init(void);
#endif

