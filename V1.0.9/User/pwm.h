#ifndef __PWM_H__
#define __PWM_H__

#include "common.h"

#define PWM_OUT1_ON()  do{\
	       PWM_OUT1_PWM(100);\
	       timer_primary_output_config(TIMER4, ENABLE);\
       }while(0)
#define PWM_OUT1_OFF() do{\
	       PWM_OUT1_PWM(0);\
	       timer_primary_output_config(TIMER4, ENABLE);\
       }while(0)
#define PWM_OUT1_PWM(i) do{\
	       timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0, (i * 100) / 6600);\
	       timer_primary_output_config(TIMER4, ENABLE);\
       }while(0)

// LCD����ƹ�����ģʽ
typedef enum
{
	PWM_OUT1_LEVEL_OFF = 0,	// �ر�
//	PWM_OUT1_LEVEL_5_100,
//	PWM_OUT1_LEVEL_10_100,		// 10%
//	PWM_OUT1_LEVEL_15_100,
	PWM_OUT1_LEVEL_20_100,		// 20%
//	PWM_OUT1_LEVEL_25_100,
//	PWM_OUT1_LEVEL_30_100,		// 30%
//	PWM_OUT1_LEVEL_35_100,
	PWM_OUT1_LEVEL_40_100,		// 40%
//	PWM_OUT1_LEVEL_45_100,
//	PWM_OUT1_LEVEL_50_100,		// 50%
//	PWM_OUT1_LEVEL_55_100,
	PWM_OUT1_LEVEL_60_100,		// 60%
//	PWM_OUT1_LEVEL_65_100,
//	PWM_OUT1_LEVEL_70_100,		// 70%
//	PWM_OUT1_LEVEL_75_100,
	PWM_OUT1_LEVEL_80_100,		// 80%
//	PWM_OUT1_LEVEL_85_100,
//	PWM_OUT1_LEVEL_90_100,		// 90%
//	PWM_OUT1_LEVEL_95_100,
	PWM_OUT1_LEVEL_ON,			// 100%����
	
	PWM_OUT1_LEVEL_AUTO
}PWM_OUT1_LEVEL_E;

// �����ṹ��
typedef struct
{
	PWM_OUT1_LEVEL_E level;

	uint16 step;
			 
}PWM_OUT1_CB;

extern PWM_OUT1_CB pwmout1CB;


/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// ��ʼ��
void PWM_OUT1_Init(void);

// ���ö�ʱ������ָ������ģʽ
void PWM_OUT1_Start(PWM_OUT1_LEVEL_E level);


#endif

