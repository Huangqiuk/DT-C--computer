#include "pwmled.h"
#include "param.h"

/******************************** ȫ�ֱ���*********************************/
LED_CB ledCB;

/******************************** �����ڲ����� ****************************/
void LED_HardPwm_InitPwm(void);
void LED_SoftPwm_InitGpio(void);
void LED_CALLBACK_Trig(uint32 param);

void LED_SetRedPwm(uint32 param);
void LED_SetYellowPwm(uint32 param);
void LED_SetGreenPwm(uint32 param);

#if LED_PWM_MODE == LED_SOFT_PWM

// ���PWM����ʼ100us��ʱ��
void LED_SoftPwm_InitTimerx(uint32 mainMFreq)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	NVIC_InitType NVIC_InitStructure;

	// TIMx clock enable   
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);
	
	// ��ʱ��TIMx��ʼ��
	TIM_DeInit(TIM3);						// ��λ��ʱ��x
	TIM_TimeBaseStructure.Period = 100-1;					// ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.Prescaler = 48-1;				// ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.ClkDiv = 0; 					// ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP; 	// TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.RepetCnt = 0;					// �ظ�����Ϊ0
	TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);		// ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ	

	TIM_ClearFlag(TIM3, TIM_FLAG_UPDATE);					// �������жϱ�־
	// TIM2 enable update irq   
	TIM_ConfigInt(TIM3, TIM_INT_UPDATE, ENABLE);

	// �ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 		// TIMx�����ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		// ��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				// �����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				// IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);

	TIM_Enable(TIM3, ENABLE);
	
}

// 100us��ʱ���жϴ�����
void TIM3_IRQHandler(void)
{
	uint8 i;
	static uint32 pwmCnt = 0;		// �жϺ���ִ�д�����ʱ��
	LED_TYPE_QUEUE* pCB = NULL;
	
	if (TIM_GetIntStatus(TIM3, TIM_INT_UPDATE) != RESET)    
	{
		// ���TIMx�����жϱ�־  
		TIM_ClrIntPendingBit(TIM3, TIM_INT_UPDATE);
		
		// LED ��������ִ�м�����Ϊ10ms
		// LED ��������ִ�м������Ϊ10ms������
		if (pwmCnt >= (LED_COUNT_TIME * 10ul))
		{			
			// ��ʱ���ص�����
			LED_CALLBACK_Trig(TRUE);

			// �жϺ���ִ�д�����ʱ��
			pwmCnt = 0;
		}

		// LED ��� PWM
		for (i = 0; i < LED_NAME_MAX; i++)
		{
			pCB = (&ledCB.typeQueue[i]);

			// ��controlPwmOut��Ϊ100�����嵥λ�����������Ӳ�����
			if (pwmCnt < pCB->controlPwmOut)
			{
				// ����ߵ�ƽ
				LED_ACTION(pCB->LED_GPIO_x, pCB->LED_PIN_x, 1);
			}
			else
			{
				// ����͵�ƽ
				LED_ACTION(pCB->LED_GPIO_x, pCB->LED_PIN_x, 0);
			}
		}
		
		// �жϺ���ִ�д�������
		pwmCnt++;		
	}
}
// LEDӲ����ʼ��
void GPIO_LEDInit(void)
{

	GPIO_InitType GPIO_InitStructure;

	/**ʱ�ӳ�ʼ��**/
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_GPIOA, ENABLE);	

	/**LED�ܽų�ʼ��**/
	GPIO_InitStruct(&GPIO_InitStructure);

	GPIO_InitStructure.Pin            = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_7;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
	GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
	GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Out_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
		
}

// ���PWM����ʼGPIO��
void LED_SoftPwm_InitGpio(void)
{

	// ORANGE
	ledCB.typeQueue[LED_NAME_ORANGE_1].LED_GPIO_x = LED_GPIOA;
	ledCB.typeQueue[LED_NAME_ORANGE_2].LED_GPIO_x = LED_GPIOA;	
	ledCB.typeQueue[LED_NAME_ORANGE_3].LED_GPIO_x = LED_GPIOB;
	ledCB.typeQueue[LED_NAME_ORANGE_4].LED_GPIO_x = LED_GPIOB;
	ledCB.typeQueue[LED_NAME_ORANGE_5].LED_GPIO_x = LED_GPIOA;

	ledCB.typeQueue[LED_NAME_ORANGE_1].LED_PIN_x = LED_ORANGE_1_PIN;
	ledCB.typeQueue[LED_NAME_ORANGE_2].LED_PIN_x = LED_ORANGE_2_PIN;	
	ledCB.typeQueue[LED_NAME_ORANGE_3].LED_PIN_x = LED_ORANGE_3_PIN;
	ledCB.typeQueue[LED_NAME_ORANGE_4].LED_PIN_x = LED_ORANGE_4_PIN;	
	ledCB.typeQueue[LED_NAME_ORANGE_5].LED_PIN_x = LED_ORANGE_5_PIN;

	// BLUE
	ledCB.typeQueue[LED_NAME_BLUE_1].LED_GPIO_x = LED_GPIOB;
	ledCB.typeQueue[LED_NAME_BLUE_2].LED_GPIO_x = LED_GPIOB;
	ledCB.typeQueue[LED_NAME_BLUE_3].LED_GPIO_x = LED_GPIOB;	
	ledCB.typeQueue[LED_NAME_BLUE_4].LED_GPIO_x = LED_GPIOA;
	ledCB.typeQueue[LED_NAME_BLUE_5].LED_GPIO_x = LED_GPIOA;
	
	ledCB.typeQueue[LED_NAME_BLUE_1].LED_PIN_x = LED_BLUE_1_PIN;
	ledCB.typeQueue[LED_NAME_BLUE_2].LED_PIN_x = LED_BLUE_2_PIN;	
	ledCB.typeQueue[LED_NAME_BLUE_3].LED_PIN_x = LED_BLUE_3_PIN;
	ledCB.typeQueue[LED_NAME_BLUE_4].LED_PIN_x = LED_BLUE_4_PIN;	
	ledCB.typeQueue[LED_NAME_BLUE_5].LED_PIN_x = LED_BLUE_5_PIN;
	
}

 
#else
// Ӳ��PWM����ʼPWM�ײ�


#endif

// LEDģ���ʼ��
void LED_Inits(uint32 mainFre)
{
// -----------------------------���PWM��ʼ��-----------------------------
#if LED_PWM_MODE == LED_SOFT_PWM
	
	// LED ���ƶ�ʱ����ʼ��
	LED_SoftPwm_InitTimerx(mainFre);
	
	// LEDӲ����ʼ��
	GPIO_LEDInit();
	
	// LED ���pwmIO ��ʼ��
	LED_SoftPwm_InitGpio();

#else

	
#endif
}

// LED��ʱ�ص�������ʵʱ����PWM����ٷֱ�
void LED_CALLBACK_Trig(uint32 param)
{
	LED_ACTION_E action;
	uint8 breathControlPwm;
	uint8 i;

	// ������������LED
	for (i = 0; i < LED_NAME_MAX; i++)
	{
		// ����Ϊ0˵���Ѿ�ִ���궯�������洦�ڹر�״̬����˴���ʱ�������ʼ
		if (0 == ledCB.typeQueue[i].loopCount)
		{
			ledCB.typeQueue[i].ledCounter = ledCB.typeQueue[i].onCycleCountTimesSetValue;
		
			// ������ʽ���ָ���һ�β���
//			switch (paramCB.runtime.ledParam[i].modeBuf)
//			{
//				case PARAM_LED_MODE_ALL_OFF:
//					LED_SET((LED_NAME)i, 100, 0, 100, 0, 0, 0);
//					break;

//				case PARAM_LED_MODE_ALL_ON:
//					LED_SET((LED_NAME)i, 100, 100, paramCB.runtime.ledParam[i].pwmBuf, 0, 0, 0);
//					break;
//					
//				case PARAM_LED_MODE_BLINK:
//					LED_SET((LED_NAME)i, (uint16)paramCB.runtime.ledParam[i].cycleBuf * 100ul, 50, paramCB.runtime.ledParam[i].pwmBuf, 0, 0, 0);
//					break;

//				case PARAM_LED_MODE_BREATH:
//					LED_SET((LED_NAME)i, (uint16)paramCB.runtime.ledParam[i].cycleBuf * 100ul, 50, paramCB.runtime.ledParam[i].pwmBuf, 0, 100, 100);
//					break;
//					
//				case PARAM_LED_MODE_ALL_ON_50:
					
					//LED_SET((LED_NAME)i, 100, 100, paramCB.runtime.ledParam[i].pwmBuf, 0, 0, 0);	
//					break;
//				
//				default:
//					break;
//			}
		}
		
		// LED����ֵ����
		ledCB.typeQueue[i].ledCounter ++;
		ledCB.typeQueue[i].ledCounter %= ledCB.typeQueue[i].cycleCountTimesSetValue;

		// һ����������ѭ��������һ
		if (((0 < ledCB.typeQueue[i].loopCount)) && (ledCB.typeQueue[i].ledCounter == (ledCB.typeQueue[i].cycleCountTimesSetValue - 1)))
		{
			ledCB.typeQueue[i].loopCount --;
		}

		// �����趨�� �� ����ʱ�����ֵ�� �� ����ʱ�����ֵ�����õ�ǰ״̬Ϊ��/��
		if (ledCB.typeQueue[i].ledCounter < ledCB.typeQueue[i].onCycleCountTimesSetValue)
		{
			action = LED_ACTION_ON;

			// ������ı�������
			ledCB.typeQueue[i].breathDownCounter = 0;
			
			// Ϊ0˵������Ҫ������ֱ������ߵ�ƽ
			if (0 == ledCB.typeQueue[i].breathUpCountTimesSetValue)
			{
				breathControlPwm = 100;
			}
			else
			{
				// ��Ҫ�������𽥱���
				if (ledCB.typeQueue[i].breathUpCounter < ledCB.typeQueue[i].breathUpCountTimesSetValue)
				{
					ledCB.typeQueue[i].breathUpCounter ++;
				}
				breathControlPwm = ledCB.typeQueue[i].breathUpCounter * 100 / ledCB.typeQueue[i].breathUpCountTimesSetValue;
			}
		}
		else
		{
			action = LED_ACTION_OFF;

			// ������ı�������
			ledCB.typeQueue[i].breathUpCounter = 0;
			
			// Ϊ0˵������Ҫ������ֱ������͵�ƽ
			if (0 == ledCB.typeQueue[i].breathDownCountTimesSetValue)
			{
				breathControlPwm = 0;	
			}
			else
			{
				// ��Ҫ�������𽥱䰵
				if (ledCB.typeQueue[i].breathDownCounter < ledCB.typeQueue[i].breathDownCountTimesSetValue)
				{			
					ledCB.typeQueue[i].breathDownCounter ++;
				}
				
				breathControlPwm = (ledCB.typeQueue[i].breathDownCountTimesSetValue - ledCB.typeQueue[i].breathDownCounter) * 100 / ledCB.typeQueue[i].breathDownCountTimesSetValue;
			}
		}
		
		ledCB.typeQueue[i].lastAction = ledCB.typeQueue[i].action;
		ledCB.typeQueue[i].action = action;	

		// �������PWM���ֵ
		ledCB.typeQueue[i].controlPwmOut = ledCB.typeQueue[i].lowLimitPwmOut + breathControlPwm * (ledCB.typeQueue[i].highLimitPwmOut - ledCB.typeQueue[i].lowLimitPwmOut)/100;

#if LED_PWM_MODE == LED_SOFT_PWM
		// ���PWM�У�LED������PWMר�ö�ʱ���ж���ִ��
#else
		// PWM�������LED
		if (NULL != ledCB.typeQueue[i].fun)
		{
			ledCB.typeQueue[i].fun(ledCB.typeQueue[i].controlPwmOut);
		}	
#endif
	}
}

/*********************************************************************
* ��   ��  ��: LED_Set
* �������: LED_NAME ledName					// LED������
			uint32 ledCycleTime					// LED��������ʱ�䣬��λms����СֵΪ100
			uint8 ledOnCycleTimeDutyRatio		// LED����ռ�ձȣ�0-100��Ĭ��100
			uint8 onBrightnessRatio, 			// LED��ʱ���ȱȣ�0-100��Ĭ��100
			uint8 offBrightnessRatio, 			// LED��ʱ���ȱȣ�0-100��Ĭ��0
			uint8 ledOnAscendRatio,  			// LED�������ٶ�ռ�ȣ�0-100������Ч��ʹ�ã��Ǻ���Ĭ��Ϊ0
			uint8 ledOnDescendRatio				// LED���½��ٶ�ռ�ȣ�0-100������Ч��ʹ�ã��Ǻ���Ĭ��Ϊ0
			int32 actionTimes					// LEDִ�д���
* ��������: void
* ��   ��  ֵ: void 
* ��������:  LEDģʽ����
**********************************************************************/
void LED_Set(LED_NAME ledName, uint32 ledCycleTime, uint8 ledOnCycleTimeDutyRatio, uint8 onBrightnessRatio, uint8 offBrightnessRatio, uint8 ledOnAscendRatio, uint8 ledOnDescendRatio, int32 actionTimes)
{
	LED_TYPE_QUEUE* pCB = (&ledCB.typeQueue[ledName]);

	if (NULL == pCB)
	{
		return;
	}

	// ����������СΪ100ms
	if (ledCycleTime <= 100)
	{
		ledCycleTime = 100;
	}
	
	// ռ�ձ����ֻ��100%
	if (ledOnCycleTimeDutyRatio > LED_PWM_OUT_MAX)
	{
		ledOnCycleTimeDutyRatio = LED_PWM_OUT_MAX;
	}

	// ����������ֻ��100%
	if (onBrightnessRatio > 100)
	{
		onBrightnessRatio = 100;
	}

	// ��󰵶����ֻ��Ϊ100%
	if (offBrightnessRatio > 100)
	{
		offBrightnessRatio = 100;
	}

	// �����������仯������ֻ����100%
	if (ledOnAscendRatio > 100)
	{
		ledOnAscendRatio = 100;
	}

	// ���Ƚ������仯������ֻ����100%
	if (ledOnDescendRatio > 100)
	{
		ledOnDescendRatio = 100;
	}
	
	// �������
	pCB->ledCounter = 0;
	pCB->breathUpCounter = 0;
	pCB->breathDownCounter = 0;
	
	// ��������
	pCB->action = LED_ACTION_OFF;
	pCB->lastAction = LED_ACTION_OFF;

	// ����LED���ڣ�ʱ�����ֵ
	pCB->cycleCountTimesSetValue = ledCycleTime / LED_COUNT_TIME;

	// LED����ռ�ձȼ�����ʱ�����
	pCB->onCycleCountTimesSetValue = pCB->cycleCountTimesSetValue * ledOnCycleTimeDutyRatio / LED_PWM_OUT_MAX;	

	// LED��ʱ���ȱ�
	pCB->highLimitPwmOut = onBrightnessRatio;
	// LED��ʱ���ȱ�
	pCB->lowLimitPwmOut = offBrightnessRatio;

	// LED ������ֵ
	pCB->breathUpCountTimesSetValue = pCB->onCycleCountTimesSetValue * ledOnAscendRatio / LED_PWM_OUT_MAX;
	// LED �����ֵ
	pCB->breathDownCountTimesSetValue = (pCB->cycleCountTimesSetValue - pCB->onCycleCountTimesSetValue) * ledOnDescendRatio / LED_PWM_OUT_MAX;

	// ѭ����������
	pCB->loopCount = actionTimes;
}

// �ر�����LED���
void LED_CloseAllLeds(void)
{

#if LED_PWM_MODE == LED_HARD_PWM

#endif
}

#if LED_PWM_MODE == LED_HARD_PWM
// ��ƿ���
void LED_SetRedPwm(uint32 param)
{
    LED_RED_PWM(param);
}

// �Ƶƿ���
void LED_SetYellowPwm(uint32 param)
{ 	
  	LED_YELLOW_PWM(param);
}

// �̵ƿ���
void LED_SetGreenPwm(uint32 param)
{
	LED_GREEN_PWM(param);
}
#endif





















