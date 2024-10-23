#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

// TIMxѡ��
#define N32_TIMERx_NUM_DEF 								TIM6
#define N32_TIMERx_IRQn_DEF								TIM6_IRQn

#define TIMER_TIME								5	// �����ʱ�����ڣ��������ʱ����Сʱ��ֱ��ʣ���λ��ms
#define TIMER_LOOP_FOREVER						-1	// ����ѭ��

#define TIMER_TASK_QUEUE_SIZE					5	// ��ʱ�����ڴ����������

typedef enum
{
	TIMER_ID_NULL = -1,

	TIMER_ID_STATE_CONTROL,						// ״̬��ģ�����
    TIMER_ID_RUNLED,
	
	
	// �û��������ID����
	TIMER_ID_TEMP_CHANEL_LOOP,				// �¶ȿ�ͨ��ѭ���л�����
	TIMER_ID_GET_AVOMENT,							// ��ȡ���ñ�����
	TIMER_ID_KEY_PROCESS,							
    TIMER_ID_RESET_BOARD,
	TIMER_ID_REOPRT_INVALID_RESULT,
	TIMER_ID_RELAY_TEST,
	TIMER_ID_OHM_INIT,    //����������һ��ŷķ����
	TIMER_ID_RESET,
	// �û��������ID����

	TIMER_ID_MAX
}TIMER_ID;

typedef enum
{
	ACTION_MODE_DO_AT_ONCE = 0,	// ����ִ��
	ACTION_MODE_ADD_TO_QUEUE	// ��ӵ����������ִ��
}ACTION_MODE_E;

// ��ʱ������
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
	uint8  actionMode[TIMER_ID_MAX];		// ִ��ģʽ
	BOOL active[TIMER_ID_MAX];				// �Ƿ�������״̬

	// ��ʱ��������У�����ʱ������ִ��ʱ��ϳ�ʱ�����뽫��ŵ��ж�����ִ�У�������������������ִ��
	struct{
		TIMER_TASK task[TIMER_TASK_QUEUE_SIZE];
		uint8 head;
		uint8 end;
	}queue;
}TIMER_CB;


void TIMER_Init(void);

BOOL TIMER_AddTask(TIMER_ID id, uint32 time, void(*action)(uint32 param), uint32 param, int32 actionTime, ACTION_MODE_E actionMode);

void TIMER_KillTask(TIMER_ID id);

// ��ʱ��������������һ������
BOOL TIMER_AddToQueue(void(*action)(uint32 param), uint32 param);

// ��ʱ��������д�����main�е���
void TIMER_TaskQueueProcess(void);

// ��ʱ���ص���������Ӳ����ʱ���ж���ʹ��
void TIMER_CALLBACK_OnTimer(void);

// ��ָͣ���Ķ�ʱ��
void TIMER_Pause(TIMER_ID id);

// �ָ�ָ���Ķ�ʱ��
void TIMER_Resume(TIMER_ID id);

// ע�����ж�ʱ��
void TIMER_KillAllTimer(void);

// ��λ��ʱ�������¿�ʼ
void TIMER_ResetTimer(TIMER_ID id);

// �ı�ָ����ʱ����ʱʱ��
BOOL TIMER_ChangeTime(TIMER_ID id, uint32 time);


#endif
