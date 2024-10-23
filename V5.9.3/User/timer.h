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
	
	TIMER_ID_STATE_CONTROL,							// 状态机模块控制
	
	//我的定时器任务
	TIMER_ID_TOOL_APP_TO_BOOT,						//工具从app跳转至boot

	//Dut_config写入
	TIMER_ID_SET_DUT_CONFIG,

	//BOOT写入
	TIMER_ID_UPGRADE_DUT_BOOT,
    
	//dut进度上报
	TIMER_ID_SET_DUT_PROGRESS_REPORT,
	
	//高标超时检测
	TIMER_ID_TIME_OUT_DETECTION,

	//高标上电开始
	TIMER_ID_TIME_GB_POWER_ON,

	//高标上电开始
	TIMER_ID_TIME_GB_POWER_OFF,

	// 延时工程模式允许
	TIMER_ID_DELAY_ENTER_UP_MODE,
	
	// dut超时无应答
	TIMER_ID_DUT_TIMEOUT,

	// 重新建立连接
	TIMER_ID_RECONNECTION,

	// 重新连接超时
	TIMER_ID_RECONNECTION_TIME_OUT,
	
	TIMER_ID_PROTOCOL_55_PARAM_TX,// 55串口发送测试
	
	TIMER_ID_CAN_TEST,

    TIMER_ID_PROTOCOL_SWITCCH,                  // 协议切换
    
    TIMER_ID_DUT_UPGRATE,
	
    // Dut_UI_VER写入
	TIMER_ID_SET_DUT_UI_VER,

    // IOT升级固件请求
	TIMER_ID_SEND_UP_APP_REQUEST,
    
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

