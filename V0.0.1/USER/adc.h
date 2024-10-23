#ifndef __ADC_H__
#define __ADC_H__

#include "common.h"
#include "timer.h"

#define ADC_SCAN_TIME							10		// AD转换间隔时间，单位:ms

#define ADC_BATTERY_BUFF_SIZE					10		// 电池电压数据结构中的缓冲区大小
#define ADC_BLE_BUFF_SIZE						10		// 蓝牙数据结构中的缓冲区大小

//=============================================================================================
#define ADC_CONVERT_IS_BUSY()   				ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC)
#define ADC_CONVERT_START()     				ADC_EnableSoftwareStartConv(ADC, ENABLE)
#define ADC_CONVERT_STOP()      				ADC_EnableSoftwareStartConv(ADC, DISABLE)
#define ADC_CONVERT_CHANNEL(X)  				ADC_ConfigRegularChannel(ADC, X, 1, ADC_SAMP_TIME_55CYCLES5)
#define ADC_CONVERT_RESULT()   					ADC_GetDat(ADC)
#define ADC_CONVERT_CLEAR_EOC()   				ADC_ClearFlag(ADC, ADC_FLAG_ENDC)

// 通道定义，与硬件ADC的模拟通道一致
typedef enum
{
	ADC_CHANNEL_BATTERY,		// 电池电压

	ADC_CHANNEL_BLE,			// 蓝牙状态

	ADC_CHANNEL_MAX
}ADC_CHANNEL_E;

// 传感器模块数据结构
typedef struct
{
	volatile BOOL scanRequest;						// 转换请求
	ADC_CHANNEL_E currentChannel;					// 当前要扫描的通道

	// 电压相关数据结构
	struct
	{
		uint16 adcBuff[ADC_BATTERY_BUFF_SIZE];
		uint8  index;								// 当前处理的缓冲区下标
		uint16 adcAverage;							// 平均值

		uint32 realtimeVoltage;			// 检测结果电压

		uint8 realtimePercent;	// 实时电压百分比

		uint8 displayPercent;	// 电池过程百分比，用来防止大跳动
	}battery;

	// 刹车相关数据结构
	struct
	{
		uint16 adcBuff[ADC_BLE_BUFF_SIZE];
		uint8  index;								// 当前处理的缓冲区下标
		uint16 adcAverage;							// 平均值
	}ble;
}ADC_CB;

extern ADC_CB adcCB;

//==============================================
// 初始化
void ADC_BaseInit(void);

// ADC处理，在main中调用
void ADC_Process(void);

#endif

