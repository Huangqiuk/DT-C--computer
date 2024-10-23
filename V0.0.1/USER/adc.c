#include "adc.h"
#include "timer.h"
//#include "param.h"

ADC_CB adcCB;

// ADC通道映射
const uint8 adcChannelMapping[] = {1, 2};

const uint16 batteryPercentTable[3][11] = 
{
	// 24V
	{
		27700,		// ≥该值，100%
		27000,		// ≥该值，90%
		26300,		// ≥该值，80%
		25600,		// ≥该值，70%
		25200,		// ≥该值，60%
		25000,		// ≥该值，50%
		24500,		// ≥该值，40%
		24200,		// ≥该值，30%
		23800,		// ≥该值，20%
		23100,		// ≥该值，10%
		21000		// ≥该值，0%，＜该值为欠压
	},

	// 36V
	{
		39500,
		38500,
		37500,
		36500,
		36000,
		35600,
		35000,
		34500,
		34000,
		33000,
		31500
	},

	// 48V
	{
		51400,
		50100,
		48800,
		47500,
		46800,
		46300,
		45500,
		44900,
		44200,
		42900,
		42000
	}
};
#define BATTERY_PERCENT_GATE_SIZE		(sizeof(batteryPercentTable[0])/sizeof(batteryPercentTable[0][0]))
#define BATTERY_PERCENT_GRADE			(100.0f/(BATTERY_PERCENT_GATE_SIZE-1))

//*********************************/
// 内部函数申明
//*********************************/
// 硬件ADC0初始化
void ADC_HwInit(void);

// 初始化结构体
void ADC_InitStr(void);

// 向缓冲区中添加一个数据
void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data);

// 设置读取请求回调函数，由定时器调用
void ADC_CALLBACK_SetScanRequest(uint32 param);

// 启动定时器任务，周期性地读取
void ADC_HwStart(void);

// 停止检测
void ADC_Stop(void);

// 设置电池连续显示变化值
void ADC_TortoiseProcess(void);

// 获取电池百分比
uint8 ADC_GetBatteryPercent(uint32 param);


//*********************************/
// 函数主体
//*********************************/
// 初始化
void ADC_BaseInit(void)
{
	// ADC硬件初始化
	ADC_HwInit();

	// 初始化结构体
	ADC_InitStr();

	// 开始扫描
	ADC_HwStart();
}

// 传感器处理，在main中调用
void ADC_Process(void)
{
	uint16 adcData;
	
	// 有检测请求时，执行一次转换
	if(adcCB.scanRequest)
	{
		// 上次转换未完成，退出，等待转换完成
		if(!ADC_CONVERT_IS_BUSY())
		{
			return;
		}
		ADC_CONVERT_CLEAR_EOC();
		
		// 上次转换结束，清除转换请求
		adcCB.scanRequest = bFALSE;       

		// 将数据添加到对应的缓冲区中
		switch(adcCB.currentChannel)
		{
			// 电池电压通道
			case ADC_CHANNEL_BATTERY: 
				adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_BATTERY, adcData);

				
				
				break;

			// 蓝牙状态通道
			case ADC_CHANNEL_BLE:
				adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_BLE, adcData);

				
				break;
				
			// 非法通道
			default:
				break;
		}

		// 切换至下一个通道
		adcCB.currentChannel ++;
		adcCB.currentChannel %= ADC_CHANNEL_MAX;

		// 选定下一个通道
		ADC_CONVERT_CHANNEL(adcChannelMapping[adcCB.currentChannel]);
		
		// 启动下一次转换
		ADC_CONVERT_START();
	}
}

// 启动定时器任务，周期性地读取温度
void ADC_HwStart(void)
{
	TIMER_KillTask(TIMER_ID_ADC_SCAN);

	adcCB.currentChannel = ADC_CHANNEL_BATTERY;

	// 注册定时器任务，周期性采样
	TIMER_AddTask(TIMER_ID_ADC_SCAN,
				ADC_SCAN_TIME,
				ADC_CALLBACK_SetScanRequest,
				0,
				TIMER_LOOP_FOREVER,
				ACTION_MODE_DO_AT_ONCE);
					
	// 启动下一次转换
	ADC_CONVERT_START();
}

// 停止检测
void ADC_Stop(void)
{
	TIMER_KillTask(TIMER_ID_ADC_SCAN);
}

// 硬件ADC0初始化
void ADC_HwInit(void)
{
	GPIO_InitType GPIO_InitStructure;
	ADC_InitType ADC_InitStructure;

    // Enable GPIO clocks
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    // Enable ADC clocks
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin            = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // RCC_ADCHCLK_DIVx
    // ADC的输入时钟不得超过64MHz
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
	RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

	// ADC configuration
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

	ADC_ConfigRegularChannel(ADC, ADC_CH_1, 1, ADC_SAMP_TIME_55CYCLES5);
    // Enable ADC
    ADC_Enable(ADC, ENABLE);
    // Check ADC Ready
    while(ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);
    // 建议每次上电执行一次校准
    // Start ADC1 calibration
    ADC_StartCalibration(ADC);
    // Check the end of ADC1 calibration
    while (ADC_GetCalibrationStatus(ADC));
    
	// 启动ADC转换
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
	
}

// 初始化结构体
void ADC_InitStr(void)
{
	uint8 i;
	
	adcCB.scanRequest = bFALSE;
	adcCB.currentChannel = (ADC_CHANNEL_E)0;

	// 相关数据结构初始化
	for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
	{
		adcCB.battery.adcBuff[i] = 0;
	}
	adcCB.battery.index = 0;
	adcCB.battery.adcAverage = 0;

	// 相关数据结构初始化
	for(i=0; i<ADC_BLE_BUFF_SIZE; i++)
	{
		adcCB.ble.adcBuff[i] = 0;
	}
	adcCB.ble.index = 0;
	adcCB.ble.adcAverage = 0;
}

// 设置读取请求回调函数，由定时器调用
void ADC_CALLBACK_SetScanRequest(uint32 param)
{
	adcCB.scanRequest = TRUE;
}


// 向缓冲区中添加一个数据
void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data)
{
	uint8 i;
	uint32 total;
	uint16 min = 0xFFFF, max = 0;
	
	switch(channel)
	{
		// 通道
		case ADC_CHANNEL_BATTERY:
			adcCB.battery.adcBuff[adcCB.battery.index++] = data;
			adcCB.battery.index %= ADC_BATTERY_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.battery.adcBuff[i] < min)
				{
					min = adcCB.battery.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.battery.adcBuff[i] > max)
				{
					max = adcCB.battery.adcBuff[i];
				}
				
				total += adcCB.battery.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.battery.adcAverage = total / (ADC_BATTERY_BUFF_SIZE-2);

			break;

		// 通道
		case ADC_CHANNEL_BLE:
			adcCB.ble.adcBuff[adcCB.ble.index++] = data;
			adcCB.ble.index %= ADC_BLE_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_BLE_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.ble.adcBuff[i] < min)
				{
					min = adcCB.ble.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.ble.adcBuff[i] > max)
				{
					max = adcCB.ble.adcBuff[i];
				}
				
				total += adcCB.ble.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.ble.adcAverage = total / (ADC_BLE_BUFF_SIZE-2);
			break;

		// 非法通道
		default:
			break;
	}
}



