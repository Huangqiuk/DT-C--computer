#include "common.h"
#include "key.h"
#include "keymsgprocess.h"
#include "timer.h"
#include "param.h"

void KEY_Init(void)
{	 
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_0);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_1);
	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	
	// 一定要在KEYMSG_RegisterKeyScanCallback()函数之前调用
	KEYMSG_Init();
	
	// 注册按键扫描接口
	KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
}

void KEY_CALLBACK_Scan(uint8* p8bitKeyValueArray)
{
	uint8 keyMask = 0x1F;			// 相当于无实体按键按下
	uint8 keyAddDecMask = 0x05;		// 加减同时按时
	uint8 keyValue = 0;

	keyValue = (gpio_input_port_get(GPIOC) & keyMask) | KEY_MASK_ADD_DEC;

	*p8bitKeyValueArray = keyValue;

	// 只要有按键触发就立刻重设自动关机时间
	if (keyValue != KEY_MASK_ALL)
	{
	}
}

