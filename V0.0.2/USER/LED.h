#ifndef _LED_H_
#define _LED_H_

#include "common.h"
#include "pwmled.h"

// ������
#define LED_PWMBULLBUFF    						100
#define LED_PWMBUFF    							100
#define LED_PWMBUFF0                           	0  



// LED ˢ����ʾ�ṹ��
typedef struct
{
	BOOL   Enblink;
	BOOL   Enblink2;
}LEDCOMMON_CB;


extern LEDCOMMON_CB ledCommonCB;


// LED ȫ�������
void LED_AllOnOff(uint32 data);

// LED ���𿪹�
void LED_SetLedOnOff(LED_NAME seg,  BOOL onoff);

// ����״̬��ʾ
void LED_RidingStateShow(BOOL  refreshAll);

// �����ƿ���
void  LED_BatLedSwitching(BOOL enable);

// ��λ�Ƹ�����˸
void  LED_AssistLedBlink(uint32 param);

// ��������״̬
void  LED_UpradeStateShow(uint32 param);


// ����״̬
void LED_ErrocodeStateShow(BOOL  refreshAll);


#endif




