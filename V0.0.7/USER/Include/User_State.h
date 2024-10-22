#ifndef _USER_STATE_H_
#define _USER_STATE_H_





// ϵͳ״̬����
typedef enum
{
    STATE_NULL = 0,     // ��״̬
    STATE_BROADCAST,    // �㲥״̬
    STATE_PAIR,             // ���״̬
    STATE_CONNECT,      // ����״̬

    STATE_SLEEP,            //�͹���״̬

    STATE_MAX                       // ״̬��
} STATE_E;


typedef struct
{
    STATE_E state;                  // ��ǰϵͳ״̬
    STATE_E preState;               // ��һ��״̬
    STATE_E persLoveState;          // ����ϲ����״̬


} STATE_CB;

extern STATE_CB stateCB;


// ״̬����ʼ��
void STATE_Init(void);

// ״̬Ǩ��
void STATE_EnterState(STATE_E state);

// ״̬������
void STATE_Process(void);


#endif  //_USER_TIMER_H_
