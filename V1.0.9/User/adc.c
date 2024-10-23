#include "adc.h"
#include "timer.h"
#include "param.h"

ADC_CB adcCB;

//uint8 adc_channel[8] ={6, 7, 14, 15, 8, 10, 11, 12 };
uint8 adc_channel[8] ={6, 7, 14, 15, 8, 12 };

#define level0_PERCENT_GATE_SIZE		(sizeof(level0PercentTable[0])/sizeof(level0PercentTable[0][0]))

//*********************************/
// 内部函数申明
//*********************************/
// 硬件ADC0初始化
void ADC_LevelHwInit(void);

// 初始化结构体
void ADC_InitStruct(void);

// 向缓冲区中添加一个数据
void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data);

// 设置读取请求回调函数，由定时器调用
void ADC_CALLBACK_SetScanRequest(uint32 param);

// 启动定时器任务，周期性地读取
void ADC_Start(void);

// 停止检测
void ADC_Stop(void);

// 设置电池连续显示变化值
void ADC_TortoiseProcess(void);

// 消抖乌龟算法
BOOL ADC_DebounceTortoiseProcess(void);

// 获取电池百分比
uint8 ADC_Getlevel0Percent(uint32 param);

//*********************************/
// 函数主体
//*********************************/
// 初始化
void ADC_BaseInit(void)
{
	// ADC硬件初始化
	ADC_LevelHwInit();

	// 初始化结构体
	ADC_InitStruct();

	// 开始扫描
	ADC_Start();
}

// 传感器处理，在main中调用
void ADC_Process(void)
{
	uint16 adcData;
	uint8 i;
	uint8 vLevel = 1;
	
	// 有检测请求时，执行一次转换
	if(adcCB.scanRequest)
	{
		// 上次转换未完成，退出，等待转换完成
		if(!ADC_CONVERT_IS_BUSY())
		{
			return;
		}
		
		// 上次转换结束，清除转换请求
		adcCB.scanRequest = FALSE;

		// 将数据添加到对应的缓冲区中
		switch(adcCB.currentChannel)
		{
			// LEVEL_IN0
			case ADC_CHANNEL_LEVEL_IN0:		

				adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN0, adcData);	
				
                // 计算电压
                adcCB.level0.voltage = adcCB.level0.adcAverage * 3300 / 4095;
                
                adcCB.level0.voltage = adcCB.level0.voltage  * 11;             
				break;

            // LEVEL_IN2
            case ADC_CHANNEL_LEVEL_IN1:
                adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN1, adcData);	
                
                // 计算电压
                adcCB.level1.voltage = adcCB.level1.adcAverage * 3300 / 4095;
                
                adcCB.level1.voltage = adcCB.level1.voltage * 11;               
                break;
                
            // LEVEL_IN2
            case ADC_CHANNEL_LEVEL_IN2:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN2, adcData);	
                
                // 计算电压
                adcCB.level2.voltage = adcCB.level2.adcAverage * 3300 / 4095;
                
                adcCB.level2.voltage = adcCB.level2.voltage * 11;  
                break;

            // LEVEL_IN3
            case ADC_CHANNEL_LEVEL_IN3:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN3, adcData);	
                
                // 计算电压
                adcCB.level3.voltage = adcCB.level3.adcAverage * 3300 / 4095;
                
                adcCB.level3.voltage = adcCB.level3.voltage * 11;                  
                break;
                
            // LEVEL_IN4
            case ADC_CHANNEL_LEVEL_IN4:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN4, adcData);	
                
                // 计算电压
                adcCB.level4.voltage = adcCB.level4.adcAverage * 3300 / 4095;
                
                adcCB.level4.voltage = adcCB.level4.voltage * 11;                  
                break;

//            // ADC_CHANNEL_DUT_USB_ADC
//            case ADC_CHANNEL_DUT_USB_ADC:
//                adcData = ADC_CONVERT_RESULT();
//                
//				ADC_AddDataToBuff(ADC_CHANNEL_DUT_USB_ADC, adcData);	
//                
//                // 计算电压
//                adcCB.dutusb.voltage = adcCB.dutusb.adcAverage * 3300 / 4095;
//                
//                adcCB.dutusb.voltage = adcCB.dutusb.voltage * 2;                  
//                break;

//            // ADC_CHANNEL_USB_CURRENT
//            case ADC_CHANNEL_USB_CURRENT:
//                adcData = ADC_CONVERT_RESULT();
//                
//				ADC_AddDataToBuff(ADC_CHANNEL_USB_CURRENT, adcData);	
//                
//                // 计算电压
//                adcCB.usbcurrent.voltage = adcCB.usbcurrent.adcAverage * 3300 / 4095;  // 单位毫伏，是经过放大100倍的
//                
////                adcCB.usbcurrent.voltage = adcCB.usbcurrent.voltage / 50 / 20 ;                  
//                break;

//            // ADC_CHANNEL_KEY_ADC
//            case ADC_CHANNEL_KEY_ADC:
//                adcData = ADC_CONVERT_RESULT();
//                
//				ADC_AddDataToBuff(ADC_CHANNEL_KEY_ADC, adcData);	
//                
//                // 计算电压
//                adcCB.key.voltage = adcCB.key.adcAverage * 3300 / 4095;
//                
//                adcCB.key.voltage = adcCB.key.voltage;                  
//                break;
                
            // 非法通道
            default:
            
                break;
				
		}

		// 切换至下一个通道
		adcCB.currentChannel ++;
		adcCB.currentChannel %= ADC_CHANNEL_MAX;

		// 选定下一个通道
		ADC_CONVERT_CHANNEL(adc_channel[adcCB.currentChannel]);
		
		// 启动下一次转换
		ADC_CONVERT_START();
	}
}

// 启动NTC，即启动定时器任务，周期性地读取温度
void ADC_Start(void)
{
	TIMER_KillTask(TIMER_ID_ADC_SCAN);

	adcCB.currentChannel = ADC_CHANNEL_LEVEL_IN0;

	// 注册定时器任务，周期性采样
	TIMER_AddTask(TIMER_ID_ADC_SCAN,
				ADC_SCAN_TIME,
				ADC_CALLBACK_SetScanRequest,
				1,
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
void ADC_LevelHwInit(void)
{
 	rcu_periph_clock_enable(RCU_GPIOA);									// 时能GPIOC时钟
    rcu_periph_clock_enable(RCU_GPIOB);									// 时能GPIOC时钟
 	rcu_periph_clock_enable(RCU_GPIOC);									// 时能GPIOC时钟
	rcu_periph_clock_enable(RCU_ADC0);									// 时能ADC0时钟

	adc_deinit(ADC0);													// 复位ADC
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);						// 72/6 = 12 Mhz(最大频率不能超过40M) 
		
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);		// PA0 - 模拟输入
	gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		                // PA0 - 模拟输入    
	gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5);		// PC3 -PC4 - 模拟输入
	gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);		
    
	// ADC0 配置
	adc_mode_config(ADC_MODE_FREE); 									// 独立模式
	adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);			// 扫描模式关闭
	adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);	// 连续转换模式关闭
	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);  // 常规通道，软件触发
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);				// 数据右对齐
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);			// 常规通道数量1

	// 设置ADC
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_6, ADC_SAMPLETIME_55POINT5);	
	adc_enable(ADC0);													// 时能ADC0
	Delayms(10);
	adc_calibration_enable(ADC0);										// ADC校准和复位校准
	
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);		// 启动ADC0转换
	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

// 初始化结构体
void ADC_InitStruct(void)
{
	uint8 i;
	
	adcCB.scanRequest = FALSE;
	adcCB.currentChannel = (ADC_CHANNEL_E)0;

	// level0相关数据结构初始化
	for(i=0; i< ADC_LEVEL_DETECTOR_BUFF_SIZE ; i++)
	{
		adcCB.level0.adcBuff[i] = 0;
	}
	
	adcCB.level0.index = 0;
	adcCB.level0.adcAverage = 0;
	adcCB.level0.voltage = 0;

	// level1相关数据清零
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level1.adcBuff[i] = 0;
	}
	
	adcCB.level1.index = 0;
	adcCB.level1.adcAverage = 0;
	adcCB.level1.voltage = 0;
    
    // level2相关数据清零
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level2.adcBuff[i] = 0;
	}
	
	adcCB.level2.index = 0;
	adcCB.level2.adcAverage = 0;
	adcCB.level2.voltage = 0;
    
    // level3相关数据清零
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level3.adcBuff[i] = 0;
	}
	
	adcCB.level3.index = 0;
	adcCB.level3.adcAverage = 0;
	adcCB.level3.voltage = 0;
    
    // level4相关数据清零
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level4.adcBuff[i] = 0;
	}
	
	adcCB.level4.index = 0;
	adcCB.level4.adcAverage = 0;
	adcCB.level4.voltage = 0;
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

	// 用来切换有电流和无电流数据填充的标志
	static BOOL noCurrentActionFlag = TRUE;
	static BOOL currentActionFlag = TRUE;
	
	switch(channel)
	{
		// level0
		case ADC_CHANNEL_LEVEL_IN0:

			adcCB.level0.adcBuff[adcCB.level0.index++] = data;
			adcCB.level0.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;
			
			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.level0.adcBuff[i] < min)
				{
					min = adcCB.level0.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.level0.adcBuff[i] > max)
				{
					max = adcCB.level0.adcBuff[i];
				}
				
				total += adcCB.level0.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.level0.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

			break;

		// level1
		case ADC_CHANNEL_LEVEL_IN1:

			adcCB.level1.adcBuff[adcCB.level1.index++] = data;
			adcCB.level1.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.level1.adcBuff[i] < min)
				{
					min = adcCB.level1.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.level1.adcBuff[i] > max)
				{
					max = adcCB.level1.adcBuff[i];
				}
				
				total += adcCB.level1.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.level1.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

			break;

		// LEVEL_IN2
	    case ADC_CHANNEL_LEVEL_IN2:
			adcCB.level2.adcBuff[adcCB.level2.index++] = data;
			adcCB.level2.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.level2.adcBuff[i] < min)
				{
					min = adcCB.level2.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.level2.adcBuff[i] > max)
				{
					max = adcCB.level2.adcBuff[i];
				}
				
				total += adcCB.level2.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.level2.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

		// LEVEL_IN3
	    case ADC_CHANNEL_LEVEL_IN3:
			adcCB.level3.adcBuff[adcCB.level3.index++] = data;
			adcCB.level3.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.level3.adcBuff[i] < min)
				{
					min = adcCB.level3.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.level3.adcBuff[i] > max)
				{
					max = adcCB.level3.adcBuff[i];
				}
				
				total += adcCB.level3.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.level3.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

		// LEVEL_IN4
	    case ADC_CHANNEL_LEVEL_IN4:
			adcCB.level4.adcBuff[adcCB.level4.index++] = data;
			adcCB.level4.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// 计算缓冲区的平均值
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// 更新最小值
				if(adcCB.level4.adcBuff[i] < min)
				{
					min = adcCB.level4.adcBuff[i];
				}

				// 更新最大值
				if(adcCB.level4.adcBuff[i] > max)
				{
					max = adcCB.level4.adcBuff[i];
				}
				
				total += adcCB.level4.adcBuff[i];
			}

			// 去掉最大最小值
			total -= min;
			total -= max;

			// 计算ADC平均值
			adcCB.level4.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

//		// DUT_USB_ADC
//	    case ADC_CHANNEL_DUT_USB_ADC:
//			adcCB.dutusb.adcBuff[adcCB.dutusb.index++] = data;
//			adcCB.dutusb.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// 计算缓冲区的平均值
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// 更新最小值
//				if(adcCB.dutusb.adcBuff[i] < min)
//				{
//					min = adcCB.dutusb.adcBuff[i];
//				}

//				// 更新最大值
//				if(adcCB.dutusb.adcBuff[i] > max)
//				{
//					max = adcCB.dutusb.adcBuff[i];
//				}
//				
//				total += adcCB.dutusb.adcBuff[i];
//			}

//			// 去掉最大最小值
//			total -= min;
//			total -= max;

//			// 计算ADC平均值
//			adcCB.dutusb.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;

//		// USB_CURRENT
//	    case ADC_CHANNEL_USB_CURRENT:
//			adcCB.usbcurrent.adcBuff[adcCB.usbcurrent.index++] = data;
//			adcCB.usbcurrent.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// 计算缓冲区的平均值
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// 更新最小值
//				if(adcCB.usbcurrent.adcBuff[i] < min)
//				{
//					min = adcCB.usbcurrent.adcBuff[i];
//				}

//				// 更新最大值
//				if(adcCB.usbcurrent.adcBuff[i] > max)
//				{
//					max = adcCB.usbcurrent.adcBuff[i];
//				}
//				
//				total += adcCB.usbcurrent.adcBuff[i];
//			}

//			// 去掉最大最小值
//			total -= min;
//			total -= max;

//			// 计算ADC平均值
//			adcCB.usbcurrent.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;

//		// KEY_ADC
//	    case ADC_CHANNEL_KEY_ADC:
//			adcCB.key.adcBuff[adcCB.key.index++] = data;
//			adcCB.key.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// 计算缓冲区的平均值
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// 更新最小值
//				if(adcCB.key.adcBuff[i] < min)
//				{
//					min = adcCB.key.adcBuff[i];
//				}

//				// 更新最大值
//				if(adcCB.key.adcBuff[i] > max)
//				{
//					max = adcCB.key.adcBuff[i];
//				}
//				
//				total += adcCB.key.adcBuff[i];
//			}

//			// 去掉最大最小值
//			total -= min;
//			total -= max;

//			// 计算ADC平均值
//			adcCB.key.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;
            
		// 非法通道
		default:
			break;
	}
}
