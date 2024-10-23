/** ******************************************************************************
  * 文 件 名: Param.h
  * 版 本 号: 初版
  * 修改作者: ZJ & Workman & Fly
  * 修改日期: 2017年08月06日
  * 功能介绍: 该文件用于骑行参数的获取与计算，提供可靠的数据给上层显示或传输
  ******************************************************************************
  * 注意事项:
  *
  * 							版权归迪太科技所有.
  *
*********************************************************************************/

/****************************** 版权归迪太科技所有 *********** 文件开始 *********/
#ifndef 	__PARAM_H__
#define 	__PARAM_H__

/*******************************************************************************
 *                                  头文件	                                   *
********************************************************************************/
#include "common.h"
#include "uartProtocol.h"
#include "pwmLed.h"
#include "iap.h"
/*******************************************************************************
 *                                  宏定义	                                   *
********************************************************************************/
#define PARAM_MCU_PAGE_NUM					64
#define PARAM_MCU_PAGE_SIZE					2048UL

// SN地址
#define PARAM_MCU_SN_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 0)

// LIME SN地址
#define PARAM_MCU_LIME_SN_ADDEESS			(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 32)

// 硬件版本地址
#define PARAM_MCU_HW_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 64)

// BOOT版本地址
#define PARAM_MCU_BOOT_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 96)

// APP版本地址
#define PARAM_MCU_APP_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 128)

// 测试标识版本地址
#define PARAM_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 320)

static uint32 PARAM_MCU_VERSION_ADDRESS[6] = {PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS, \
											PARAM_MCU_SN_ADDEESS, PARAM_MCU_LIME_SN_ADDEESS, PARAM_MCU_HW_ADDEESS}; 

// 显示单位定义枚举定义
typedef enum
{
	UNIT_METRIC = 0,			// 公制
	UNIT_INCH,					// 英制

	UINT_COUNT	
}UNIT_E;


// 通讯错误码
#define PARAM_COMMUNICATION_ERROR_CODE					10

// 低压保护错误码
#define PARAM_UNDERVOLTAGE_PROTECTTION_ERROR_CODE		2

// 无错误错误码
#define PARAM_NO_ERROR_ERROR_CODE		0

#define METRIC_TO_INCH_COFF		(0.621371f)	// 1 km = 0.621371192237334 inch

// 电池电量数据源
typedef enum
{
	BATTERY_DATA_SRC_ADC = 0,				// HMI根据电压计算方式
	BATTERY_DATA_SRC_BMS,					// BMS上报

	BATTERY_DATA_SRC_COUNT
}BATTERY_DATA_SRC_E;

// 系统状态
typedef enum
{
	SYSTEM_STATE_STOP = 0,

	SYSTEM_STATE_RUNNING,

	SYSTEM_STATE_MAX
}SYSTEM_STATE;

// LED动作模式
typedef enum
{
	PARAM_LED_MODE_ALL_OFF = 0,					// 常灭
	PARAM_LED_MODE_ALL_ON = 1,					// 常亮
	PARAM_LED_MODE_BLINK = 2,					// 闪烁
	PARAM_LED_MODE_BREATH = 3,					// 呼吸
	PARAM_LED_MODE_BREATH_50 = 4,				// 呼吸 最亮50%亮度
	PARAM_LED_MODE_BREATH_20 = 5,				// 呼吸 最亮20%亮度

	PARAM_LED_MODE_ALL_ON_50 = 6,				// 常亮 50%亮度
	PARAM_LED_MODE_ALL_ON_20 = 7,				// 常亮 20%亮度

	PARAM_LED_MODE_ALL_OFF_50 = 0x16,			// 常灭 50%亮度(预留)
	PARAM_LED_MODE_ALL_OFF_20 = 0x17,			// 常灭 20%亮度(预留)

	PARAM_LED_MODE_INVALID = 0xFF,				// 无效的模式
}PARAM_LED_MODE_E;


// ■■系统注册表，掉电保存在NVM中■■
typedef struct
{
	uint8 temp;
	BOOL uartLevel;
}NVM_CB;

// NVM区尺寸
#define PARAM_NVM_DATA_SIZE						sizeof(NVM_CB)

// ■■运行过程中数据，掉电不保存■■
typedef struct
{
	// SN号
	struct
	{
		uint8 snLenth;
		uint8 snStr[30+1];
	}SnCode;

	// LIME SN号
	struct
	{
		uint8 LimeSnLenth;
		uint8 LimeSnStr[30+1];
	}LimeSnCode;

	// 硬件版本号
	struct
	{
		uint8 hwVersionLenth;
		uint8 hwVersion[30+1];
	}hwVersion;

	// BOOT版本号
	struct
	{
		uint8 bootVersionLenth;
		uint8 bootVersion[30+1];
	}bootVersion;

	// APP版本号
	struct
	{
		uint8 appVersionLenth;
		uint8 appVersion[30+1];
	}appVersion;
	
	// 仪表唯一序列号
	struct
	{
		uint32 sn0;
		uint32 sn1;
		uint32 sn2;
		uint8 snStr[24+1];
	}watchUid;

	struct
	{
		uint8 tipPic;

		uint8 ridingWarningPic;

		uint8 updataPercent;

		uint8 errorCode;
		
		uint8 phase;
		uint8 ridingError;
		
	}limeUi;
	uint8 interfaceCode;	

	// 测试标志
	uint32 testflag[16];

	uint32 ageTestFlag;
	uint8 pucTestFlag;

	// LED运行模式
	PARAM_LED_MODE_E ledMode[LED_NAME_MAX];

	uint32 segContrl;
	uint8 nfcFlag;
	uint8 matchingResults;
	// 电池参数
	struct
	{
		uint16 voltage;							// 当前电池电压，单位:mV
		uint16 current; 						// 当前电池电流，单位:mA
		uint8 lowVoltageAlarm;					// 电池容量低告警
		uint8 percent;							// 电池百分比
		uint8 number;							// 电池块数
		BATTERY_DATA_SRC_E batteryDataSrc;		// 电池容量数据源
		uint8 state;							// 电池状态
		uint8 preState;
		uint8 charingValue;
		uint8 batLeve;
		uint8 virPerLattice;
	}battery;

	uint16 ridingSpeedLimit;
	uint16 ridingSpeed;
	uint8 ridingSpeedUnit;
	uint8 upDatingValue;

	
	// 显示时间已校正标志
	BOOL displayTimeCorrectedFlag;
	// 显示时间已校正标志
	uint8 displayTimeMode;
	
	// 光电传感器参数
	struct
	{	
		uint16 sampleValue;

		BOOL varyDirection;

		BOOL enable;
	}photoSensor;

	struct
	{
		BOOL  ageTestEnable;			// 
		uint32 txCount;
		uint32 rxCount;
		uint8 ageTestResoult;
	}ageTest;

	struct
	{
		uint32 timeInterval;
		BOOL onOff;
	}stopMode;	
}RUNTIME_DATA_CB;

/*******************************************************************************
 *                                  结构体	                                   *
********************************************************************************/
// ■■ 码表骑行参数 : 分为两大类:保存NVM与不保存NVM ■■
typedef struct
{
	RUNTIME_DATA_CB runtime;

	union
	{
  		NVM_CB param;

		uint8 array[PARAM_NVM_DATA_SIZE];
	}nvm;
	uint8 preValue[PARAM_NVM_DATA_SIZE];

	BOOL nvmWriteRequest;			// NVM更新请求
	BOOL nvmWriteEnable;			// NVM更新使能

	// 参数计算标志
	BOOL recaculateRequest;		// 参数计算请求标志，TRUE:请求计算；FALSE:不需要计算
}PARAM_CB;

#define PARAM_SIZE	(sizeof(PARAM_CB))

extern PARAM_CB paramCB;		

/******************************************************************************
* 【外部接口宏】
******************************************************************************/
// 设置参数计算标志
#define PARAM_SetRecaculateRequest(a)		paramCB.recaculateRequest=a

// 数码管控制值
#define PARAM_SetSegContrl(a)				paramCB.runtime.segContrl=a
// 数码管控制值
#define PARAM_GetSegContrl()				(paramCB.runtime.segContrl)


//=======================================================================
#define PARAM_GetTestMode()						(paramCB.runtime.ageTest.ageTestEnable)

#define PARAM_SetTxCount(a)						paramCB.runtime.ageTest.txCount = a
#define PARAM_GetTxCount()						(paramCB.runtime.ageTest.txCount)

#define PARAM_SetRxCount(a)						paramCB.runtime.ageTest.rxCount = a
#define PARAM_GetRxCount()						(paramCB.runtime.ageTest.rxCount)

#define PARAM_SetAgeTest(a)						paramCB.runtime.ageTest.ageTestResoult = a
#define PARAM_GetAgeTest(a)						(paramCB.runtime.ageTest.ageTestResoult)


#define PARAM_SetStopModeTimeInterval(a)		paramCB.runtime.stopMode.timeInterval=a+100
#define PARAM_GetStopModeTimeInterval()		(paramCB.runtime.stopMode.timeInterval)

#define PARAM_SetStopModeOnOff(a)				paramCB.runtime.stopMode.onOff=a
#define PARAM_GetStopModeOnOff()				(paramCB.runtime.stopMode.onOff)

#define PARAM_SetLimeUiTipPic(a)				paramCB.runtime.limeUi.tipPic=a
#define PARAM_GetLimeUiTipPic()					(paramCB.runtime.limeUi.tipPic)

#define PARAM_SetLimeUiRidingWarningPic(a)		paramCB.runtime.limeUi.ridingWarningPic=a
#define PARAM_GetLimeUiRidingWarningPic()		(paramCB.runtime.limeUi.ridingWarningPic)

#define PARAM_SetLimeUiUpdataPercent(a)			paramCB.runtime.limeUi.updataPercent=a
#define PARAM_GetLimeUiUpdataPercent()			(paramCB.runtime.limeUi.updataPercent)

#define PARAM_SetLimeUiErrorCode(a)				paramCB.runtime.limeUi.errorCode=a
#define PARAM_GetLimeUiErrorCode()				(paramCB.runtime.limeUi.errorCode)


#define PARAM_SetRidingUiPhase(a)				paramCB.runtime.limeUi.phase=a
#define PARAM_GetRidingUiPhase()				(paramCB.runtime.limeUi.phase)

#define PARAM_SetRidingError(a)					paramCB.runtime.limeUi.ridingError=a
#define PARAM_GetRidingError()					(paramCB.runtime.limeUi.ridingError)

#define PARAM_SetInterfaceCode(a)				paramCB.runtime.interfaceCode=a
#define PARAM_GetInterfaceCode()				(paramCB.runtime.interfaceCode)

#define PARAM_SetAgeTestFlag(a)					paramCB.runtime.ageTestFlag=a
#define PARAM_GetAgeTestFlag()					(paramCB.runtime.ageTestFlag)

#define PARAM_SetPucTestFlag(a)					paramCB.runtime.pucTestFlag=a
#define PARAM_GetPucTestFlag()					(paramCB.runtime.pucTestFlag)


#define PARAM_SetLedMode(i,a)					paramCB.runtime.ledMode[i]=a
#define PARAM_GetLedMode(i)						(paramCB.runtime.ledMode[i])

// 设置电池百分比
#define PARAM_SetBatteryPercent(a)			paramCB.runtime.battery.percent=a
// 获取电池百分比
#define PARAM_GetBatteryPercent()			(paramCB.runtime.battery.percent)	

// 设置电池块数
#define PARAM_SetBatteryNumber(a)			paramCB.runtime.battery.number=a
// 获取电池块数
#define PARAM_GetBatteryNumber()			(paramCB.runtime.battery.number)


#define PARAM_SetVirBatteryLattice(a)		paramCB.runtime.battery.virPerLattice=a
#define PARAM_GetVirBatteryLattice()		(paramCB.runtime.battery.virPerLattice)

#define PARAM_SetRidingSpeedLimit(a)			paramCB.runtime.ridingSpeedLimit=a
#define PARAM_GetRidingSpeedLimit()				(paramCB.runtime.ridingSpeedLimit)

#define PARAM_SetRidingSpeed(a)					paramCB.runtime.ridingSpeed=a
#define PARAM_GetRidingSpeed()					(paramCB.runtime.ridingSpeed)

#define PARAM_SetRidingSpeedUnit(a)				paramCB.runtime.ridingSpeedUnit=a
#define PARAM_GetRidingSpeedUnit()				(paramCB.runtime.ridingSpeedUnit)
											

#define PARAM_SetUartLevel(a)							paramCB.nvm.param.uartLevel=a
#define PARAM_GetUartLevel()							(paramCB.nvm.param.uartLevel)

#define PARAM_SetMatchResults(a)			paramCB.runtime.matchingResults=a
#define PARAM_GetMatchResults()				(paramCB.runtime.matchingResults)

/******************************************************************************
* 【外部接口声明】
******************************************************************************/
// 模块初始化，完成模块初始化
void PARAM_Init(void);

// 模块过程处理
void PARAM_Process(void);

// LED功能测试
void PARAM_LedFunctionTest(uint32 param);

#endif

