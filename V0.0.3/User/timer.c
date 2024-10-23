#include "common.h"
#include "timer.h"

TIMER_CB timerCB;

// ��ʱ��Ӳ���ײ��ʼ��
void TIMx_INT_Init(uint16 arr, uint16 psc)
{
	timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER1);							// ʱ��ʹ��
	timer_deinit(STM32_TIMERx_NUM_DEF);
	
	timer_initpara.prescaler = psc;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = arr;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(STM32_TIMERx_NUM_DEF, &timer_initpara);

	timer_flag_clear(STM32_TIMERx_NUM_DEF, TIMER_FLAG_UP);			// �������жϱ�־
	timer_interrupt_enable(STM32_TIMERx_NUM_DEF, TIMER_INT_UP);		// ʹ��ָ����TIM1�ж�,��������ж�

	nvic_irq_enable(STM32_TIMERx_IRQn_DEF, 2, 2);						// �����ж����ȼ�
}

// ��ʱ��x�жϷ������
void TIMER1_IRQHandler(void)	// TIMxӲ����ʱ���ж�ʸ�����
{
	if (timer_interrupt_flag_get(STM32_TIMERx_NUM_DEF, TIMER_INT_FLAG_UP) != RESET)  //���TIMx�����жϷ������
	{	
		// ���TIMx�����жϱ�־  
		timer_interrupt_flag_clear(STM32_TIMERx_NUM_DEF, TIMER_INT_FLAG_UP);			
		
		// ��ʱ���ص�����
		TIMER_CALLBACK_OnTimer();		
	}		
}

// ��ʼ����ʱ��
void TIMER_InitTimerx(void)
{	 
	TIMx_INT_Init(9, 35999);	 // (35999+1) / 72MHz * (9+1) =  5ms
}

// ����Ӳ����ʱ��
void TIMER_StartTimerx(void)
{
	timer_enable(STM32_TIMERx_NUM_DEF);
}

// ֹͣӲ����ʱ��
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
				if (ACTION_MODE_DO_AT_ONCE == timerCB.actionMode[i])
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



