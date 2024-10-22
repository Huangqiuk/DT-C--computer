#include "USER_Common.h"

//------------------
//KEY��ʱ����ʱʱ�䣺20ms
#define USER_TIMER         APP_TIMER_TICKS(TIMER_TIME)  

APP_TIMER_DEF(m_user_timer_id);   

//------------------

TIMER_CB timerCB;

// ��ʱ���ص���������Ӳ����ʱ���ж���ʹ��
void TIMER_CALLBACK_OnTimer(void* param);


void Time_PHY_Init(){

	uint32_t err_code = 0;	
	err_code = app_timer_create(&m_user_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              TIMER_CALLBACK_OnTimer);
	APP_ERROR_CHECK(err_code);
}


// ����Ӳ����ʱ��
void TIMER_StartTimerx(void)
{
	uint32_t err_code = 0;	
  //������ʱ��
	err_code = app_timer_start(m_user_timer_id, USER_TIMER, NULL);
	APP_ERROR_CHECK(err_code);	
}

// ֹͣӲ����ʱ��
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

	// ���ⶨʱ�����ݽṹ��ʼ��
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
	
	// Ӳ����ʱ��x��ʼ��
	Time_PHY_Init();
		
	// ������ʱ��x
	TIMER_StartTimerx();
}	


uint8_t TIMER_AddTask(TIMER_ID id, uint32_t time, void(*action)(uint32_t param), uint32_t param, int32_t actionTime, ACTION_MODE_E actionMode)
{
	// ִ�д���Ϊ0��������
	if(0 == actionTime)
	{
		return false;
	}

	timerCB.top[id] = time/TIMER_TIME;
	timerCB.action[id] = action;
	timerCB.param[id] = param;
	timerCB.loopCount[id] = actionTime-1;	// ����������1����Ϊ�ظ�����
	timerCB.count[id] = time/TIMER_TIME; 	// ������ֵ���ŵ���󣬿��Ա�֤��ǰ���������OK�Ժ������������ʱ��
	timerCB.actionMode[id] = actionMode;
	timerCB.active[id] = true;
	
	return true;
}

// ���ٶ�ʱ������
void TIMER_KillTask(TIMER_ID id)
{
	uint8_t i;
	
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}
	
	// ����ֵ���㣬���ɽ���ǰ������ʱ���ͷ�
	timerCB.count[id] = 0; // ������ֵ���ŵ���ǰ�����Ա�֤�ڵ�һʱ��ֹͣ������ʱ��

	// ͬʱҪɾ����������������е�����
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

// ��ʱ���������������һ������
uint8_t TIMER_AddToQueue(void(*action)(uint32_t param), uint32_t param)
{
	// ������������ʧ����Ϣ
	if((timerCB.queue.end+1)%TIMER_TASK_QUEUE_SIZE == timerCB.queue.head)
	{
		return false;
	}

	// ����δ��������
	timerCB.queue.task[timerCB.queue.end].action = action;
	timerCB.queue.task[timerCB.queue.end].param  = param;
	timerCB.queue.end ++;
	timerCB.queue.end %= TIMER_TASK_QUEUE_SIZE;

	return true;
}

// ��ʱ��������д�������main�е���
void TIMER_TaskQueueProcess(void)
{
	void(*action)(uint32_t param) = 0;
	uint32_t param;
	TIMER_TASK* pTask = 0;
	
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

void TIMER_CALLBACK_OnTimer(void* param)
{
	uint8_t i;
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
			// ����ֵΪ0��˵��������ʱ��ʱ�䵽
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
				// ѭ������Ϊ0�������ظ�����ǰ������ʱ����Դ�ͷ�
				else{}

				// ִ��ģʽΪ����ִ�У������ж���ִ�ж���
				if(ACTION_MODE_DO_AT_ONCE == timerCB.actionMode[i])
				{
					(*timerCB.action[i])(timerCB.param[i]);
				}
				// ����ģʽ�����ӵ���������У����ж���ִ��
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

	timerCB.active[id] = false;
}

// �ָ�ָ���Ķ�ʱ��
void TIMER_Resume(TIMER_ID id)
{
	if((TIMER_ID_NULL == id)||(id >= TIMER_ID_MAX))
	{
		return;
	}

	timerCB.active[id] = true;
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
uint8_t TIMER_ChangeTime(TIMER_ID id, uint32_t time)
{
	timerCB.top[id] = time/TIMER_TIME;
	timerCB.count[id] = time/TIMER_TIME;	
		
	return true;
}

