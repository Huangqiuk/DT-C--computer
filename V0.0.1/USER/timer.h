#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

// TIMx选择
#define N32_TIMERx_NUM_DEF 								TIM6
#define N32_TIMERx_IRQn_DEF								TIM6_IRQn

#define TIMER_TIME								5	// 软件定时器周期，即软件定时器最小时间分辨率，单位：ms
#define TIMER_LOOP_FOREVER						-1	// 无限循环

#define TIMER_TASK_QUEUE_SIZE					5	// 定时器后期处理任务队列

typedef enum
{
	TIMER_ID_NULL = -1,

	TIMER_ID_STATE_CONTROL,						// 状态机模块控制
    TIMER_ID_RUNLED,
	
	
	// 用户添加任务ID区域
	TIMER_ID_TEMP_CHANEL_LOOP,				// 温度卡通道循环切换任务
	TIMER_ID_GET_AVOMENT,							// 获取万用表数据
	TIMER_ID_KEY_PROCESS,							
    TIMER_ID_RESET_BOARD,
	TIMER_ID_REOPRT_INVALID_RESULT,
	TIMER_ID_RELAY_TEST,
	TIMER_ID_OHM_INIT,    //开机后先切一次欧姆档，
	TIMER_ID_RESET,
	// 用户添加任务ID区域

	TIMER_ID_MAX
}TIMER_ID;

typedef enum
{
	ACTION_MODE_DO_AT_ONCE = 0,	// 立即执行
	ACTION_MODE_ADD_TO_QUEUE	// 添加到任务队列中执行
}ACTION_MODE_E;

// 定时器任务
typedef struct{
	void(*action)(uint32 param);
	uint32	param;
}TIMER_TASK;

typedef struct
{
	uint32	top[TIMER_ID_MAX];
	volatile uint32  count[TIMER_ID_MAX];
	void(*action[TIMER_ID_MAX])(uint32 param);
	uint32	param[TIMER_ID_MAX];
	int32  loopCount[TIMER_ID_MAX];
	uint8  actionMode[TIMER_ID_MAX];		// 执行模式
	BOOL active[TIMER_ID_MAX];				// 是否处于运行状态

	// 定时器任务队列，当定时器任务执行时间较长时，必须将其放到中断以外执行，即将其放入任务队列中执行
	struct{
		TIMER_TASK task[TIMER_TASK_QUEUE_SIZE];
		uint8 head;
		uint8 end;
	}queue;
}TIMER_CB;


void TIMER_Init(void);

BOOL TIMER_AddTask(TIMER_ID id, uint32 time, void(*action)(uint32 param), uint32 param, int32 actionTime, ACTION_MODE_E actionMode);

void TIMER_KillTask(TIMER_ID id);

// 向定时器任务队列中添加一个任务
BOOL TIMER_AddToQueue(void(*action)(uint32 param), uint32 param);

// 定时器任务队列处理，在main中调用
void TIMER_TaskQueueProcess(void);

// 定时器回调函数，在硬件定时器中断中使用
void TIMER_CALLBACK_OnTimer(void);

// 暂停指定的定时器
void TIMER_Pause(TIMER_ID id);

// 恢复指定的定时器
void TIMER_Resume(TIMER_ID id);

// 注销所有定时器
void TIMER_KillAllTimer(void);

// 复位定时器，重新开始
void TIMER_ResetTimer(TIMER_ID id);

// 改变指定定时器定时时间
BOOL TIMER_ChangeTime(TIMER_ID id, uint32 time);


#endif
