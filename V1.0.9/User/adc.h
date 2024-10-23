#ifndef __ADC_H__
#define __ADC_H__

#include "common.h"
#include "timer.h"

#define ADC_SCAN_TIME							5		// ADת�����ʱ�䣬��λ:ms
#define ADC_BATTERY_DEFAULT_FILTER_TIME			1		// ��ذٷֱ��˲�ʱ�䣬��λ:s��Ϊ0ʱ�൱�ڲ�����

#define ADC_LEVEL_DETECTOR_BUFF_SIZE			10		// ������ݽṹ�еĻ�������С
#define ADC_ZERO_ELECTRIC_CONTINUED_TIME		3		// ��������Ϊ0��ʱ�䣬��λ��

//=============================================================================================
#define ADC_CONVERT_IS_BUSY()   				adc_flag_get(ADC0, ADC_FLAG_EOC)
#define ADC_CONVERT_START()						adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
//#define ADC_CONVERT_STOP()      				adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, DISABLE)	// ֹͣADC0ת��
#define ADC_CONVERT_CHANNEL(X)  				adc_regular_channel_config(ADC0, 0, X, ADC_SAMPLETIME_55POINT5);
#define ADC_CONVERT_RESULT()   					adc_regular_data_read(ADC0)


// ͨ�����壬��Ӳ��ADC��ģ��ͨ��һ��
typedef enum
{
	ADC_CHANNEL_LEVEL_IN0 = 0,
	ADC_CHANNEL_LEVEL_IN1,    
	ADC_CHANNEL_LEVEL_IN2,
	ADC_CHANNEL_LEVEL_IN3,    
	ADC_CHANNEL_LEVEL_IN4,
	ADC_CHANNEL_DUT_USB_ADC,    
	ADC_CHANNEL_USB_CURRENT,
	ADC_CHANNEL_KEY_ADC,
    
	ADC_CHANNEL_MAX
}ADC_CHANNEL_E;

// ������ģ�����ݽṹ
typedef struct
{
	volatile BOOL scanRequest;						// ת������
	ADC_CHANNEL_E currentChannel;					// ��ǰҪɨ���ͨ��

	// level0������ݽṹ
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE];		// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ
		uint32 voltage;										// �������ѹ		
	}level0;

	// level1���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}level1;

	// level2������ݽṹ
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE];		// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ
		uint32 voltage;										// �������ѹ		
	}level2;

	// level3���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}level3;

	// level4���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}level4;

	// dutusb���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}dutusb;

	// usbcurrent���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}usbcurrent;

	// key���ADC����
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// ������״̬�µ�ѹֵ�ɼ�		
		uint8 index;										// ��ǰ����Ļ������±�		
		uint16 adcAverage;									// ƽ��ֵ		
		uint32 voltage; 									// �������ѹ		
	}key;    
}ADC_CB;

extern ADC_CB adcCB;

//==============================================
// ��ʼ��
void ADC_BaseInit(void);

// ADC������main�е���
void ADC_Process(void);

// ��ѹ�������
uint8 ADC_GetBatteryPercent(uint32 param);


#endif

