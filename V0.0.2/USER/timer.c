#include "common.h"
#include "timer.h"

TIMER_CB timerCB;

// 定时器x硬件底层初始化
void TIMx_INT_Init(uint16 arr, uint32 psc)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	NVIC_InitType NVIC_InitStructure;

	// TIM6 clock enable   
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM6, ENABLE);
	
	// 定时器TIMx初始化
	TIM_DeInit(N32_TIMERx_NUM_DEF);						// 复位定时器x
	TIM_TimeBaseStructure.Period = arr;					// 设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.Prescaler = psc;				// 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.ClkDiv = 0; 					// 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP; 	// TIM向上计数模式
	TIM_TimeBaseStructure.RepetCnt = 0;					// 重复次数为0
	TIM_InitTimeBase(N32_TIMERx_NUM_DEF, &TIM_TimeBaseStructure);		// 根据指定的参数初始化TIMx的时间基数单位	

	TIM_ClearFlag(N32_TIMERx_NUM_DEF, TIM_FLAG_UPDATE);					// 清除溢出中断标志
	// TIM2 enable update irq   
	TIM_ConfigInt(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE, ENABLE);

	// 中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = N32_TIMERx_IRQn_DEF; 		// TIMx更新中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;		// 先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				// 从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				// IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure); 
}

// 定时器x中断服务程序
void TIM6_IRQHandler(void)
{
	if (TIM_GetIntStatus(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE) != RESET)    
	{
		// 清除TIMx更新中断标志  
		TIM_ClrIntPendingBit(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE);
		
		// 定时器回调函数
		TIMER_CALLBACK_OnTimer();
	}
}

// 初始化定时器
void TIMER_InitTimerx(void)
{
	TIMx_INT_Init(9, 23999);	 				// (23999+1) / 48MHz * (9+1) =  5ms
}

// 启动硬件定时器
void TIMER_StartTimerx(void)
{
	// 使能TIMx
	TIM_Enable(N32_TIMERx_NUM_DEF, ENABLE);
}

// 停止硬件定时器
void TIMER_StopTimerx(void)
{
	TIM_Enable(N32_TIMERx_NUM_DEF, DISABLE);
}

void TIMER_DoNothing(uint32 param)
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



