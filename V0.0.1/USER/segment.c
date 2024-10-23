#include "common.h"
#include "segment.h"
#include "timer.h"
#include "delay.h"
#include "param.h"
#include "state.h"
#include "TM1628_Driver.h"
#include "string.h"
									
SEGMENT_CB segCB;

// 设置显示内容
// 数码管全部显示
void SEGMENT_DIS_AllState(uint8 segBuff[]);

// 禁止停车
void SEGMENT_DIS_NoParkState(uint32 param);

// 低速
void SEGMENT_DIS_LowSpeedState(uint32 param);

// 电量容量
void SEGMENT_DIS_BatteryQuantityState(uint32 param);	

// 电量显示
void SEGMENT_DIS_BatteryLowSpeedState(uint32 param);

// 使能显示函数开关
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable);

// 设置显示参数
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param);

// 数码管刷新请求回调函数
void SEGMENT_CALLBACK_RefreshRequest(uint32 param);

// 闪烁处理回调函数，由定时器调用
void SEGMENT_CALLBACK_BlinkProcess(uint32 data);

// 闪烁处理回调函数，由定时器调用
void SEGMENT_CALLBACK_BlinkProcess(uint32 data);

void SEGMENT_DIS_OneFlowerState(uint32 param);

// 待机
void SEGMENT_DIS_WaitState(uint32 param);

// 不可使用
void SEGMENT_DIS_UnavailableState(uint32 param);

// 预约
void SEGMENT_DIS_ReservedState(uint32 param);

// 龟速图标
void SEGMENT_DIS_LowSpeedLogoState(uint32 param);

#if 1
// 单元测试函数
uint8 SEGMENT_UnitTest(void)
{	
	static uint32 test = 0;
	
	if (10 > paramCB.runtime.testflag[1])
	{
		SEGMENT_DIS_LowSpeedState(test%3);
		SEGMENT_DIS_UnavailableState(test%3);
		SEGMENT_DIS_ReservedState(test%3);
		SEGMENT_DIS_PetalsState(test%7);
		SEGMENT_DIS_BatteryQuantityState(test % 7);
		SEGMENT_DIS_NoParkState(test%3);
		SEGMENT_DIS_LowSpeedLogoState(test%3);
		SEGMENT_DIS_WaitState(test%3);
		
		if (test > 100)
		{
//			SEGMENT_DIS_NumState(paramCB.runtime.testflag[1]);
		}
		
		test++;
		test%=120;
	}
	else
	{
		test++;
		test%=35;
		if (test < 28)
		{			
			SEGMENT_DisplayTest(test);
		}
		else
		{
			SEGMENT_DisplayTest(27);
		}
	}
	
	return 0;
}

#endif

// 数码管刷新回调函数
void SEGMENT_CALLBACK_Refresh(uint32 param)
{
	// 数码段的值写入到TM1628
	TM1628_DisplayAutoAddr(0x00, (UINT16*)segCB.mapData, SEG_COUNT);
}

// 数码管刷新请求回调函数
void SEGMENT_CALLBACK_RefreshRequest(uint32 param)
{
	segCB.refreshRequest = (BOOL)param;
}

// 闪烁处理回调函数，由定时器调用
void SEGMENT_CALLBACK_BlinkProcess(uint32 data)
{
	// 闪烁状态翻转
	segCB.blinkPhase = ~segCB.blinkPhase;
}

// 数码管初始化
void SEGMENT_Init(void)
{
	uint8 i;

	// 数码管驱动芯片初始化
	TM1628_Init();
	
	// 闪烁相位
//	TIMER_AddTask(TIMER_ID_SEGMENT_BLINK,
//					500,
//					SEGMENT_CALLBACK_BlinkProcess,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

//	// 数码管刷新间隔
//	TIMER_AddTask(TIMER_ID_SEGMENT_REFRESH_TIME,
//					100,
//					SEGMENT_CALLBACK_RefreshRequest,
//					bTRUE,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

	// 显示数据结构初始化	
	segCB.blinkPhase = 0x00;

	for (i = 0; i < COM_COUNT; i++)
	{
		segCB.blinkEnable[i] = 0;
		segCB.state[i] = 0x00;
	}
	
	// 总开关
	SEGMENT_SetGroupOnOff(SEG_MASK_ALL, bFALSE);
//	segCB.mapData[0] = 0xFFFF;
//	segCB.mapData[1] = 0xFFFF;
//	segCB.mapData[2] = 0xFFFF;
//	segCB.mapData[3] = 0xFFFF;
//	segCB.mapData[4] = 0xFFFF;
//	
//	// 刷新数码管
//	SEGMENT_CALLBACK_Refresh(bTRUE);
//	
//	while(1);
}

// 数码管处理函数
void SEGMENT_Process(void)
{
	uint8 j,i = 0;
	
	// 检测刷新标志
	if (segCB.refreshRequest)
	{
		// 清除刷新标志
		segCB.refreshRequest = bFALSE;
		
		// 根据segCB结构体 段数值、开关掩码、闪烁掩码，确定 输出段数值
		segCB.outputSeg[COM_NUMBER_COM0] = (((segCB.blinkEnable[COM_NUMBER_COM0] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM0]) ^ segCB.state[COM_NUMBER_COM0]) & segCB.onoff[COM_NUMBER_COM0];
		segCB.outputSeg[COM_NUMBER_COM1] = (((segCB.blinkEnable[COM_NUMBER_COM1] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM1]) ^ segCB.state[COM_NUMBER_COM1]) & segCB.onoff[COM_NUMBER_COM1];
		segCB.outputSeg[COM_NUMBER_COM2] = (((segCB.blinkEnable[COM_NUMBER_COM2] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM2]) ^ segCB.state[COM_NUMBER_COM2]) & segCB.onoff[COM_NUMBER_COM2];
		segCB.outputSeg[COM_NUMBER_COM3] = (((segCB.blinkEnable[COM_NUMBER_COM3] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM3]) ^ segCB.state[COM_NUMBER_COM3]) & segCB.onoff[COM_NUMBER_COM3];
		segCB.outputSeg[COM_NUMBER_COM4] = (((segCB.blinkEnable[COM_NUMBER_COM4] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM4]) ^ segCB.state[COM_NUMBER_COM4]) & segCB.onoff[COM_NUMBER_COM4];
		segCB.outputSeg[COM_NUMBER_COM5] = (((segCB.blinkEnable[COM_NUMBER_COM5] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM5]) ^ segCB.state[COM_NUMBER_COM5]) & segCB.onoff[COM_NUMBER_COM5];
		segCB.outputSeg[COM_NUMBER_COM6] = (((segCB.blinkEnable[COM_NUMBER_COM6] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM6]) ^ segCB.state[COM_NUMBER_COM6]) & segCB.onoff[COM_NUMBER_COM6];
		segCB.outputSeg[COM_NUMBER_COM7] = (((segCB.blinkEnable[COM_NUMBER_COM7] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM7]) ^ segCB.state[COM_NUMBER_COM7]) & segCB.onoff[COM_NUMBER_COM7];
		segCB.outputSeg[COM_NUMBER_COM8] = (((segCB.blinkEnable[COM_NUMBER_COM8] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM8]) ^ segCB.state[COM_NUMBER_COM8]) & segCB.onoff[COM_NUMBER_COM8];
		segCB.outputSeg[COM_NUMBER_COM9] = (((segCB.blinkEnable[COM_NUMBER_COM9] & segCB.blinkPhase) & segCB.state[COM_NUMBER_COM9]) ^ segCB.state[COM_NUMBER_COM9]) & segCB.onoff[COM_NUMBER_COM9];

		for (j = 0; j < SEG_COUNT; j++)
		{
			segCB.mapData[j] = 0x00;
			for (i = 0; i < COM_COUNT; i++)
			{
				segCB.mapData[j] |= (((segCB.outputSeg[i] & (0x01 << j))?1:0) << i);
			}
		}
		
		// 刷新数码管
		SEGMENT_CALLBACK_Refresh(bTRUE);
	}
}

// 设置总开关
void SEGMENT_SetGroupOnOff(SEG_MASK_E segMask, BOOL onoff)
{
	// 所有
	if (segMask & SEG_MASK_ALL)
	{
		if (onoff)
		{
			segCB.onoff[COM_NUMBER_COM0] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM1] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM2] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM3] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM4] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM5] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM6] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM7] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM8] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM9] |= 0xFF;
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM0] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM1] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM2] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM3] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM4] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM5] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM6] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM7] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM8] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM9] &= (~0xFF);
		}

		return;
	
	}
	
	// 待机界面
	if(segMask & SEG_MASK_FREE)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM0] |= (0x01 << 4);
			segCB.onoff[COM_NUMBER_COM9] |= (0x01 << 3);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM0] &= (~(0x01 << 4));
			segCB.onoff[COM_NUMBER_COM9] &= (~(0x01 << 3));
		}
	}

	// 预约界面
	if(segMask & SEG_MASK_RESERVED)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM6] |= (0x01 << 2);
			segCB.onoff[COM_NUMBER_COM7] |= (0x01 << 2);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM6] &= (~(0x01 << 2));
			segCB.onoff[COM_NUMBER_COM7] &= (~(0x01 << 2));
		}
	}

	// 错误界面
	if(segMask & SEG_MASK_ERROR)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM6] |= (0x01 << 0);
			segCB.onoff[COM_NUMBER_COM7] |= (0x01 << 0);
			
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM6] &= (~(0x01 << 0));
			segCB.onoff[COM_NUMBER_COM7] &= (~(0x01 << 0));
			
		}
	}

	// 龟速界面
	if(segMask & SEG_MASK_LOWSPEED)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM8] |= (0x01 << 2);
			segCB.onoff[COM_NUMBER_COM9] |= (0x01 << 2);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM8] &= (~(0x01 << 2));
			segCB.onoff[COM_NUMBER_COM9] &= (~(0x01 << 2));
		}
	}

	// 禁停界面
	if(segMask & SEG_MASK_NOPARKING)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM8] |= (0x01 << 0);
			segCB.onoff[COM_NUMBER_COM9] |= (0x01 << 0);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM8] &= (~(0x01 << 0));
			segCB.onoff[COM_NUMBER_COM9] &= (~(0x01 << 0));
		}
	}

	// 速度界面
	if(segMask & SEG_MASK_SPEED)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM4] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM5] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM6] |= (0x01 << 3);
			segCB.onoff[COM_NUMBER_COM7] |= (0x01 << 3);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM4] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM5] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM6] &= (~(0x01 << 3));
			segCB.onoff[COM_NUMBER_COM7] &= (~(0x01 << 3));
		}
	}

	// 电量界面
	if(segMask & SEG_MASK_BATTERY)
	{
		if(onoff)
		{
			segCB.onoff[COM_NUMBER_COM0] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM1] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM2] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM3] |= 0xFF;
			segCB.onoff[COM_NUMBER_COM6] |= (0x01 << 1);
			segCB.onoff[COM_NUMBER_COM7] |= (0x01 << 1);
			segCB.onoff[COM_NUMBER_COM8] |= 0x0A;
			segCB.onoff[COM_NUMBER_COM9] |= (0x01 << 1);
		}
		else
		{
			segCB.onoff[COM_NUMBER_COM0] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM1] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM2] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM3] &= (~0xFF);
			segCB.onoff[COM_NUMBER_COM6] &= (~(0x01 << 1));
			segCB.onoff[COM_NUMBER_COM7] &= (~(0x01 << 1));
			segCB.onoff[COM_NUMBER_COM8] &= (~0x0A);
			segCB.onoff[COM_NUMBER_COM9] &= (~(0x01 << 1));
		}
	}
}

// 数码管全部显示
void SEGMENT_DIS_AllState(uint8 segBuff[])
{		
	// 数值置位
	segCB.state[COM_NUMBER_COM0] = segBuff[0];
	segCB.state[COM_NUMBER_COM1] = segBuff[1];
	segCB.state[COM_NUMBER_COM2] = segBuff[2];
	segCB.state[COM_NUMBER_COM3] = segBuff[3];
	segCB.state[COM_NUMBER_COM4] = segBuff[4];
	segCB.state[COM_NUMBER_COM5] = segBuff[5];
	segCB.state[COM_NUMBER_COM6] = segBuff[6];
	segCB.state[COM_NUMBER_COM7] = segBuff[7];
	segCB.state[COM_NUMBER_COM8] = segBuff[8];
	segCB.state[COM_NUMBER_COM9] = segBuff[9];
}

// 龟速图标
void SEGMENT_DIS_LowSpeedLogoState(uint32 param)
{
	if (param)
	{
		segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM8] | (0x01 << 2));
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] | (0x01 << 2));
	}
	else
	{
		segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM8] & (~0x01));
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] & (~0x01));
	}
}

// 禁停
void SEGMENT_DIS_NoParkState(uint32 param)
{		
	if (param)
	{
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM8] | 0x01);
		segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM9] | 0x01);
	}
	else
	{
		segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM8] & (~0x01));
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] & (~0x01));
	}
}

// 不可使用
void SEGMENT_DIS_UnavailableState(uint32 param)
{	
	segCB.state[COM_NUMBER_COM0] = (segCB.state[COM_NUMBER_COM0] & (~0xFF));
	segCB.state[COM_NUMBER_COM1] = (segCB.state[COM_NUMBER_COM1] & (~0xFF));
	segCB.state[COM_NUMBER_COM2] = (segCB.state[COM_NUMBER_COM2] & (~0xFF));
	segCB.state[COM_NUMBER_COM3] = (segCB.state[COM_NUMBER_COM3] & (~0xFF));
	segCB.state[COM_NUMBER_COM4] = (segCB.state[COM_NUMBER_COM4] & (~0xFF));
	segCB.state[COM_NUMBER_COM5] = (segCB.state[COM_NUMBER_COM5] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = (segCB.state[COM_NUMBER_COM6] & (~0xFF));
	segCB.state[COM_NUMBER_COM7] = (segCB.state[COM_NUMBER_COM7] & (~0xFF));
	segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM8] & (~0xFF));
	segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] & (~0xFF));
	
	segCB.state[COM_NUMBER_COM6] = (segCB.state[COM_NUMBER_COM6] | 0x01);
	segCB.state[COM_NUMBER_COM7] = (segCB.state[COM_NUMBER_COM7] | 0x01);
	
	switch (param)
	{
		case 2:
			segCB.state[COM_NUMBER_COM8] |= (segCB.state[COM_NUMBER_COM8] | 0x08);
			segCB.state[COM_NUMBER_COM4] |= (segCB.state[COM_NUMBER_COM4] | 0x04);
			segCB.state[COM_NUMBER_COM5] |= (segCB.state[COM_NUMBER_COM5] | 0x04);
			break;
			
		case 6:
			segCB.state[COM_NUMBER_COM4] |= (segCB.state[COM_NUMBER_COM4] | 0x00);
			segCB.state[COM_NUMBER_COM5] |= (segCB.state[COM_NUMBER_COM5] | 0x02);
			segCB.state[COM_NUMBER_COM6] |= (segCB.state[COM_NUMBER_COM6] | 0x08);
			segCB.state[COM_NUMBER_COM7] |= (segCB.state[COM_NUMBER_COM7] | 0x00);
			break;

		case 10:
			segCB.state[COM_NUMBER_COM8] |= (segCB.state[COM_NUMBER_COM8] | 0x08);
			segCB.state[COM_NUMBER_COM4] |= (segCB.state[COM_NUMBER_COM4] | 0x04);
			break;

		case 11:
			segCB.state[COM_NUMBER_COM0] |= (segCB.state[COM_NUMBER_COM0] | 0x0A);
			segCB.state[COM_NUMBER_COM1] |= (segCB.state[COM_NUMBER_COM1] | 0x0A);
			segCB.state[COM_NUMBER_COM2] |= (segCB.state[COM_NUMBER_COM2] | 0x0A);
			segCB.state[COM_NUMBER_COM3] |= (segCB.state[COM_NUMBER_COM3] | 0x0A);
			segCB.state[COM_NUMBER_COM6] |= (segCB.state[COM_NUMBER_COM6] | 0x02);
			segCB.state[COM_NUMBER_COM7] |= (segCB.state[COM_NUMBER_COM7] | 0x02);
			segCB.state[COM_NUMBER_COM8] |= (segCB.state[COM_NUMBER_COM8] | 0x0A);
			segCB.state[COM_NUMBER_COM9] |= (segCB.state[COM_NUMBER_COM9] | 0x02);
			break;

		default:
			segCB.state[COM_NUMBER_COM4] |= (segCB.state[COM_NUMBER_COM4] | 0x00);
			segCB.state[COM_NUMBER_COM5] |= (segCB.state[COM_NUMBER_COM5] | 0x02);
			segCB.state[COM_NUMBER_COM6] |= (segCB.state[COM_NUMBER_COM6] | 0x08);
			segCB.state[COM_NUMBER_COM7] |= (segCB.state[COM_NUMBER_COM7] | 0x00);
			break;
	}	
}

// 预约
void SEGMENT_DIS_ReservedState(uint32 param)
{	
	if (param)
	{
		segCB.state[COM_NUMBER_COM6] = (segCB.state[COM_NUMBER_COM6] | 0x04);
		segCB.state[COM_NUMBER_COM7] = (segCB.state[COM_NUMBER_COM7] | 0x04);
	}
	else
	{
		segCB.state[COM_NUMBER_COM6] = (segCB.state[COM_NUMBER_COM6] & (~0x04));
		segCB.state[COM_NUMBER_COM7] = (segCB.state[COM_NUMBER_COM7] & (~0x04));
	}
}

// 低速
void SEGMENT_DIS_LowSpeedState(uint32 param)
{		
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	
	if (param)
	{
		segCB.state[COM_NUMBER_COM8] = (segCB.state[COM_NUMBER_COM8] | 0x04);
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] | 0x04);

		if (0 == nowSpeed)
		{
			speedLevel = 0;
		}
		else if (nowSpeed < Level)
		{
			speedLevel = 1;
		}
		else if (nowSpeed < Level*2)
		{
			speedLevel = 2;
		}
		else if (nowSpeed < Level*3)
		{
			speedLevel = 3;
		}
		else if (nowSpeed < Level*4)
		{
			speedLevel = 4;
		}
		else if (nowSpeed < Level*5)
		{
			speedLevel = 5;
		}
		else
		{
			speedLevel = 6;
		}

		/*switch (speedLevel)
		{
			case 0:
				maskCode  = 0x00;
				maskCode1 = 0x00;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
				
			case 1:
				maskCode  = 0x00;
				maskCode1 = 0x04;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				
				break;
			
			case 2:
				maskCode  = 0x04;
				maskCode1 = 0x04;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
			
			case 3:
				maskCode  = 0x04;
				maskCode1 = 0x04;
				maskCode2 = 0x00;
				maskCode3 = 0x08;
				break;
			
			case 4:
				maskCode  = 0x04;
				maskCode1 = 0x04;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			case 5:
				maskCode  = 0x04;
				maskCode1 = 0x0C;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
				
			case 6:
				maskCode  = 0x0C;
				maskCode1 = 0x0C;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			default:
				break;		
		}

		segCB.state[COM_NUMBER_COM4] = maskCode | (segCB.state[COM_NUMBER_COM4] & (~0xFF));
		segCB.state[COM_NUMBER_COM5] = maskCode1 | (segCB.state[COM_NUMBER_COM5] & (~0xFF));
		segCB.state[COM_NUMBER_COM6] = maskCode2 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 3)));
		segCB.state[COM_NUMBER_COM7] = maskCode3 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 3)));*/

		SEGMENT_DIS_BatteryLowSpeedState(speedLevel);
	}
}

// 待机
void SEGMENT_DIS_WaitState(uint32 param)
{		
	if (param)
	{
		segCB.state[COM_NUMBER_COM0] = (segCB.state[COM_NUMBER_COM0] | 0x10);
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] | 0x08);
	}
	else
	{
		segCB.state[COM_NUMBER_COM0] = (segCB.state[COM_NUMBER_COM0] & (~0x10));
		segCB.state[COM_NUMBER_COM9] = (segCB.state[COM_NUMBER_COM9] & (~0x08));
	}
}

// 电量显示
void SEGMENT_DIS_BatteryLowSpeedState(uint32 param)
{
	uint8 maskCode, maskCode1, maskCode2, maskCode3, maskCode4, maskCode5, maskCode6, maskCode7 = 0;

	switch (param)
	{
		case 0:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x04;
			maskCode3 = 0x04;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			
			break;
		case 1:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x04;
			maskCode3 = 0x04;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;

			break;
		
		case 2:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x04;
			maskCode3 = 0x04;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 3:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x0C;
			maskCode3 = 0x0C;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 4:
			maskCode  = 0x0C;
			maskCode1 = 0x0C;
			maskCode2 = 0x0C;
			maskCode3 = 0x0C;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 5:
			maskCode  = 0x0C;
			maskCode1 = 0x0C;
			maskCode2 = 0x0C;
			maskCode3 = 0x0C;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
			
		case 6:
			maskCode  = 0x0C;
			maskCode1 = 0x0C;
			maskCode2 = 0x0C;
			maskCode3 = 0x0C;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
		
		default:
			maskCode  = 0x0C;
			maskCode1 = 0x0C;
			maskCode2 = 0x0C;
			maskCode3 = 0x0C;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;		
	}
			
	segCB.state[COM_NUMBER_COM0] = maskCode  | (segCB.state[COM_NUMBER_COM0] & (~0xFF));
	segCB.state[COM_NUMBER_COM1] = maskCode1 | (segCB.state[COM_NUMBER_COM1] & (~0xFF));
	segCB.state[COM_NUMBER_COM2] = maskCode2 | (segCB.state[COM_NUMBER_COM2] & (~0xFF));
	segCB.state[COM_NUMBER_COM3] = maskCode3 | (segCB.state[COM_NUMBER_COM3] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode4 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM7] = maskCode5 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM8] = maskCode6 | (segCB.state[COM_NUMBER_COM8] & (~0x0A));
	segCB.state[COM_NUMBER_COM9] = maskCode7 | (segCB.state[COM_NUMBER_COM9] & (~(0x01 << 1)));
}

// 电量显示
void SEGMENT_DIS_BatteryQuantityState(uint32 param)
{
	uint8 maskCode, maskCode1, maskCode2, maskCode3, maskCode4, maskCode5, maskCode6, maskCode7 = 0;

	switch (param)
	{
		case 0:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			
			break;
		case 1:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x01;
			maskCode3 = 0x01;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;

			break;
		
		case 2:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x04;
			maskCode3 = 0x04;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 3:
			maskCode  = 0x02;
			maskCode1 = 0x02;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 4:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 5:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
			
		case 6:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
		
		default:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;		
	}
	
	segCB.state[COM_NUMBER_COM0] = maskCode  | (segCB.state[COM_NUMBER_COM0] & (~0xFF));
	segCB.state[COM_NUMBER_COM1] = maskCode1 | (segCB.state[COM_NUMBER_COM1] & (~0xFF));
	segCB.state[COM_NUMBER_COM2] = maskCode2 | (segCB.state[COM_NUMBER_COM2] & (~0xFF));
	segCB.state[COM_NUMBER_COM3] = maskCode3 | (segCB.state[COM_NUMBER_COM3] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode4 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM7] = maskCode5 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM8] = maskCode6 | (segCB.state[COM_NUMBER_COM8] & (~0x0A));
	segCB.state[COM_NUMBER_COM9] = maskCode7 | (segCB.state[COM_NUMBER_COM9] & (~(0x01 << 1)));
}

// 充电界面显示
void SEGMENT_DIS_BatteryChargingState(uint32 param)
{
	uint8 maskCode, maskCode1, maskCode2, maskCode3 = 0;

	/*switch (param)
	{
		case 0:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;

			break;
		case 1:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x01;
			maskCode3 = 0x01;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;

			break;
		
		case 2:
			maskCode  = 0x04;
			maskCode1 = 0x04;
			maskCode2 = 0x04;
			maskCode3 = 0x04;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 3:
			maskCode  = 0x02;
			maskCode1 = 0x02;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 4:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x08;
			maskCode7 = 0x00;
			break;
		
		case 5:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x00;
			maskCode5 = 0x00;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
			
		case 6:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;
		
		default:
			maskCode  = 0x0A;
			maskCode1 = 0x0A;
			maskCode2 = 0x0A;
			maskCode3 = 0x0A;
			maskCode4 = 0x02;
			maskCode5 = 0x02;
			maskCode6 = 0x0A;
			maskCode7 = 0x02;
			break;		
	}

	segCB.state[COM_NUMBER_COM0] = maskCode  | (segCB.state[COM_NUMBER_COM0] & (~0xFF));
	segCB.state[COM_NUMBER_COM1] = maskCode1 | (segCB.state[COM_NUMBER_COM1] & (~0xFF));
	segCB.state[COM_NUMBER_COM2] = maskCode2 | (segCB.state[COM_NUMBER_COM2] & (~0xFF));
	segCB.state[COM_NUMBER_COM3] = maskCode3 | (segCB.state[COM_NUMBER_COM3] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode4 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM7] = maskCode5 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 1)));
	segCB.state[COM_NUMBER_COM8] = maskCode6 | (segCB.state[COM_NUMBER_COM8] & (~0x0A));
	segCB.state[COM_NUMBER_COM9] = maskCode7 | (segCB.state[COM_NUMBER_COM9] & (~(0x01 << 1)));*/

	switch (param)
	{
		case 0:
				maskCode  = 0x00;
				maskCode1 = 0x00;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
				
			case 1:
				maskCode  = 0x00;
				maskCode1 = 0x02;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				
				break;
			
			case 2:
				maskCode  = 0x02;
				maskCode1 = 0x02;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
			
			case 3:
				maskCode  = 0x02;
				maskCode1 = 0x02;
				maskCode2 = 0x00;
				maskCode3 = 0x08;
				break;
			
			case 4:
				maskCode  = 0x02;
				maskCode1 = 0x02;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			case 5:
				maskCode  = 0x02;
				maskCode1 = 0x0A;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
				
			case 6:
				maskCode  = 0x0A;
				maskCode1 = 0x0A;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			default:
				break;	
	}
			
	segCB.state[COM_NUMBER_COM4] = maskCode | (segCB.state[COM_NUMBER_COM4] & (~0xFF));
	segCB.state[COM_NUMBER_COM5] = maskCode1 | (segCB.state[COM_NUMBER_COM5] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode2 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 3)));
	segCB.state[COM_NUMBER_COM7] = maskCode3 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 3)));
}

// 花瓣界面
void SEGMENT_DIS_PetalsState(uint32 param)
{
	uint8 maskCode, maskCode1, maskCode2, maskCode3;
	
	switch (param)
	{
		case 0:
				maskCode  = 0x00;
				maskCode1 = 0x00;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
				
			case 1:
				maskCode  = 0x00;
				maskCode1 = 0x01;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				
				break;
			
			case 2:
				maskCode  = 0x04;
				maskCode1 = 0x04;
				maskCode2 = 0x00;
				maskCode3 = 0x00;
				break;
			
			case 3:
				maskCode  = 0x02;
				maskCode1 = 0x02;
				maskCode2 = 0x00;
				maskCode3 = 0x08;
				break;
			
			case 4:
				maskCode  = 0x02;
				maskCode1 = 0x02;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			case 5:
				maskCode  = 0x02;
				maskCode1 = 0x0A;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
				
			case 6:
				maskCode  = 0x0A;
				maskCode1 = 0x0A;
				maskCode2 = 0x08;
				maskCode3 = 0x08;
				break;
			
			default:
				break;	
	}
			
	segCB.state[COM_NUMBER_COM4] = maskCode | (segCB.state[COM_NUMBER_COM4] & (~0xFF));
	segCB.state[COM_NUMBER_COM5] = maskCode1 | (segCB.state[COM_NUMBER_COM5] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode2 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 3)));
	segCB.state[COM_NUMBER_COM7] = maskCode3 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 3)));
}

void SEGMENT_DIS_OneFlowerState(uint32 param)
{
	uint8 maskCode, maskCode1, maskCode2, maskCode3;
	
	switch (param)
	{
		case 0:
			maskCode  = 0x00;
			maskCode1 = 0x02;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			
			break;
		
		case 1:
			maskCode  = 0x02;
			maskCode1 = 0x00;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			break;
		
		case 2:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x00;
			maskCode3 = 0x08;
			break;
		
		case 3:
			maskCode  = 0x00;
			maskCode1 = 0x00;
			maskCode2 = 0x08;
			maskCode3 = 0x00;
			break;
		
		case 4:
			maskCode  = 0x00;
			maskCode1 = 0x08;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			break;
			
		case 5:
			maskCode  = 0x08;
			maskCode1 = 0x00;
			maskCode2 = 0x00;
			maskCode3 = 0x00;
			break;
		
		default:
			break;		
	}

	segCB.state[COM_NUMBER_COM4] = maskCode | (segCB.state[COM_NUMBER_COM4] & (~0xFF));
	segCB.state[COM_NUMBER_COM5] = maskCode1 | (segCB.state[COM_NUMBER_COM5] & (~0xFF));
	segCB.state[COM_NUMBER_COM6] = maskCode2 | (segCB.state[COM_NUMBER_COM6] & (~(0x01 << 3)));
	segCB.state[COM_NUMBER_COM7] = maskCode3 | (segCB.state[COM_NUMBER_COM7] & (~(0x01 << 3)));
}

// 一瓣界面
void SEGMENT_ShowUpDatingState(BOOL refreshAll)
{
	SEGMENT_DIS_BatteryQuantityState(6);
	
	if (paramCB.runtime.upDatingValue > 5)
	{
		return;
	}
	
	SEGMENT_DIS_OneFlowerState(paramCB.runtime.upDatingValue);
}

// 电量显示数值
void SEGMENT_BatteryShowMenu(BOOL refreshAll)
{
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// 找出电量格数
	// 0  <= x <  15  
	// 15 <= x <  30
	// 30 <= x <  45
	// 45 <= x <  60
	// 60 <= x <  80
	// 80 <= x <= 100
	
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}

	SEGMENT_DIS_BatteryQuantityState(batLevel);
}

// 预约
void SEGMENT_ShowReservedState(BOOL refreshAll)
{
	SEGMENT_DIS_ReservedState(bTRUE);
}

// 不可使用
void SEGMENT_ShowUnavailableState(BOOL refreshAll)
{
	SEGMENT_DIS_UnavailableState(PARAM_GetLimeUiErrorCode());
}

// 禁行
void SEGMENT_ShowNoRidingState(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// 找出电量格数
	// 0  <= x <  15  
	// 15 <= x <  30
	// 30 <= x <  45
	// 45 <= x <  60
	// 60 <= x <  80
	// 80 <= x <= 100
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}


	if (0 == nowSpeed)
	{
		speedLevel = 0;
	}
	else if (nowSpeed < Level)
	{
		speedLevel = 1;
	}
	else if (nowSpeed < Level*2)
	{
		speedLevel = 2;
	}
	else if (nowSpeed < Level*3)
	{
		speedLevel = 3;
	}
	else if (nowSpeed < Level*4)
	{
		speedLevel = 4;
	}
	else if (nowSpeed < Level*5)
	{
		speedLevel = 5;
	}
	else
	{
		speedLevel = 6;
	}

	SEGMENT_DIS_UnavailableState(bFALSE);
	
	SEGMENT_DIS_PetalsState(batLevel);

	SEGMENT_DIS_BatteryQuantityState(speedLevel);

	
}

// 禁停
void SEGMENT_ShowNoParkState(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// 找出电量格数
	// 0  <= x <  15  
	// 15 <= x <  30
	// 30 <= x <  45
	// 45 <= x <  60
	// 60 <= x <  80
	// 80 <= x <= 100
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}

	if (0 == nowSpeed)
	{
		speedLevel = 0;
	}
	else if (nowSpeed < Level)
	{
		speedLevel = 1;
	}
	else if (nowSpeed < Level*2)
	{
		speedLevel = 2;
	}
	else if (nowSpeed < Level*3)
	{
		speedLevel = 3;
	}
	else if (nowSpeed < Level*4)
	{
		speedLevel = 4;
	}
	else if (nowSpeed < Level*5)
	{
		speedLevel = 5;
	}
	else
	{
		speedLevel = 6;
	}
	
	SEGMENT_DIS_PetalsState(batLevel);

	SEGMENT_DIS_BatteryQuantityState(speedLevel);

	SEGMENT_DIS_NoParkState(bTRUE);
}

// 待机
void SEGMENT_ShowWaitState(BOOL refreshAll)
{
	SEGMENT_DIS_WaitState(bTRUE);
}

// 低速骑行界面
void SEGMENT_ShowLowSpeedGUI(BOOL refreshAll)
{
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// 找出电量格数
	// 0  <= x <  15  
	// 15 <= x <  30
	// 30 <= x <  45
	// 45 <= x <  60
	// 60 <= x <  80
	// 80 <= x <= 100
	
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}
	SEGMENT_DIS_LowSpeedState(bTRUE);
	SEGMENT_DIS_PetalsState(batLevel);
	
}

// 正常骑行界面
void SEGMENT_ShowRidingNormalGUI(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// 找出电量格数
	// 0  <= x <  15  
	// 15 <= x <  30
	// 30 <= x <  45
	// 45 <= x <  60
	// 60 <= x <  80
	// 80 <= x <= 100
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}

	if (0 == nowSpeed)
	{
		speedLevel = 0;
	}
	else if (nowSpeed < Level)
	{
		speedLevel = 1;
	}
	else if (nowSpeed < Level*2)
	{
		speedLevel = 2;
	}
	else if (nowSpeed < Level*3)
	{
		speedLevel = 3;
	}
	else if (nowSpeed < Level*4)
	{
		speedLevel = 4;
	}
	else if (nowSpeed < Level*5)
	{
		speedLevel = 5;
	}
	else
	{
		speedLevel = 6;
	}
	
	SEGMENT_DIS_PetalsState(batLevel);

	SEGMENT_DIS_BatteryQuantityState(speedLevel);
}

// 充电界面
void SEGMENT_ShowCharingGUI(BOOL refreshAll)
{
	uint8 localParam = PARAM_GetBatteryPercent();
	uint8 batLevel;
	
	// 找出电量格数
	// 0  <= x <=  15  
	// 16 <= x <=  35
	// 36 <= x <=  55
	// 56 <= x <=  70
	// 71 <= x <=  85
	// 86 <= x <= 100
	if (localParam == 0)
	{
		batLevel = 0;
	}
	else if (localParam <= 15)
	{
		batLevel = 1;
	}
	else if (localParam <= 35)
	{
		batLevel = 2;
	}
	else if (localParam <= 50)
	{
		batLevel = 3;
	}
	else if (localParam <= 65)
	{
		batLevel = 4;
	}
	else if (localParam <= 85)
	{
		batLevel = 5;
	}
	else if (localParam <= 100)
	{
		batLevel = 6;
	}
	else
	{
		batLevel = 6;
	}
	
	if (!refreshAll)
	{
		batLevel = paramCB.runtime.battery.batLeve;
	}
	
	SEGMENT_DIS_BatteryQuantityState(6);
	
	SEGMENT_DIS_PetalsState(batLevel);
}

// 使能显示模块
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable)
{
	DISPLAY_ITEM_E i;

	// 禁能或使能所有显示单元
	if (DISPLAY_MAX == eUnit)	
	{
		for (i = (DISPLAY_ITEM_E)0; i < DISPLAY_MAX; i++)
		{
			segCB.dispCtrl[i].enable = enable;
		}

		return;
	}

	// 禁能或使能指定显示单元
	segCB.dispCtrl[eUnit].enable = enable;
}

// 设置显示参数
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param)
{
	// 更新当前参数
	segCB.dispCtrl[eUnit].param = param;
}











// 横向数码管跑马灯数组
/*static uint8 segControl[30] = {
	0x00, 	// 全灭
	0x00,		// R1 : 第1行第1个
	0x00010000,		// R2 : 第1行第2个
	0x00020000,		// W1 : 第1行第3个
	0x00040000,		// W2 : 第2行第1个
	
	0x00000001,		// A1 : 第3行第1个
	0x00000100,		// A2 : 第3行第2个
	0x00000020,		// F1 : 第4行第1个
	0x00000002,		// B1 : 第4行第2个	
	0x00002000,		// F2 : 第4行第3个
	
	0x00000200,		// B2 : 第4行第4个	
	0x00000040,		// G1 : 第5行第1个
	0x00004000,		// G2 : 第5行第2个
	0x00000010,		// E1 : 第6行第1个
	0x00000004,		// C1 : 第6行第2个
	
	0x00001000,		// E2 : 第6行第3个
	0x00000400,		// C2 : 第6行第4个
	0x00000008,		// D1 : 第7行第1个
	0x00000800,		// D2 : 第7行第2个
	0x00008000,		// W5 : 第7行第3个
	
	0x00080000,		// W3 : 第8行第1个
	0x00100000,		// W4 : 第8行第2个
	0x07600000,		// W10: 第9行第1个
	0x08000000,		// W6 : 第10行第1个
	0x10000000,		// W7 : 第10行第2个
	
	0x20000000,		// W8 : 第10行第3个
	0x40000000,		// W9 : 第10行第4个
	0xFFFFFFFF,	// 
	0x55555555,	// 
	0xAAAAAAAA,	// 
	0
	
};*/

static uint8 segControl[10] = {
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
};


uint8 segRunning = 0;

// 显示测试函数
void SEGMENT_DisplayTest(uint32 param)
{	
	if (30 == param)
	{
		PARAM_SetSegContrl(segControl[segRunning]);
	    SEGMENT_DIS_AllState(segControl);
	    segRunning ++;
	    if (28 <= segRunning)
	    {
			segRunning = 0;
	    }
	}
	else
	{
		segRunning = 0;			
		PARAM_SetSegContrl(segControl[param]);
		SEGMENT_DIS_AllState(segControl);
	}
}
