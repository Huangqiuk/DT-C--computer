#ifndef     __SEGMENT_H__
#define     __SEGMENT_H__

#include "common.h"

#define SEG_COUNT    5


// 结构体中每个数字对应原理图中的名称
typedef enum
{
	COM_NUMBER_COM0 = 0,	// 数字段
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

// 组合名称
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


// 显示函数项枚举定义
typedef enum
{
	DISPLAY_ALL,				// 数码管全部显示
	
	DISPLAY_MAX
}DISPLAY_ITEM_E;


// 显示控制单元
typedef struct
{
	BOOL enable;							// 显示开关

	uint32 param;
	void (*pService)(uint32 param);			// 函数指针
}DISP_CTRL_CB;

typedef struct
{
	DISP_CTRL_CB dispCtrl[DISPLAY_MAX];
	
	// 数值表
	volatile uint8 state[COM_COUNT];			// 段数值
	volatile uint8 onoff[COM_COUNT];			// 开关掩码
	volatile uint8 blinkEnable[COM_COUNT];		// 闪烁掩码，在刷新时，将数码管的数值与该值相异或后输出
	volatile uint8 blinkPhase;

	volatile uint8 outputSeg[COM_COUNT];		// 段数值

	volatile uint16 mapData[SEG_COUNT];			// 段数值

	volatile uint8 refreshRequest;				// 刷新标志

	volatile uint8 currentScanCom;				// 当前被扫描的列，取值范围0-12，代表COM1-COM13

	uint8 RefreshDuty;							// 亮度占空比调节
}SEGMENT_CB;

extern SEGMENT_CB segCB;


// 数码管初始化
void SEGMENT_Init(void);

// 数码管过程处理
void SEGMENT_Process(void);

// 设置总开关
void SEGMENT_SetGroupOnOff(SEG_MASK_E segMask, BOOL onoff);

// 设置闪烁
void SEGMENT_SetGroupBlink(SEG_MASK_E segMask, BOOL onoff);

// 数码管刷新回调函数
void SEGMENT_CALLBACK_Refresh(uint32 param);

// 使能显示函数开关
void SEGMENT_SetItemEnable(DISPLAY_ITEM_E eUnit, BOOL enable);

// 设置显示参数
void SEGMENT_UpdataParam(DISPLAY_ITEM_E eUnit, uint32 param);

//==========================================================
uint8 SEGMENT_UnitTest(void);

// 显示测试函数
void SEGMENT_DisplayTest(uint32 param);

// 显示 LOGO (00 数字闪烁)
void SEGMENT_ShowLogoGui(BOOL refreshAll);

// 显示 通讯错误 (04 数字常亮)
void SEGMENT_ShowCommunicationErrorGui(BOOL refreshAll);

// 显示提示界面
void SEGMENT_ShowTipGUI(BOOL refreshAll);

// 显示不可用界面
void SEGMENT_ShowUnavailableGUI(BOOL refreshAll);

// 暂停/预约界面
void SEGMENT_ShowPausedGUI(BOOL refreshAll);

// 充电提示界面
void SEGMENT_ShowTipChargingGUI(BOOL refreshAll);

// 骑行正常显示
void SEGMENT_ShowRidingNormalGUI(BOOL refreshAll);

// 骑行异常UI显示
void SEGMENT_ShowRidingUnusualGUI(BOOL refreshAll);

// 升级页面
void SEGMENT_ShowUpdataGUI(BOOL refreshAll);

// 数码管全部显示
void SEGMENT_DIS_AllState(uint8 segBuff[]);

// 花瓣界面
void SEGMENT_DIS_PetalsState(uint32 param);

// 电量显示
void SEGMENT_DIS_BatteryQuantityState(uint32 param);

// 一瓣界面
void SEGMENT_ShowUpDatingState(BOOL refreshAll);

// 预约
void SEGMENT_ShowReservedState(BOOL refreshAll);

// 不可使用
void SEGMENT_ShowUnavailableState(BOOL refreshAll);

// 禁停
void SEGMENT_ShowNoParkState(BOOL refreshAll);

// 待机
void SEGMENT_ShowWaitState(BOOL refreshAll);

// 低速骑行界面
void SEGMENT_ShowLowSpeedGUI(BOOL refreshAll);

// 充电界面
void SEGMENT_ShowCharingGUI(BOOL refreshAll);

// 充电界面显示
void SEGMENT_DIS_BatteryChargingState(uint32 param);

// 电量显示数值
void SEGMENT_BatteryShowMenu(BOOL refreshAll);

// 禁行
void SEGMENT_ShowNoRidingState(BOOL refreshAll);

	
#endif

