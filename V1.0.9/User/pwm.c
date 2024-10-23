#include "pwm.h"
#include "timer.h"

PWM_OUT1_CB pwmout1CB;

/*********************************************************
* ■■■■■函数定义区■■■■■■■■■■■■■■■■■■
*********************************************************/
void PWM_OUT1_Init(void)
{	
	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;

	// 打开APB总线上的GPIOA时钟
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_AF);

	// 设置GPIOA上的TIM0 1通道对应引脚PA.8为第二功能推挽输出 
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);

	// 打开TIM0时钟
	rcu_periph_clock_enable(RCU_TIMER4);

	timer_initpara.period            = 100-1;
	timer_initpara.prescaler         = 72-1;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER4,&timer_initpara);

	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
	timer_channel_output_config(TIMER4,TIMER_CH_0,&timer_ocintpara);

	timer_channel_output_mode_config(TIMER4,TIMER_CH_0,TIMER_OC_MODE_PWM0);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0, 0);      //占空比为0
	timer_channel_output_shadow_config(TIMER4,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

	timer_auto_reload_shadow_enable(TIMER4);
	timer_enable(TIMER4);
	timer_primary_output_config(TIMER4, ENABLE);

	// 初始化关掉背光
	PWM_OUT1_Start(PWM_OUT1_LEVEL_OFF);

}

// 启动定时器，并指定工作模式
void PWM_OUT1_Start(PWM_OUT1_LEVEL_E level)
{
//	if (level >= PWM_OUT1_LEVEL_AUTO)
//	{
//		return;
//	}

//	// 新模式与当前工作中的模式相同，不予执行
//	if (pwmout1CB.level == level)
//	{
//		return;
//	}
//	pwmout1CB.level = level;

//	switch (level)
//	{
//		case PWM_OUT1_LEVEL_OFF:
//			PWM_OUT1_OFF();
//			break;

//		case PWM_OUT1_LEVEL_ON:
//			PWM_OUT1_ON();
//			break;

//	/*	case PWM_OUT1_LEVEL_5_100:
//			PWM_OUT1_PWM(5);
//			break;

//		case PWM_OUT1_LEVEL_10_100:
//			PWM_OUT1_PWM(10);
//			break;

//		case PWM_OUT1_LEVEL_15_100:
//			PWM_OUT1_PWM(15);
//			break;
//*/
//		case PWM_OUT1_LEVEL_20_100:
//			PWM_OUT1_PWM(20);
//			break;
///*
//		case PWM_OUT1_LEVEL_25_100:
//			PWM_OUT1_PWM(25);
//			break;

//		case PWM_OUT1_LEVEL_30_100:
//			PWM_OUT1_PWM(30);
//			break;

//		case PWM_OUT1_LEVEL_35_100:
//			PWM_OUT1_PWM(35);
//			break;
//*/
//		case PWM_OUT1_LEVEL_40_100:
//			PWM_OUT1_PWM(40);
//			break;
///*
//		case PWM_OUT1_LEVEL_45_100:
//			PWM_OUT1_PWM(45);
//			break;

//		case PWM_OUT1_LEVEL_50_100:
//			PWM_OUT1_PWM(50);
//			break;

//		case PWM_OUT1_LEVEL_55_100:
//			PWM_OUT1_PWM(55);
//			break;
//*/
//		case PWM_OUT1_LEVEL_60_100:
//			PWM_OUT1_PWM(60);
//			break;
///*
//		case PWM_OUT1_LEVEL_65_100:
//			PWM_OUT1_PWM(65);
//			break;

//		case PWM_OUT1_LEVEL_70_100:
//			PWM_OUT1_PWM(70);
//			break;

//		case PWM_OUT1_LEVEL_75_100:
//			PWM_OUT1_PWM(75);
//			break;
//*/
//		case PWM_OUT1_LEVEL_80_100:
//			PWM_OUT1_PWM(80);
//			break;
///*
//		case PWM_OUT1_LEVEL_85_100:
//			PWM_OUT1_PWM(85);
//			break;

//		case PWM_OUT1_LEVEL_90_100:
//			PWM_OUT1_PWM(90);
//			break;

//		case PWM_OUT1_LEVEL_95_100:
//			PWM_OUT1_PWM(95);
//			break;
//*/			
//		default:
//			PWM_OUT1_OFF();
//			break;
//	}
}
