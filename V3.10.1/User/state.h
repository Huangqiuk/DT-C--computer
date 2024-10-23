#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// ���óɹ��󡢷����ϼ��˵�ǰ�ĵȴ�ʱ��
#define TIME_WAIT_POWER_ON_OFF					2000ul		// ���ػ��ĵȴ�ʱ��

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// ��ʼ���а���¼����λ�ü�¼�ĳ�ʱʱ��

// ϵͳ״̬����
typedef enum
{
	STATE_NULL = 0,								// ��״̬

	STATE_ENTRY,								// ���״̬

	STATE_JUMP,									// ��תAPP

	STATE_MAX									// ״̬��
}STATE_E;

typedef struct
{	
	STATE_E state;					// ��ǰϵͳ״̬
	STATE_E preState;				// ��һ��״̬
	STATE_E persLoveState;			// ����ϲ����״̬

	
}STATE_CB;

extern STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void);

// ״̬Ǩ��
void STATE_EnterState(uint32 state);

// ״̬������
void STATE_Process(void);

#endif


