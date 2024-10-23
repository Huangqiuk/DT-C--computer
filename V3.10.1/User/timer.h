#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

// TIMx选择
#define STM32_TIMERx_NUM_DEF					TIMER1					// 选择TIMx(中断函数名字和时钟都需要修改)
#define STM32_TIMERx_IRQn_DEF					TIMER1_IRQn

#define TIMER_TIME								5	// 软件定时器周期，即软件定时器最小时间分辨率，单位：ms
#define TIMER_LOOP_FOREVER						-1	// 无限循环

#define TIMER_TASK_QUEUE_SIZE					15	// 定时器后期处理任务队列

typedef enum
{
	TIMER_ID_NULL = -1,

	TIMER_ID_IAP_RUN_WINDOW_TIME,
	
	TIMER_ID_KEY_SCAN,
	
	TIMER_ID_IAP_ECO_REQUEST,

	TIMER_ID_BLE_CONTROL,

	TIMER_ID_GSM_CONTROL,

	TIMER_ID_GSM_POWER_CONTROL,

	TIMER_ID_ECO_JUMP,
	
	TIMER_ID_MCU_UID,							// MCU_UID

	TIMER_ID_STATE_CONTROL,
	
	TIMER_ID_UART_RX_TIME_OUT_CONTROL,				// 大石协议V9.2需要开启接收超时定时器
	
	TIMER_ID_BLE_CONNECT_STATE,						// 蓝牙连接状态
	
	TIMER_ID_BLE_CONNECT_VERIFICATE,				// 蓝牙连接权鉴验证
	
	TIMER_ID_BLE_UPDATE_STATE,						// 蓝牙无线升级状态显示
	
	TIMER_ID_BLE_SEND_RST_ACK,						// 蓝牙升级进入boot时，发送复位应答

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
