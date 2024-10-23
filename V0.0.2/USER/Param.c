/********************************************************************************
  * 文 件 名: Param.c
  * 版 本 号: 初版
  * 修改作者: Fly & ZJ & Workman
  * 修改日期: 2017年08月06日
  * 功能介绍: 该文件用于骑行参数的获取与计算，提供可靠的数据给上层显示或传输            
  ******************************************************************************
  * 注意事项:
  *
  * 							版权归迪太科技所有.
  *s
*********************************************************************************/

/*******************************************************************************
 *                                  头文件	                                   *
********************************************************************************/
#include "Param.h"
#include "iap.h"
#include "timer.h"
#include "Uartprotocol.h"
#include "powerCtl.h"

// 函数声明
void NVM_Load(void);
void NVM_Save(BOOL saveAll);
void NVM_Save_FactoryReset(BOOL saveAll);
void NVM_Load_FactoryReset(void);
void NVM_Format(void);
BOOL NVM_IsFormatOK(void);
void PARAM_SetDefaultRunningData(void);
void PARAM_CaculateTripDistance(void);
uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);

/*******************************************************************************
 *                                  变量定义                                   *
********************************************************************************/
PARAM_CB paramCB;

const uint32 PARAM_MCU_VERSION_ADDRESS[] = {PARAM_MCU_DT_SN_ADDEESS, PARAM_MCU_CUSTOMER_SN_ADDEESS, PARAM_MCU_HW_ADDEESS, \
											PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, \
											PARAM_MCU_BLE_UPDATA_CRC_ADDEESS, PARAM_MCU_QR_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS	};
// NVM格式化标识
const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

uint8 flashBuf[PARAM_MCU_PAGE_SIZE];
uint8 userSetFlashBuf[PARAM_MCU_PAGE_SIZE];
uint32 flashBackupFlag;

// 加载系统参数
void NVM_Load(void)
{
//==========================================================================================
	// 读取备份区的标识数据是否有效标志
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS, 
							(uint32*)&flashBackupFlag, 
							1);

	// 备份数据有效
	if (0x12345678 == flashBackupFlag)
	{
		// 拷贝备份区数据到运行区
		IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS, 
								(uint32*)flashBuf, 
								PARAM_MCU_PAGE_SIZE / 4);
		
		// 将拷贝的数据写入数据运行区
		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
								(uint32*)flashBuf, 
								PARAM_MCU_PAGE_SIZE / 4);

		// 清备份数据标志
		flashBackupFlag = 0x00000000;
		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + 1020, 
								(uint32*)&flashBackupFlag, 
								1);
	}

	// 读取NVM中的数据
	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
							(uint32 *)paramCB.nvm.array, 
							(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// 保存系统参数
void NVM_Save(BOOL saveAll)
{
//==============================================================================	
	// 先从最后一个扇区读取数据拷贝到倒数第二个扇区备份，然后写入一个标志到备区，接着更新最后一个扇区，最后擦除备区的标志
	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
							(uint32*)flashBuf, 
							PARAM_MCU_PAGE_SIZE / 4);

	flashBuf[1020] = 0x78;
	flashBuf[1021] = 0x56;
	flashBuf[1022] = 0x34;
	flashBuf[1023] = 0x12;
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS, 
							(uint32*)flashBuf, 
							PARAM_MCU_PAGE_SIZE / 4);
						
	// 只保存变化的数据
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
									(uint32*)paramCB.nvm.array, 
									(PARAM_NVM_DATA_SIZE + 3) / 4);

	// 清标志
	flashBackupFlag = 0x00000000;
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + 1020, 
							(uint32*)&flashBackupFlag, 
							1);
}

// 初始化设置搬参数
void NVM_Load_FactoryReset(void)
{
	// 从备区地址里面取出来数据拷贝到系统参数区域
	IAP_FlashReadWordArray(PARAM_NVM_FACTORY_RESET_ADDRESS, 
							(uint32*)paramCB.nvm.array, 
							(PARAM_MCU_PAGE_SIZE + 3) / 4);
							
	// 只保存变化的数据
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
									(uint32*)paramCB.nvm.array, 
									(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// 保存初始化设置参数
void NVM_Save_FactoryReset(BOOL saveAll)
{
	// 只保存变化的数据
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_FACTORY_RESET_ADDRESS, 
						(uint32*)paramCB.nvm.array, 
						(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// nvm写模式
void NVM_CALLBACK_EnableWrite(uint32 param)
{
	paramCB.nvmWriteEnable = TRUE;
}

// 默认运行参数
void PARAM_SetDefaultRunningData(void)
{
	
	uint8 i;
	uint8 * pBuff = NULL;
	// ■■ 不需要保存至NVM的参数 ■■

	paramCB.runtime.jumpFlag = (BOOL)0;
	
	// 固件版本号
	for (i = 0;i < 3;i++)
	{
		paramCB.runtime.devVersion[i] = DEV_VERSION[i];
	}
	
	paramCB.runtime.fwBinNum = FW_BINNUM;
	paramCB.runtime.fwBinVersion.devVersionRemain = DEV_VERSION_REMAIN;
	paramCB.runtime.fwBinVersion.devReleaseVersion = DEV_RELEASE_VERSION;
	paramCB.runtime.fwBinVersion.fwUpdateVersion = FW_UPDATE_VERSION;
	paramCB.runtime.fwBinVersion.fwDebugReleaseVersion = FW_DEBUG_RELEASE_VERSION;
	
	// 仪表唯一序列号
	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0);
	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F4); 
	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F8);

	pBuff = (uint8 *)&(paramCB.runtime.appVersion);

	paramCB.runtime.bleRadio = 0;	
	
	/*
	IAP_FlashReadWordArray(PARAM_MCU_BOOT_ADDEESS,(uint32 *)pBuff, 8);
	
	// 如果读取的版本号不符，则强制更新为当前APP版本
	if ((BOOT_VERSION_LENGTH != pBuff[0]) || (!PARAM_CmpareN((const uint8*)&pBuff[1], (const uint8*)BOOT_VERSION, BOOT_VERSION_LENGTH))) 
	{
		pBuff[0] = BOOT_VERSION_LENGTH;
		for(i = 0; i < BOOT_VERSION_LENGTH ; i++)
		{
			pBuff[i + 1] = BOOT_VERSION[i];
		}
		IAP_FlashWriteWordArrayWithErase(PARAM_MCU_APP_ADDEESS, (uint32*)pBuff, (BOOT_VERSION_LENGTH + 4) / 4);
	}
	*/
	
}

// 格式化参数
void NVM_Format(void)
{
	uint8 i;
	
	// 写格式化标识
	for (i = 0; i < NVM_FORMAT_FLAG_SIZE; i++)
	{
		paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
	}

	// ■■ 需要保存至NVM的参数 ■■
	// 设置系统参数
	paramCB.nvm.param.common.battery.voltage = 36;
	paramCB.nvm.param.common.battery.filterLevel = 1;								
	paramCB.nvm.param.common.speed.limitVal = 250; 										// 限速值，单位:0.1Km/h
	paramCB.nvm.param.common.unit = UNIT_METRIC;										// 单位, 公制与英制
	paramCB.nvm.param.common.powerOffTime = 10;											// 自动关机时间，单位:分钟；>=0min
	paramCB.nvm.param.common.assistMax = ASSIST_3;										// 支持档位
	paramCB.nvm.param.common.assist = ASSIST_1;											// 档位
	paramCB.nvm.param.common.busAliveTime = 10000;										// 总线故障超时时间
	paramCB.nvm.param.common.existBle = FALSE;											// 是否有蓝牙
	paramCB.nvm.param.common.workAssistRatio = 2;										// 机器与人做功助力比
	
	// 自出厂以来的参数初始化
	paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// 个人记录，单位:0.1Km
	paramCB.nvm.param.common.record.total.distance = 0; 				// 总里程，单位:0.1Km
	paramCB.nvm.param.common.record.total.ridingTime = 0;				// 总骑行时间，单位:s
	paramCB.nvm.param.common.record.total.calories = 0; 				// 总卡路里，单位:KCal


	// 自0点以来的参数初始化
	paramCB.nvm.param.common.record.today.trip = 0; 					// 今日里程，单位:0.1Km 	
	paramCB.nvm.param.common.record.today.calories = 0; 				// 今日卡路里，单位:KCal
	paramCB.nvm.param.common.record.today.ridingTime = 0;				// 今日骑行时间清零
	paramCB.nvm.param.common.record.today.month = 1;					// 今日参数对应的日期
	paramCB.nvm.param.common.record.today.day = 1;						// 今日参数对应的日期
	paramCB.nvm.param.common.record.today.year = 2019;					// 今日参数对应的日期

	paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor = 110; 						// 测速传感器一圈磁钢数

	paramCB.nvm.param.protocol.driver.currentLimit = 12000;								// 电流门限，单位:mA
	paramCB.nvm.param.protocol.driver.lowVoltageThreshold = 30000;						// 欠压门限，单位:mV

	paramCB.nvm.param.common.percentageMethod = BATTERY_DATA_SRC_ADC;					// 电量获取方式(0:控制器上报电压) (1:控制器上报电量) (2:仪表自己测量电压电量)

	paramCB.nvm.param.protocol.driver.controlMode = 2;									// 控制器驱动模式
	paramCB.nvm.param.protocol.driver.zeroStartOrNot = TRUE;							// 零启动或非零启动
	paramCB.nvm.param.protocol.driver.switchCruiseMode = FALSE;							// 巡航切换模式
	paramCB.nvm.param.protocol.driver.switchCruiseWay = FALSE;							// 切换巡航的方式
	paramCB.nvm.param.protocol.driver.assistSensitivity = 1;							// 助力灵敏度
	paramCB.nvm.param.protocol.driver.assistStartIntensity = 3;							// 助力启动强度
	paramCB.nvm.param.protocol.driver.assistSteelType = 5;								// 助力磁钢类型
	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum = 0;						// 限速电机换向霍尔磁钢数(未使用)
	paramCB.nvm.param.protocol.driver.speedLimitSwitch = FALSE;							// 限速开关
	paramCB.nvm.param.protocol.driver.CruiseEnabled = TRUE;							// 巡航使能标志(默认使能)
	
	paramCB.nvm.param.protocol.wheelSizeID = PARAM_WHEEL_SIZE_16_INCH;					// 轮径

	paramCB.nvm.param.protocol.driver.breakType = FALSE;								// 刹车类型0：霍尔刹车(AD)；1：断电刹车(电平)
	paramCB.nvm.param.common.lockFlag = FALSE;											// 0-开锁状态，1-锁车状态。
	paramCB.nvm.param.protocol.rgbSwitch = 1;
	paramCB.nvm.param.protocol.ledRgb = 4;
	paramCB.nvm.param.protocol.rgbMode = 4;
	
	// 保存参数
	NVM_Save(TRUE);
}

// 判断NVM是否已格式化
BOOL NVM_IsFormatOK(void)
{
	uint16 i;

	// 逐个比较，只要发现一个不同，即认为未格式化
	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
	{
		if(formatFlag[i] != paramCB.nvm.param.common.nvmFormat[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}

// 设置脏标识
void NVM_SetDirtyFlag(BOOL writeAtOnce)
{
	paramCB.nvmWriteRequest = TRUE;

	if (writeAtOnce)
	{
		paramCB.nvmWriteEnable = TRUE;
	}
}

// 计算骑行时间
void PARAM_CALLBACK_CaculateRidingTime(uint32 param)
{

}

/*********************************************************************
* 函 数 名: PARAM_Init
* 函数入参: void
* 函数出参: 无
* 返 回 值: uint32 
* 功能描述: 初始化参数
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年08月06日
*     修改描述: 新函数 		   
**********************************************************************/
void PARAM_Init(void)
{
	// 设置默认的运行数据
	PARAM_SetDefaultRunningData();
}



uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
{
	uint16 i = 0;

	while(i < len)
	{
		if(*(str1 +i) != *(str2 +i))
		{
			return 0;
		}
		i++;
	}
	return 1;
}


