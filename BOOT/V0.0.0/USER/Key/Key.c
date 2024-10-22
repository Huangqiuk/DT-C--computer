#include "main.h"
#include "Keymsgprocess.h"

KEY_CB keyCB;
//																			
//void KEY_Init(void)
//{	 
//    uint8_t i;

//	//硬件初始化
//    Button_Init();
//	//管理结构初始化
//	// 一定要在KEYMSG_RegisterKeyScanCallback()函数之前调用
//	KEYMSG_Init();
//	// 注册按键扫描接口
//	KEYMSG_RegisterKeyScanCallback(KEY_CALLBACK_Scan);
//}

//uint8_t testkey = 0;
//void KEY_CALLBACK_Scan(uint8_t* p8bitKeyValueArray)
//{
//	uint8_t keyMask = 0x7F;
//	uint8_t mask = (0xFF>>(8-KEY_FILTER_LEVEL_N));
//	uint8_t i;
//	uint8_t keyValue = 0;
//	uint8_t keyRead = 0xFF;
//	
//	//没有按键按下keyRead是0xFF
//	keyRead = ~ADC_Button_Read();		//读取ADC键值 - 哪个按键被按下哪个按键被置0
//	//读取助力按键 - A2
//	keyRead &= GPIO_Input_Pin_Data_Get(GPIOA, GPIO_PIN_2)? 0xff : 0x7f;//被按下最高位置0，否则置1
//	//把读到的键值发给串口
//	
//	KEYVALUE2HMI = keyRead;
//	return;
//	// ■■这里增加按键滤波措施■■
//	// 【第一步】将读取到的按键放入缓冲区
//	for(i=0; i<KEY_NAME_MAX; i++)		// 组合按键不需要在这里滤波，因此这里循环上限-1
//	{
//		// 读取到的每个按键的信息添加到对应的缓冲区里
//		keyCB.buff[i] <<= 1;
//		keyCB.buff[i] |= (keyRead>>i)&0x01;
//	}
//	
//	// 【第二步】缓冲区处理，连续N次读到同一个状态才确认，N可设定
//	for(i=0; i<KEY_NAME_MAX; i++)
//	{
//		// 当前按键缓冲区从低位开始连续N个均为1，判定为释放
//		if((keyCB.buff[i] & mask) == mask)
//		{
//			keyCB.lastValue[i] = 0x01<<i;
//		}
//		// 从低位开始连续N个均为0，则判定为按下
//		else if(0 == (keyCB.buff[i] & mask))
//		{
//			keyCB.lastValue[i] = 0;
//		}
//		// 其余场景，为不稳定期，不处理
//		else
//		{}

//		keyValue |= keyCB.lastValue[i];
//	}
//	
//	testkey = keyValue;
//	*p8bitKeyValueArray = keyValue;
//}





