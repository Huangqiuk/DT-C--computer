#include "powerCtl.h"
#include "common.h" 
#include "timer.h"

// ״̬��ʼ��
void POWER_Init(void)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);
	POWER_ON();

	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
	// LCD�����
	LCD_BL_OFF();
}


