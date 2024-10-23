#include "powerCtl.h"
#include "common.h" 
#include "timer.h"

// 状态初始化
void POWER_Init(void)
{
	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);
	POWER_ON();

	// 打开时钟
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
	// LCD背光关
	LCD_BL_OFF();
}


