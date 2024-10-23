#include "common.h"
#include "timer.h"

TIMER_CB timerCB;

// ��ʱ��xӲ���ײ��ʼ��
void TIMx_INT_Init(uint16 arr, uint32 psc)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	NVIC_InitType NVIC_InitStructure;

	// TIM6 clock enable   
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM6, ENABLE);
	
	// ��ʱ��TIMx��ʼ��
	TIM_DeInit(N32_TIMERx_NUM_DEF);						// ��λ��ʱ��x
	TIM_TimeBaseStructure.Period = arr;					// ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.Prescaler = psc;				// ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.ClkDiv = 0; 					// ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP; 	// TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.RepetCnt = 0;					// �ظ�����Ϊ0
	TIM_InitTimeBase(N32_TIMERx_NUM_DEF, &TIM_TimeBaseStructure);		// ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ	

	TIM_ClearFlag(N32_TIMERx_NUM_DEF, TIM_FLAG_UPDATE);					// �������жϱ�־
	// TIM2 enable update irq   
	TIM_ConfigInt(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE, ENABLE);

	// �ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = N32_TIMERx_IRQn_DEF; 		// TIMx�����ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;		// ��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				// �����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				// IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure); 
}

// ��ʱ��x�жϷ������
void TIM6_IRQHandler(void)
{
	if (TIM_GetIntStatus(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE) != RESET)    
	{
		// ���TIMx�����жϱ�־  
		TIM_ClrIntPendingBit(N32_TIMERx_NUM_DEF, TIM_INT_UPDATE);
		
		// ��ʱ���ص�����
		TIMER_CALLBACK_OnTimer();
	}
}

// ��ʼ����ʱ��
void TIMER_InitTimerx(void)
{
	TIMx_INT_Init(9, 23999);	 				// (23999+1) / 48MHz * (9+1) =  5ms
}

// ����Ӳ����ʱ��
void TIMER_StartTimerx(void)
{
	// ʹ��TIMx
	TIM_Enable(N32_TIMERx_NUM_DEF, ENABLE);
}

// ֹͣӲ����ʱ��
void TIMER_StopTimerx(void)
{
	TIM_Enable(N32_TIMERx_NUM_DEF, DISABLE);
}

void TIMER_DoNothing(uint32 param)
{}

void TIMER_Init(void)
{
	TIMER_ID i;

	// ���ⶨʱ�����ݽṹ��ʼ��
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
	
	// Ӳ����ʱ��x��ʼ��
	TIMER_InitTimerx();
		
	// ������ʱ��x
	TIMER_StartTimerx();
}

BOOL TIMER_AddTask(TIMER_ID id, uint32 time, void(*action)(uint32 param), uint32 param, int32 actionTime, ACTION_MODE_E actionMode)
{
	// ִ�д���Ϊ0�������
	if(0 == actionTime)
	{
		return FALSE;
	}

	timerCB.top[id] = time/TIMER_TIME;
	timerCB.action[id] = action;
	timerCB.param[id] = param;
	timerCB.loopCount[id] = actionTime-1;	// ����������1����Ϊ�ظ�����
	timerCB.count[id] = time/TIMER_TIME; 	// ������ֵ���ŵ���󣬿��Ա�֤��ǰ���������OK�Ժ�����������ʱ��
	timerCB.actionMode[id] = actionMode;
	timerCB.active[id] = TRUE;
	
	return TRUE;
}

// ���ٶ�ʱ������
void TIMER_KillTask(TIMER_ID id)
{
	uint8 i;
	
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}
	
	// ����ֵ���㣬���ɽ���ǰ�����ʱ���ͷ�
	timerCB.count[id] = 0; // ������ֵ���ŵ���ǰ�����Ա�֤�ڵ�һʱ��ֹͣ�����ʱ��

	// ͬʱҪɾ���������������е�����
	i= timerCB.queue.head;
	while(i != timerCB.queue.end)
	{
		// �������е�ǰ��ʱ����Ӧ�����񣬾�ɾ��
		if((timerCB.queue.task[i].action == timerCB.action[id])&&
			(timerCB.queue.task[i].param == timerCB.param[id]))
		{
			timerCB.queue.task[i].action = TIMER_DoNothing;
		}
		
		i ++;
		i %= TIMER_TASK_QUEUE_SIZE;
	}
}

// ��ʱ��������������һ������
BOOL TIMER_AddToQueue(void(*action)(uint32 param), uint32 param)
{
	// ������������ʧ����Ϣ
	if((timerCB.queue.end+1)%TIMER_TASK_QUEUE_SIZE == timerCB.queue.head)
	{
		return FALSE;
	}

	// ����δ�������
	timerCB.queue.task[timerCB.queue.end].action = action;
	timerCB.queue.task[timerCB.queue.end].param  = param;
	timerCB.queue.end ++;
	timerCB.queue.end %= TIMER_TASK_QUEUE_SIZE;

	return TRUE;
}

// ��ʱ��������д�����main�е���
void TIMER_TaskQueueProcess(void)
{
	void(*action)(uint32 param) = NULL;
	uint32 param;
	TIMER_TASK* pTask = NULL;
	
	// ���пգ��˳�
	if(timerCB.queue.head == timerCB.queue.end)
	{
		return;
	}

	// ���зǿգ�ȡ��head��������ִ��֮
	pTask = &timerCB.queue.task[timerCB.queue.head];
	action = pTask->action;
	param  = pTask->param;
	(*action)(param);

	// ɾ������
	timerCB.queue.head ++;
	timerCB.queue.head %= TIMER_TASK_QUEUE_SIZE;
}

void TIMER_CALLBACK_OnTimer(void)
{
	uint8 i;
	for(i=0; i<TIMER_ID_MAX; i++)
	{
		// ����ֵ����0��˵����ǰ��ʱ�����ڻ״̬����Ҫ���д���
		if(timerCB.count[i] > 0)
		{
			// ��������ͣ�Ķ�ʱ��
			if(!timerCB.active[i])
			{
				continue;
			}
			
			// ����ֵ�ݼ�
			timerCB.count[i] --;
			// ����ֵΪ0��˵�������ʱ��ʱ�䵽
			if(0 == timerCB.count[i])
			{
				// ���������ѭ�������������������ü�����ֵ
				if(timerCB.loopCount[i] < 0)
				{
					timerCB.count[i] = timerCB.top[i];
				}
				// ����ѭ��
				else if(timerCB.loopCount[i] > 0)
				{
					timerCB.count[i] = timerCB.top[i];
					
					// ʣ��ѭ�������ݼ�
					timerCB.loopCount[i] --;
				}
				// ѭ������Ϊ0�������ظ�����ǰ�����ʱ����Դ�ͷ�
				else{}

				// ִ��ģʽΪ����ִ�У������ж���ִ�ж���
				if(ACTION_MODE_DO_AT_ONCE == timerCB.actionMode[i])
				{
					(*timerCB.action[i])(timerCB.param[i]);
				}
				// ����ģʽ����ӵ���������У����ж���ִ��
				else
				{
					TIMER_AddToQueue(timerCB.action[i], timerCB.param[i]);
				}
			}
		}
	}
}

// ��ָͣ���Ķ�ʱ��
void TIMER_Pause(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = FALSE;
}

// �ָ�ָ���Ķ�ʱ��
void TIMER_Resume(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = TRUE;
}

// ע�����ж�ʱ��
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

// ��λ��ʱ�������¿�ʼ
void TIMER_ResetTimer(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.count[id] = timerCB.top[id];
}

// �ı�ָ����ʱ����ʱʱ��
BOOL TIMER_ChangeTime(TIMER_ID id, uint32 time)
{
	timerCB.top[id] = time/TIMER_TIME;
	timerCB.count[id] = time/TIMER_TIME;	
		
	return TRUE;
}



