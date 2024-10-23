//#include "adc.h"
//#include "timer.h"
//#include "param.h"

//ADC_CB adcCB;

//// ADCͨ��ӳ��
//const uint8 adcChannelMapping[] = {1};

//const uint16 batteryPercentTable[3][12] = 
//{
//	// 24V
//	{
//		29000,
//		27700,		// �ݸ�ֵ��100%
//		27000,		// �ݸ�ֵ��90%
//		26300,		// �ݸ�ֵ��80%
//		25600,		// �ݸ�ֵ��70%
//		25200,		// �ݸ�ֵ��60%
//		25000,		// �ݸ�ֵ��50%
//		24500,		// �ݸ�ֵ��40%
//		24200,		// �ݸ�ֵ��30%
//		23800,		// �ݸ�ֵ��20%
//		23100,		// �ݸ�ֵ��10%
//		21000		// �ݸ�ֵ��0%������ֵΪǷѹ
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
//// �ڲ���������
////*********************************/
//// Ӳ��ADC0��ʼ��
//void ADC_HwInit(void);

//// ��ʼ���ṹ��
//void ADC_InitStr(void);

//// �򻺳��������һ������
//void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data);

//// ���ö�ȡ����ص��������ɶ�ʱ������
//void ADC_CALLBACK_SetScanRequest(uint32 param);

//// ������ʱ�����������Եض�ȡ
//void ADC_HwStart(void);

//// ֹͣ���
//void ADC_Stop(void);

//// ���õ��������ʾ�仯ֵ
//void ADC_TortoiseProcess(void);


////*********************************/
//// ��������
////*********************************/
//// ��ʼ��
//void ADC_BaseInit(void)
//{
//	// ADCӲ����ʼ��
//	ADC_HwInit();

//	// ��ʼ���ṹ��
//	ADC_InitStr();

//	// ��ʼɨ��
//	ADC_HwStart();
//}

//// ������������main�е���
//void ADC_Process(void)
//{
//	uint16 adcData;
//	
//	// �м������ʱ��ִ��һ��ת��
//	if(adcCB.scanRequest)
//	{
//		// �ϴ�ת��δ��ɣ��˳����ȴ�ת�����
//		if(!ADC_CONVERT_IS_BUSY())
//		{
//			return;
//		}
//		ADC_CONVERT_CLEAR_EOC();
//		
//		// �ϴ�ת�����������ת������
//		adcCB.scanRequest = FALSE;       

//		// ��������ӵ���Ӧ�Ļ�������
//		switch(adcCB.currentChannel)
//		{
//			// ��ص�ѹͨ��
//			case ADC_CHANNEL_BATTERY: 
//				adcData = ADC_CONVERT_RESULT();
//				ADC_AddDataToBuff(ADC_CHANNEL_BATTERY, adcData);

//				// ADC����Ϊ��������Դʱ�������м���
//				if (BATTERY_DATA_SRC_ADC != PARAM_GetBatteryDataSrc())
//				{
//					break;
//				}
//				
//				// ������ʵʱ��ѹ
//				adcCB.battery.realtimeVoltage = adcCB.battery.adcAverage;
//				adcCB.battery.realtimeVoltage *= 21;
//				adcCB.battery.realtimeVoltage *= 3300;
//				adcCB.battery.realtimeVoltage /= 4096;
//				adcCB.battery.realtimeVoltage += 300;


//				//adcCB.battery.realtimeVoltage = adcCB.battery.adcAverage;
//				//adcCB.battery.realtimeVoltage *= paramCB.nvm.param.common.adRatioRef;
//				//adcCB.battery.realtimeVoltage /= 4096;

//				// ����ʵʱ�ٷֱ�
//				adcCB.battery.realtimePercent = ADC_GetBatteryPercent(adcCB.battery.realtimeVoltage);

//				// ��������İٷֱ���ʾƽ������
//				ADC_TortoiseProcess();
//				break;
//				
//			// �Ƿ�ͨ��
//			default:
//				break;
//		}

//		// �л�����һ��ͨ��
//		adcCB.currentChannel ++;
//		adcCB.currentChannel %= ADC_CHANNEL_MAX;

//		// ѡ����һ��ͨ��
//		ADC_CONVERT_CHANNEL(adcChannelMapping[adcCB.currentChannel]);
//		
//		// ������һ��ת��
//		ADC_CONVERT_START();
//	}
//}

//// ������ʱ�����������Եض�ȡ�¶�
//void ADC_HwStart(void)
//{
//	TIMER_KillTask(TIMER_ID_ADC_SCAN);

//	adcCB.currentChannel = ADC_CHANNEL_BATTERY;

//	// ע�ᶨʱ�����������Բ���
//	TIMER_AddTask(TIMER_ID_ADC_SCAN,
//				ADC_SCAN_TIME,
//				ADC_CALLBACK_SetScanRequest,
//				0,
//				TIMER_LOOP_FOREVER,
//				ACTION_MODE_DO_AT_ONCE);
//					
//	// ������һ��ת��
//	ADC_CONVERT_START();
//}

//// ֹͣ���
//void ADC_Stop(void)
//{
//	TIMER_KillTask(TIMER_ID_ADC_SCAN);
//}

//// Ӳ��ADC0��ʼ��
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
//    // ADC������ʱ�Ӳ��ó���64MHz
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
//    // ����ÿ���ϵ�ִ��һ��У׼
//    // Start ADC1 calibration
//    ADC_StartCalibration(ADC);
//    // Check the end of ADC1 calibration
//    while (ADC_GetCalibrationStatus(ADC));
//    
//	// ����ADCת��
//    ADC_EnableSoftwareStartConv(ADC, ENABLE);
//	
//}

//// ��ʼ���ṹ��
//void ADC_InitStr(void)
//{
//	uint8 i;
//	
//	adcCB.scanRequest = FALSE;
//	adcCB.currentChannel = (ADC_CHANNEL_E)0;

//	// ������ݽṹ��ʼ��
//	for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
//	{
//		adcCB.battery.adcBuff[i] = 0;
//	}
//	adcCB.battery.index = 0;
//	adcCB.battery.adcAverage = 0;

//	// ������ݽṹ��ʼ��
//	for(i=0; i<ADC_BLE_BUFF_SIZE; i++)
//	{
//		adcCB.ble.adcBuff[i] = 0;
//	}
//	adcCB.ble.index = 0;
//	adcCB.ble.adcAverage = 0;
//}

//// ���ö�ȡ����ص��������ɶ�ʱ������
//void ADC_CALLBACK_SetScanRequest(uint32 param)
//{
//	adcCB.scanRequest = TRUE;
//}


//// �򻺳��������һ������
//void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data)
//{
//	uint8 i;
//	uint32 total;
//	uint16 min = 0xFFFF, max = 0;
//	
//	switch(channel)
//	{
//		// ͨ��
//		case ADC_CHANNEL_BATTERY:
//			adcCB.battery.adcBuff[adcCB.battery.index++] = data;
//			adcCB.battery.index %= ADC_BATTERY_BUFF_SIZE;

//			// ���㻺������ƽ��ֵ
//			total = 0;
//			for(i=0; i<ADC_BATTERY_BUFF_SIZE; i++)
//			{
//				// ������Сֵ
//				if(adcCB.battery.adcBuff[i] < min)
//				{
//					min = adcCB.battery.adcBuff[i];
//				}

//				// �������ֵ
//				if(adcCB.battery.adcBuff[i] > max)
//				{
//					max = adcCB.battery.adcBuff[i];
//				}
//				
//				total += adcCB.battery.adcBuff[i];
//			}

//			// ȥ�������Сֵ
//			total -= min;
//			total -= max;

//			// ����ADCƽ��ֵ
//			adcCB.battery.adcAverage = total / (ADC_BATTERY_BUFF_SIZE-2);

//			break;

//		// �Ƿ�ͨ��
//		default:
//			break;
//	}
//}

//// ��ȡ��ذٷֱ�
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

//	// �ڶ�Ӧ��ѹ�ĵ�����̱��������������ذٷֱ�
//	for (i = 0; i < BATTERY_PERCENT_GATE_SIZE; i++)
//	{
//		// ����õ��İٷֱ�Ӧ�ܸ���0%-100%
//		// ��100%��ʼ����
//		if (param >= batteryPercentTable[vLevel][i])
//		{
//			if (0 == i)
//			{
//				return 100;
//			}

//			// y = kx+b�����ȼ���k
//			k = (float)((batteryPercent[i-1] - batteryPercent[i])/100.0f) / (float)(batteryPercentTable[vLevel][i-1] - batteryPercentTable[vLevel][i]);
//			b = batteryPercent[i]/100.0f - k * batteryPercentTable[vLevel][i];
//			
//			// ��������֮��Ŀ��
//			//spanVal = batteryPercentTable[vLevel][i-1] - batteryPercentTable[vLevel][i];

//			// ����ʵ�ʵİٷֵ�
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

//// ���õ�ذٷֱ�������ʾ�仯ֵ
//void ADC_TortoiseProcess(void)
//{
//	static uint16 adcScanCount = 0;		// ����Ĵ�����ADCɨ�����ʹ��ͬһ����ʱ��������ʹ�øò����Զ�ʱ�����з�Ƶ����Ƶ�����趨���˲��ȼ��й�
//	static uint16 keynoteTimeCnt = 0;
//	static BOOL keynoteFlag = FALSE;	

//	// �ڿ������һ������buf���ڶ��µ�ذٷֱȻ���
//	if (!keynoteFlag)
//	{
//		keynoteTimeCnt ++;
//		if (keynoteTimeCnt < ((ADC_BATTERY_BUFF_SIZE*ADC_SCAN_TIME*ADC_CHANNEL_MAX)/(ADC_SCAN_TIME*ADC_CHANNEL_MAX)))
//		{
//			adcCB.battery.displayPercent = ADC_GetBatteryPercent(adcCB.battery.realtimeVoltage);
//			
//			// ��ʵʱ�ĵ�ذٷֱȷ��������
//			PARAM_SetBatteryPercent(adcCB.battery.displayPercent);
//			
//			return;
//		}
//		keynoteFlag = TRUE;
//	}

//	// ����ǰ��ѹ���������
//	PARAM_SetBatteryVoltage(adcCB.battery.realtimeVoltage);

//	// ��ADCɨ�趨ʱ������Ƶ����
//	adcScanCount ++;
//	if (adcScanCount < ((uint32)PARAM_GetBatteryCapVaryTime()*1000UL/(ADC_SCAN_TIME*ADC_CHANNEL_MAX)))
//	{
//		return;
//	}
//	adcScanCount = 0;

//	// ��ʾ�ٷֱ� �� ʵʱ�ٷֱȣ�����ʾ�ٷֱ�����һ���ٷֵ�
//	if(adcCB.battery.displayPercent < adcCB.battery.realtimePercent)
//	{
//		adcCB.battery.displayPercent++;
//		if(adcCB.battery.displayPercent > 100)
//		{
//			adcCB.battery.displayPercent = 100;
//		}
//	}
//	// ��ʾ�ٷֱ� �� ʵʱ�ٷֱȣ�����ʾ�ٷֱȽ���һ���ٷֵ�
//	else if(adcCB.battery.displayPercent > adcCB.battery.realtimePercent)
//	{
//		if(adcCB.battery.displayPercent > 0)
//		{
//			adcCB.battery.displayPercent--;
//		}
//	}

//	// �������İٷֱȷ��������
//	PARAM_SetBatteryPercent(adcCB.battery.displayPercent);

//	// ��ǰ����С��10����λ�͵��־
//	PARAM_SetBatteryLowAlarm((BOOL)(PARAM_GetBatteryPercent() < 10));
//}
