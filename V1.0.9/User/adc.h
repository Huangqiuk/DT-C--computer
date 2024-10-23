#ifndef __ADC_H__
#define __ADC_H__

#include "common.h"
#include "timer.h"

#define ADC_SCAN_TIME							5		// AD转换间隔时间，单位:ms
#define ADC_BATTERY_DEFAULT_FILTER_TIME			1		// 电池百分比滤波时间，单位:s，为0时相当于不处理

#define ADC_LEVEL_DETECTOR_BUFF_SIZE			10		// 电池数据结构中的缓冲区大小
#define ADC_ZERO_ELECTRIC_CONTINUED_TIME		3		// 电流持续为0的时间，单位秒

//=============================================================================================
#define ADC_CONVERT_IS_BUSY()   				adc_flag_get(ADC0, ADC_FLAG_EOC)
#define ADC_CONVERT_START()						adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
//#define ADC_CONVERT_STOP()      				adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, DISABLE)	// 停止ADC0转换
#define ADC_CONVERT_CHANNEL(X)  				adc_regular_channel_config(ADC0, 0, X, ADC_SAMPLETIME_55POINT5);
#define ADC_CONVERT_RESULT()   					adc_regular_data_read(ADC0)


// 通道定义，与硬件ADC的模拟通道一致
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

// 传感器模块数据结构
typedef struct
{
	volatile BOOL scanRequest;						// 转换请求
	ADC_CHANNEL_E currentChannel;					// 当前要扫描的通道

	// level0相关数据结构
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE];		// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值
		uint32 voltage;										// 检测结果电压		
	}level0;

	// level1相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}level1;

	// level2相关数据结构
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE];		// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值
		uint32 voltage;										// 检测结果电压		
	}level2;

	// level3相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}level3;

	// level4相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}level4;

	// dutusb相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}dutusb;

	// usbcurrent相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}usbcurrent;

	// key相关ADC数据
	struct
	{
		uint16 adcBuff[ADC_LEVEL_DETECTOR_BUFF_SIZE]; 	// 非骑行状态下电压值采集		
		uint8 index;										// 当前处理的缓冲区下标		
		uint16 adcAverage;									// 平均值		
		uint32 voltage; 									// 检测结果电压		
	}key;    
}ADC_CB;

extern ADC_CB adcCB;

//==============================================
// 初始化
void ADC_BaseInit(void);

// ADC处理，在main中调用
void ADC_Process(void);

// 电压计算电量
uint8 ADC_GetBatteryPercent(uint32 param);


#endif

