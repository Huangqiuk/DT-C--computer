#ifndef __PWM_LED_H__
#define __PWM_LED_H__

/********************************* ͷ�ļ�************************************/
#include "common.h"

/********************************* ģ���������޸� ************************************/
// ��Ӳ��PWMģʽ�л�
#define LED_PWM_MODE							LED_HARD_PWM				// LED����ģʽ
#define LED_SOFT_PWM							0							// LED���PWMģʽ
#define LED_HARD_PWM							1 							// LEDӲ��PWMģʽ

// ����˿�
#define LED_GPIO 								GPIOB

#define LED_RED_PIN 							GPIO_PIN_15
#define LED_YELLOW_PIN 							GPIO_PIN_13
#define LED_GREEN_PIN 							GPIO_PIN_12

// LED������
typedef enum
{
	LED_NAME_RED = 0,
	LED_NAME_YELLOW,
	LED_NAME_GREEN,

	LED_NAME_MAX
}LED_NAME;

// LED����
#define LED_ACTION(GPIOx, PIN_x, ACTION)		(ACTION?GPIO_SetBits(GPIOx,PIN_x):GPIO_ResetBits(GPIOx,PIN_x))


// ��ƿ���
#define	LED_RED_PWM(i)							do{\
													TIM_SetCmp4(TIM9, i);\
												}while(0)
	
// �Ƶƿ���
#define	LED_YELLOW_PWM(i)						do{\
													TIM_SetCmp2(TIM9, i);\
												}while(0)

// �̵ƿ���
#define	LED_GREEN_PWM(i)						do{\
													TIM_SetCmp1(TIM9, i);\
												}while(0)

// LED��������ִ�м��,��λΪms
// Ӳ���ɵ�����ΧΪ(TIMER_TIME(��ʱ������)~100);
// ����ɵ�����ΧΪ10����������Ϊ����ֵ;
#define LED_COUNT_TIME							10ul		// ��������

// ��һ������ʱִ�У�ִ��ָ��LED,1s���ڣ���ռ50Ч����˸ LED_Set(x, 1000, 50, 100, 0, 0, 0)
// ����������ʱִ�У�ִ��ָ��LED,ָ������,��ռ50Ч����˸ LED_Set(x, y, 50, 100, 0, 0, 0)
// ����������ʱִ�У�ִ��ָ��LED,ָ������,ָ����ռ�������ڵİٷֱ���˸ LED_Set(x, y, z, 100, 0, 0, 0)
// ���ĸ�����ʱִ�У�ִ��ָ��LED,ָ������,ָ����ռ�������ڵİٷֱȣ�ָ��������Ⱥ���Ч�� LED_Set(x, y, z, 100, 0, 0, 0)
// ���������ʱִ�У�ִ��ָ��LED,ָ������,ָ����ռ�������ڵİٷֱȣ�ָ��������Ⱥ���Ч�� ����������ֵ LED_Set(x, y, z, 100, 0, 0, 0)
// ����������ʱִ��: Ԥ��
// ���߸�����ʱִ��: LED_Setһ��
#define LED_SET(...) GetMacro(__VA_ARGS__,LED_SET_ALL, LED_SET_ALL_LOOP_FOREVER, LED_SET_BREATH, LED_SET_HIGHLIMIT, LED_SET_FLASH_ACTIONTIMES, LED_SET_FLASH, LED_SET_ON_RATIO, LED_SET_ON, ...)(__VA_ARGS__)

// �ӿڵ����غ꺯��
#define LED_SET_ON(x)								LED_Set(x, 1000, 100, 100, 0, 0, 0, -1)

#define LED_SET_ON_RATIO(x,y)						LED_Set(x, 1000, y, 100, 0, 0, 0, -1)

#define LED_SET_FLASH(x,y,z)						LED_Set(x, y, z, 100, 0, 0, 0, -1)

#define LED_SET_FLASH_ACTIONTIMES(x,y,z,a)			LED_Set(x, y, z, 100, 0, 0, 0, a)

#define LED_SET_HIGHLIMIT(x,y,z,a,b)				LED_Set(x, y, z, b, 0, 0, 0, a)

#define LED_SET_BREATH(x,y,z,a,b,c)					LED_Set(x, y, z, 100, 0, 0, 0, a)

#define LED_SET_ALL_LOOP_FOREVER(x,y,z,a,b,c,d)		LED_Set(x, y, z, a, b, c, d, -1)

#define LED_SET_ALL(x,y,z,a,b,c,d,e)				LED_Set(x, y, z, a, b, c, d, e)

#define GetMacro(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME


/********************************* ģ��ͨ�ò��ֽ�ֹ�޸� ************************************/
// LED������ʶ
typedef enum
{
	LED_ACTION_OFF = 0,					// �ر�
	LED_ACTION_ON						// ��
}LED_ACTION_E;

// LED ռ�ձ����ֵ
#define LED_PWM_OUT_MAX							100			// ռ�ձ����ֵ

// LED��������
typedef struct{
	// ���PWM
	volatile uint32 controlPwmOut;			// �������PWM

	// ����ֵ	
	uint16 ledCounter;						// LED ������ֵ
	uint16 breathUpCounter;					// LED ������ֵ ���ں���Ч������
	uint16 breathDownCounter;				// LED �����ֵ	���ں���Ч������

	// ������ֵ
	uint16 cycleCountTimesSetValue;			// LED�������ڼ�ʱ��
	uint16 onCycleCountTimesSetValue;		// ��ʱ�����	
	uint8 highLimitPwmOut;					// LED��ʱ���ȱȣ�0-100��Ĭ��100	
	uint8 lowLimitPwmOut;					// LED��ʱ���ȱȣ�0-100��Ĭ��100
	uint16 breathUpCountTimesSetValue;		// LED������ֵ ���ں���Ч������
	uint16 breathDownCountTimesSetValue;	// LED�����ֵ ���ں���Ч������
	int32  loopCount;						// ѭ����������
	
	// ����״̬
	LED_ACTION_E action;
	LED_ACTION_E lastAction;
	
	void (*fun)(uint32 param);

	// LEDӲ������
	void* LED_GPIO_x;
	uint16 LED_PIN_x;
}LED_TYPE_QUEUE;

typedef struct{
	LED_TYPE_QUEUE typeQueue[LED_NAME_MAX];		// ÿ��LED��һ����������
}LED_CB;

extern LED_CB ledCB;

/********************************* �����ⲿ����***************************/
// Led��ʼ��
void LED_Inits(uint32 mainFre);	

// ����LED����ģʽ
void LED_Set(LED_NAME ledName, uint32 ledCycleTime, uint8 ledOnDutyRatio, uint8 onBrightnessRatio, uint8 offBrightnessRatio, uint8 ledOnAscendRatio, uint8 ledOnDescendRatio,int32 actionTimes);

// �������ڻ�ȡ�ӿ�
uint8 LED_GetControlCycle(LED_NAME ledName);

// ���ܲ��Խӿ�
void LED_CloseAllLeds(void);

void RunLed_Init(void);
#endif

