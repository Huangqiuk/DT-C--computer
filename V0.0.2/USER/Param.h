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
#include "uartprotocol.h"
/*******************************************************************************
 *                                  宏定义	                                   *
********************************************************************************/
#define PARAM_MCU_PAGE_NUM					64
#define PARAM_MCU_PAGE_SIZE					2048UL

// DT SN地址
#define PARAM_MCU_DT_SN_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 0)

// 客户SN地址
#define PARAM_MCU_CUSTOMER_SN_ADDEESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 32)

// 硬件版本地址
#define PARAM_MCU_HW_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 64)

// BOOT版本地址
#define PARAM_MCU_BOOT_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 96)

// APP版本地址
#define PARAM_MCU_APP_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 128)

// APP升级标志地址
#define PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 160)

// APP升级成功标志地址
#define PARAM_UPDATA_SUCCESS_FLAG_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 168)

// APP升级CRC地址
#define PARAM_MCU_BLE_UPDATA_CRC_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 176)

// 二维码地址
#define PARAM_MCU_QR_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 184)

// 测试标识版本地址
#define PARAM_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 328)

#define NVM_FORMAT_FLAG_STRING				"202012101277"
#define NVM_FORMAT_FLAG_SIZE				sizeof(NVM_FORMAT_FLAG_STRING)

// NVM参数在FLASH中的首地址
#define PARAM_NVM_START_ADDRESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 2)*PARAM_MCU_PAGE_SIZE)

// NVM参数在FLASH中的备用地址
#define PARAM_NVM_BACKUP_START_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 3)*PARAM_MCU_PAGE_SIZE)

// 恢复出厂设置的备份区域
#define PARAM_NVM_FACTORY_RESET_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 512)

// 骑行记录最大条数
//#define RECORD_COUNT_MAX					10


// 显示单位定义枚举定义
typedef enum
{
	UNIT_METRIC = 0,			// 公制
	UNIT_INCH,					// 英制

	UINT_COUNT	
}UNIT_E;

// 电动自行车开机界面显示码
#define PARAM_E_BIKE_CODE						0x0E0B
// 电动滑板车开机界面显示码
#define PARAM_E_HUA_CODE						0x0E05
// 升级显示字符UP
#define PARAM_E_UPDATA_CODE						0x1112

#define METRIC_TO_INCH_COFF		(0.621371f)	// 1 km = 0.621371192237334 inch

// 电池电量数据源
typedef enum
{
	//BATTERY_DATA_SRC_ADC = 0,				// 电压由仪表测量
	
	BATTERY_DATA_SRC_CONTROLLER,			// 电压由控制器上报,仪表计算电量
	BATTERY_DATA_SRC_BMS,					// 电量直接由控制器上报
	BATTERY_DATA_SRC_ADC,					// 电压电量由仪表测量

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

// 设置轮径ID
typedef enum{
	//PARAM_WHEEL_SIZE_4_5_INCH = 0,
	//PARAM_WHEEL_SIZE_6_INCH,
	//PARAM_WHEEL_SIZE_6_4_INCH,
	//PARAM_WHEEL_SIZE_8_INCH,
	//PARAM_WHEEL_SIZE_10_INCH,
	PARAM_WHEEL_SIZE_12_INCH,
	PARAM_WHEEL_SIZE_14_INCH,
	PARAM_WHEEL_SIZE_16_INCH,
	PARAM_WHEEL_SIZE_18_INCH,
	PARAM_WHEEL_SIZE_20_INCH,
	PARAM_WHEEL_SIZE_22_INCH,
	PARAM_WHEEL_SIZE_24_INCH,
	PARAM_WHEEL_SIZE_26_INCH,
	PARAM_WHEEL_SIZE_27_INCH,
	PARAM_WHEEL_SIZE_27_5_INCH,
	PARAM_WHEEL_SIZE_28_INCH,
	PARAM_WHEEL_SIZE_29_INCH,
	PARAM_WHEEL_SIZE_700C,
	
	PARAM_WHEEL_SIZE_MAX
}PARAM_WHEEL_SIZE_ID;

// 错误代码类型定义
typedef enum
{
	ERROR_TYPE_NO_ERROR = 0x00,							// 无错误
	ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x20,			// 通讯接收超时
	ERROR_TYPE_BAT_ERROR = 0x10,						// 电池故障
	ERROR_TYPE_DRIVER_ERROR = 0x08,						// 控制器故障
	ERROR_TYPE_HALL_ERROR = 0x04,						// 霍尔故障
	ERROR_TYPE_BREAK_ERROR = 0x02,						// 刹车故障
	ERROR_TYPE_TURN_ERROR = 0x01,						// 转把故障	
	
	//ERROR_TYPE_BATTERY_UNDER_VOLTAGE_ERROR = 6,		// 电池欠压
	//ERROR_TYPE_MOTOR_ERROR = 0x08,					// 电机故障
	//ERROR_TYPE_TURN_ERROR = 8,						// 转把故障
	//ERROR_TYPE_THROTTLE_ERROR = 0x40,					// 油门故障
	
}ERROR_TYPE_E;

// 助力档位枚举定义
typedef enum
{
	ASSIST_0 = 0,
	ASSIST_1,
	ASSIST_2,
	ASSIST_3,
	ASSIST_4,
	ASSIST_5,
	ASSIST_6,
	ASSIST_7,
	ASSIST_8,
	ASSIST_9,
	ASSIST_CRUISE,
	ASSIST_P,

	ASSIST_ID_COUNT
}ASSIST_ID_E;

// 产品类型定义
typedef enum
{
	HMI_TYPE_BC18EU = 0,				// 迪太码表BC18，TFT屏，UART通讯
	HMI_TYPE_BC18SU,					// 迪太码表BC18，TFT屏，BLE，UART通讯
	HMI_TYPE_BC28EU,					// 迪太码表BC28，TFT屏，UART通讯
	HMI_TYPE_BC28SU,					// 迪太码表BC28，TFT屏，BLE，UART通讯
	HMI_TYPE_BC28NU,					// 迪太码表BC28，TFT屏，BLE，GSM，GPS，UART通讯
	HMI_TYPE_BC18EC,					// 天腾码表BC18，TFT屏，CAN通讯
	HMI_TYPE_BC28EC,					// 天腾码表BC28，TFT屏，CAN通讯

	
	HMI_TYPE_BN400,						// 数码管仪表BN400, 数码管显示, UART通讯

	HMI_TYPE_MAX,
}HMI_TYPE_E;


// ■■系统注册表，掉电保存在NVM中■■
typedef struct
{
	// 系统参数
	struct
	{
		PARAM_WHEEL_SIZE_ID wheelSizeID;	// 轮径ID
		BOOL resFactorySet;					// 恢复到出厂设置

		uint8 rgbSwitch;				// RGB灯开关	1-开启氛围灯，0-关闭氛围灯
		uint8 ledRgb;						// 控制器LED灯的RGB颜色	0-255 代表不同颜色（用于单色呼吸时调整呼吸颜色）
		uint8 rgbMode;					// RGB灯模式设置	0：关闭	1：单色呼吸	2：全彩呼吸	3：变色呼吸	4：流星模式	5：警灯模式
		uint8 headlight;				// 大灯
		uint8 dayLight;					// 日行灯
		uint8 leftLight;				// 左转灯
		uint8 rightLight;				// 右转灯
		uint16 powerMax;				// 最大电机功率,单位:W
		
		// 驱动器设置参数
		struct
		{
			uint8 steelNumOfSpeedSensor;		// 测试传感器一圈磁钢数(即 测速磁钢)
			uint16 currentLimit; 				// 电流门限，单位:mA
			uint16 lowVoltageThreshold;			// 欠压门限，单位:mV

			uint8 controlMode;					// 驱动器控制模式
			BOOL zeroStartOrNot;				// 零启动或非零启动
			BOOL switchCruiseWay;				// 切换巡航的方式
			BOOL switchCruiseMode;				// 巡航切换模式
			uint8 assistSensitivity;			// 助力灵敏度
			uint8 assistStartIntensity;			// 助力启动强度
			uint8 assistSteelType;				// 助力磁钢类型
			uint8 reversalHolzerSteelNum;		// 限速电机换向霍尔磁钢数
			BOOL speedLimitSwitch;				// 限速位开关
			BOOL CruiseEnabled;					// 巡航使能

			BOOL breakType;						// 刹车类型
		}driver;
	}protocol;

	// 公共参数
	struct
	{
		//HMI_TYPE_E hmiType;					// 码表类型
		UNIT_E unit;						// 单位, 公制与英制 	
		ASSIST_ID_E assistMax;				// 最大助力档位
		ASSIST_ID_E assist; 				// 助力档位
		
		uint32 busAliveTime;				// 通讯故障超时时间，单位:ms
		uint8  brightness;					// 背光亮度
		uint8  powerOffTime; 				// 自动关机时间，单位:分钟
		BOOL existBle;						// 是否存在蓝牙

		uint8  workAssistRatio;				// 机器与人做功助力比

		uint8 percentageMethod;				// 电量计算方式

		BOOL lockFlag; 						// 锁的状态
		
		// 速度类
		struct
		{
			uint16 limitVal;				// 限速门限，单位:0.1Km/h
			uint8  filterLevel;				// 速度平滑度 111
		}speed;

		// 电池类
		struct
		{
			uint8 voltage;					// 电池供电电压，单位:V
			uint8 filterLevel;				// 电量变化时间，单位:秒
		}battery;
		
		struct
		{
			// 自出厂以来的统计数据
			struct
			{
				uint32 distance;			// 出厂以来总里程，单位:0.1Km				
				uint32 preDistance;			// 出厂以来总里程，单位:0.1Km
				uint16 maxTripOf24hrs;		// 个人记录，单位:0.1Km
				uint32 ridingTime;			// 骑行时间，单位:s
				uint32 calories;			// 出厂以来总卡路里，单位:KCal
			}total;

			// 自Trip清零以来的统计数据
			struct
			{
				uint32 distance;			// Trip，单位:0.1Km
				uint32 calories;			// 卡路里，单位:KCal
				uint16 speedMax;			// 最大速度，单位:0.1Km/h

				uint32 ridingTime;			// 总骑行时间秒
			}trip;

			// 自0点以来的统计数据
			struct
			{
				uint16 trip;				// 今日里程，单位:0.1Km 	
				uint16 calories;			// 今日卡路里，单位:KCal

				uint32 ridingTime;			// 今日骑行累计时间，单位:s

				uint16 year;				// 今日参数对应的日期
				uint8 month; 				// 今日参数对应的日期
				uint8 day;					// 今日参数对应的日期
			}today;

		}record;

		// 放到最后，可以保证只有前面的值都写完之后才开始写格式化标识字符串
		uint8 nvmFormat[NVM_FORMAT_FLAG_SIZE];			// NVM格式化标识
	}common;
}NVM_CB;

// NVM区尺寸
#define PARAM_NVM_DATA_SIZE						sizeof(NVM_CB)

// ■■运行过程中数据，掉电不保存■■
typedef struct
{
	// DT SN号
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

	// 测试标志
	uint32 testflag[16];

	uint32 ageTestFlag;
	uint8 pucTestFlag;

	uint32 segContrl;

	BOOL jumpFlag;
	
	uint8 devVersion[3];					// 仪表型号
	uint8 fwBinNum;							// 固件版本编号

	struct
	{
		uint8 devVersionRemain;				// 设备发布版本号(需求版本号)
		uint8 devReleaseVersion;			// 软件升级更改发布版本号(产品定义版本号)
		uint8 fwUpdateVersion;				// 调试发布版本号(方案版本号)
		uint8 fwDebugReleaseVersion;		// 调试临时版本号
	}fwBinVersion;
	
	uint16 perimeter[PARAM_WHEEL_SIZE_MAX];	// 轮子周长，单位:mm
	
	BOOL bleConnectState;
	
	uint8 bleRadio; 

	BOOL RgbLedCtrlFlag; //氛围灯设置状态
	uint8 blueNumber;
	
	uint8 carNum[14]; //车辆序列号
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

		uint32 array[(PARAM_NVM_DATA_SIZE + 3) /4];
	}nvm;
	uint32 preValue[(PARAM_NVM_DATA_SIZE + 3) /4];

	BOOL nvmWriteRequest;			// NVM更新请求
	BOOL nvmWriteEnable;			// NVM更新使能

	// 参数计算标志
	BOOL recaculateRequest;		// 参数计算请求标志，TRUE:请求计算；FALSE:不需要计算
}PARAM_CB;

#define PARAM_SIZE	(sizeof(PARAM_CB))

extern PARAM_CB paramCB;		
extern const uint32 PARAM_MCU_VERSION_ADDRESS[];

/******************************************************************************
* 【外部接口宏】
******************************************************************************/
// 设置参数计算标志
#define PARAM_SetRecaculateRequest(a)			paramCB.recaculateRequest=a

// 设置巡航开关，0:表示关闭；1:表示打开
#define PARAM_SetCruiseSwitch(enable)			paramCB.runtime.cruiseEnabe=enable
// 获取巡航开关，0:表示关闭；1:表示打开
#define PARAM_GetCruiseSwitch()					(paramCB.runtime.cruiseEnabe)

// 设置大灯开关 
#define PARAM_SetLightSwitch(enable)		do{\
												paramCB.runtime.lightSwitch=enable;\
											}while(0)
// 获取大灯开关
#define PARAM_GetLightSwitch()				(paramCB.runtime.lightSwitch)

// 设置尾灯开关 
#define PARAM_SetTailLightSwitch(a)				paramCB.runtime.tailLight=a

// 获取尾灯开关 
#define PARAM_GetTailLightSwitch()				(paramCB.runtime.tailLight)



// 打开或关闭推车助力
#define PARAM_EnablePushAssist(enable)		do{\
												paramCB.runtime.pushAssistOn=enable;\
											}while(0)
// 获取推车助力功能状态
#define PARAM_IsPushAssistOn()				(paramCB.runtime.pushAssistOn)

// 数码管控制值
#define PARAM_SetSegContrl(a)					paramCB.runtime.segContrl=a
// 数码管控制值
#define PARAM_GetSegContrl()					(paramCB.runtime.segContrl)

// 设置蓝牙是否连接状态
#define	PARAM_SetBleConnectState(a)			paramCB.runtime.bleConnectState=a
// 获取蓝牙是否连接状态
#define	PARAM_GetBleConnectState()			(paramCB.runtime.bleConnectState)

//=======================================================================
#define PARAM_GetTestMode()						(paramCB.runtime.ageTest.ageTestEnable)

#define PARAM_SetTxCount(a)						paramCB.runtime.ageTest.txCount = a
#define PARAM_GetTxCount()						(paramCB.runtime.ageTest.txCount)

#define PARAM_SetRxCount(a)						paramCB.runtime.ageTest.rxCount = a
#define PARAM_GetRxCount()						(paramCB.runtime.ageTest.rxCount)

#define PARAM_SetAgeTest(a)						paramCB.runtime.ageTest.ageTestResoult = a
#define PARAM_GetAgeTest(a)						(paramCB.runtime.ageTest.ageTestResoult)


#define PARAM_SetStopModeTimeInterval(a)		paramCB.runtime.stopMode.timeInterval=a+100
#define PARAM_GetStopModeTimeInterval()			(paramCB.runtime.stopMode.timeInterval)

#define PARAM_SetStopModeOnOff(a)				paramCB.runtime.stopMode.onOff=a
#define PARAM_GetStopModeOnOff()				(paramCB.runtime.stopMode.onOff)

#define PARAM_SetAgeTestFlag(a)					paramCB.runtime.ageTestFlag=a
#define PARAM_GetAgeTestFlag()					(paramCB.runtime.ageTestFlag)

#define PARAM_SetPucTestFlag(a)					paramCB.runtime.pucTestFlag=a
#define PARAM_GetPucTestFlag()					(paramCB.runtime.pucTestFlag)
//==========================================================================================

// 设置日期时间
#define PARAM_SetRTC(YY,MM,DD,hh,mm,ss)		do{\
													paramCB.runtime.rtc.year=YY;\
													paramCB.runtime.rtc.month=MM;\
													paramCB.runtime.rtc.day=DD;\
													paramCB.runtime.rtc.hour=hh;\
													paramCB.runtime.rtc.minute=mm;\
													paramCB.runtime.rtc.second=ss;\
												}while(0)
// 设置系统时间秒
#define PARAM_SetRTCSecond(a)				paramCB.runtime.rtc.second=a
// 获取系统时间秒
#define PARAM_GetRTCSecond()				(paramCB.runtime.rtc.second)
	
// 设置系统时间分
#define PARAM_SetRTCMin(a)					paramCB.runtime.rtc.minute=a
// 获取系统时间分
#define PARAM_GetRTCMin()					(paramCB.runtime.rtc.minute)
	
// 设置系统时间时
#define PARAM_SetRTCHour(a)					paramCB.runtime.rtc.hour=a
// 获取系统时间时
#define PARAM_GetRTCHour()					(paramCB.runtime.rtc.hour)
	
// 设置系统时间日
#define PARAM_SetRTCDay(a)					paramCB.runtime.rtc.day=a
// 获取系统时间日
#define PARAM_GetRTCDay()					(paramCB.runtime.rtc.day)
	
// 设置系统时间月
#define PARAM_SetRTCMonth(a)				paramCB.runtime.rtc.month=a
// 获取系统时间月
#define PARAM_GetRTCMonth()					(paramCB.runtime.rtc.month)
	
// 设置系统时间年
#define PARAM_SetRTCYear(a)					paramCB.runtime.rtc.year=a
// 获取系统时间年
#define PARAM_GetRTCYear()					(paramCB.runtime.rtc.year)


// 设置电池百分比
#define PARAM_SetBatteryPercent(a)				paramCB.runtime.battery.percent=a
// 获取电池百分比
#define PARAM_GetBatteryPercent()				(paramCB.runtime.battery.percent)	

#define PARAM_SetErrorCode(a)				paramCB.runtime.errorType=a
// 获取错误代码
#define PARAM_GetErrorCode()				(paramCB.runtime.errorType)

// 设置电池电量数据源
#define PARAM_SetBatteryDataSrc(a)			paramCB.runtime.battery.batteryDataSrc=a
// 获取电池电量数据源
#define PARAM_GetBatteryDataSrc()			(paramCB.runtime.battery.batteryDataSrc)


// 设置巡航状态，0:静止；1:正在巡航 222
#define PARAM_SetCruiseState(enable)		paramCB.runtime.cruiseState=enable
// 获取巡航状态，0:静止；1:正在巡航
#define PARAM_GetCruiseState()				(paramCB.runtime.cruiseState)

// 设置6km推车助力状态 222
#define PARAM_SetPushAssistState(a)			paramCB.runtime.PushAssistState=a
// 获取6km推车助力状态
#define PARAM_GetPushAssistState()			(paramCB.runtime.PushAssistState)

// 设置电池电流，单位:mA 222
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// 获取电流，单位:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)


// 设置速度，单位:0.1Km/h 222
#define PARAM_SetSpeed(a)					paramCB.runtime.speed=a
// 获取速度，单位:0.1Km/h
//#define PARAM_GetSpeed()					(paramCB.runtime.speed)


// 设置助力档位
//#define PARAM_SetAssistLevel(a)				do{\
												paramCB.runtime.assist=a;\
											}while(0)
// 获取助力档位
//#define PARAM_GetAssistLevel()				(paramCB.runtime.assist)

// 设置RGB灯开关
#define PARAM_SetRgbSwitch(a)				paramCB.nvm.param.protocol.rgbSwitch=a
// 获取RGB灯开关
#define PARAM_GetRgbSwitch()				(paramCB.nvm.param.protocol.rgbSwitch)

// 设置RGB颜色
#define PARAM_SetRgbColour(a)				paramCB.nvm.param.protocol.ledRgb=a
// 获取RGB颜色
#define PARAM_GetRgbColour()				(paramCB.nvm.param.protocol.ledRgb)

// 设置RGB灯模式
#define PARAM_SetRgbMode(a)					paramCB.nvm.param.protocol.rgbMode=a
// 获取RGB灯模式
#define PARAM_GetRgbMode()					(paramCB.nvm.param.protocol.rgbMode)

// 设置大灯开关
#define PARAM_Setheadlight(a)				paramCB.nvm.param.protocol.headlight=a
// 获取大灯开关
#define PARAM_Getheadlight()				(paramCB.nvm.param.protocol.headlight) 

// 设置日行灯开关
#define PARAM_SetdayLight(a)				paramCB.nvm.param.protocol.dayLight=a
// 获取日行开关
#define PARAM_GetdayLight()					(paramCB.nvm.param.protocol.dayLight) 

// 设置左转灯开关
#define PARAM_SetleftLight(a)				paramCB.nvm.param.protocol.leftLight=a
// 获取左转灯开关
#define PARAM_GetleftLight()					(paramCB.nvm.param.protocol.leftLight) 

// 设置右转灯开关
#define PARAM_SetrightLight(a)				paramCB.nvm.param.protocol.rightLight=a
// 获取右转灯开关
#define PARAM_GetrightLight()					(paramCB.nvm.param.protocol.rightLight) 


// 设置助力档位
#define PARAM_SetAssistLevel(a)				do{\
												paramCB.nvm.param.common.assist=a;\
											}while(0)
// 获取助力档位
#define PARAM_GetAssistLevel()				(paramCB.nvm.param.common.assist)

// 设置电机功率,单位:W
#define PARAM_SetPower(a)					paramCB.runtime.power=a
// 获取电机功率,单位:W
#define PARAM_GetPower()					(paramCB.runtime.power)	

// 获取出厂以来的总骑行时间，单位:s
#define PARAM_GetTotalRidingTime()			(paramCB.nvm.param.common.record.total.ridingTime)
// 获取出厂以来的总卡路里，单位:KCal
#define PARAM_GetTotalCalories()			(paramCB.nvm.param.common.record.total.calories)

// 用户最大速度清零
#define PARAM_ClearUserMaxSpeed()			paramCB.runtime.record.userMaxSpeed=0
// 获取用户最大速度
#define PARAM_GetUserMaxSpeed()				(paramCB.runtime.record.userMaxSpeed)


// 设置锁的状态
#define PARAM_SetLockFlag(a)				paramCB.nvm.param.common.lockFlag=a;
// 获取锁的状态
#define PARAM_GetLockFlag()					(paramCB.nvm.param.common.lockFlag)

// 设置是否锁车
#define PARAM_SetIsLocked(locked)			paramCB.runtime.isLocked=locked
// 获取锁车状态
#define PARAM_GetIsLocked()					(paramCB.runtime.isLocked)

// 设置最大电机功率,单位:W
#define PARAM_SetPowerMax(a)				paramCB.nvm.param.protocol.powerMax=a
// 获取最大电机功率,单位:W
#define PARAM_GetPowerMax()					(paramCB.nvm.param.protocol.powerMax)


// 设置电池电压等级,单位:V 111
#define PARAM_SetBatteryVoltageLevel(a)		paramCB.nvm.param.common.battery.voltage=a 
// 获取电池电压等级,单位:V
#define PARAM_GetBatteryVoltageLevel()		(paramCB.nvm.param.common.battery.voltage)

// 设置电池电压，单位:mV
#define PARAM_SetBatteryVoltage(mV)			paramCB.runtime.battery.voltage=mV
// 获取电池电压，单位:mV
#define PARAM_GetBatteryVoltage()			(paramCB.runtime.battery.voltage)


// 设置电池电流，单位:mA
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// 获取电流，单位:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)


// 设置背光亮度 111
#define PARAM_SetBrightness(a)				paramCB.nvm.param.common.brightness=a
// 获取背光亮度
#define PARAM_GetBrightness()				(paramCB.nvm.param.common.brightness)

// 设置自动关机时间，单位:分钟；>=0min 111
#define PARAM_SetPowerOffTime(a)			paramCB.nvm.param.common.powerOffTime=a
// 获取自动关机时间，单位:分钟；>=0min
#define PARAM_GetPowerOffTime()				(paramCB.nvm.param.common.powerOffTime)

// 设置驱动器控制模式 111
#define PARAM_SetDriverControlMode(a)		paramCB.nvm.param.protocol.driver.controlMode=a
// 获取驱动器控制模式
#define PARAM_GetDriverControlMode()		(paramCB.nvm.param.protocol.driver.controlMode)

// 设置最大档位 111
#define PARAM_SetMaxAssist(a)				do{\
												paramCB.nvm.param.common.assistMax=a;\
											}while(0)
// 获取最大档位
#define PARAM_GetMaxAssist()				(paramCB.nvm.param.common.assistMax)

// 设置零启动或非零启动 111
#define PARAM_SetZeroStartOrNot(a)			paramCB.nvm.param.protocol.driver.zeroStartOrNot=a
// 获取零启动或非零启动
#define PARAM_GetZeroStartOrNot()			(paramCB.nvm.param.protocol.driver.zeroStartOrNot)

// 设置切换巡航的方式 111
#define PARAM_SetSwitchCruiseWay(a)			paramCB.nvm.param.protocol.driver.switchCruiseWay=a
// 获取切换巡航的方式
#define PARAM_GetSwitchCruiseWay()			(paramCB.nvm.param.protocol.driver.switchCruiseWay)

// 设置巡航切换模式 111
#define PARAM_SetSwitchCruiseMode(a)		paramCB.nvm.param.protocol.driver.switchCruiseMode=a
// 获取巡航切换模式
#define PARAM_GetSwitchCruiseMode()			(paramCB.nvm.param.protocol.driver.switchCruiseMode)

// 设置巡航使能
#define PARAM_SetCruiseEnabled(a)			paramCB.nvm.param.protocol.driver.CruiseEnabled=a
// 获取巡航使能
#define PARAM_GetCruiseEnabled()			(paramCB.nvm.param.protocol.driver.CruiseEnabled)


// 设置一圈磁钢数，即测速磁钢 111
#define PARAM_SetCycleOfSteelNum(a)			paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor=a
// 获取一圈磁钢数，即测速磁钢
#define PARAM_GetCycleOfSteelNum()			(paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor)

// 设置轮径ID 111
#define PARAM_SetWheelSizeID(a)				paramCB.nvm.param.protocol.wheelSizeID=a
// 获取轮径ID
#define PARAM_GetWheelSizeID()				(paramCB.nvm.param.protocol.wheelSizeID)

// 设置助力灵敏度 111
#define PARAM_SetAssistSensitivity(a)		paramCB.nvm.param.protocol.driver.assistSensitivity=a
// 获取助力灵敏度
#define PARAM_GetAssistSensitivity()		(paramCB.nvm.param.protocol.driver.assistSensitivity)

// 设置助力启动强度 111
#define PARAM_SetAssistStartIntensity(a)	paramCB.nvm.param.protocol.driver.assistStartIntensity=a
// 获取助力启动强度
#define PARAM_GetAssistStartIntensity()		(paramCB.nvm.param.protocol.driver.assistStartIntensity)

// 设置限速电机换向霍尔磁钢数 11
#define PARAM_SetReversalHolzerSteelNum(a)	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum=a
// 获取限速电机换向霍尔磁钢数
#define PARAM_GetReversalHolzerSteelNum()	(paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum)

// 设置限速门限，单位:0.1Km/h	 111
#define PARAM_SetSpeedLimit(a)				paramCB.nvm.param.common.speed.limitVal=a
// 获取限速门限，单位:0.1Km/h	
//#define PARAM_GetSpeedLimit()				(paramCB.nvm.param.common.speed.limitVal)

// 设置电流门限，单位:mA 111
#define PARAM_SetCurrentLimit(a)			paramCB.nvm.param.protocol.driver.currentLimit=a
// 获取电流门限，单位:mA
#define PARAM_GetCurrentLimit()				(paramCB.nvm.param.protocol.driver.currentLimit)

// 设置欠压门限，单位:mV 111
#define PARAM_SetLowVoltageThreshold(a)		paramCB.nvm.param.protocol.driver.lowVoltageThreshold=a
// 获取欠压门限，单位:mV
#define PARAM_GetLowVoltageThreshold()		(paramCB.nvm.param.protocol.driver.lowVoltageThreshold)

// 设置助力磁钢类型
#define PARAM_SetSteelType(a)				paramCB.nvm.param.protocol.driver.assistSteelType=a
// 获取助力磁钢类型
#define PARAM_GetSteelType()				(paramCB.nvm.param.protocol.driver.assistSteelType)

// 设置限速开关 111
#define PARAM_SetSpeedLimitSwitch(a)		paramCB.nvm.param.protocol.driver.speedLimitSwitch=a
// 获取限速开关
#define PARAM_GetSpeedLimitSwitch()			(paramCB.nvm.param.protocol.driver.speedLimitSwitch)

// 设置电池电量变化时间，单位:s 111
#define PARAM_SetBatteryCapVaryTime(a)		paramCB.nvm.param.common.battery.filterLevel=a
// 获取电池电量变化时间，单位:s
#define PARAM_GetBatteryCapVaryTime()		(paramCB.nvm.param.common.battery.filterLevel)

// 设置总线通讯故障超时时间，单位:ms 111
#define PARAM_SetBusAliveTime(a)			paramCB.nvm.param.common.busAliveTime=a
// 获取总线通讯故障超时时间，单位:ms
#define PARAM_GetBusAliveTime()				(paramCB.nvm.param.common.busAliveTime)

// 设置速度变化平滑度
#define PARAM_SetSpeedFilterLevel(a)		paramCB.nvm.param.common.speed.filterLevel=a
// 获取速度变化平滑度
#define PARAM_GetSpeedFilterLevel()			(paramCB.nvm.param.common.speed.filterLevel)

// 设置恢复到出厂设置
#define PARAM_SetResFactorySet(a)				paramCB.nvm.param.protocol.resFactorySet=a
// 获取恢复到出厂设置
#define PARAM_GetResFactorySet()				(paramCB.nvm.param.protocol.resFactorySet)


// 设置运动状态保持时间，单位:s
#define PARAM_SetMoveTimeKeepTime(a)		paramCB.runtime.record.moveKeepTime=a
// 获取运动状态保持时间，单位:s
#define PARAM_GetMoveTimeKeepTime()			(paramCB.runtime.record.moveKeepTime)

// 设置静止状态保持时间，单位:s
#define PARAM_SetStopTimeKeepTime(a)		paramCB.runtime.record.stopKeepTime=a
// 获取静止状态保持时间，单位:s
#define PARAM_GetStopTimeKeepTime()			(paramCB.runtime.record.stopKeepTime)


// 设置定时获取参数的时间
#define PARAM_SetParamCycleTime(ms)			paramCB.runtime.paramCycleTime=ms
// 获取定时获取参数的时间
#define PARAM_GetParamCycleTime()			(paramCB.runtime.paramCycleTime)

// 设置骑行记录条数
#define PARAM_SetRecordCount(count)			paramCB.runtime.recordCount=count
// 获取骑行记录条数
#define PARAM_GetRecordCount()				(paramCB.runtime.recordCount)

// 设置电池电流，单位:mA
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// 获取电流，单位:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)

// 设置电池容量低告警
#define PARAM_SetBatteryLowAlarm(a)			paramCB.runtime.battery.lowVoltageAlarm=a
// 获取电池容量低告警
#define PARAM_GetBatteryLowAlarm()			(paramCB.runtime.battery.lowVoltageAlarm)

// 设置单位, 公制与英制 
#define PARAM_SetUnit(a)					paramCB.nvm.param.common.unit=a
// 获取单位, 公制与英制 
#define PARAM_GetUnit()						(paramCB.nvm.param.common.unit)

// 设置是否有蓝牙
#define PARAM_SetExistBle(a)				paramCB.nvm.param.common.existBle=a
// 获取是否有蓝牙
#define PARAM_GetExistBle()					(paramCB.nvm.param.common.existBle)

// 设置显示模式
#define PARAM_SetDisMode(a)					paramCB.runtime.disMode=a
// 获取显示模式
#define PARAM_GetDisMode()					(paramCB.runtime.disMode)

// 设置总里程
#define PARAM_SetTotalDistance(a)			paramCB.runtime.totalDistance=a
// 获取总里程
//#define PARAM_GetTotalDistance()			(paramCB.runtime.totalDistance)

// 设置本次里程
#define PARAM_SetTripDistance(a)			paramCB.runtime.tripMileage=a
// 获取本次里程
//#define PARAM_GetTripDistance()				(paramCB.runtime.tripMileage)

// 设置关机状态
#define PARAM_SetShutDownState(a)			paramCB.runtime.shutDownState=a
// 获取关机状态
#define PARAM_GetShutDownState()			(paramCB.runtime.shutDownState)

// 设置油门ADC
#define PARAM_SetThrottleAdc(a)				paramCB.runtime.throttleAdc=a
// 获取油门ADC
#define PARAM_GetThrottleAdc()				(paramCB.runtime.throttleAdc)

// 设置电子刹车ADC
#define PARAM_SetBreakAdc(a)				paramCB.runtime.breakAdc=a
// 获取电子刹车ADC
#define PARAM_GetBreakAdc()					(paramCB.runtime.breakAdc)

// 设置氛围灯
#define PARAM_SetAtmosphereLight(a)			paramCB.runtime.atmosphereLight=a
// 获取氛围灯
#define PARAM_GetAtmosphereLight()			(paramCB.runtime.atmosphereLight)

// 设置刹车类型
#define PARAM_SetBreakType(a)				paramCB.nvm.param.protocol.driver.breakType=(a)
// 获取刹车类型
#define PARAM_GetBreakType()				(paramCB.nvm.param.protocol.driver.breakType)


// 设置系统做功助力比
#define PARAM_SetWorkAssistRatio(a)			paramCB.nvm.param.common.workAssistRatio=a
// 获取系统做功助力比
#define PARAM_GetWorkAssistRatio()			(paramCB.nvm.param.common.workAssistRatio)

// 设置电机运行状态
#define PARAM_SetMotorState(a)				paramCB.runtime.motorState=a
// 获取电机运行状态
#define PARAM_GetMotorState()				(paramCB.runtime.motorState)

// 设置数据保存状态
#define PARAM_SetSaveDateState(a)			paramCB.runtime.saveDateState=a
// 获取数据运行状态
#define PARAM_GetSaveDateState()			(paramCB.runtime.saveDateState)

// 设置温度数据
#define PARAM_SetTempData(a)				paramCB.runtime.tempData=a
// 获取温度数据
#define PARAM_GetTempData()					(paramCB.runtime.tempData)

// 设置控制器过温标志
#define PARAM_SetControllerOvertemp(a)			paramCB.runtime.controllerOvertemp=a
// 获取控制器过温标志
#define PARAM_GetControllerOvertemp()			(paramCB.runtime.controllerOvertemp)

// 设置电机过温标志
#define PARAM_SetmotorOvertemp(a)			paramCB.runtime.motorOvertemp=a
// 获取电机过温标志
#define PARAM_GetmotorOvertemp()			(paramCB.runtime.motorOvertemp)

// 设置电池过温标志
#define PARAM_SetbatteryOvertemp(a)			paramCB.runtime.batteryOvertemp=a
// 获取电池过温标志
#define PARAM_GetbatteryOvertemp()			(paramCB.runtime.batteryOvertemp)

// 设置蜂鸣器
#define PARAM_SetBuzzer(a)			paramCB.runtime.buzzer=a
// 获取蜂鸣器
#define PARAM_GetBuzzer()			(paramCB.runtime.buzzer)

// 设置能量回收等级
#define PARAM_SetEnergyRecoveryLevel(a)			paramCB.runtime.energyRecoveryLevel=a
// 获取能量回收等级
#define PARAM_GetEnergyRecoveryLevel()			(paramCB.runtime.energyRecoveryLevel)

// 获取仪表编号
#define PARAM_GetDevVersion()				(paramCB.runtime.devVersion)

// 获取固件版本编号
#define PARAM_GetFwBinNum()					(paramCB.runtime.fwBinNum)

// 获取固件版本号保留位
#define PARAM_GetDevVersionRemain()			(paramCB.runtime.fwBinVersion.devVersionRemain)


// 获取固件发布版本号
#define PARAM_Get_DevReleaseVersion()		(paramCB.runtime.fwBinVersion.devReleaseVersion)


// 获取固件需求升级版本号
#define PARAM_Get_FwUpdateVersion()			(paramCB.runtime.fwBinVersion.fwUpdateVersion)


// 获取固件修复bug升级版本号
#define PARAM_Get_FwDebugReleaseVersion()	(paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)


/******************************************************************************
* 【外部接口声明】
******************************************************************************/
// 模块初始化，完成模块初始化
void PARAM_Init(void);

// 模块过程处理
void PARAM_Process(void);

// NVM更新，传入参数指明是否立即写入
void NVM_SetDirtyFlag(BOOL writeAtOnce);

uint16 PARAM_GetPerimeter(uint8 wheelSizeID);

void NVM_Save(BOOL saveAll);

uint16 PARAM_GetSpeed(void);

// 返回出厂设置
void PARAM_FactoryReset(void);

// 获取总里程接口
uint16 PARAM_GetTotalDistance(void);

// 获取速度接口
uint16 PARAM_GetSpeed(void);
#endif









