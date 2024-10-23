#include "common.h"
#include "timer.h"

TIMER_CB timerCB;

// 定时器硬件底层初始化
void TIMx_INT_Init(uint16 arr, uint16 psc)
{
	timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER1);							// 时钟使能
	timer_deinit(STM32_TIMERx_NUM_DEF);
	
	timer_initpara.prescaler = psc;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = arr;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(STM32_TIMERx_NUM_DEF, &timer_initpara);

	timer_flag_clear(STM32_TIMERx_NUM_DEF, TIMER_FLAG_UP);			// 清除溢出中断标志
	timer_interrupt_enable(STM32_TIMERx_NUM_DEF, TIMER_INT_UP);		// 使能指定的TIM1中断,允许更新中断

	nvic_irq_enable(STM32_TIMERx_IRQn_DEF, 2, 2);						// 设置中断优先级
}

// 定时器x中断服务程序
void TIMER1_IRQHandler(void)	// TIMx硬件定时器中断矢量入口
{
	if (timer_interrupt_flag_get(STM32_TIMERx_NUM_DEF, TIMER_INT_FLAG_UP) != RESET)  //检查TIMx更新中断发生与否
	{	
		// 清除TIMx更新中断标志  
		timer_interrupt_flag_clear(STM32_TIMERx_NUM_DEF, TIMER_INT_FLAG_UP);			
		
		// 定时器回调函数
		TIMER_CALLBACK_OnTimer();		
	}		
}

// 初始化定时器
void TIMER_InitTimerx(void)
{	 
	TIMx_INT_Init(9, 35999);	 // (35999+1) / 72MHz * (9+1) =  5ms
}

// 启动硬件定时器
void TIMER_StartTimerx(void)
{
	timer_enable(STM32_TIMERx_NUM_DEF);
}

// 停止硬件定时器
void TIMER_StopTimerx(void)
{
	timer_disable(STM32_TIMERx_NUM_DEF);
}

void TIMER_DoNothing(uint32 param)
{}

void TIMER_Init(void)
{
	TIMER_ID i;
	
	for(i=(TIMER_ID)0; i<TIMER_ID_MAX; i++)
	{
		timerCB.count[i] = 0;
		timerCB.top[i] = 0;
		timerCB.action[i] = TIMER_DoNothing;
		timerCB.param[i] = 0;
		timerCB.loopCount[i] = 0;
		timerCB.active[i] = TRUE;
	}

	timerCB.queue.head = 0;
	timerCB.queue.end  = 0;
	
	// 硬件定时器x初始化
	TIMER_InitTimerx();
		
	// 启动定时器x
	TIMER_StartTimerx();
}

BOOL TIMER_AddTask(TIMER_ID id, uint32 time, void(*action)(uint32 param), uint32 param, int32 actionTime, ACTION_MODE_E actionMode)
{
	// 执行次数为0，不添加
	if(0 == actionTime)
	{
		return FALSE;
	}

	timerCB.top[id] = time/TIMER_TIME;
	timerCB.action[id] = action;
	timerCB.param[id] = param;
	timerCB.loopCount[id] = actionTime-1;	// 动作次数减1，即为重复次数
	timerCB.count[id] = time/TIMER_TIME; 	// 这条赋值语句放到最后，可以保证在前面参数设置OK以后才启动软件定时器
	timerCB.actionMode[id] = actionMode;
	timerCB.active[id] = TRUE;
	
	return TRUE;
}

// 销毁定时器任务
void TIMER_KillTask(TIMER_ID id)
{
	uint8 i;
	
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
BOOL TIMER_AddToQueue(void(*action)(uint32 param), uint32 param)
{
	// 队列满，返回失败信息
	if((timerCB.queue.end+1)%TIMER_TASK_QUEUE_SIZE == timerCB.queue.head)
	{
		return FALSE;
	}

	// 队列未满，添加
	timerCB.queue.task[timerCB.queue.end].action = action;
	timerCB.queue.task[timerCB.queue.end].param  = param;
	timerCB.queue.end ++;
	timerCB.queue.end %= TIMER_TASK_QUEUE_SIZE;

	return TRUE;
}

// 定时器任务队列处理，在main中调用
void TIMER_TaskQueueProcess(void)
{
	void(*action)(uint32 param) = NULL;
	uint32 param;
	TIMER_TASK* pTask = NULL;
	
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

void TIMER_CALLBACK_OnTimer(void)
{
	uint8 i;
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
				if (ACTION_MODE_DO_AT_ONCE == timerCB.actionMode[i])
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

	timerCB.active[id] = FALSE;
}

// 恢复指定的定时器
void TIMER_Resume(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = TRUE;
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
BOOL TIMER_ChangeTime(TIMER_ID id, uint32 time)
{
	timerCB.top[id] = time/TIMER_TIME;
	timerCB.count[id] = time/TIMER_TIME;	
		
	return TRUE;
}



