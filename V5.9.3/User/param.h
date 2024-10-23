#ifndef __PARAM_H__
#define __PARAM_H__

#include "common.h"

#define SN_ADDR1 0x1FFFF7E8
#define SN_ADDR2 0x1FFFF7EC
#define SN_ADDR3 0x1FFFF7F0

// ■■运行过程中数据，掉电不保存■■
typedef struct
{
	uint8 flagArr[64];

	uint8 appVersion[32];
	uint8 bootVersion[32];
	uint8 uiVersion[32];
	uint8 qrCodeStr[64];
	uint8 hardVersion[32];
	uint8 snCode[32];
	uint8 bluMac[32];
	// 仪表唯一序列号
	struct
	{
		uint32 sn0;
		uint32 sn1;
		uint32 sn2;
		uint8 snStr[25];
	} watchUid;

	BOOL qrRegEnable;
	BOOL jumpFlag;

	uint8 devVersion[3]; // 仪表型号
	uint16 fwBinNum;	 // 固件版本编号
	struct
	{
		uint8 devVersionRemain;		 // 设备发布版本号(需求版本号)
		uint8 devReleaseVersion;	 // 软件升级更改发布版本号(产品定义版本号)
		uint8 fwUpdateVersion;		 // 调试发布版本号(方案版本号)
		uint8 fwDebugReleaseVersion; // 调试临时版本号
	} fwBinVersion;

} RUNTIME_DATA_CB;

typedef struct
{
	// 系统出厂参数
	struct
	{
		uint16 newWheelSize;
		uint16 newPerimeter;
		uint8 unit;
		uint8 speedLimitVal;		 // 单位:1KM/H
		uint8 steelNumOfSpeedSensor; // 测试传感器一圈磁钢数(即 测速磁钢)
		uint8 brightness;
		uint8 powerOffTime;

		uint8 protocol;
		uint8 batVoltage;
		uint8 percentageMethod;
		uint8 pushAssistSwitch;
		uint8 defaultAssist;
		uint8 maxAssist;
		uint8 logo;
	} sys;

} NVM_CB;

// 出厂NVM区尺寸
#define PARAM_FACTORY_NVM_DATA_SIZE sizeof(FACTORY_NVM_CB)

// 获取仪表编号
#define PARAM_GetDevVersion() (paramCB.runtime.devVersion)

// 获取固件版本编号
#define PARAM_GetFwBinNum() (paramCB.runtime.fwBinNum)

// 获取固件版本号保留位
#define PARAM_GetDevVersionRemain() (paramCB.runtime.fwBinVersion.devVersionRemain)

// 获取固件发布版本号
#define PARAM_Get_DevReleaseVersion() (paramCB.runtime.fwBinVersion.devReleaseVersion)

// 获取固件需求升级版本号
#define PARAM_Get_FwUpdateVersion() (paramCB.runtime.fwBinVersion.fwUpdateVersion)

// 获取固件修复bug升级版本号
#define PARAM_Get_FwDebugReleaseVersion() (paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)

/*******************************************************************************
 *                                  结构体	                                   *
 ********************************************************************************/
// ■■ 码表骑行参数 : 分为两大类:保存NVM与不保存NVM ■■
typedef struct
{
	RUNTIME_DATA_CB runtime;
	//	union
	//	{
	//  		FACTORY_NVM_CB param;

	//		uint8 array[PARAM_FACTORY_NVM_DATA_SIZE];
	//	}nvm;
	// uint8 preValue[PARAM_FACTORY_NVM_DATA_SIZE];

	BOOL factoryNvmWriteRequest;
	//==================================================================================
} PARAM_CB;

extern PARAM_CB paramCB;
extern const uint32 PARAM_MCU_VERSION_ADDRESS[];

typedef enum
{
	FLASH_AGING_TEST_NULL,
	FLASH_AGING_TEST_OK,
	FLASH_AGING_TEST_NG,
} FLASH_AGING_TEST_RES_E;

// 函数声明
void PARAM_Init(void);

uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);

void PARAM_Process(void);

void FACTORY_NVM_SetDirtyFlag(BOOL req);

#endif
