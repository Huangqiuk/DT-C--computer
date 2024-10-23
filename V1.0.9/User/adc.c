#include "adc.h"
#include "timer.h"
#include "param.h"

ADC_CB adcCB;

//uint8 adc_channel[8] ={6, 7, 14, 15, 8, 10, 11, 12 };
uint8 adc_channel[8] ={6, 7, 14, 15, 8, 12 };

#define level0_PERCENT_GATE_SIZE		(sizeof(level0PercentTable[0])/sizeof(level0PercentTable[0][0]))

//*********************************/
// �ڲ���������
//*********************************/
// Ӳ��ADC0��ʼ��
void ADC_LevelHwInit(void);

// ��ʼ���ṹ��
void ADC_InitStruct(void);

// �򻺳��������һ������
void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data);

// ���ö�ȡ����ص��������ɶ�ʱ������
void ADC_CALLBACK_SetScanRequest(uint32 param);

// ������ʱ�����������Եض�ȡ
void ADC_Start(void);

// ֹͣ���
void ADC_Stop(void);

// ���õ��������ʾ�仯ֵ
void ADC_TortoiseProcess(void);

// �����ڹ��㷨
BOOL ADC_DebounceTortoiseProcess(void);

// ��ȡ��ذٷֱ�
uint8 ADC_Getlevel0Percent(uint32 param);

//*********************************/
// ��������
//*********************************/
// ��ʼ��
void ADC_BaseInit(void)
{
	// ADCӲ����ʼ��
	ADC_LevelHwInit();

	// ��ʼ���ṹ��
	ADC_InitStruct();

	// ��ʼɨ��
	ADC_Start();
}

// ������������main�е���
void ADC_Process(void)
{
	uint16 adcData;
	uint8 i;
	uint8 vLevel = 1;
	
	// �м������ʱ��ִ��һ��ת��
	if(adcCB.scanRequest)
	{
		// �ϴ�ת��δ��ɣ��˳����ȴ�ת�����
		if(!ADC_CONVERT_IS_BUSY())
		{
			return;
		}
		
		// �ϴ�ת�����������ת������
		adcCB.scanRequest = FALSE;

		// ��������ӵ���Ӧ�Ļ�������
		switch(adcCB.currentChannel)
		{
			// LEVEL_IN0
			case ADC_CHANNEL_LEVEL_IN0:		

				adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN0, adcData);	
				
                // �����ѹ
                adcCB.level0.voltage = adcCB.level0.adcAverage * 3300 / 4095;
                
                adcCB.level0.voltage = adcCB.level0.voltage  * 11;             
				break;

            // LEVEL_IN2
            case ADC_CHANNEL_LEVEL_IN1:
                adcData = ADC_CONVERT_RESULT();
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN1, adcData);	
                
                // �����ѹ
                adcCB.level1.voltage = adcCB.level1.adcAverage * 3300 / 4095;
                
                adcCB.level1.voltage = adcCB.level1.voltage * 11;               
                break;
                
            // LEVEL_IN2
            case ADC_CHANNEL_LEVEL_IN2:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN2, adcData);	
                
                // �����ѹ
                adcCB.level2.voltage = adcCB.level2.adcAverage * 3300 / 4095;
                
                adcCB.level2.voltage = adcCB.level2.voltage * 11;  
                break;

            // LEVEL_IN3
            case ADC_CHANNEL_LEVEL_IN3:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN3, adcData);	
                
                // �����ѹ
                adcCB.level3.voltage = adcCB.level3.adcAverage * 3300 / 4095;
                
                adcCB.level3.voltage = adcCB.level3.voltage * 11;                  
                break;
                
            // LEVEL_IN4
            case ADC_CHANNEL_LEVEL_IN4:
                adcData = ADC_CONVERT_RESULT();
                
				ADC_AddDataToBuff(ADC_CHANNEL_LEVEL_IN4, adcData);	
                
                // �����ѹ
                adcCB.level4.voltage = adcCB.level4.adcAverage * 3300 / 4095;
                
                adcCB.level4.voltage = adcCB.level4.voltage * 11;                  
                break;

//            // ADC_CHANNEL_DUT_USB_ADC
//            case ADC_CHANNEL_DUT_USB_ADC:
//                adcData = ADC_CONVERT_RESULT();
//                
//				ADC_AddDataToBuff(ADC_CHANNEL_DUT_USB_ADC, adcData);	
//                
//                // �����ѹ
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
//                // �����ѹ
//                adcCB.usbcurrent.voltage = adcCB.usbcurrent.adcAverage * 3300 / 4095;  // ��λ�������Ǿ����Ŵ�100����
//                
////                adcCB.usbcurrent.voltage = adcCB.usbcurrent.voltage / 50 / 20 ;                  
//                break;

//            // ADC_CHANNEL_KEY_ADC
//            case ADC_CHANNEL_KEY_ADC:
//                adcData = ADC_CONVERT_RESULT();
//                
//				ADC_AddDataToBuff(ADC_CHANNEL_KEY_ADC, adcData);	
//                
//                // �����ѹ
//                adcCB.key.voltage = adcCB.key.adcAverage * 3300 / 4095;
//                
//                adcCB.key.voltage = adcCB.key.voltage;                  
//                break;
                
            // �Ƿ�ͨ��
            default:
            
                break;
				
		}

		// �л�����һ��ͨ��
		adcCB.currentChannel ++;
		adcCB.currentChannel %= ADC_CHANNEL_MAX;

		// ѡ����һ��ͨ��
		ADC_CONVERT_CHANNEL(adc_channel[adcCB.currentChannel]);
		
		// ������һ��ת��
		ADC_CONVERT_START();
	}
}

// ����NTC����������ʱ�����������Եض�ȡ�¶�
void ADC_Start(void)
{
	TIMER_KillTask(TIMER_ID_ADC_SCAN);

	adcCB.currentChannel = ADC_CHANNEL_LEVEL_IN0;

	// ע�ᶨʱ�����������Բ���
	TIMER_AddTask(TIMER_ID_ADC_SCAN,
				ADC_SCAN_TIME,
				ADC_CALLBACK_SetScanRequest,
				1,
				TIMER_LOOP_FOREVER,
				ACTION_MODE_DO_AT_ONCE);
					
	// ������һ��ת��
	ADC_CONVERT_START();
}

// ֹͣ���
void ADC_Stop(void)
{
	TIMER_KillTask(TIMER_ID_ADC_SCAN);
}

// Ӳ��ADC0��ʼ��
void ADC_LevelHwInit(void)
{
 	rcu_periph_clock_enable(RCU_GPIOA);									// ʱ��GPIOCʱ��
    rcu_periph_clock_enable(RCU_GPIOB);									// ʱ��GPIOCʱ��
 	rcu_periph_clock_enable(RCU_GPIOC);									// ʱ��GPIOCʱ��
	rcu_periph_clock_enable(RCU_ADC0);									// ʱ��ADC0ʱ��

	adc_deinit(ADC0);													// ��λADC
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);						// 72/6 = 12 Mhz(���Ƶ�ʲ��ܳ���40M) 
		
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);		// PA0 - ģ������
	gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		                // PA0 - ģ������    
	gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5);		// PC3 -PC4 - ģ������
	gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);		
    
	// ADC0 ����
	adc_mode_config(ADC_MODE_FREE); 									// ����ģʽ
	adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);			// ɨ��ģʽ�ر�
	adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);	// ����ת��ģʽ�ر�
	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);  // ����ͨ�����������
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);				// �����Ҷ���
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);			// ����ͨ������1

	// ����ADC
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_6, ADC_SAMPLETIME_55POINT5);	
	adc_enable(ADC0);													// ʱ��ADC0
	Delayms(10);
	adc_calibration_enable(ADC0);										// ADCУ׼�͸�λУ׼
	
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);		// ����ADC0ת��
	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

// ��ʼ���ṹ��
void ADC_InitStruct(void)
{
	uint8 i;
	
	adcCB.scanRequest = FALSE;
	adcCB.currentChannel = (ADC_CHANNEL_E)0;

	// level0������ݽṹ��ʼ��
	for(i=0; i< ADC_LEVEL_DETECTOR_BUFF_SIZE ; i++)
	{
		adcCB.level0.adcBuff[i] = 0;
	}
	
	adcCB.level0.index = 0;
	adcCB.level0.adcAverage = 0;
	adcCB.level0.voltage = 0;

	// level1�����������
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level1.adcBuff[i] = 0;
	}
	
	adcCB.level1.index = 0;
	adcCB.level1.adcAverage = 0;
	adcCB.level1.voltage = 0;
    
    // level2�����������
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level2.adcBuff[i] = 0;
	}
	
	adcCB.level2.index = 0;
	adcCB.level2.adcAverage = 0;
	adcCB.level2.voltage = 0;
    
    // level3�����������
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level3.adcBuff[i] = 0;
	}
	
	adcCB.level3.index = 0;
	adcCB.level3.adcAverage = 0;
	adcCB.level3.voltage = 0;
    
    // level4�����������
	for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
	{
		adcCB.level4.adcBuff[i] = 0;
	}
	
	adcCB.level4.index = 0;
	adcCB.level4.adcAverage = 0;
	adcCB.level4.voltage = 0;
}

// ���ö�ȡ����ص��������ɶ�ʱ������
void ADC_CALLBACK_SetScanRequest(uint32 param)
{
	adcCB.scanRequest = TRUE;
}

// �򻺳��������һ������
void ADC_AddDataToBuff(ADC_CHANNEL_E channel, uint16 data)
{
	uint8 i;
	uint32 total;
	uint16 min = 0xFFFF, max = 0;

	// �����л��е������޵����������ı�־
	static BOOL noCurrentActionFlag = TRUE;
	static BOOL currentActionFlag = TRUE;
	
	switch(channel)
	{
		// level0
		case ADC_CHANNEL_LEVEL_IN0:

			adcCB.level0.adcBuff[adcCB.level0.index++] = data;
			adcCB.level0.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;
			
			// ���㻺������ƽ��ֵ
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// ������Сֵ
				if(adcCB.level0.adcBuff[i] < min)
				{
					min = adcCB.level0.adcBuff[i];
				}

				// �������ֵ
				if(adcCB.level0.adcBuff[i] > max)
				{
					max = adcCB.level0.adcBuff[i];
				}
				
				total += adcCB.level0.adcBuff[i];
			}

			// ȥ�������Сֵ
			total -= min;
			total -= max;

			// ����ADCƽ��ֵ
			adcCB.level0.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

			break;

		// level1
		case ADC_CHANNEL_LEVEL_IN1:

			adcCB.level1.adcBuff[adcCB.level1.index++] = data;
			adcCB.level1.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// ���㻺������ƽ��ֵ
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// ������Сֵ
				if(adcCB.level1.adcBuff[i] < min)
				{
					min = adcCB.level1.adcBuff[i];
				}

				// �������ֵ
				if(adcCB.level1.adcBuff[i] > max)
				{
					max = adcCB.level1.adcBuff[i];
				}
				
				total += adcCB.level1.adcBuff[i];
			}

			// ȥ�������Сֵ
			total -= min;
			total -= max;

			// ����ADCƽ��ֵ
			adcCB.level1.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

			break;

		// LEVEL_IN2
	    case ADC_CHANNEL_LEVEL_IN2:
			adcCB.level2.adcBuff[adcCB.level2.index++] = data;
			adcCB.level2.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// ���㻺������ƽ��ֵ
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// ������Сֵ
				if(adcCB.level2.adcBuff[i] < min)
				{
					min = adcCB.level2.adcBuff[i];
				}

				// �������ֵ
				if(adcCB.level2.adcBuff[i] > max)
				{
					max = adcCB.level2.adcBuff[i];
				}
				
				total += adcCB.level2.adcBuff[i];
			}

			// ȥ�������Сֵ
			total -= min;
			total -= max;

			// ����ADCƽ��ֵ
			adcCB.level2.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

		// LEVEL_IN3
	    case ADC_CHANNEL_LEVEL_IN3:
			adcCB.level3.adcBuff[adcCB.level3.index++] = data;
			adcCB.level3.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// ���㻺������ƽ��ֵ
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// ������Сֵ
				if(adcCB.level3.adcBuff[i] < min)
				{
					min = adcCB.level3.adcBuff[i];
				}

				// �������ֵ
				if(adcCB.level3.adcBuff[i] > max)
				{
					max = adcCB.level3.adcBuff[i];
				}
				
				total += adcCB.level3.adcBuff[i];
			}

			// ȥ�������Сֵ
			total -= min;
			total -= max;

			// ����ADCƽ��ֵ
			adcCB.level3.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

		// LEVEL_IN4
	    case ADC_CHANNEL_LEVEL_IN4:
			adcCB.level4.adcBuff[adcCB.level4.index++] = data;
			adcCB.level4.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

			// ���㻺������ƽ��ֵ
			total = 0;
			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
			{
				// ������Сֵ
				if(adcCB.level4.adcBuff[i] < min)
				{
					min = adcCB.level4.adcBuff[i];
				}

				// �������ֵ
				if(adcCB.level4.adcBuff[i] > max)
				{
					max = adcCB.level4.adcBuff[i];
				}
				
				total += adcCB.level4.adcBuff[i];
			}

			// ȥ�������Сֵ
			total -= min;
			total -= max;

			// ����ADCƽ��ֵ
			adcCB.level4.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

	    	break;

//		// DUT_USB_ADC
//	    case ADC_CHANNEL_DUT_USB_ADC:
//			adcCB.dutusb.adcBuff[adcCB.dutusb.index++] = data;
//			adcCB.dutusb.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// ���㻺������ƽ��ֵ
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// ������Сֵ
//				if(adcCB.dutusb.adcBuff[i] < min)
//				{
//					min = adcCB.dutusb.adcBuff[i];
//				}

//				// �������ֵ
//				if(adcCB.dutusb.adcBuff[i] > max)
//				{
//					max = adcCB.dutusb.adcBuff[i];
//				}
//				
//				total += adcCB.dutusb.adcBuff[i];
//			}

//			// ȥ�������Сֵ
//			total -= min;
//			total -= max;

//			// ����ADCƽ��ֵ
//			adcCB.dutusb.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;

//		// USB_CURRENT
//	    case ADC_CHANNEL_USB_CURRENT:
//			adcCB.usbcurrent.adcBuff[adcCB.usbcurrent.index++] = data;
//			adcCB.usbcurrent.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// ���㻺������ƽ��ֵ
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// ������Сֵ
//				if(adcCB.usbcurrent.adcBuff[i] < min)
//				{
//					min = adcCB.usbcurrent.adcBuff[i];
//				}

//				// �������ֵ
//				if(adcCB.usbcurrent.adcBuff[i] > max)
//				{
//					max = adcCB.usbcurrent.adcBuff[i];
//				}
//				
//				total += adcCB.usbcurrent.adcBuff[i];
//			}

//			// ȥ�������Сֵ
//			total -= min;
//			total -= max;

//			// ����ADCƽ��ֵ
//			adcCB.usbcurrent.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;

//		// KEY_ADC
//	    case ADC_CHANNEL_KEY_ADC:
//			adcCB.key.adcBuff[adcCB.key.index++] = data;
//			adcCB.key.index %= ADC_LEVEL_DETECTOR_BUFF_SIZE;

//			// ���㻺������ƽ��ֵ
//			total = 0;
//			for(i=0; i<ADC_LEVEL_DETECTOR_BUFF_SIZE; i++)
//			{
//				// ������Сֵ
//				if(adcCB.key.adcBuff[i] < min)
//				{
//					min = adcCB.key.adcBuff[i];
//				}

//				// �������ֵ
//				if(adcCB.key.adcBuff[i] > max)
//				{
//					max = adcCB.key.adcBuff[i];
//				}
//				
//				total += adcCB.key.adcBuff[i];
//			}

//			// ȥ�������Сֵ
//			total -= min;
//			total -= max;

//			// ����ADCƽ��ֵ
//			adcCB.key.adcAverage = total / (ADC_LEVEL_DETECTOR_BUFF_SIZE - 2);

//	    	break;
            
		// �Ƿ�ͨ��
		default:
			break;
	}
}
