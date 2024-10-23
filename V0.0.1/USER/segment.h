#ifndef     __SEGMENT_H__
#define     __SEGMENT_H__

#include "common.h"

#define SEG_COUNT    5


// �ṹ����ÿ�����ֶ�Ӧԭ��ͼ�е�����
typedef enum
{
	COM_NUMBER_COM0 = 0,	// ���ֶ�
	COM_NUMBER_COM1,		
	COM_NUMBER_COM2,		
	COM_NUMBER_COM3,	
	COM_NUMBER_COM4,		
	COM_NUMBER_COM5,		
	COM_NUMBER_COM6,		
	COM_NUMBER_COM7,		
	COM_NUMBER_COM8,		
	COM_NUMBER_COM9,
	
	COM_COUNT = 10,
}COM_BIT;

// �������
typedef enum
{
	SEG_BIT_FREE = 0,
	SEG_BIT_RESERVED,
	SEG_BIT_ERROR,
	SEG_BIT_LOWSPEED,
	SEG_BIT_NOPARKING,
	SEG_BIT_SPEED,
	SEG_BIT_BATTERY,
	
	SEG_BIT_ALL
}SEG_BIT_E;

typedef enum
{
	SEG_MASK_FREE 		= 1<<SEG_BIT_FREE,
	SEG_MASK_RESERVED	= 1<<SEG_BIT_RESERVED,
	SEG_MASK_ERROR		= 1<<SEG_BIT_ERROR,
	SEG_MASK_LOWSPEED	= 1<<SEG_BIT_LOWSPEED,
	SEG_MASK_NOPARKING	= 1<<SEG_BIT_NOPARKING,
	SEG_MASK_SPEED		= 1<<SEG_BIT_SPEED,
	SEG_MASK_BATTERY	= 1<<SEG_BIT_BATTERY,
		

	SEG_MASK_ALL 		= 1<<SEG_BIT_ALL,
}SEG_MASK_E;


// ��ʾ������ö�ٶ���
typedef enum
{
	DISPLAY_ALL,				// �����ȫ����ʾ
	
	DISPLAY_MAX
}DISPLAY_ITEM_E;


// ��ʾ���Ƶ�Ԫ
typedef struct
{
	BOOL enable;							// ��ʾ����

	uint32 param;
	void (*pService)(uint32 param);			// ����ָ��
}DISP_CTRL_CB;

typedef struct
{
	DISP_CTRL_CB dispCtrl[DISPLAY_MAX];
	
	// ��ֵ��
	volatile uint8 state[COM_COUNT];			// ����ֵ
	volatile uint8 onoff[COM_COUNT];			// ��������
	volatile uint8 blinkEnable[COM_COUNT];		// ��˸���룬��ˢ��ʱ��������ܵ���ֵ���ֵ���������
	volatile uint8 blinkPhase;

	volatile uint8 outputSeg[COM_COUNT];		// ����ֵ

	volatile uint16 mapData[SEG_COUNT];			// ����ֵ

	volatile uint8 refreshRequest;				// ˢ�±�־

	volatile uint8 currentScanCom;				// ��ǰ��ɨ����У�ȡֵ��Χ0-12������COM1-COM13

	uint8 RefreshDuty;							// ����ռ�ձȵ���
}SEGMENT_CB;

extern SEGMENT_CB segCB;


// ����ܳ�ʼ��
void SEGMENT_Init(void);

// ����ܹ��̴���
void SEGMENT_Process(void);

// �����ܿ���
void SEGMENT_SetGroupOnOff(SEG_MASK_E segMask, BOOL onoff);

// ������˸
void SEGMENT_SetGroupBlink(SEG_MASK_E segMask, BOOL onoff);

// �����ˢ�»ص�����
void SEGMENT_CALLBACK_Refresh(uint32 param);

// ʹ����ʾ��������
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable);

// ������ʾ����
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param);

//==========================================================
uint8 SEGMENT_UnitTest(void);

// ��ʾ���Ժ���
void SEGMENT_DisplayTest(uint32 param);

// ��ʾ LOGO (00 ������˸)
void SEGMENT_ShowLogoGui(BOOL refreshAll);

// ��ʾ ͨѶ���� (04 ���ֳ���)
void SEGMENT_ShowCommunicationErrorGui(BOOL refreshAll);

// ��ʾ��ʾ����
void SEGMENT_ShowTipGUI(BOOL refreshAll);

// ��ʾ�����ý���
void SEGMENT_ShowUnavailableGUI(BOOL refreshAll);

// ��ͣ/ԤԼ����
void SEGMENT_ShowPausedGUI(BOOL refreshAll);

// �����ʾ����
void SEGMENT_ShowTipChargingGUI(BOOL refreshAll);

// ����������ʾ
void SEGMENT_ShowRidingNormalGUI(BOOL refreshAll);

// �����쳣UI��ʾ
void SEGMENT_ShowRidingUnusualGUI(BOOL refreshAll);

// ����ҳ��
void SEGMENT_ShowUpdataGUI(BOOL refreshAll);

// �����ȫ����ʾ
void SEGMENT_DIS_AllState(uint8 segBuff[]);

// �������
void SEGMENT_DIS_PetalsState(uint32 param);

// ������ʾ
void SEGMENT_DIS_BatteryQuantityState(uint32 param);

// һ�����
void SEGMENT_ShowUpDatingState(BOOL refreshAll);

// ԤԼ
void SEGMENT_ShowReservedState(BOOL refreshAll);

// ����ʹ��
void SEGMENT_ShowUnavailableState(BOOL refreshAll);

// ��ͣ
void SEGMENT_ShowNoParkState(BOOL refreshAll);

// ����
void SEGMENT_ShowWaitState(BOOL refreshAll);

// �������н���
void SEGMENT_ShowLowSpeedGUI(BOOL refreshAll);

// ������
void SEGMENT_ShowCharingGUI(BOOL refreshAll);

// ��������ʾ
void SEGMENT_DIS_BatteryChargingState(uint32 param);

// ������ʾ��ֵ
void SEGMENT_BatteryShowMenu(BOOL refreshAll);

// ����
void SEGMENT_ShowNoRidingState(BOOL refreshAll);

	
#endif

