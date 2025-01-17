#include "USER_Common.h"

//------------------
//KEY定时器超时时间：20ms
#define USER_TIMER         APP_TIMER_TICKS(TIMER_TIME)  

APP_TIMER_DEF(m_user_timer_id);   

//------------------

TIMER_CB timerCB;

// 定时器回调函数，在硬件定时器中断中使用
void TIMER_CALLBACK_OnTimer(void* param);


void Time_PHY_Init(){

	uint32_t err_code = 0;	
	err_code = app_timer_create(&m_user_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              TIMER_CALLBACK_OnTimer);
	APP_ERROR_CHECK(err_code);
}


// 启动硬件定时器
void TIMER_StartTimerx(void)
{
	uint32_t err_code = 0;	
  //启动定时器
	err_code = app_timer_start(m_user_timer_id, USER_TIMER, NULL);
	APP_ERROR_CHECK(err_code);	
}

// 停止硬件定时器
void TIMER_StopTimerx(void)
{
	uint32_t err_code = 0;	
	err_code = app_timer_stop(m_user_timer_id);
	APP_ERROR_CHECK(err_code);	
}

//--------------------------------------------
void TIMER_DoNothing(uint32_t param)
{}
	
void TIMER_Init(void)
{
	TIMER_ID i;

	// 虚拟定时器数据结构初始化
	for(i=(TIMER_ID)0; i<TIMER_ID_MAX; i++)
	{
		timerCB.count[i] = 0;
		timerCB.top[i] = 0;
		timerCB.action[i] = TIMER_DoNothing;
		timerCB.param[i] = 0;
		timerCB.loopCount[i] = 0;
		timerCB.active[i] = true;
	}

	timerCB.queue.head = 0;
	timerCB.queue.end  = 0;
	
	// 硬件定时器x初始化
	Time_PHY_Init();
		
	// 启动定时器x
	TIMER_StartTimerx();
}	


uint8_t TIMER_AddTask(TIMER_ID id, uint32_t time, void(*action)(uint32_t param), uint32_t param, int32_t actionTime, ACTION_MODE_E actionMode)
{
	// 执行次数为0，不添加
	if(0 == actionTime)
	{
		return false;
	}

	timerCB.top[id] = time/TIMER_TIME;
	timerCB.action[id] = action;
	timerCB.param[id] = param;
	timerCB.loopCount[id] = actionTime-1;	// 动作次数减1，即为重复次数
	timerCB.count[id] = time/TIMER_TIME; 	// 这条赋值语句放到最后，可以保证在前面参数设置OK以后才启动软件定时器
	timerCB.actionMode[id] = actionMode;
	timerCB.active[id] = true;
	
	return true;
}

// 销毁定时器任务
void TIMER_KillTask(TIMER_ID id)
{
	uint8_t i;
	
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}
	
	// 计数值清零，即可将当前软件定时器释放
	timerCB.count[id] = 0; // 这条赋值语句放到最前，可以保证在第一时间停止软件定时器

	// 同时要删除已添加入任务队列的任务
	i= timerCB.queue.head;
	while(i != timerCB.queue.end)
	{
		// 队列中有当前定时器对应的任务，就删除
		if((timerCB.queue.task[i].action == timerCB.action[id])&&
			(timerCB.queue.task[i].param == timerCB.param[id]))
		{
			timerCB.queue.task[i].action = TIMER_DoNothing;
		}
		
		i ++;
		i %= TIMER_TASK_QUEUE_SIZE;
	}
}

// 向定时器任务队列中添加一个任务
uint8_t TIMER_AddToQueue(void(*action)(uint32_t param), uint32_t param)
{
	// 队列满，返回失败信息
	if((timerCB.queue.end+1)%TIMER_TASK_QUEUE_SIZE == timerCB.queue.head)
	{
		return false;
	}

	// 队列未满，添加
	timerCB.queue.task[timerCB.queue.end].action = action;
	timerCB.queue.task[timerCB.queue.end].param  = param;
	timerCB.queue.end ++;
	timerCB.queue.end %= TIMER_TASK_QUEUE_SIZE;

	return true;
}

// 定时器任务队列处理，在main中调用
void TIMER_TaskQueueProcess(void)
{
	void(*action)(uint32_t param) = 0;
	uint32_t param;
	TIMER_TASK* pTask = 0;
	
	// 队列空，退出
	if(timerCB.queue.head == timerCB.queue.end)
	{
		return;
	}

	// 队列非空，取出head处的任务执行之
	pTask = &timerCB.queue.task[timerCB.queue.head];
	action = pTask->action;
	param  = pTask->param;
	(*action)(param);

	// 删除任务
	timerCB.queue.head ++;
	timerCB.queue.head %= TIMER_TASK_QUEUE_SIZE;
}

void TIMER_CALLBACK_OnTimer(void* param)
{
	uint8_t i;
	for(i=0; i<TIMER_ID_MAX; i++)
	{
		// 计数值大于0，说明当前定时器处于活动状态，需要进行处理
		if(timerCB.count[i] > 0)
		{
			// 跳过被暂停的定时器
			if(!timerCB.active[i])
			{
				continue;
			}
			
			// 计数值递减
			timerCB.count[i] --;
			// 计数值为0，说明软件定时器时间到
			if(0 == timerCB.count[i])
			{
				// 如果是无限循环，则无条件重新设置计数初值
				if(timerCB.loopCount[i] < 0)
				{
					timerCB.count[i] = timerCB.top[i];
				}
				// 有限循环
				else if(timerCB.loopCount[i] > 0)
				{
					timerCB.count[i] = timerCB.top[i];
					
					// 剩余循环次数递减
					timerCB.loopCount[i] --;
				}
				// 循环次数为0，不予重复，当前软件定时器资源释放
				else{}

				// 执行模式为立即执行，则在中断中执行动作
				if(ACTION_MODE_DO_AT_ONCE == timerCB.actionMode[i])
				{
					(*timerCB.action[i])(timerCB.param[i]);
				}
				// 其它模式，添加到任务队列中，在中断外执行
				else
				{
					TIMER_AddToQueue(timerCB.action[i], timerCB.param[i]);
				}
				
			}
		}
	}
}

// 暂停指定的定时器
void TIMER_Pause(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = false;
}

// 恢复指定的定时器
void TIMER_Resume(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = true;
}

// 注销所有定时器
void TIMER_KillAllTimer(void)
{
	TIMER_ID i;
		
	for(i=(TIMER_ID)0; i<TIMER_ID_MAX; i++)
	{
		timerCB.count[i] = 0;
		timerCB.loopCount[i] = 0;
		timerCB.queue.task[i].action = TIMER_DoNothing;
	}
}

// 复位定时器，重新开始
void TIMER_ResetTimer(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.count[id] = timerCB.top[id];
}

// 改变指定定时器定时时间
uint8_t TIMER_ChangeTime(TIMER_ID id, uint32_t time)
{
	timerCB.top[id] = time/TIMER_TIME;
	timerCB.count[id] = time/TIMER_TIME;	
		
	return true;
}


