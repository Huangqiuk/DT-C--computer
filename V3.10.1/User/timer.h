#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

// TIMxѡ��
#define STM32_TIMERx_NUM_DEF					TIMER1					// ѡ��TIMx(�жϺ������ֺ�ʱ�Ӷ���Ҫ�޸�)
#define STM32_TIMERx_IRQn_DEF					TIMER1_IRQn

#define TIMER_TIME								5	// �����ʱ�����ڣ��������ʱ����Сʱ��ֱ��ʣ���λ��ms
#define TIMER_LOOP_FOREVER						-1	// ����ѭ��

#define TIMER_TASK_QUEUE_SIZE					15	// ��ʱ�����ڴ����������

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
	
	TIMER_ID_UART_RX_TIME_OUT_CONTROL,				// ��ʯЭ��V9.2��Ҫ�������ճ�ʱ��ʱ��
	
	TIMER_ID_BLE_CONNECT_STATE,						// ��������״̬
	
	TIMER_ID_BLE_CONNECT_VERIFICATE,				// ��������Ȩ����֤
	
	TIMER_ID_BLE_UPDATE_STATE,						// ������������״̬��ʾ
	
	TIMER_ID_BLE_SEND_RST_ACK,						// ������������bootʱ�����͸�λӦ��

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
