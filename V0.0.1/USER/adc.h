#ifndef __ADC_H__
#define __ADC_H__

#include "common.h"
#include "timer.h"

#define ADC_SCAN_TIME							10		// ADת�����ʱ�䣬��λ:ms

#define ADC_BATTERY_BUFF_SIZE					10		// ��ص�ѹ���ݽṹ�еĻ�������С
#define ADC_BLE_BUFF_SIZE						10		// �������ݽṹ�еĻ�������С

//=============================================================================================
#define ADC_CONVERT_IS_BUSY()   				ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC)
#define ADC_CONVERT_START()     				ADC_EnableSoftwareStartConv(ADC, ENABLE)
#define ADC_CONVERT_STOP()      				ADC_EnableSoftwareStartConv(ADC, DISABLE)
#define ADC_CONVERT_CHANNEL(X)  				ADC_ConfigRegularChannel(ADC, X, 1, ADC_SAMP_TIME_55CYCLES5)
#define ADC_CONVERT_RESULT()   					ADC_GetDat(ADC)
#define ADC_CONVERT_CLEAR_EOC()   				ADC_ClearFlag(ADC, ADC_FLAG_ENDC)

// ͨ�����壬��Ӳ��ADC��ģ��ͨ��һ��
typedef enum
{
	ADC_CHANNEL_BATTERY,		// ��ص�ѹ

	ADC_CHANNEL_BLE,			// ����״̬

	ADC_CHANNEL_MAX
}ADC_CHANNEL_E;

// ������ģ�����ݽṹ
typedef struct
{
	volatile BOOL scanRequest;						// ת������
	ADC_CHANNEL_E currentChannel;					// ��ǰҪɨ���ͨ��

	// ��ѹ������ݽṹ
	struct
	{
		uint16 adcBuff[ADC_BATTERY_BUFF_SIZE];
		uint8  index;								// ��ǰ����Ļ������±�
		uint16 adcAverage;							// ƽ��ֵ

		uint32 realtimeVoltage;			// �������ѹ

		uint8 realtimePercent;	// ʵʱ��ѹ�ٷֱ�

		uint8 displayPercent;	// ��ع��̰ٷֱȣ�������ֹ������
	}battery;

	// ɲ��������ݽṹ
	struct
	{
		uint16 adcBuff[ADC_BLE_BUFF_SIZE];
		uint8  index;								// ��ǰ����Ļ������±�
		uint16 adcAverage;							// ƽ��ֵ
	}ble;
}ADC_CB;

extern ADC_CB adcCB;

//==============================================
// ��ʼ��
void ADC_BaseInit(void);

// ADC������main�е���
void ADC_Process(void);

#endif

