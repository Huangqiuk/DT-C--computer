#ifndef _USER_TIMER_H_
#define _USER_TIMER_H_

#include "USER_Common.h"


#define TIMER_TIME								10	// 软件定时器周期，即软件定时器最小时间分辨率，单位：ms
#define TIMER_LOOP_FOREVER						-1	// 无限循环

#define TIMER_TASK_QUEUE_SIZE					5	// 定时器后期处理任务队列



typedef enum
{
	TIMER_ID_NULL = -1,

	TIMER_ID_KEY_SCAN,	//按键扫描
	TIMER_ID_1MIN_TIMEOUT,//1min超时定时器
	TIMER_ID_LED_R,		//RED LED控制定时器
	TIMER_ID_LED_YG,		//YG LED控制定时器
	TIMER_ID_10SEC_TIMEOUT,//10S超时定时器
	
	TIMER_ID_MAX
}TIMER_ID;

typedef enum
{
	ACTION_MODE_DO_AT_ONCE = 0,	// 立即执行
	ACTION_MODE_ADD_TO_QUEUE	// 添加到任务队列中执行
}ACTION_MODE_E;

// 定时器任务
typedef struct{
	void(*action)(uint32_t param);
	uint32_t	param;
}TIMER_TASK;

typedef struct
{
	uint32_t	top[TIMER_ID_MAX];
	volatile uint32_t  count[TIMER_ID_MAX];
	void(*action[TIMER_ID_MAX])(uint32_t param);
	uint32_t	param[TIMER_ID_MAX];
	int32_t  loopCount[TIMER_ID_MAX];
	uint8_t  actionMode[TIMER_ID_MAX];		// 执行模式
	uint8_t active[TIMER_ID_MAX];				// 是否处于运行状态

	// 定时器任务队列，当定时器任务执行时间较长时，必须将其放到中断以外执行，即将其放入任务队列中执行
	struct{
		TIMER_TASK task[TIMER_TASK_QUEUE_SIZE];
		uint8_t head;
		uint8_t end;
	}queue;
}TIMER_CB;

extern TIMER_CB timerCB;

//定时器初始化
void TIMER_Init(void);

//定时器ID ；定时时长（ms）；回调函数；参数；执行次数；执行模式（int，main）
uint8_t TIMER_AddTask(TIMER_ID id, uint32_t time, void(*action)(uint32_t param), uint32_t param, int32_t actionTime, ACTION_MODE_E actionMode);

void TIMER_KillTask(TIMER_ID id);

// 向定时器任务队列中添加一个任务
uint8_t TIMER_AddToQueue(void(*action)(uint32_t param), uint32_t param);

// 定时器任务队列处理，在main中调用
void TIMER_TaskQueueProcess(void);

// 定时器回调函数，在硬件定时器中断中使用
void TIMER_CALLBACK_OnTimer(void* param);

// 暂停指定的定时器
void TIMER_Pause(TIMER_ID id);

// 恢复指定的定时器
void TIMER_Resume(TIMER_ID id);

// 注销所有定时器
void TIMER_KillAllTimer(void);

// 复位定时器，重新开始
void TIMER_ResetTimer(TIMER_ID id);

// 改变指定定时器定时时间
uint8_t TIMER_ChangeTime(TIMER_ID id, uint32_t time);


//-----100hz------
//------10ms------
void Time_Init();





#endif	//_USER_TIMER_H_
