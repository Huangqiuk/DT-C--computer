#ifndef _TIME3_H_
#define _TIME3_H_

#include "main.h"



#define TIMER_TIME								1	// �����ʱ�����ڣ��������ʱ����Сʱ��ֱ��ʣ���λ��ms
#define TIMER_LOOP_FOREVER						-1	// ����ѭ��

#define TIMER_TASK_QUEUE_SIZE					5	// ��ʱ�����ڴ����������



typedef enum
{
	TIMER_ID_NULL = -1,

//	TIMER_ID_1,//����1							
//	TIMER_ID_2,//����2
//	
//	TIMER_ID_KEY_HMI_CMD,//�����ϱ����ʱ��
//	
//	TIMER_ID_KEY_CONN,//���ӻ��Զ�ʱ��
//	
//	TIMER_ID_NFC_DELAY,//NFC
//	TIMER_ID_KEY_SCAN,//����ɨ��ʱ��
	TIMER_ID_JUMP,								// ��ת
	
	TIMER_ID_STATE_CONTROL,						// ״̬��ģ�����

//	TIMER_ID_MENU_QUIT,							// �˳��˵�����
//	
//	TIMER_ID_STATE_ON,							// ��������

//	TIMER_ID_FUNCTION_TEST,						// �ϻ����Խ���

//	TIMER_ID_AGE_SEND,							// �ϻ�����Ͷ�ʱ��
//	
//	TIMER_ID_PROTOCOL_PARAM_TX,					// Э������ʱ�·�

//	TIMER_ID_PROTOCOL_TXRX_TIME_OUT,			// Э���ͨѶ��ʱ
//	
//	TIMER_ID_PROTOCOL_SPEED_FILTER,				// �ٶ�ƽ��

//	TIMER_ID_KEY_CLICK_FUN,						// ������˫��
//	
//	TIMER_ID_AUTO_POWER_OFF,					// �Զ��ػ�

//	TIMER_ID_ADC_SCAN,							// ADת�����

//	TIMER_ID_BUZZER_WORK,						// ����������

	TIMER_ID_TEST,								//	����
	
//	TIMER_ID_BLE_NUMBER,

//    TIMER_ID_UP_FINISH_REPORT,
//    
    TIMER_ID_IAP_RUN_WINDOW_TIME,
    
	TIMER_ID_MAX
}TIMER_ID;

typedef enum
{
	ACTION_MODE_DO_AT_ONCE = 0,	// ����ִ��
	ACTION_MODE_ADD_TO_QUEUE	// ��ӵ����������ִ��
}ACTION_MODE_E;

// ��ʱ������
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
	uint8_t  actionMode[TIMER_ID_MAX];		// ִ��ģʽ
	uint8_t active[TIMER_ID_MAX];				// �Ƿ�������״̬

	// ��ʱ��������У�����ʱ������ִ��ʱ��ϳ�ʱ�����뽫��ŵ��ж�����ִ�У�������������������ִ��
	struct{
		TIMER_TASK task[TIMER_TASK_QUEUE_SIZE];
		uint8_t head;
		uint8_t end;
	}queue;
}TIMER_CB;

extern TIMER_CB timerCB;

//��ʱ����ʼ��
void TIMER_Init(void);

uint8_t TIMER_AddTask(TIMER_ID id, uint32_t time, void(*action)(uint32_t param), uint32_t param, int32_t actionTime, ACTION_MODE_E actionMode);

void TIMER_KillTask(TIMER_ID id);

// ��ʱ��������������һ������
uint8_t TIMER_AddToQueue(void(*action)(uint32_t param), uint32_t param);

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
uint8_t TIMER_ChangeTime(TIMER_ID id, uint32_t time);



//-----1Khz------
//------1ms------
void Time3_Init();




#endif

