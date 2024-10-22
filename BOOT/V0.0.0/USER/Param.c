///********************************************************************************
//  * 文 件 名: Param.c
//  * 版 本 号: 初版
//  * 修改作者: Fly & ZJ & Workman
//  * 修改日期: 2017年08月06日
//  * 功能介绍: 该文件用于骑行参数的获取与计算，提供可靠的数据给上层显示或传输            
//  ******************************************************************************
//  * 注意事项:
//  *
//  * 							版权归迪太科技所有.
//  *
//*********************************************************************************/

///*******************************************************************************
// *                                  头文件	                                   *
//********************************************************************************/
#include "Param.h"
//#include "iap.h"
//#include "timer.h"
//#include "powerCtl.h"
//#include "uartProtocol.h"
//#include "uartDrive.h"

//// 函数声明
//void NVM_Load(void);
//void NVM_Save(BOOL saveAll);
//void NVM_Save_FactoryReset(BOOL saveAll);
//void NVM_Load_FactoryReset(void);
//void NVM_Format(void);
//BOOL NVM_IsFormatOK(void);
//void PARAM_SetDefaultRunningData(void);
//void PARAM_CaculateTripDistance(void);
//uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);


///*******************************************************************************
// *                                  变量定义                                   *
//********************************************************************************/
//PARAM_CB paramCB;

//const uint32 PARAM_MCU_VERSION_ADDRESS[] = {PARAM_MCU_DT_SN_ADDEESS, PARAM_MCU_CUSTOMER_SN_ADDEESS, PARAM_MCU_HW_ADDEESS, \
//											PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, \
//											PARAM_MCU_BLE_UPDATA_CRC_ADDEESS, PARAM_MCU_QR_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS	};
//// NVM格式化标识
//const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

//uint8 flashBuf[PARAM_MCU_PAGE_SIZE];
//uint8 userSetFlashBuf[PARAM_MCU_PAGE_SIZE];
//uint32 flashBackupFlag;

//// 加载系统参数
//void NVM_Load(void)
//{
//	// 读取NVM中的数据
//	/*IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// 数据同时保存备份
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.preValue, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);*/
////==========================================================================================
//	// 读取备份区的标识数据是否有效标志
//	IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//							(uint32*)&flashBackupFlag, 
//							1);

//	// 备份数据有效
//	if (0x12345678 == flashBackupFlag)
//	{
//		// 拷贝备份区数据到运行区
//		IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS, 
//								(uint32*)flashBuf, 
//								PARAM_MCU_PAGE_SIZE / 4);

//		// 将拷贝的数据写入数据运行区
//		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//								(uint32*)flashBuf, 
//								PARAM_MCU_PAGE_SIZE / 4);

//		// 清备份数据标志
//		flashBackupFlag = 0x00000000;
//		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//								(uint32*)&flashBackupFlag, 
//								1);
//	}

//	// 读取NVM中的数据
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// 保存系统参数
//void NVM_Save(BOOL saveAll)
//{
//	// 只保存变化的数据
//	/*IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//						(uint32*)paramCB.nvm.array, 
//						(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// 数据同时保存备份
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32*)paramCB.preValue, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);*/
////==============================================================================	
//	// 先从最后一个扇区读取数据拷贝到倒数第二个扇区备份，然后写入一个标志到备区，接着更新最后一个扇区，最后擦除备区的标志
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32*)flashBuf, 
//							PARAM_MCU_PAGE_SIZE / 4);

//	flashBuf[(PARAM_MCU_PAGE_SIZE - 4)] = 0x78;
//	flashBuf[(PARAM_MCU_PAGE_SIZE - 3)] = 0x56;
//	flashBuf[(PARAM_MCU_PAGE_SIZE - 2)] = 0x34;
//	flashBuf[(PARAM_MCU_PAGE_SIZE - 1)] = 0x12;
//	
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS, 
//							(uint32*)flashBuf, 
//							PARAM_MCU_PAGE_SIZE / 4);
//						
//	// 只保存变化的数据
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//									(uint32*)paramCB.nvm.array, 
//									(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// 清标志
//	flashBackupFlag = 0x00000000;
//	
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//							(uint32*)&flashBackupFlag, 
//							1);
//}

//// 初始化设置搬参数
//void NVM_Load_FactoryReset(void)
//{ 
//	uint16 value, i =0;
//	
//	// 从备区地址里面取出来数据拷贝到系统参数区域
//	IAP_FlashReadWordArray(PARAM_NVM_FACTORY_RESET_ADDRESS, 
//							(uint32*)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// 判断数据区是否都为空
//	for (i = 0;i < PARAM_NVM_DATA_SIZE; i++)
//	{
//		if (0xFF == paramCB.nvm.array[i])
//		{
//			value ++;
//		}
//	}

//	// 数据区全部都是空，直接返回，不写入，防止死机
//	if (PARAM_NVM_DATA_SIZE == value)
//	{
//		return;
//	}
//	// 只保存变化的数据
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//									(uint32*)paramCB.nvm.array, 
//									(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// 保存初始化设置参数
//void NVM_Save_FactoryReset(BOOL saveAll)
//{
//	// 只保存变化的数据
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_FACTORY_RESET_ADDRESS, 
//						(uint32*)paramCB.nvm.array, 
//						(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// nvm写模式
//void NVM_CALLBACK_EnableWrite(uint32 param)
//{
//	paramCB.nvmWriteEnable = TRUE;
//}

//// 默认运行参数
//void PARAM_SetDefaultRunningData(void)
//{
//	uint8 i;
//	uint8 * pBuff = NULL;
//	// ■■ 不需要保存至NVM的参数 ■■

//	// ■■ 不需要保存至NVM的参数 ■■
//	paramCB.runtime.fwVer = FW_VERSION;
//	paramCB.runtime.hwVer = HW_VERSION;
//	paramCB.runtime.uiVersion = UI_VERSION;
//	paramCB.runtime.btCapacity = BT_CAPACITY;
//	
//	// 固件版本号
//	paramCB.runtime.devVersion[0] = DEV_VERSION[0];
//	paramCB.runtime.devVersion[1] = DEV_VERSION[1];
//	paramCB.runtime.devVersion[2] = DEV_VERSION[2];
//	
//	paramCB.runtime.fwBinNum = FW_BINNUM;
//	paramCB.runtime.fwBinVersion.devVersionRemain = DEV_VERSION_REMAIN;
//	paramCB.runtime.fwBinVersion.devReleaseVersion = DEV_RELEASE_VERSION;
//	paramCB.runtime.fwBinVersion.fwUpdateVersion = FW_UPDATE_VERSION;
//	paramCB.runtime.fwBinVersion.fwDebugReleaseVersion = FW_DEBUG_RELEASE_VERSION;
//	
//	// 仪表唯一序列号
//	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0);
//	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F4); 
//	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F8);

//	pBuff = (uint8 *)&(paramCB.runtime.appVersion);
//	
//	IAP_FlashReadWordArray(PARAM_MCU_APP_ADDEESS,(uint32 *)pBuff, 8);
//	
//	// 如果读取的版本号不符，则强制更新为当前APP版本
//	if ((APP_VERSION_LENGTH != pBuff[0]) || (!PARAM_CmpareN((const uint8*)&pBuff[1], (const uint8*)APP_VERSION, APP_VERSION_LENGTH))) 
//	{
//		pBuff[0] = APP_VERSION_LENGTH;
//		for(i = 0; i < APP_VERSION_LENGTH ; i++)
//		{
//			pBuff[i + 1] = APP_VERSION[i];
//		}
//		IAP_FlashWriteWordArrayWithErase(PARAM_MCU_APP_ADDEESS, (uint32*)pBuff, (APP_VERSION_LENGTH + 4) / 4);
//	}

//	pBuff = (uint8 *)&(paramCB.runtime.bootVersion);
//	IAP_FlashReadWordArray(PARAM_MCU_BOOT_ADDEESS, (uint32 *)pBuff, 8);

//	pBuff = (uint8 *)&(paramCB.runtime.SnCode);
//	IAP_FlashReadWordArray(PARAM_MCU_DT_SN_ADDEESS, (uint32 *)pBuff, 8);

//	pBuff = (uint8 *)&(paramCB.runtime.LimeSnCode);
//	IAP_FlashReadWordArray(PARAM_MCU_CUSTOMER_SN_ADDEESS, (uint32 *)pBuff, 8);

//	pBuff = (uint8 *)&(paramCB.runtime.hwVersion);
//	IAP_FlashReadWordArray(PARAM_MCU_HW_ADDEESS, (uint32 *)pBuff, 8);

//	pBuff = (uint8 *)&(paramCB.runtime.testflag);
//	IAP_FlashReadWordArray(PARAM_MCU_TEST_FLAG_ADDEESS, (uint32 *)pBuff, 16);
//	
//	// 驱动器运行参数
//	paramCB.runtime.cruiseEnabe = FALSE;			// 巡航开关，0:表示关闭；1:表示打开 
//	paramCB.runtime.cruiseState = FALSE; 			// 当前巡航状态，1:表示进入巡航状态 
//	paramCB.runtime.pushAssistOn = FALSE; 			// 6km/h推行功能，0:表示关闭；1:表示打开
//	paramCB.runtime.isHavePushAssist = TRUE;		// 有推车助力功能(APP读取系统信息用)
//	paramCB.runtime.lightSwitch = FALSE;			// 大灯开关	

//	paramCB.runtime.battery.voltage = 40000;		// 当前电池电压，单位:mV 	
//	paramCB.runtime.battery.current = 0;			// 当前电池电流，单位:mA 	
//	paramCB.runtime.battery.lowVoltageAlarm = 0;	// 电池容量低告警
//	//paramCB.runtime.battery.state = 0;				// 电池状态
//	//paramCB.runtime.battery.preState = 0;			// 电池上一状态
//	
//	paramCB.runtime.speed = 0; 						// 当前速度，单位:0.1Km/h
//	//paramCB.runtime.pulseNum = 0;					// 当前脉冲个数

//	paramCB.runtime.errorType = ERROR_TYPE_NO_ERROR;		// 当前错误代码
//	//paramCB.runtime.ridingState = SYSTEM_STATE_STOP;		// 当前骑行状态，0:表示静止；1:表示运行状态；	
//	paramCB.runtime.overSpeedAlarm = FALSE; 				// 超速告警，1:表示超速 	
//	paramCB.runtime.underVoltageAlarm = FALSE;				// 欠压告警，0:表示不欠压；1:表示欠压

//	//paramCB.runtime.torque = 0;								// 力矩
//	paramCB.runtime.paramCycleTime = 200;					// 定时获取参数的时间
//	
//	// 码表显示参数
//	paramCB.runtime.battery.percent = 100;							// 电池百分比
//	paramCB.runtime.battery.batteryDataSrc = (BATTERY_DATA_SRC_E)paramCB.nvm.param.common.percentageMethod;		// 电量计算方式
//	paramCB.runtime.assist = ASSIST_0;								// 助力档位	
//	paramCB.runtime.recordCount = 0;								// 骑行统计数据条数
//	//paramCB.runtime.ringStatus = 0;									// 来电提醒

//	//paramCB.runtime.warningStatus = 0;								// 设防/撤防
//	paramCB.runtime.power = 0;										// 电机功率,单位:W
//	paramCB.runtime.bleConnectState = FALSE;						// 蓝牙是否连接，1:表示连接；0:表示未连接

//	paramCB.runtime.bleRadio = 0;
//	
//	paramCB.runtime.battery.powerOffstate = 0;

//	paramCB.runtime.blueUpgradePer = 0;  							// 蓝牙升级百分比
//	
//	// 车轮周长，单位:mm
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_12_INCH] = 957;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_14_INCH] = 1117;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_16_INCH] = 1276;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_18_INCH] = 1436;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_20_INCH] = 1595;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_22_INCH] = 1755;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_24_INCH] = 1914;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_26_INCH] = 2074;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_27_INCH] = 2153;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_27_5_INCH] = 2193;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_28_INCH] = 2233;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_29_INCH] = 2313;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_700C] = 2193;		// 就是27.5

//	// 喜德盛特殊要求
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_24_INCH] = 1820;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_27_5_INCH] = 2130;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_700C] = 2130;		// 就是27.5
//	
//	
//}

////// 根据助力档位数设定每个档位的PWM值上下限
////void NVM_SetPWMLimitValue(void)
////{
////	switch (paramCB.nvm.param.common.assistMax)
////	{
////		// 这组数据没有测试，需要根据样品测试后补充
////		//#warning "这组数据没有测试，需要根据样品测试后补充"
////		case ASSIST_1:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 96;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;
////			break;

////		// 根据KDS的样品测试得到的数据
////		case ASSIST_3:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 114;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_2] = 178;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_3] = 234;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;

////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_1] = 38;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_2] = 59;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_3] = 78;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_P] = 10;

////			// 喜德盛专用pwm 1档： 255* 0.235=60  2档:  255* 0.55=140  3档：255 * 0.9=230
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_1] = 60;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_2] = 140;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_3] = 230;
////			
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_4] = 217;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_5] = 255;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_P] = 50;
////			break;

////		// 根据KDS的样品测试得到的数据
////		case ASSIST_5:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 127;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_2] = 163;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_3] = 191;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_4] = 216;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_5] = 244;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;

////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_1] = 42;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_2] = 54;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_3] = 63;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_4] = 72;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_5] = 81;
////			paramCB.nvm.param.protocol.pwmBottom[ASSIST_P] = 10;
////			
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_1] = 102;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_2] = 140;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_3] = 179;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_4] = 217;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_5] = 255;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_P] = 50;
////			break;

////		// 这组数据没有测试，需要根据样品测试后补充
////		#warning "这组数据没有测试，需要根据样品测试后补充"
////		case ASSIST_6:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 50;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_2] = 60;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_3] = 70;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_4] = 80;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_5] = 90;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_6] = 98;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;
////			break;

////		// 这组数据没有测试，需要根据样品测试后补充
////		#warning "这组数据没有测试，需要根据样品测试后补充"
////		case ASSIST_9:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 50;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_2] = 56;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_3] = 62;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_4] = 68;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_5] = 74;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_6] = 80;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_7] = 86;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_8] = 92;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_9] = 98;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;
////			break;

////		default:
////			break;
////	}
////}

////// 格式化参数
////void NVM_Format(void)
////{
////	uint8 i;
////	
////	// 写格式化标识
////	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
////	{
////		paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
////	}

////	// ■■ 需要保存至NVM的参数 ■■
////	// 设置系统参数
////	paramCB.nvm.param.common.battery.voltage = 36;										// 电池电压等级，单位:V
////	paramCB.nvm.param.common.battery.filterLevel = 1; 									// 电量显示变化速度，单位:秒
////	paramCB.nvm.param.common.speed.limitVal = 250;										// 限速值，单位:0.1Km/h
////	paramCB.nvm.param.common.speed.filterLevel = 3; 									// 速度变化平滑度
////	paramCB.nvm.param.common.unit = UNIT_METRIC;										// 单位, 公制与英制
////	paramCB.nvm.param.common.brightness = 0;												// 背光亮度
////	paramCB.nvm.param.common.powerOffTime = 10; 										// 自动关机时间，单位:分钟；>=0min
////	paramCB.nvm.param.common.assistMax = ASSIST_4;										// 支持最大档位
////	paramCB.nvm.param.common.busAliveTime = 10000;		// 总线故障超时时间
////	paramCB.nvm.param.common.workAssistRatio = 2;										// 机器与人做功助力比
////	
////	paramCB.nvm.param.common.existBle = TRUE;											// 是否有蓝牙
////	paramCB.nvm.param.common.percentageMethod = BATTERY_DATA_SRC_BMS ;					// 电量获取方式(0:控制器上报电压) (1:控制器上报电量) (2:仪表自己测量电压电量)

////	// 自出厂以来的参数初始化
////	paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// 个人记录，单位:0.1Km
////	paramCB.nvm.param.common.record.total.distance = 0; 				// 总里程，单位:0.1Km
////	paramCB.nvm.param.common.record.total.ridingTime = 0;				// 总骑行时间，单位:s
////	paramCB.nvm.param.common.record.total.calories = 0; 				// 总卡路里，单位:KCal

////	// 自Trip清零以来的参数初始化
////	paramCB.nvm.param.common.record.trip.ridingTime = 0;				// 总骑行时间秒
////	paramCB.nvm.param.common.record.trip.distance = 0;					// Trip，单位:0.1Km
////	paramCB.nvm.param.common.record.trip.calories = 0;					// 卡路里，单位:KCal
////	paramCB.nvm.param.common.record.trip.speedMax = 0;					// 最大速度 
////	paramCB.runtime.trip.speedAverage = 0;								// 平均速度，单位:0.1Km/h	

////	// 自0点以来的参数初始化
////	paramCB.nvm.param.common.record.today.trip = 0; 					// 今日里程，单位:0.1Km 	
////	paramCB.nvm.param.common.record.today.calories = 0; 				// 今日卡路里，单位:KCal
////	paramCB.nvm.param.common.record.today.ridingTime = 0;				// 今日骑行时间清零
////	paramCB.nvm.param.common.record.today.month = 1;					// 今日参数对应的日期
////	paramCB.nvm.param.common.record.today.day = 1;						// 今日参数对应的日期
////	paramCB.nvm.param.common.record.today.year = 2019;					// 今日参数对应的日期

////	paramCB.nvm.param.protocol.driver.steelNumOfStartAssist = 2;						// 助力开始的磁钢数
////	paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor = 1;						// 测速传感器一圈磁钢数
////	paramCB.nvm.param.protocol.driver.assistDirectionForward = FALSE;					// 助力正反
////	paramCB.nvm.param.protocol.driver.assistPercent = 64;								// 助力比例
////	paramCB.nvm.param.protocol.driver.slowStart = 2;									// 缓启动参数
////	paramCB.nvm.param.protocol.driver.turnBarSpeed6kmphLimit = FALSE;					// 转把是否限速6km/h
////	paramCB.nvm.param.protocol.driver.turnBarLevel = FALSE; 							// 转把是否分档
////	paramCB.nvm.param.protocol.driver.currentLimit = 15000; 							// 电流门限，单位:mA
////	paramCB.nvm.param.protocol.driver.lowVoltageThreshold = 30000;						// 欠压门限，单位:mV

////	paramCB.nvm.param.protocol.driver.controlMode = 2;									// 控制器驱动模式
////	paramCB.nvm.param.protocol.driver.zeroStartOrNot = FALSE;							// 零启动或非零启动
////	paramCB.nvm.param.protocol.driver.switchCruiseMode = FALSE; 						// 巡航切换模式
////	paramCB.nvm.param.protocol.driver.switchCruiseWay = FALSE;							// 切换巡航的方式
////	paramCB.nvm.param.protocol.driver.assistSensitivity = 3;							// 助力灵敏度
////	paramCB.nvm.param.protocol.driver.assistStartIntensity = 3; 						// 助力启动强度
////	paramCB.nvm.param.protocol.driver.assistSteelType = 12;								// 助力磁钢类型
////	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum = 0;						// 限速电机换向霍尔磁钢数(未使用)
////	paramCB.nvm.param.protocol.driver.turnBarSpeedLimit = 55;							// 转把限速
////	paramCB.nvm.param.protocol.driver.motorFuture = 100;								// 电机特性参数
////	paramCB.nvm.param.protocol.driver.turnBarFunction = 0;								// 转把功能
////	paramCB.nvm.param.protocol.driver.motorPulseCode = 0;								// 电机相位编码
////	paramCB.nvm.param.protocol.driver.assistSensorSignalNum = 6;						// 助力传感器信号数
////	paramCB.nvm.param.protocol.driver.assistTrim = 1;									// 助力微调
////	paramCB.nvm.param.protocol.driver.absBrake = 2; 									// ABS刹车
////	paramCB.nvm.param.protocol.driver.hmiProperty = 70; 								// 仪表属性
////	paramCB.nvm.param.protocol.driver.assistSwitch = TRUE;								// 助力开关
////	paramCB.nvm.param.protocol.driver.speedLimitSwitch = FALSE; 						// 限速开关
////	paramCB.nvm.param.common.pushSpeedSwitch = 1;										// 默认有推车助力
////	
////	paramCB.nvm.param.protocol.wheelSizeID = PARAM_WHEEL_SIZE_26_INCH;					// 轮径
////	paramCB.nvm.param.protocol.pushSpeedLimit = 250; 									// 推车助力限速值，单位:0.1Km/h
////	paramCB.nvm.param.protocol.powerMax = 250;											// 最大电机功率,单位:W

////	paramCB.nvm.param.protocol.newWheelSizeInch = 275;									// 新轮径
////	paramCB.nvm.param.protocol.newPerimeter = 2220;										// 新的轮径周长
////	paramCB.nvm.param.common.uartProtocol = 0;											// 协议选择 0：KM5S	1：锂电2号		2：八方	 3：J协议
////	paramCB.nvm.param.common.pushAssistSwitch=0;										// 推车助力										
////	paramCB.nvm.param.protocol.eBikeName = 0;											// 车名	0：PACE500 	1：PACE350	2：LEVEL	3：SINCH	4：AVENTURE  5：PACE
////	paramCB.nvm.param.protocol.powerMax = 250;											// 最大电机功率,单位:W	
////	paramCB.nvm.param.protocol.batteryCap = 0;											// 电池容量
////	paramCB.nvm.param.protocol.showWheelSizeInch = 0;									// 显示周长
////	paramCB.nvm.param.protocol.tiresSize = 0;											// 车轮宽度
////	
////	paramCB.nvm.param.protocol.runProtocol =0;											//协议
////	paramCB.nvm.param.common.defaultAssist = 1;											// 默认档位
////	paramCB.nvm.param.common.logoMenu = 0;												// LOGO界面
////	paramCB.nvm.param.common.uartLevel = 1;												// 串口电平		0:--3.3V       1:--5V
////	paramCB.nvm.param.protocol.driver.cruiseSwitch = FALSE;								// 定速巡航功能开关
////	paramCB.nvm.param.protocol.beepSwitch = FALSE;										// 蜂鸣器开关
////	paramCB.nvm.param.protocol.limitSpeedBeepOn = 0;									// 高速蜂鸣器提醒
////	paramCB.nvm.param.protocol.resFactorySet = TRUE;									// 恢复到出厂设置

////	paramCB.nvm.param.protocol.menuPassword	= PARAM_NVM_MENU_PASSWORD;					// 菜单界面密码
////	paramCB.nvm.param.protocol.powerOnPassword = PARAM_NVM_POWERON_PASSWORD;			// 开机界面密码
////	paramCB.nvm.param.protocol.powerPasswordSwitch = 0;									// 关闭开机密码 0是有密码 1是无密码
////	paramCB.nvm.param.protocol.menuPasswordSwitch = 0;									// 关闭菜单密码 0是有密码 1是无密码
////	paramCB.nvm.param.protocol.carModel[0] = 'B';										// 车型
////	paramCB.nvm.param.protocol.carModel[1] = 'N';
////	
////	paramCB.nvm.param.protocol.driver.smartType = SMART_TYPE_S;							// S_智能车类型
////	paramCB.nvm.param.protocol.driver.assistModeEnable = FALSE;							// 助力模式开关
////	
////	paramCB.nvm.param.protocol.driver.assistMode = ASSIST_MODE_TORQUE;					// 助力模式
////	
////	// 根据最大档位，设定每个档位PWM值的上下限
////	NVM_SetPWMLimitValue();

////	// 保存参数
////	NVM_Save(TRUE);

////}

//// 判断NVM是否已格式化
//BOOL NVM_IsFormatOK(void)
//{
//	uint16 i;

//	// 逐个比较，只要发现一个不同，即认为未格式化
//	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
//	{
//		if(formatFlag[i] != paramCB.nvm.param.common.nvmFormat[i])
//		{
//			return FALSE;
//		}
//	}

//	return TRUE;
//}

//// 设置脏标识
//void NVM_SetDirtyFlag(BOOL writeAtOnce)
//{
//	paramCB.nvmWriteRequest = TRUE;

//	if (writeAtOnce)
//	{
//		paramCB.nvmWriteEnable = TRUE;
//	}
//}

//// 新的一天开始啦!!
//// 当前的数据与历史记录比较，并且清除今日数据
//void PARAM_NewDayStart(void)
//{
//	// 当前数据与历史记录比较
//	// 今日里程＞历史最佳，更新之
//	if (paramCB.nvm.param.common.record.today.trip > paramCB.nvm.param.common.record.total.maxTripOf24hrs)
//	{
//		paramCB.nvm.param.common.record.total.maxTripOf24hrs = paramCB.nvm.param.common.record.today.trip;
//	}

//	// 今日里程清零
//	paramCB.nvm.param.common.record.today.trip = 0;

//	// 今日骑行时间清零
//	paramCB.nvm.param.common.record.today.ridingTime = 0;

//	// 今日骑行卡路里清零
//	paramCB.nvm.param.common.record.today.calories = 0;

//	// 保存新日期
//	paramCB.nvm.param.common.record.today.day = paramCB.runtime.rtc.day;
//	paramCB.nvm.param.common.record.today.month = paramCB.runtime.rtc.month;
//	paramCB.nvm.param.common.record.today.year = paramCB.runtime.rtc.year;

//	// 计算自Trip清零以来的平均速度，计算公式:Trip以来总里程 ÷ Trip以来骑行时间
//	//if(0 != paramCB.nvm.param.common.record.trip.ridingTime)
//	//{
//	//	paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.distance * 3600 / paramCB.nvm.param.common.record.trip.ridingTime;
//	//}

//	// NVM更新，非立即写入
//	NVM_SetDirtyFlag(FALSE);	
//}

///*********************************************************************
//* 函 数 名: PARAM_CaculateCaloriesByParameters
//* 函数入参: ASSIST_ID_E assistLevel 		示例: 档位
//			uint16 workAssistRatio		示例: 助力比	
//			uint16 motorPower			示例: 电机功率,单位:W
//			uint16 speed				示例: 速度，单位:0.1km/h
//			uint16 torque				示例: 力矩, 单位:N.m
//			uint16 time					示例: 时间, 单位:s
//* 函数出参: 无
//* 返 回 值: uint16	示例: 返回采样时间内的卡路里
//* 功能描述:   根据档位、电机功率、助力比、速度等信息计算出卡路里
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年06月21日
//*     修改描述: 新函数 		   
//**********************************************************************/
//uint16 PARAM_CaculateCaloriesByParameters(ASSIST_ID_E assistLevel, uint16 workAssistRatio, uint16 motorPower, uint16 speed, uint16 torque, uint16 time)
//{
//	uint32 uwPowerByPeople = 0;			// 单位:W
//	uint16 uwCalories = 0;				// 单位:卡
//	float fCalCoff = 1 / 4.184;			// 1 W = 1 kj/ms = 1 j/s; 1 卡 = 4.184 j
//	
//	// 计算人产生的功率
//	if ( ASSIST_0 == assistLevel )
//	{
//		// 无助力，根据速度进行计算， P=F*V; F=G*f; G是人与车的重量，f是摩擦系数
//		uwPowerByPeople = 750UL * speed / 3600;		// 750 * 0.01 * speed * 100.f / 3600
//	}
//	else
//	{
//		// 有助力，根据助力比与电机功率来进行计算，示例:助力比为3/1，表示电机出3份力，人出1份力		
//		uwPowerByPeople = workAssistRatio ? (motorPower / workAssistRatio) : 0;
//	}

//	// 积分计算卡路里,单位:cal
//	uwCalories = uwPowerByPeople * (time / 1000.0) * fCalCoff;

//	return uwCalories;
//}

///*********************************************************************
//* 函 数 名: PARAM_CaculateTripDistance
//* 函数入参: PARAM_CB* pParamCB
//* 函数出参: 无
//* 返 回 值: void 
//* 功能描述: 参数计算过程，包括速度、里程、卡路里等等
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年08月06日
//*     修改描述: 新函数 		   
//**********************************************************************/
//void PARAM_CaculateTripDistance(void)
//{
//	float fTempAvgSpeed = 0.0;
//	float fTempTrip = 0.0;
//	
//	static float tripBuff = 0.0;
//		
//	// 计算单次小里程，单位:0.1KM，公式: 速度*采样时间
//	fTempTrip = (float)paramCB.runtime.speed * PARAM_GetParamCycleTime() / 3600000.f;

//	tripBuff += fTempTrip;		// Trip缓冲区

//	// 自Trip清零以来的里程、今天骑行里程刷新
//	if (tripBuff > 1.0f)
//	{
//		tripBuff -= 1.0f;

//		// 自Trip清零以来的里程刷新
//		paramCB.nvm.param.common.record.trip.distance ++;

//		// 今天骑行里程刷新
//		paramCB.nvm.param.common.record.today.trip ++;

//		// 个人单日骑行最大里程刷新
//		if ( paramCB.nvm.param.common.record.total.maxTripOf24hrs < paramCB.nvm.param.common.record.today.trip )	// 刷新个人最佳
//		{
//			paramCB.nvm.param.common.record.total.maxTripOf24hrs = paramCB.nvm.param.common.record.today.trip;
//		}

//		// 自出厂以来总里程刷新
//		paramCB.nvm.param.common.record.total.distance ++;
//		
//		// 计算自Trip清零以来的平均速度，公式:Trip清零以来 / Trip清零以来骑行时间
//		// 骑行时间为0时的处理
//		if(0 == paramCB.nvm.param.common.record.trip.ridingTime)
//		{
//			fTempAvgSpeed = 0;
//		}
//		// 骑行时间不为0时的处理
//		else
//		{
//			fTempAvgSpeed = (float)paramCB.nvm.param.common.record.trip.distance * 3600.0f  / paramCB.nvm.param.common.record.trip.ridingTime;
//		}

//		if (abs(fTempAvgSpeed - paramCB.runtime.trip.speedAverage) >= 1)
//		{
//			paramCB.runtime.trip.speedAverage = fTempAvgSpeed;

//			// 平均速度不能大于最大速度
//			if (paramCB.runtime.trip.speedAverage >= paramCB.nvm.param.common.record.trip.speedMax)
//			{
//				paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.speedMax * 0.9f;
//			}
//		}

//		// 硬件修改无E2PPROM，里程数据修改为1KM写入Flash一次
//		if ((paramCB.nvm.param.common.record.total.distance - paramCB.nvm.param.common.record.total.preDistance) >= 10)
//		{
//			// 更新比较值
//			paramCB.nvm.param.common.record.total.preDistance = paramCB.nvm.param.common.record.total.distance;
//			
//			// NVM更新,立即写入
//			NVM_SetDirtyFlag(TRUE);
//		}
//	}
//}

//// 卡路里计算
//void PARAM_CaculateCalories(void)
//{
//	static uint16 calorieBuff = 0;

//	// 获取单个参数上报周期的卡路里，添加到缓冲区中
//	calorieBuff += PARAM_CaculateCaloriesByParameters(paramCB.runtime.assist,
//												PARAM_GetWorkAssistRatio(), 
//												paramCB.runtime.power, 
//												paramCB.runtime.speed, 
//												paramCB.runtime.torque, 
//												paramCB.runtime.paramCycleTime);

//	// 缓存的卡路里超过一定值时，保存到NVM中
//	if (calorieBuff >= 1000)
//	{
//		calorieBuff -= 1000;

//		// 自出厂以来的卡路里增加
//		paramCB.nvm.param.common.record.total.calories ++;
//		
//		// 自Trip清零以来的卡路里增加
//		paramCB.nvm.param.common.record.trip.calories ++;

//		// 自0点以来的卡路里增加
//		paramCB.nvm.param.common.record.today.calories ++;

//		// NVM更新，非立即写入
//		NVM_SetDirtyFlag(FALSE);
//	}
//}

//// 计算速度类
//void PARAM_CaculateSpeed(void)
//{
//	// 计算自Trip清零以来最高速度
//	if ( paramCB.nvm.param.common.record.trip.speedMax < paramCB.runtime.speed )
//	{
//		paramCB.nvm.param.common.record.trip.speedMax = paramCB.runtime.speed;

//		// NVM更新，非立即写入
//		NVM_SetDirtyFlag(FALSE);
//	}

//	// 设置超速告警
//	if ( paramCB.runtime.speed >= paramCB.nvm.param.common.speed.limitVal )
//	{		
//		paramCB.runtime.overSpeedAlarm = TRUE;
//	}
//	else
//	{
//		paramCB.runtime.overSpeedAlarm = FALSE;
//	}

//	// 计算用户最大速度
//	if (paramCB.runtime.speed > paramCB.runtime.record.userMaxSpeed)
//	{
//		paramCB.runtime.record.userMaxSpeed = paramCB.runtime.speed;
//	}
//}

//// 计算骑行时间
//void PARAM_CALLBACK_CaculateRidingTime(uint32 param)
//{
//	// 当前时间与系统保存的今日骑行日期不一致，认为是新的一天开始了
//	if ((paramCB.runtime.rtc.year != paramCB.nvm.param.common.record.today.year)
//	|| (paramCB.runtime.rtc.day != paramCB.nvm.param.common.record.today.day)
//	|| (paramCB.runtime.rtc.month != paramCB.nvm.param.common.record.today.month))
//	{
//		PARAM_NewDayStart();
//	}

//	// 速度为0，骑行时间不计时
//	if (0 == paramCB.runtime.speed)
//	{
//		// 运动时间清零，静止时间递增
//		paramCB.runtime.record.moveKeepTime = 0;

//		if (paramCB.runtime.record.stopKeepTime < 0xFFFF)
//		{
//			paramCB.runtime.record.stopKeepTime ++;
//		}
//		
//		return ;
//	}
//	// 速度不为0，说明在运动
//	else
//	{
//		// 运动时间递增，静止时间清零
//		if (paramCB.runtime.record.moveKeepTime < 0xFFFF)
//		{
//			paramCB.runtime.record.moveKeepTime ++;
//		}
//		paramCB.runtime.record.stopKeepTime = 0;
//	}

//	// 非记录状态不进行计算
//	//if (RECORD_STATE_STANDBY == RECORD_GetState())
//	//{
//	//	return;
//	//}

//	// 自出厂以来的骑行时间增加
//	paramCB.nvm.param.common.record.total.ridingTime ++;

//	// 自Trip清零以来的骑行时间增加
//	paramCB.nvm.param.common.record.trip.ridingTime ++;

//	// 自0点以来的骑行时间增加
//	paramCB.nvm.param.common.record.today.ridingTime ++;

//	// NVM更新，非立即写入，默认用定时器定时写入，否则存在很快把Flash写坏的风险
//	NVM_SetDirtyFlag(FALSE);

//	// 根据电池电压，计算电池电量百分比
//	// 电量计算由ADC模块完成
//}

///*********************************************************************
//* 函 数 名: PARAM_Init
//* 函数入参: void
//* 函数出参: 无
//* 返 回 值: uint32 
//* 功能描述: 初始化参数
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年08月06日
//*     修改描述: 新函数 		   
//**********************************************************************/
//void PARAM_Init(void)
//{
//	// 读取NVM
//	NVM_Load();
//	
//	// 未格式化，则格式化
//	if (!NVM_IsFormatOK())
//	{
//		// 保证擦写的时候有电
//		POWER_ON();
//		
//		NVM_Format();
//	}
//	
//	// 设置默认的运行数据
//	PARAM_SetDefaultRunningData();
//	
//	// 计算自Trip清零以来的平均速度，计算公式:Trip以来总里程 ÷ Trip以来骑行时间
//	if (0 != paramCB.nvm.param.common.record.trip.ridingTime)
//	{
//		paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.distance * 3600.0f / paramCB.nvm.param.common.record.trip.ridingTime;

//		// 平均速度不能大于最大速度
//		if (paramCB.runtime.trip.speedAverage >= paramCB.nvm.param.common.record.trip.speedMax)
//		{
//			paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.speedMax * 0.9f;
//		}
//	}

//	// 注册计算骑行时间定时器
//	TIMER_AddTask(TIMER_ID_PARAM_TIME,
//					1000UL,
//					PARAM_CALLBACK_CaculateRidingTime,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

//	// 注册间隔写数据到NVM定时器
//	TIMER_AddTask(TIMTR_ID_NVM_WRITE_REQUEST,
//					600000UL,
//					NVM_CALLBACK_EnableWrite,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);
//	
//	// 擦写完恢复仪表上电时自锁先关的状态
//	POWER_OFF();
//}

///*********************************************************************
//* 函 数 名: PARAM_Process
//* 函数入参: void
//* 函数出参: 无
//* 返 回 值: void 
//* 功能描述: 模块过程处理函数
//***********
//* 修改历史:
//*   1.修改作者: ZJ
//*     修改日期: 2017年08月06日
//*     修改描述: 新函数 		   
//**********************************************************************/
//void PARAM_Process(void)
//{
//	// 参数需要重新计算时，计算
//	if (paramCB.recaculateRequest)
//	{
//		// 清除计算请求
//		paramCB.recaculateRequest = FALSE;
//		
//		// 计算速度类数据
//		PARAM_CaculateSpeed();

//		// 计算里程类数据
//		PARAM_CaculateTripDistance();

//		// 计算卡路里类数据
//		PARAM_CaculateCalories();
//		
//		// 计算骑行时间类数据
//		// 时间类数据由定时器调度
//	}

//	// 计算功率类数据
//	paramCB.runtime.power = 1.0f * paramCB.runtime.battery.voltage * paramCB.runtime.battery.current / 1000000.0f;

//	// NVM有写入请求，并且写入使能时，才可以写入
//	if ((paramCB.nvmWriteRequest) && (paramCB.nvmWriteEnable))
//	{
//		paramCB.nvmWriteRequest = FALSE;
//		paramCB.nvmWriteEnable = FALSE;

//		// 重新复位写数据到NVM定时器
//		TIMER_ResetTimer(TIMTR_ID_NVM_WRITE_REQUEST);
//		
//		// 差异化保存
//		NVM_Save(FALSE);
//	}
//	// 判断是否需要恢复出厂设置
//	if (!PARAM_GetResFactorySet())
//	{
//		PARAM_SetResFactorySet(TRUE);
//		PARAM_FactoryReset();  
//	}
//}

//// Trip清零
//void PARAM_ClearTrip(void)
//{
//	// Trip里程清零
//	paramCB.nvm.param.common.record.trip.distance = 0;

//	// 平均速度和最高速度清零
//	paramCB.runtime.trip.speedAverage = 0;
//	paramCB.nvm.param.common.record.trip.speedMax = 0;

//	// Trip骑行时间清零
//	paramCB.nvm.param.common.record.trip.ridingTime = 0;

//	// Trip骑行卡路里清零
//	paramCB.nvm.param.common.record.trip.calories = 0;

//	// NVM更新，立即写入
//	NVM_SetDirtyFlag(TRUE);
//}

//uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
//{
//	uint16 i = 0;

//	while (i < len)
//	{
//		if (*(str1 +i) != *(str2 +i))
//		{
//			return 0;
//		}
//		i++;
//	}
//	return 1;
//}

//// 获取对应轮径ID的周长
//uint16 PARAM_GetPerimeter(uint8 wheelSizeID)
//{
//	CHECK_PARAM_OVER_SAME_RETURN_VALUE(wheelSizeID, PARAM_WHEEL_SIZE_MAX, NULL);
//	
//	return (uint16)paramCB.runtime.perimeter[wheelSizeID];
//}

//// 设置对应轮径ID的周长
//void PARAM_SetPerimeter(uint8 wheelSizeID, uint8 perimeterMs)
//{
//	CHECK_PARAM_OVER_SAME_RETURN(wheelSizeID, PARAM_WHEEL_SIZE_MAX);

//	paramCB.runtime.perimeter[wheelSizeID] = perimeterMs;
//}


//// 获取速度接口
//uint16 PARAM_GetSpeed(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.runtime.speed;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.runtime.speed;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.runtime.speed;
//			break;
//	}

//	return (uint16)temp;
//}

//uint32 PARAM_GetTrip(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.record.trip.distance;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.record.trip.distance;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.record.trip.distance;
//			break;
//	}

//	return temp;
//}

//uint16 PARAM_GetTodayTrip(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.record.today.trip;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.record.today.trip;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.record.today.trip;
//			break;
//	}

//	return (uint16)temp;
//}

//uint16 PARAM_GetPersonalBestTrip(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.record.total.maxTripOf24hrs;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.record.total.maxTripOf24hrs;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.record.total.maxTripOf24hrs;
//			break;
//	}

//	return (uint16)temp;
//}

//uint32 PARAM_GetTotalDistance(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.record.total.distance;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.record.total.distance;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.record.total.distance;
//			break;
//	}

//	return temp;
//}

//uint16 PARAM_GetTripAverageSpeed(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.runtime.trip.speedAverage;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.runtime.trip.speedAverage;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.runtime.trip.speedAverage;
//			break;
//	}

//	return (uint16)temp;
//}

//uint16 PARAM_GetTripMaxSpeed(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.record.trip.speedMax;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.record.trip.speedMax;
//			temp *= METRIC_TO_INCH_COFF;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.record.trip.speedMax;
//			break;
//	}

//	return (uint16)temp;
//}

//uint16 PARAM_GetSpeedLimit(void)
//{
//	uint32 temp;
//	
//	switch (paramCB.nvm.param.common.unit)
//	{
//		case UNIT_METRIC:
//			temp = paramCB.nvm.param.common.speed.limitVal;
//			break;

//		case UNIT_INCH:
//			temp = paramCB.nvm.param.common.speed.limitVal;
//			temp = (temp * METRIC_TO_INCH_COFF) + 0.5f;
//			break;

//		default:
//			temp = paramCB.nvm.param.common.speed.limitVal;
//			break;
//	}

//	return (uint16)temp;
//}

//ASSIST_ID_E PARAM_GetAssistLevel(void)
//{
//	if (PARAM_IsPushAssistOn())
//	{
//		return ASSIST_P;
//	}
//	else
//	{
//		return paramCB.runtime.assist;
//	}
//}

//// 返回出厂设置
//void PARAM_FactoryReset(void)
//{
//	// 格式化参数
//	//NVM_Format();

//	//uint8 i;

//	// 写格式化标识
//	//for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
//	//{
//	//	paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
//	//}

//	// ■■ 需要保存至NVM的参数 ■■
//	// 设置系统参数
//	//paramCB.nvm.param.common.unit = UNIT_METRIC;										// 单位, 公制与英制
//	//paramCB.nvm.param.common.brightness = BACK_LIGHT_LEVEL_ON;							// 背光亮度
//	//paramCB.nvm.param.common.powerOffTime = 10;											// 自动关机时间，单位:分钟；>=0min									// 机器与人做功助力比
//	
//	// 自出厂以来的参数初始化
//	//paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// 个人记录，单位:0.1Km
//	//paramCB.nvm.param.common.record.total.distance = 0;					// 总里程，单位:0.1Km
//	//paramCB.nvm.param.common.record.total.ridingTime = 0;				// 总骑行时间，单位:s
//	//paramCB.nvm.param.common.record.total.calories = 0;					// 总卡路里，单位:KCal

//	// 自Trip清零以来的参数初始化
//	paramCB.nvm.param.common.record.trip.ridingTime = 0;				// 总骑行时间秒
//	paramCB.nvm.param.common.record.trip.distance = 0;					// Trip，单位:0.1Km
//	paramCB.nvm.param.common.record.trip.calories = 0;					// 卡路里，单位:KCal
//	paramCB.nvm.param.common.record.trip.speedMax = 0;					// 最大速度	
//	paramCB.runtime.trip.speedAverage = 0;								// 平均速度，单位:0.1Km/h	

//	// 自0点以来的参数初始化
//	paramCB.nvm.param.common.record.today.trip = 0;						// 今日里程，单位:0.1Km		
//	paramCB.nvm.param.common.record.today.calories = 0;					// 今日卡路里，单位:KCal
//	paramCB.nvm.param.common.record.today.ridingTime = 0;				// 今日骑行时间清零
//	//paramCB.nvm.param.common.record.today.month = 1;						// 今日参数对应的日期
//	//paramCB.nvm.param.common.record.today.day = 1;						// 今日参数对应的日期
//	//paramCB.nvm.param.common.record.today.year = 2019;					// 今日参数对应的日期

//	// 保存参数
//	//NVM_Save(TRUE);
//	NVM_Load_FactoryReset();
//}

////uint16 PARAM_GetPushSpeedLimit(void)
////{
////	uint32 temp;
////	
////	switch (paramCB.nvm.param.common.unit)
////	{
////		case UNIT_METRIC:
////			temp = paramCB.nvm.param.protocol.pushSpeedLimit;
////			break;

////		case UNIT_INCH:
////			temp = paramCB.nvm.param.protocol.pushSpeedLimit;
////			temp *= METRIC_TO_INCH_COFF;
////			break;

////		default:
////			temp = paramCB.nvm.param.protocol.pushSpeedLimit;
////			break;
////	}

////	return (uint16)temp;
////}

////// 获取档位PWM上限值,注意外面需要判断读数是否非法
////uint8 PARAM_GetPwmTopLimit(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmTop[asist];
////}

////// 获取档位PWM下限值,注意外面需要判断读数是否非法
////uint8 PARAM_GetPwmBottomLimit(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmBottom[asist];
////}

////// 获取档位PWM固定值
////uint8 PARAM_GetPwmFixed(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmFixed[asist];
////}

////// 设置档位PWM限值
////void PARAM_SetPwmLimit(uint8 asist, uint8 pwm)
////{
////	CHECK_PARAM_OVER_SAME_RETURN(asist, ASSIST_ID_COUNT);

////	paramCB.nvm.param.protocol.pwmTop[asist] = pwm;
////}


