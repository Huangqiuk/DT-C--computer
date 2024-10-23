#include "common.h"
#include "segment.h"
#include "timer.h"
#include "delay.h"
#include "param.h"
#include "state.h"
#include "TM1628_Driver.h"
#include "string.h"
									
SEGMENT_CB segCB;

// ������ʾ����
// �����ȫ����ʾ
void SEGMENT_DIS_AllState(uint8 segBuff[]);

// ��ֹͣ��
void SEGMENT_DIS_NoParkState(uint32 param);

// ����
void SEGMENT_DIS_LowSpeedState(uint32 param);

// ��������
void SEGMENT_DIS_BatteryQuantityState(uint32 param);	

// ������ʾ
void SEGMENT_DIS_BatteryLowSpeedState(uint32 param);

// ʹ����ʾ��������
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable);

// ������ʾ����
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param);

// �����ˢ������ص�����
void SEGMENT_CALLBACK_RefreshRequest(uint32 param);

// ��˸����ص��������ɶ�ʱ������
void SEGMENT_CALLBACK_BlinkProcess(uint32 data);

// ��˸����ص��������ɶ�ʱ������
void SEGMENT_CALLBACK_BlinkProcess(uint32 data);

void SEGMENT_DIS_OneFlowerState(uint32 param);

// ����
void SEGMENT_DIS_WaitState(uint32 param);

// ����ʹ��
void SEGMENT_DIS_UnavailableState(uint32 param);

// ԤԼ
void SEGMENT_DIS_ReservedState(uint32 param);

// ����ͼ��
void SEGMENT_DIS_LowSpeedLogoState(uint32 param);

#if 1
// ��Ԫ���Ժ���
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

// �����ˢ�»ص�����
void SEGMENT_CALLBACK_Refresh(uint32 param)
{
	// ����ε�ֵд�뵽TM1628
	TM1628_DisplayAutoAddr(0x00, (UINT16*)segCB.mapData, SEG_COUNT);
}

// �����ˢ������ص�����
void SEGMENT_CALLBACK_RefreshRequest(uint32 param)
{
	segCB.refreshRequest = (BOOL)param;
}

// ��˸����ص��������ɶ�ʱ������
void SEGMENT_CALLBACK_BlinkProcess(uint32 data)
{
	// ��˸״̬��ת
	segCB.blinkPhase = ~segCB.blinkPhase;
}

// ����ܳ�ʼ��
void SEGMENT_Init(void)
{
	uint8 i;

	// ���������оƬ��ʼ��
	TM1628_Init();
	
	// ��˸��λ
//	TIMER_AddTask(TIMER_ID_SEGMENT_BLINK,
//					500,
//					SEGMENT_CALLBACK_BlinkProcess,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

//	// �����ˢ�¼��
//	TIMER_AddTask(TIMER_ID_SEGMENT_REFRESH_TIME,
//					100,
//					SEGMENT_CALLBACK_RefreshRequest,
//					bTRUE,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

	// ��ʾ���ݽṹ��ʼ��	
	segCB.blinkPhase = 0x00;

	for (i = 0; i < COM_COUNT; i++)
	{
		segCB.blinkEnable[i] = 0;
		segCB.state[i] = 0x00;
	}
	
	// �ܿ���
	SEGMENT_SetGroupOnOff(SEG_MASK_ALL, bFALSE);
//	segCB.mapData[0] = 0xFFFF;
//	segCB.mapData[1] = 0xFFFF;
//	segCB.mapData[2] = 0xFFFF;
//	segCB.mapData[3] = 0xFFFF;
//	segCB.mapData[4] = 0xFFFF;
//	
//	// ˢ�������
//	SEGMENT_CALLBACK_Refresh(bTRUE);
//	
//	while(1);
}

// ����ܴ�����
void SEGMENT_Process(void)
{
	uint8 j,i = 0;
	
	// ���ˢ�±�־
	if (segCB.refreshRequest)
	{
		// ���ˢ�±�־
		segCB.refreshRequest = bFALSE;
		
		// ����segCB�ṹ�� ����ֵ���������롢��˸���룬ȷ�� �������ֵ
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
		
		// ˢ�������
		SEGMENT_CALLBACK_Refresh(bTRUE);
	}
}

// �����ܿ���
void SEGMENT_SetGroupOnOff(SEG_MASK_E segMask, BOOL onoff)
{
	// ����
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
	
	// ��������
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

	// ԤԼ����
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

	// �������
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

	// ���ٽ���
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

	// ��ͣ����
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

	// �ٶȽ���
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

	// ��������
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

// �����ȫ����ʾ
void SEGMENT_DIS_AllState(uint8 segBuff[])
{		
	// ��ֵ��λ
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

// ����ͼ��
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

// ��ͣ
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

// ����ʹ��
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

// ԤԼ
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

// ����
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

// ����
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

// ������ʾ
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

// ������ʾ
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

// ��������ʾ
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

// �������
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

// һ�����
void SEGMENT_ShowUpDatingState(BOOL refreshAll)
{
	SEGMENT_DIS_BatteryQuantityState(6);
	
	if (paramCB.runtime.upDatingValue > 5)
	{
		return;
	}
	
	SEGMENT_DIS_OneFlowerState(paramCB.runtime.upDatingValue);
}

// ������ʾ��ֵ
void SEGMENT_BatteryShowMenu(BOOL refreshAll)
{
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// �ҳ���������
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

// ԤԼ
void SEGMENT_ShowReservedState(BOOL refreshAll)
{
	SEGMENT_DIS_ReservedState(bTRUE);
}

// ����ʹ��
void SEGMENT_ShowUnavailableState(BOOL refreshAll)
{
	SEGMENT_DIS_UnavailableState(PARAM_GetLimeUiErrorCode());
}

// ����
void SEGMENT_ShowNoRidingState(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// �ҳ���������
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

// ��ͣ
void SEGMENT_ShowNoParkState(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// �ҳ���������
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

// ����
void SEGMENT_ShowWaitState(BOOL refreshAll)
{
	SEGMENT_DIS_WaitState(bTRUE);
}

// �������н���
void SEGMENT_ShowLowSpeedGUI(BOOL refreshAll)
{
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// �ҳ���������
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

// �������н���
void SEGMENT_ShowRidingNormalGUI(BOOL refreshAll)
{
	uint8 speedLevel;
	uint16 nowSpeed = PARAM_GetRidingSpeed();
	uint16 maxSpeed = PARAM_GetRidingSpeedLimit();
	uint16 Level = (maxSpeed / 6);
	uint8 batLevel;
	uint8 localParam = PARAM_GetBatteryPercent();
	
	// �ҳ���������
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

// ������
void SEGMENT_ShowCharingGUI(BOOL refreshAll)
{
	uint8 localParam = PARAM_GetBatteryPercent();
	uint8 batLevel;
	
	// �ҳ���������
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

// ʹ����ʾģ��
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable)
{
	DISPLAY_ITEM_E i;

	// ���ܻ�ʹ��������ʾ��Ԫ
	if (DISPLAY_MAX == eUnit)	
	{
		for (i = (DISPLAY_ITEM_E)0; i < DISPLAY_MAX; i++)
		{
			segCB.dispCtrl[i].enable = enable;
		}

		return;
	}

	// ���ܻ�ʹ��ָ����ʾ��Ԫ
	segCB.dispCtrl[eUnit].enable = enable;
}

// ������ʾ����
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param)
{
	// ���µ�ǰ����
	segCB.dispCtrl[eUnit].param = param;
}











// ������������������
/*static uint8 segControl[30] = {
	0x00, 	// ȫ��
	0x00,		// R1 : ��1�е�1��
	0x00010000,		// R2 : ��1�е�2��
	0x00020000,		// W1 : ��1�е�3��
	0x00040000,		// W2 : ��2�е�1��
	
	0x00000001,		// A1 : ��3�е�1��
	0x00000100,		// A2 : ��3�е�2��
	0x00000020,		// F1 : ��4�е�1��
	0x00000002,		// B1 : ��4�е�2��	
	0x00002000,		// F2 : ��4�е�3��
	
	0x00000200,		// B2 : ��4�е�4��	
	0x00000040,		// G1 : ��5�е�1��
	0x00004000,		// G2 : ��5�е�2��
	0x00000010,		// E1 : ��6�е�1��
	0x00000004,		// C1 : ��6�е�2��
	
	0x00001000,		// E2 : ��6�е�3��
	0x00000400,		// C2 : ��6�е�4��
	0x00000008,		// D1 : ��7�е�1��
	0x00000800,		// D2 : ��7�е�2��
	0x00008000,		// W5 : ��7�е�3��
	
	0x00080000,		// W3 : ��8�е�1��
	0x00100000,		// W4 : ��8�е�2��
	0x07600000,		// W10: ��9�е�1��
	0x08000000,		// W6 : ��10�е�1��
	0x10000000,		// W7 : ��10�е�2��
	
	0x20000000,		// W8 : ��10�е�3��
	0x40000000,		// W9 : ��10�е�4��
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

// ��ʾ���Ժ���
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
