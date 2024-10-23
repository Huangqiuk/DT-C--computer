#ifndef __STATE_H__
#define __STATE_H__				

#define TIME_WAIT_AFTER_OK						1000ul		// ���óɹ��󡢷����ϼ��˵�ǰ�ĵȴ�ʱ��
#define TIME_WAIT_POWER_ON_OFF					2000ul		// ���ػ��ĵȴ�ʱ��

#define TIME_WAIT_NO_POSITON_RECORD				(600000ul)	// ��ʼ���а���¼����λ�ü�¼�ĳ�ʱʱ��


#define LED_PWMBUFF    							100
#define LED_PWMBUFF0                           	0  

// ϵͳ״̬����
typedef enum
{
	STATE_NULL = 0,								// ��״̬

	STATE_ENTRY,								// ���״̬

	STATE_JUMP,									// ��תAPP

	STATE_MAX									// ״̬��
}STATE_E;

typedef  enum
{

	UPDATE_PROGRESS_NULL=0,
	UPDATE_PROGRESS_0,
	UPDATE_PROGRESS_20,
	UPDATE_PROGRESS_40,
	UPDATE_PROGRESS_60,
	UPDATE_PROGRESS_80,
	UPDATE_PROGRESS_100,

	UPDATE_PROGRESS_MAX

}BLUE_UPDATE;


typedef struct
{	
	STATE_E state;					// ��ǰϵͳ״̬
	STATE_E preState;				// ��һ��״̬
	STATE_E persLoveState;			// ����ϲ����״̬

	BLUE_UPDATE  ledPrePro;
	BLUE_UPDATE  ledNewPro;

	
}STATE_CB;

extern STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void);

// ״̬Ǩ��
void STATE_EnterState(uint32 state);

// ״̬������
void STATE_Process(void);

void STATE_UpradeStateShow(uint32 param);

#endif


