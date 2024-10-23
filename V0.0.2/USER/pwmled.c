#include "pwmled.h"
#include "param.h"

/******************************** 全局变量*********************************/
LED_CB ledCB;

/******************************** 函数内部声明 ****************************/
void LED_HardPwm_InitPwm(void);
void LED_SoftPwm_InitGpio(void);
void LED_CALLBACK_Trig(uint32 param);

void LED_SetRedPwm(uint32 param);
void LED_SetYellowPwm(uint32 param);
void LED_SetGreenPwm(uint32 param);

#if LED_PWM_MODE == LED_SOFT_PWM

// 软件PWM，初始100us定时器
void LED_SoftPwm_InitTimerx(uint32 mainMFreq)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	NVIC_InitType NVIC_InitStructure;

	// TIMx clock enable   
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);
	
	// 定时器TIMx初始化
	TIM_DeInit(TIM3);						// 复位定时器x
	TIM_TimeBaseStructure.Period = 100-1;					// 设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.Prescaler = 48-1;				// 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.ClkDiv = 0; 					// 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP; 	// TIM向上计数模式
	TIM_TimeBaseStructure.RepetCnt = 0;					// 重复次数为0
	TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);		// 根据指定的参数初始化TIMx的时间基数单位	

	TIM_ClearFlag(TIM3, TIM_FLAG_UPDATE);					// 清除溢出中断标志
	// TIM2 enable update irq   
	TIM_ConfigInt(TIM3, TIM_INT_UPDATE, ENABLE);

	// 中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 		// TIMx更新中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		// 先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				// 从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				// IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);

	TIM_Enable(TIM3, ENABLE);
	
}

// 100us定时器中断处理函数
void TIM3_IRQHandler(void)
{
	uint8 i;
	static uint32 pwmCnt = 0;		// 中断函数执行次数计时器
	LED_TYPE_QUEUE* pCB = NULL;
	
	if (TIM_GetIntStatus(TIM3, TIM_INT_UPDATE) != RESET)    
	{
		// 清除TIMx更新中断标志  
		TIM_ClrIntPendingBit(TIM3, TIM_INT_UPDATE);
		
		// LED 触发函数执行间隔最低为10ms
		// LED 触发函数执行间隔必须为10ms整数倍
		if (pwmCnt >= (LED_COUNT_TIME * 10ul))
		{			
			// 定时器回调函数
			LED_CALLBACK_Trig(TRUE);

			// 中断函数执行次数计时器
			pwmCnt = 0;
		}

		// LED 输出 PWM
		for (i = 0; i < LED_NAME_MAX; i++)
		{
			pCB = (&ledCB.typeQueue[i]);

			// 将controlPwmOut认为100个脉冲单位，根据其调整硬件输出
			if (pwmCnt < pCB->controlPwmOut)
			{
				// 输出高电平
				LED_ACTION(pCB->LED_GPIO_x, pCB->LED_PIN_x, 1);
			}
			else
			{
				// 输出低电平
				LED_ACTION(pCB->LED_GPIO_x, pCB->LED_PIN_x, 0);
			}
		}
		
		// 中断函数执行次数自增
		pwmCnt++;		
	}
}
// LED硬件初始化
void GPIO_LEDInit(void)
{

	GPIO_InitType GPIO_InitStructure;

	/**时钟初始化**/
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_GPIOA, ENABLE);	

	/**LED管脚初始化**/
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

// 软件PWM，初始GPIO口
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
// 硬件PWM，初始PWM底层


#endif

// LED模块初始化
void LED_Inits(uint32 mainFre)
{
// -----------------------------软件PWM初始化-----------------------------
#if LED_PWM_MODE == LED_SOFT_PWM
	
	// LED 控制定时器初始化
	LED_SoftPwm_InitTimerx(mainFre);
	
	// LED硬件初始化
	GPIO_LEDInit();
	
	// LED 软件pwmIO 初始化
	LED_SoftPwm_InitGpio();

#else

	
#endif
}

// LED定时回调函数，实时处理PWM输出百分比
void LED_CALLBACK_Trig(uint32 param)
{
	LED_ACTION_E action;
	uint8 breathControlPwm;
	uint8 i;

	// 遍历处理所有LED
	for (i = 0; i < LED_NAME_MAX; i++)
	{
		// 次数为0说明已经执行完动作，后面处于关闭状态，因此从灭时间计数开始
		if (0 == ledCB.typeQueue[i].loopCount)
		{
			ledCB.typeQueue[i].ledCounter = ledCB.typeQueue[i].onCycleCountTimesSetValue;
		
			// 根据样式来恢复上一次操作
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
		
		// LED计数值自增
		ledCB.typeQueue[i].ledCounter ++;
		ledCB.typeQueue[i].ledCounter %= ledCB.typeQueue[i].cycleCountTimesSetValue;

		// 一个周期跑完循环次数减一
		if (((0 < ledCB.typeQueue[i].loopCount)) && (ledCB.typeQueue[i].ledCounter == (ledCB.typeQueue[i].cycleCountTimesSetValue - 1)))
		{
			ledCB.typeQueue[i].loopCount --;
		}

		// 根据设定的 亮 保持时间计数值和 灭 保持时间计数值，设置当前状态为亮/灭
		if (ledCB.typeQueue[i].ledCounter < ledCB.typeQueue[i].onCycleCountTimesSetValue)
		{
			action = LED_ACTION_ON;

			// 反方向的变量归零
			ledCB.typeQueue[i].breathDownCounter = 0;
			
			// 为0说明不需要呼吸，直接输出高电平
			if (0 == ledCB.typeQueue[i].breathUpCountTimesSetValue)
			{
				breathControlPwm = 100;
			}
			else
			{
				// 需要呼吸，逐渐变亮
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

			// 反方向的变量归零
			ledCB.typeQueue[i].breathUpCounter = 0;
			
			// 为0说明不需要呼吸，直接输出低电平
			if (0 == ledCB.typeQueue[i].breathDownCountTimesSetValue)
			{
				breathControlPwm = 0;	
			}
			else
			{
				// 需要呼吸，逐渐变暗
				if (ledCB.typeQueue[i].breathDownCounter < ledCB.typeQueue[i].breathDownCountTimesSetValue)
				{			
					ledCB.typeQueue[i].breathDownCounter ++;
				}
				
				breathControlPwm = (ledCB.typeQueue[i].breathDownCountTimesSetValue - ledCB.typeQueue[i].breathDownCounter) * 100 / ledCB.typeQueue[i].breathDownCountTimesSetValue;
			}
		}
		
		ledCB.typeQueue[i].lastAction = ledCB.typeQueue[i].action;
		ledCB.typeQueue[i].action = action;	

		// 计算呼吸PWM输出值
		ledCB.typeQueue[i].controlPwmOut = ledCB.typeQueue[i].lowLimitPwmOut + breathControlPwm * (ledCB.typeQueue[i].highLimitPwmOut - ledCB.typeQueue[i].lowLimitPwmOut)/100;

#if LED_PWM_MODE == LED_SOFT_PWM
		// 软件PWM中，LED控制在PWM专用定时器中断中执行
#else
		// PWM控制输出LED
		if (NULL != ledCB.typeQueue[i].fun)
		{
			ledCB.typeQueue[i].fun(ledCB.typeQueue[i].controlPwmOut);
		}	
#endif
	}
}

/*********************************************************************
* 函   数  名: LED_Set
* 函数入参: LED_NAME ledName					// LED控制码
			uint32 ledCycleTime					// LED工作周期时间，单位ms，最小值为100
			uint8 ledOnCycleTimeDutyRatio		// LED亮的占空比，0-100，默认100
			uint8 onBrightnessRatio, 			// LED亮时亮度比，0-100，默认100
			uint8 offBrightnessRatio, 			// LED灭时亮度比，0-100，默认0
			uint8 ledOnAscendRatio,  			// LED亮上升速度占比，0-100，呼吸效果使用，非呼吸默认为0
			uint8 ledOnDescendRatio				// LED亮下降速度占比，0-100，呼吸效果使用，非呼吸默认为0
			int32 actionTimes					// LED执行次数
* 函数出参: void
* 返   回  值: void 
* 功能描述:  LED模式设置
**********************************************************************/
void LED_Set(LED_NAME ledName, uint32 ledCycleTime, uint8 ledOnCycleTimeDutyRatio, uint8 onBrightnessRatio, uint8 offBrightnessRatio, uint8 ledOnAscendRatio, uint8 ledOnDescendRatio, int32 actionTimes)
{
	LED_TYPE_QUEUE* pCB = (&ledCB.typeQueue[ledName]);

	if (NULL == pCB)
	{
		return;
	}

	// 工作周期最小为100ms
	if (ledCycleTime <= 100)
	{
		ledCycleTime = 100;
	}
	
	// 占空比最大只能100%
	if (ledOnCycleTimeDutyRatio > LED_PWM_OUT_MAX)
	{
		ledOnCycleTimeDutyRatio = LED_PWM_OUT_MAX;
	}

	// 最大亮度最大只能100%
	if (onBrightnessRatio > 100)
	{
		onBrightnessRatio = 100;
	}

	// 最大暗度最大只能为100%
	if (offBrightnessRatio > 100)
	{
		offBrightnessRatio = 100;
	}

	// 亮度升缓慢变化率最慢只能是100%
	if (ledOnAscendRatio > 100)
	{
		ledOnAscendRatio = 100;
	}

	// 亮度降缓慢变化率最慢只能是100%
	if (ledOnDescendRatio > 100)
	{
		ledOnDescendRatio = 100;
	}
	
	// 清零计数
	pCB->ledCounter = 0;
	pCB->breathUpCounter = 0;
	pCB->breathDownCounter = 0;
	
	// 动作重置
	pCB->action = LED_ACTION_OFF;
	pCB->lastAction = LED_ACTION_OFF;

	// 计算LED周期，时间计数值
	pCB->cycleCountTimesSetValue = ledCycleTime / LED_COUNT_TIME;

	// LED亮的占空比计算亮时间计数
	pCB->onCycleCountTimesSetValue = pCB->cycleCountTimesSetValue * ledOnCycleTimeDutyRatio / LED_PWM_OUT_MAX;	

	// LED亮时亮度比
	pCB->highLimitPwmOut = onBrightnessRatio;
	// LED暗时亮度比
	pCB->lowLimitPwmOut = offBrightnessRatio;

	// LED 亮计数值
	pCB->breathUpCountTimesSetValue = pCB->onCycleCountTimesSetValue * ledOnAscendRatio / LED_PWM_OUT_MAX;
	// LED 灭计数值
	pCB->breathDownCountTimesSetValue = (pCB->cycleCountTimesSetValue - pCB->onCycleCountTimesSetValue) * ledOnDescendRatio / LED_PWM_OUT_MAX;

	// 循环次数控制
	pCB->loopCount = actionTimes;
}

// 关闭所有LED输出
void LED_CloseAllLeds(void)
{

#if LED_PWM_MODE == LED_HARD_PWM

#endif
}

#if LED_PWM_MODE == LED_HARD_PWM
// 红灯控制
void LED_SetRedPwm(uint32 param)
{
    LED_RED_PWM(param);
}

// 黄灯控制
void LED_SetYellowPwm(uint32 param)
{ 	
  	LED_YELLOW_PWM(param);
}

// 绿灯控制
void LED_SetGreenPwm(uint32 param)
{
	LED_GREEN_PWM(param);
}
#endif





















