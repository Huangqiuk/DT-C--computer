#ifndef _TIME3_H_
#define _TIME3_H_

#include "main.h"



#define TIMER_TIME								1	// 软件定时器周期，即软件定时器最小时间分辨率，单位：ms
#define TIMER_LOOP_FOREVER						-1	// 无限循环

#define TIMER_TASK_QUEUE_SIZE					5	// 定时器后期处理任务队列



typedef enum
{
	TIMER_ID_NULL = -1,

//	TIMER_ID_1,//测试1							
//	TIMER_ID_2,//测试2
//	
//	TIMER_ID_KEY_HMI_CMD,//按键上报命令定时器
//	
//	TIMER_ID_KEY_CONN,//连接活性定时器
//	
//	TIMER_ID_NFC_DELAY,//NFC
//	TIMER_ID_KEY_SCAN,//按键扫描时基
	TIMER_ID_JUMP,								// 跳转
	
	TIMER_ID_STATE_CONTROL,						// 状态机模块控制

//	TIMER_ID_MENU_QUIT,							// 退出菜单控制
//	
//	TIMER_ID_STATE_ON,							// 开机控制

//	TIMER_ID_FUNCTION_TEST,						// 老化测试界面

//	TIMER_ID_AGE_SEND,							// 老化命令发送定时器
//	
//	TIMER_ID_PROTOCOL_PARAM_TX,					// 协议层命令定时下发

//	TIMER_ID_PROTOCOL_TXRX_TIME_OUT,			// 协议层通讯超时
//	
//	TIMER_ID_PROTOCOL_SPEED_FILTER,				// 速度平滑

//	TIMER_ID_KEY_CLICK_FUN,						// 按键单双击
//	
//	TIMER_ID_AUTO_POWER_OFF,					// 自动关机

//	TIMER_ID_ADC_SCAN,							// AD转换间隔

//	TIMER_ID_BUZZER_WORK,						// 蜂鸣器任务

	TIMER_ID_TEST,								//	测试
	
//	TIMER_ID_BLE_NUMBER,

//    TIMER_ID_UP_FINISH_REPORT,
//    
    TIMER_ID_IAP_RUN_WINDOW_TIME,
    
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

uint8_t TIMER_AddTask(TIMER_ID id, uint32_t time, void(*action)(uint32_t param), uint32_t param, int32_t actionTime, ACTION_MODE_E actionMode);

void TIMER_KillTask(TIMER_ID id);

// 向定时器任务队列中添加一个任务
uint8_t TIMER_AddToQueue(void(*action)(uint32_t param), uint32_t param);

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
uint8_t TIMER_ChangeTime(TIMER_ID id, uint32_t time);



//-----1Khz------
//------1ms------
void Time3_Init();




#endif

