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
	
	TIMER_ID_STATE_CONTROL,							// ״̬��ģ�����
	
	//�ҵĶ�ʱ������
	TIMER_ID_TOOL_APP_TO_BOOT,						//���ߴ�app��ת��boot

	//Dut_configд��
	TIMER_ID_SET_DUT_CONFIG,

	//BOOTд��
	TIMER_ID_UPGRADE_DUT_BOOT,
    
	//dut�����ϱ�
	TIMER_ID_SET_DUT_PROGRESS_REPORT,
	
	//�߱곬ʱ���
	TIMER_ID_TIME_OUT_DETECTION,

	//�߱��ϵ翪ʼ
	TIMER_ID_TIME_GB_POWER_ON,

	//�߱��ϵ翪ʼ
	TIMER_ID_TIME_GB_POWER_OFF,

	// ��ʱ����ģʽ����
	TIMER_ID_DELAY_ENTER_UP_MODE,
	
	// dut��ʱ��Ӧ��
	TIMER_ID_DUT_TIMEOUT,

	// ���½�������
	TIMER_ID_RECONNECTION,

	// �������ӳ�ʱ
	TIMER_ID_RECONNECTION_TIME_OUT,
	
	TIMER_ID_PROTOCOL_55_PARAM_TX,// 55���ڷ��Ͳ���
	
	TIMER_ID_CAN_TEST,

    TIMER_ID_PROTOCOL_SWITCCH,                  // Э���л�
    
    TIMER_ID_DUT_UPGRATE,
	
    // Dut_UI_VERд��
	TIMER_ID_SET_DUT_UI_VER,

    // IOT�����̼�����
	TIMER_ID_SEND_UP_APP_REQUEST,
    
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

