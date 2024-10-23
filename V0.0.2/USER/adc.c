//#include "adc.h"
//#include "timer.h"
//#include "param.h"

//ADC_CB adcCB;

//// ADC通道映射
//const uint8 adcChannelMapping[] = {1};

//const uint16 batteryPercentTable[3][12] = 
//{
//	// 24V
//	{
//		29000,
//		27700,		// ≥该值，100%
//		27000,		// ≥该值，90%
//		26300,		// ≥该值，80%
//		25600,		// ≥该值，70%
//		25200,		// ≥该值，60%
//		25000,		// ≥该值，50%
//		24500,		// ≥该值，40%
//		24200,		// ≥该值，30%
//		23800,		// ≥该值，20%
//		23100,		// ≥该值，10%
//		21000		// ≥该值，0%，＜该值为欠压
//	},

//	// 36V
//	{
//		41000,
//		39500,
//		38500,
//		37500,
//		36500,
//		36000,
//		35600,
//		35000,
//		34500,
//		34000,
//		33000,
//		31500
//	},

//	// 48V
//	{
//		54000,
//		51400,
//		50100,
//		48800,
//		47500,
//		46800,
//		46300,
//		45500,
//		44900,
//		44200,
//		42900,
//		42000
//	}
//};

//const uint16 batteryPercent[12] = 
//{
//	10000,
//	9000,
//	7500,
//	6000,
//	4500,
//	3692,
//	3115,
//	2000,
//	1000,
//	800,
//	500,
//	0
//};

//#define BATTERY_PERCENT_GATE_SIZE		(sizeof(batteryPercentTable[0])/sizeof(batteryPercentTable[0][0]))
//#define BATTERY_PERCENT_GRADE			(100.0f/(BATTERY_PERCENT_GATE_SIZE-1))

////*********************************/
//// 内部函数申明
////*********************************/
//// 硬件ADC0初始化
//void ADC_HwInit(void);

//// 初始化结构体
//void ADC_InitStr(void);

//// 向缓冲区中添加一个数据
//void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data);

//// 设置读取请求回调函数，由定时器调用
//void ADC_CALLBACK_SetScanRequest(uint32 param);

//// 启动定时器任务，周期性地读取
//void ADC_HwStart(void);

//// 停止检测
//void ADC_Stop(void);

//// 设置电池连续显示变化值
//void ADC_TortoiseProcess(void);


////*********************************/
//// 函数主体
////*********************************/
//// 初始化
//void ADC_BaseInit(void)
//{
//	// ADC硬件初始化
//	ADC_HwInit();

//	// 初始化结构体
//	ADC_InitStr();

//	// 开始扫描
//	ADC_HwStart();
//}

//// 传感器处理，在main中调用
//void ADC_Process(void)
//{
//	uint16 adcData;
//	
//	// 有检测请求时，执行一次转换
//	if(adcCB.scanRequest)
//	{
//		// 上次转换未完成，退出，等待转换完成
//		if(!ADC_CONVERT_IS_BUSY())
//		{
//			return;
//		}
//		ADC_CONVERT_CLEAR_EOC();
//		
//		// 上次转换结束，清除转换请求
//		adcCB.scanRequest = FALSE;       

//		// 将数据添加到对应的缓冲区中
//		switch(adcCB.currentChannel)
//		{
//			// 电池电压通道
//			case ADC_CHANNEL_BATTERY: 
//				adcData = ADC_CONVERT_RESULT();
//				ADC_AddDataToBuff(ADC_CHANNEL_BATTERY, adcData);

//				// ADC不作为电量数据源时，不进行计算
//				if (BATTERY_DATA_SRC_ADC != PARAM_GetBatteryDataSrc())
//				{
//					break;
//				}
//				
//				// 计算检测实时电压
//				adcCB.battery.realtimeVoltage = adcCB.battery.adcAverage;
//				adcCB.battery.realtimeVoltage *= 21;
//				adcCB.battery.realtimeVoltage *= 3300;
//				adcCB.battery.realtimeVoltage /= 4096;
//				adcCB.battery.realtimeVoltage += 300;


//				//adcCB.battery.realtimeVoltage = adcCB.battery.adcAverage;
//				//adcCB.battery.realtimeVoltage *= paramCB.nvm.param.common.adRatioRef;
//				//adcCB.battery.realtimeVoltage /= 4096;

//				// 计算实时百分比
//				adcCB.battery.realtimePercent = ADC_GetBatteryPercent(adcCB.battery.realtimeVoltage);

//				// 电池容量的百分比显示平滑处理
//				ADC_TortoiseProcess();
//				break;
//				
//			// 非法通道
//			default:
//				break;
//		}

//		// 切换至下一个通道
//		adcCB.currentChannel ++;
//		adcCB.currentChannel %= ADC_CHANNEL_MAX;

//		// 选定下一个通道
//		ADC_CONVERT_CHANNEL(adcChannelMapping[adcCB.currentChannel]);
//		
//		// 启动下一次转换
//		ADC_CONVERT_START();
//	}
//}

//// 启动定时器任务，周期性地读取温度
//void ADC_HwStart(void)
//{
//	TIMER_KillTask(TIMER_ID_ADC_SCAN);

//	adcCB.currentChannel = ADC_CHANNEL_BATTERY;

//	// 注册定时器任务，周期性采样
//	TIMER_AddTask(TIMER_ID_ADC_SCAN,
//				ADC_SCAN_TIME,
//				ADC_CALLBACK_SetScanRequest,
//				0,
//				TIMER_LOOP_FOREVER,
//				ACTION_MODE_DO_AT_ONCE);
//					
//	// 启动下一次转换
//	ADC_CONVERT_START();
//}

//// 停止检测
//void ADC_Stop(void)
//{
//	TIMER_KillTask(TIMER_ID_ADC_SCAN);
//}

//// 硬件ADC0初始化
//void ADC_HwInit(void)
//{
//	GPIO_InitType GPIO_InitStructure;
//	ADC_InitType ADC_InitStructure;

//    // Enable GPIO clocks
//    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
//    // Enable ADC clocks
//    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

//    GPIO_InitStruct(&GPIO_InitStructure);
//    GPIO_InitStructure.Pin            = GPIO_PIN_0;
//    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
//    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
//    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
//    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Analog;
//    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

//    // RCC_ADCHCLK_DIVx
//    // ADC的输入时钟不得超过64MHz
//    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
//	  RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source

//	// ADC configuration
//    ADC_InitStructure.MultiChEn      = DISABLE;
//    ADC_InitStructure.ContinueConvEn = DISABLE;
//    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
//    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
//    ADC_InitStructure.ChsNumber      = 1;
//    ADC_Init(ADC, &ADC_InitStructure);

//	 ADC_ConfigRegularChannel(ADC, ADC_CH_1, 1, ADC_SAMP_TIME_55CYCLES5);
//    // Enable ADC
//    ADC_Enable(ADC, ENABLE);
//    // Check ADC Ready
//    while(ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET);
//    // 建议每次上电执行一次校准
//    // Start ADC1 calibration
//    ADC_StartCalibration(ADC);
//    // Check the end of ADC1 calibration
//    while (ADC_GetCalibrationStatus(ADC));
//    
//	// 启动ADC转换
//    ADC_EnableSoftwareStartConv(ADC, ENABLE);
//	
//}

//// 初始化结构体
//void ADC_InitStr(void)
//{
//	uint8 i;
//	
//	adcCB.scanRequest = FALSE;
//	adcCB.currentChannel = (ADC_CHANNEL_E)0;

//	// 相关数据结构初始化
//	for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
//	{
//		adcCB.battery.adcBuff[i] = 0;
//	}
//	adcCB.battery.index = 0;
//	adcCB.battery.adcAverage = 0;

//	// 相关数据结构初始化
//	for(i=0; i<ADC_BLE_BUFF_SIZE; i++)
//	{
//		adcCB.ble.adcBuff[i] = 0;
//	}
//	adcCB.ble.index = 0;
//	adcCB.ble.adcAverage = 0;
//}

//// 设置读取请求回调函数，由定时器调用
//void ADC_CALLBACK_SetScanRequest(uint32 param)
//{
//	adcCB.scanRequest = TRUE;
//}


//// 向缓冲区中添加一个数据
//void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data)
//{
//	uint8 i;
//	uint32 total;
//	uint16 min = 0xFFFF, max = 0;
//	
//	switch(channel)
//	{
//		// 通道
//		case ADC_CHANNEL_BATTERY:
//			adcCB.battery.adcBuff[adcCB.battery.index++] = data;
//			adcCB.battery.index %= ADC_BATTERY_BUFF_SIZE;

//			// 计算缓冲区的平均值
//			total = 0;
//			for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
//			{
//				// 更新最小值
//				if(adcCB.battery.adcBuff[i] < min)
//				{
//					min = adcCB.battery.adcBuff[i];
//				}

//				// 更新最大值
//				if(adcCB.battery.adcBuff[i] > max)
//				{
//					max = adcCB.battery.adcBuff[i];
//				}
//				
//				total += adcCB.battery.adcBuff[i];
//			}

//			// 去掉最大最小值
//			total -= min;
//			total -= max;

//			// 计算ADC平均值
//			adcCB.battery.adcAverage = total / (ADC_BATTERY_BUFF_SIZE-2);

//			break;

//		// 非法通道
//		default:
//			break;
//	}
//}

//// 获取电池百分比
//uint8 ADC_GetBatteryPercent(uint32 param)
//{
//	uint8 i;
//	uint8 vLevel;
////	uint16 spanVal;
//	uint8 percent;
//	float k = 0.0f, b = 0.0f;
//	float fPercent;

//	switch (PARAM_GetBatteryVoltageLevel())
//	{
//		case 24:
//			vLevel = 0;
//			break;

//		case 36:
//			vLevel = 1;
//			break;

//		case 48:
//			vLevel = 2;
//			break;

//		default:
//			vLevel = 1;
//			break;
//	}

//	// 在对应电压的电量里程碑中搜索，计算电池百分比
//	for (i = 0; i < BATTERY_PERCENT_GATE_SIZE; i++)
//	{
//		// 计算得到的百分比应能覆盖0%-100%
//		// 从100%开始搜索
//		if (param >= batteryPercentTable[vLevel][i])
//		{
//			if (0 == i)
//			{
//				return 100;
//			}

//			// y = kx+b，首先计算k
//			k = (float)((batteryPercent[i-1] - batteryPercent[i])/100.0f) / (float)(batteryPercentTable[vLevel][i-1] - batteryPercentTable[vLevel][i]);
//			b = batteryPercent[i]/100.0f - k * batteryPercentTable[vLevel][i];
//			
//			// 两个级别之间的跨度
//			//spanVal = batteryPercentTable[vLevel][i-1] - batteryPercentTable[vLevel][i];

//			// 计算实际的百分点
//			//percent = (100 - i*10) + ((param - batteryPercentTable[vLevel][i])*10 / spanVal);

//			fPercent = k * param + b;

//			if (fPercent >= 99.5f)
//			{
//				fPercent = 100;
//			}

//			percent = (uint8)fPercent;
//			
//			return percent;
//		}
//	}
//	
//	return 0;
//}

//// 设置电池百分比连续显示变化值
//void ADC_TortoiseProcess(void)
//{
//	static uint16 adcScanCount = 0;		// 这里的处理与ADC扫描控制使用同一个定时器，这里使用该参数对定时器进行分频，分频数与设定的滤波等级有关
//	static uint16 keynoteTimeCnt = 0;
//	static BOOL keynoteFlag = FALSE;	

//	// 在开机后的一个更新buf周期定下电池百分比基调
//	if (!keynoteFlag)
//	{
//		keynoteTimeCnt ++;
//		if (keynoteTimeCnt < ((ADC_BATTERY_BUFF_SIZE*ADC_SCAN_TIME*ADC_CHANNEL_MAX)/(ADC_SCAN_TIME*ADC_CHANNEL_MAX)))
//		{
//			adcCB.battery.displayPercent = ADC_GetBatteryPercent(adcCB.battery.realtimeVoltage);
//			
//			// 将实时的电池百分比放入参数层
//			PARAM_SetBatteryPercent(adcCB.battery.displayPercent);
//			
//			return;
//		}
//		keynoteFlag = TRUE;
//	}

//	// 将当前电压放入参数层
//	PARAM_SetBatteryVoltage(adcCB.battery.realtimeVoltage);

//	// 对ADC扫描定时器作分频处理
//	adcScanCount ++;
//	if (adcScanCount < ((uint32)PARAM_GetBatteryCapVaryTime()*1000UL/(ADC_SCAN_TIME*ADC_CHANNEL_MAX)))
//	{
//		return;
//	}
//	adcScanCount = 0;

//	// 显示百分比 ＜ 实时百分比，则显示百分比增大一个百分点
//	if(adcCB.battery.displayPercent < adcCB.battery.realtimePercent)
//	{
//		adcCB.battery.displayPercent++;
//		if(adcCB.battery.displayPercent > 100)
//		{
//			adcCB.battery.displayPercent = 100;
//		}
//	}
//	// 显示百分比 ＞ 实时百分比，则显示百分比降低一个百分点
//	else if(adcCB.battery.displayPercent > adcCB.battery.realtimePercent)
//	{
//		if(adcCB.battery.displayPercent > 0)
//		{
//			adcCB.battery.displayPercent--;
//		}
//	}

//	// 将处理后的百分比放入参数层
//	PARAM_SetBatteryPercent(adcCB.battery.displayPercent);

//	// 当前电量小于10，置位低电标志
//	PARAM_SetBatteryLowAlarm((BOOL)(PARAM_GetBatteryPercent() < 10));
//}
