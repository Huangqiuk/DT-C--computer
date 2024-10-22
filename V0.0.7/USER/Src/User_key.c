#include "USER_Common.h"


KEY_CB keyCB;
			


	//按键扫描
	//KEYMSG_CALLBACK_Scan(1);	


void KEY_Init(void)
{	 
    uint8_t i;

	//硬件初始化
	nrf_gpio_cfg_input(KEY_M, NRF_GPIO_PIN_PULLUP);//上拉输入模式
	nrf_gpio_cfg_input(KEY_DEC, NRF_GPIO_PIN_PULLUP);//上拉输入模式
	nrf_gpio_cfg_input(KEY_ADD, NRF_GPIO_PIN_PULLUP);//上拉输入模式
	
	//管理结构初始化
	// 一定要在KEYMSG_RegisterKeyScanCallback()函数之前调用
	KEYMSG_Init();
	// 注册按键扫描接口
	KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
	//启动按键定时器 
		TIMER_AddTask(TIMER_ID_KEY_SCAN, 20, KEYMSG_CALLBACK_Scan, 0, -1, 0);
	//TIMER_AddTask(TIMER_ID_KEY_SCAN, 20, KEYMSG_CALLBACK_Scan, 0, -1, 0);
}

void KEY_CALLBACK_Scan(uint8_t* p8bitKeyValueArray)
{
	
	uint8_t mask = (0xFF>>(8-KEY_FILTER_LEVEL_N));
	uint8_t i = 0;
	uint8_t keyRead = 0xff;
	uint8_t keyValue[KEY_STATE_BYTE_COUNT] = {0};
	
	
	//keyRead <<= 1;
	keyRead = nrf_gpio_pin_read(KEY_ADD);
	keyRead <<= 1;
	keyRead |= nrf_gpio_pin_read(KEY_DEC);
	keyRead <<= 1;
	keyRead |= nrf_gpio_pin_read(KEY_M);
	
	//NRF_LOG_INFO(" 0x%x ", keyRead);
	// ■■这里增加按键滤波措施■■
	// 【第一步】将读取到的按键放入缓冲区
	for(i=0; i<KEY_NAME_MAX - 3; i++)		// 组合按键不需要在这里滤波，因此这里循环上限-1
	{
		// 读取到的每个按键的信息添加到对应的缓冲区里
		keyCB.buff[i] <<= 1;
		keyCB.buff[i] |= (keyRead>>i) & 0x01;
	}
	
	// 【第二步】缓冲区处理，连续N次读到同一个状态才确认，N可设定
	for (i = 0; i < KEY_NAME_MAX - 3; i++)
	{
		// 当前按键缓冲区从低位开始连续N个均为1，判定为释放
		if ((keyCB.buff[i] & mask) == mask)
		{
			keyCB.lastValue[i] = 0x01 << (i % 8);
		}
		// 从低位开始连续N个均为0，则判定为按下
		else if (0 == (keyCB.buff[i] & mask))
		{
			keyCB.lastValue[i] = 0;
		}
		// 其余场景，为不稳定期，不处理
		else
		{
		}

		keyValue[i / 8] |= keyCB.lastValue[i];
	}
	//组合按键处理
	keyValue[0] |= 0x38; //组合按键复位
	if(0 == (keyValue[0] & (KEY_MASK_M|KEY_MASK_DEC))){
		// M -
		keyValue[0] &= ~KEY_MASK_M_DEC;
	}
	if(0 == (keyValue[0] & (KEY_MASK_M|KEY_MASK_ADD))){
		// M +
		keyValue[0] &= ~KEY_MASK_M_ADD;
	}
	if(0 == (keyValue[0] & (KEY_MASK_DEC|KEY_MASK_ADD))){
		// - +
		keyValue[0] &= ~KEY_MASK_DEC_ADD;
	}
	
	
	p8bitKeyValueArray[0] = keyValue[0];
	//NRF_LOG_INFO("key %x", ~(KEY_MASK_M|KEY_MASK_DEC));

	//p8bitKeyValueArray[1] = keyValue[1];
}


