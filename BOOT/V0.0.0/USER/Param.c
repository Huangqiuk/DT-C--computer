///********************************************************************************
//  * �� �� ��: Param.c
//  * �� �� ��: ����
//  * �޸�����: Fly & ZJ & Workman
//  * �޸�����: 2017��08��06��
//  * ���ܽ���: ���ļ��������в����Ļ�ȡ����㣬�ṩ�ɿ������ݸ��ϲ���ʾ����            
//  ******************************************************************************
//  * ע������:
//  *
//  * 							��Ȩ���̫�Ƽ�����.
//  *
//*********************************************************************************/

///*******************************************************************************
// *                                  ͷ�ļ�	                                   *
//********************************************************************************/
#include "Param.h"
//#include "iap.h"
//#include "timer.h"
//#include "powerCtl.h"
//#include "uartProtocol.h"
//#include "uartDrive.h"

//// ��������
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
// *                                  ��������                                   *
//********************************************************************************/
//PARAM_CB paramCB;

//const uint32 PARAM_MCU_VERSION_ADDRESS[] = {PARAM_MCU_DT_SN_ADDEESS, PARAM_MCU_CUSTOMER_SN_ADDEESS, PARAM_MCU_HW_ADDEESS, \
//											PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, \
//											PARAM_MCU_BLE_UPDATA_CRC_ADDEESS, PARAM_MCU_QR_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS	};
//// NVM��ʽ����ʶ
//const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

//uint8 flashBuf[PARAM_MCU_PAGE_SIZE];
//uint8 userSetFlashBuf[PARAM_MCU_PAGE_SIZE];
//uint32 flashBackupFlag;

//// ����ϵͳ����
//void NVM_Load(void)
//{
//	// ��ȡNVM�е�����
//	/*IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// ����ͬʱ���汸��
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.preValue, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);*/
////==========================================================================================
//	// ��ȡ�������ı�ʶ�����Ƿ���Ч��־
//	IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//							(uint32*)&flashBackupFlag, 
//							1);

//	// ����������Ч
//	if (0x12345678 == flashBackupFlag)
//	{
//		// �������������ݵ�������
//		IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS, 
//								(uint32*)flashBuf, 
//								PARAM_MCU_PAGE_SIZE / 4);

//		// ������������д������������
//		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//								(uint32*)flashBuf, 
//								PARAM_MCU_PAGE_SIZE / 4);

//		// �屸�����ݱ�־
//		flashBackupFlag = 0x00000000;
//		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//								(uint32*)&flashBackupFlag, 
//								1);
//	}

//	// ��ȡNVM�е�����
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32 *)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// ����ϵͳ����
//void NVM_Save(BOOL saveAll)
//{
//	// ֻ����仯������
//	/*IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//						(uint32*)paramCB.nvm.array, 
//						(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// ����ͬʱ���汸��
//	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
//							(uint32*)paramCB.preValue, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);*/
////==============================================================================	
//	// �ȴ����һ��������ȡ���ݿ����������ڶ����������ݣ�Ȼ��д��һ����־�����������Ÿ������һ�������������������ı�־
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
//	// ֻ����仯������
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//									(uint32*)paramCB.nvm.array, 
//									(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// ���־
//	flashBackupFlag = 0x00000000;
//	
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + (PARAM_MCU_PAGE_SIZE - 4), 
//							(uint32*)&flashBackupFlag, 
//							1);
//}

//// ��ʼ�����ð����
//void NVM_Load_FactoryReset(void)
//{ 
//	uint16 value, i =0;
//	
//	// �ӱ�����ַ����ȡ�������ݿ�����ϵͳ��������
//	IAP_FlashReadWordArray(PARAM_NVM_FACTORY_RESET_ADDRESS, 
//							(uint32*)paramCB.nvm.array, 
//							(PARAM_NVM_DATA_SIZE + 3) / 4);

//	// �ж��������Ƿ�Ϊ��
//	for (i = 0;i < PARAM_NVM_DATA_SIZE; i++)
//	{
//		if (0xFF == paramCB.nvm.array[i])
//		{
//			value ++;
//		}
//	}

//	// ������ȫ�����ǿգ�ֱ�ӷ��أ���д�룬��ֹ����
//	if (PARAM_NVM_DATA_SIZE == value)
//	{
//		return;
//	}
//	// ֻ����仯������
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
//									(uint32*)paramCB.nvm.array, 
//									(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// �����ʼ�����ò���
//void NVM_Save_FactoryReset(BOOL saveAll)
//{
//	// ֻ����仯������
//	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_FACTORY_RESET_ADDRESS, 
//						(uint32*)paramCB.nvm.array, 
//						(PARAM_NVM_DATA_SIZE + 3) / 4);
//}

//// nvmдģʽ
//void NVM_CALLBACK_EnableWrite(uint32 param)
//{
//	paramCB.nvmWriteEnable = TRUE;
//}

//// Ĭ�����в���
//void PARAM_SetDefaultRunningData(void)
//{
//	uint8 i;
//	uint8 * pBuff = NULL;
//	// ���� ����Ҫ������NVM�Ĳ��� ����

//	// ���� ����Ҫ������NVM�Ĳ��� ����
//	paramCB.runtime.fwVer = FW_VERSION;
//	paramCB.runtime.hwVer = HW_VERSION;
//	paramCB.runtime.uiVersion = UI_VERSION;
//	paramCB.runtime.btCapacity = BT_CAPACITY;
//	
//	// �̼��汾��
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
//	// �Ǳ�Ψһ���к�
//	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0);
//	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F4); 
//	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F8);

//	pBuff = (uint8 *)&(paramCB.runtime.appVersion);
//	
//	IAP_FlashReadWordArray(PARAM_MCU_APP_ADDEESS,(uint32 *)pBuff, 8);
//	
//	// �����ȡ�İ汾�Ų�������ǿ�Ƹ���Ϊ��ǰAPP�汾
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
//	// ���������в���
//	paramCB.runtime.cruiseEnabe = FALSE;			// Ѳ�����أ�0:��ʾ�رգ�1:��ʾ�� 
//	paramCB.runtime.cruiseState = FALSE; 			// ��ǰѲ��״̬��1:��ʾ����Ѳ��״̬ 
//	paramCB.runtime.pushAssistOn = FALSE; 			// 6km/h���й��ܣ�0:��ʾ�رգ�1:��ʾ��
//	paramCB.runtime.isHavePushAssist = TRUE;		// ���Ƴ���������(APP��ȡϵͳ��Ϣ��)
//	paramCB.runtime.lightSwitch = FALSE;			// ��ƿ���	

//	paramCB.runtime.battery.voltage = 40000;		// ��ǰ��ص�ѹ����λ:mV 	
//	paramCB.runtime.battery.current = 0;			// ��ǰ��ص�������λ:mA 	
//	paramCB.runtime.battery.lowVoltageAlarm = 0;	// ��������͸澯
//	//paramCB.runtime.battery.state = 0;				// ���״̬
//	//paramCB.runtime.battery.preState = 0;			// �����һ״̬
//	
//	paramCB.runtime.speed = 0; 						// ��ǰ�ٶȣ���λ:0.1Km/h
//	//paramCB.runtime.pulseNum = 0;					// ��ǰ�������

//	paramCB.runtime.errorType = ERROR_TYPE_NO_ERROR;		// ��ǰ�������
//	//paramCB.runtime.ridingState = SYSTEM_STATE_STOP;		// ��ǰ����״̬��0:��ʾ��ֹ��1:��ʾ����״̬��	
//	paramCB.runtime.overSpeedAlarm = FALSE; 				// ���ٸ澯��1:��ʾ���� 	
//	paramCB.runtime.underVoltageAlarm = FALSE;				// Ƿѹ�澯��0:��ʾ��Ƿѹ��1:��ʾǷѹ

//	//paramCB.runtime.torque = 0;								// ����
//	paramCB.runtime.paramCycleTime = 200;					// ��ʱ��ȡ������ʱ��
//	
//	// �����ʾ����
//	paramCB.runtime.battery.percent = 100;							// ��ذٷֱ�
//	paramCB.runtime.battery.batteryDataSrc = (BATTERY_DATA_SRC_E)paramCB.nvm.param.common.percentageMethod;		// �������㷽ʽ
//	paramCB.runtime.assist = ASSIST_0;								// ������λ	
//	paramCB.runtime.recordCount = 0;								// ����ͳ����������
//	//paramCB.runtime.ringStatus = 0;									// ��������

//	//paramCB.runtime.warningStatus = 0;								// ���/����
//	paramCB.runtime.power = 0;										// �������,��λ:W
//	paramCB.runtime.bleConnectState = FALSE;						// �����Ƿ����ӣ�1:��ʾ���ӣ�0:��ʾδ����

//	paramCB.runtime.bleRadio = 0;
//	
//	paramCB.runtime.battery.powerOffstate = 0;

//	paramCB.runtime.blueUpgradePer = 0;  							// ���������ٷֱ�
//	
//	// �����ܳ�����λ:mm
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
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_700C] = 2193;		// ����27.5

//	// ϲ��ʢ����Ҫ��
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_24_INCH] = 1820;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_27_5_INCH] = 2130;
//	paramCB.runtime.perimeter[PARAM_WHEEL_SIZE_700C] = 2130;		// ����27.5
//	
//	
//}

////// ����������λ���趨ÿ����λ��PWMֵ������
////void NVM_SetPWMLimitValue(void)
////{
////	switch (paramCB.nvm.param.common.assistMax)
////	{
////		// ��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�
////		//#warning "��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�"
////		case ASSIST_1:
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_1] = 96;
////			paramCB.nvm.param.protocol.pwmTop[ASSIST_P] = 40;
////			break;

////		// ����KDS����Ʒ���Եõ�������
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

////			// ϲ��ʢר��pwm 1���� 255* 0.235=60  2��:  255* 0.55=140  3����255 * 0.9=230
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_0] = 0;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_1] = 60;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_2] = 140;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_3] = 230;
////			
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_4] = 217;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_5] = 255;
////			paramCB.nvm.param.protocol.pwmFixed[ASSIST_P] = 50;
////			break;

////		// ����KDS����Ʒ���Եõ�������
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

////		// ��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�
////		#warning "��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�"
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

////		// ��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�
////		#warning "��������û�в��ԣ���Ҫ������Ʒ���Ժ󲹳�"
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

////// ��ʽ������
////void NVM_Format(void)
////{
////	uint8 i;
////	
////	// д��ʽ����ʶ
////	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
////	{
////		paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
////	}

////	// ���� ��Ҫ������NVM�Ĳ��� ����
////	// ����ϵͳ����
////	paramCB.nvm.param.common.battery.voltage = 36;										// ��ص�ѹ�ȼ�����λ:V
////	paramCB.nvm.param.common.battery.filterLevel = 1; 									// ������ʾ�仯�ٶȣ���λ:��
////	paramCB.nvm.param.common.speed.limitVal = 250;										// ����ֵ����λ:0.1Km/h
////	paramCB.nvm.param.common.speed.filterLevel = 3; 									// �ٶȱ仯ƽ����
////	paramCB.nvm.param.common.unit = UNIT_METRIC;										// ��λ, ������Ӣ��
////	paramCB.nvm.param.common.brightness = 0;												// ��������
////	paramCB.nvm.param.common.powerOffTime = 10; 										// �Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min
////	paramCB.nvm.param.common.assistMax = ASSIST_4;										// ֧�����λ
////	paramCB.nvm.param.common.busAliveTime = 10000;		// ���߹��ϳ�ʱʱ��
////	paramCB.nvm.param.common.workAssistRatio = 2;										// ������������������
////	
////	paramCB.nvm.param.common.existBle = TRUE;											// �Ƿ�������
////	paramCB.nvm.param.common.percentageMethod = BATTERY_DATA_SRC_BMS ;					// ������ȡ��ʽ(0:�������ϱ���ѹ) (1:�������ϱ�����) (2:�Ǳ��Լ�������ѹ����)

////	// �Գ��������Ĳ�����ʼ��
////	paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// ���˼�¼����λ:0.1Km
////	paramCB.nvm.param.common.record.total.distance = 0; 				// ����̣���λ:0.1Km
////	paramCB.nvm.param.common.record.total.ridingTime = 0;				// ������ʱ�䣬��λ:s
////	paramCB.nvm.param.common.record.total.calories = 0; 				// �ܿ�·���λ:KCal

////	// ��Trip���������Ĳ�����ʼ��
////	paramCB.nvm.param.common.record.trip.ridingTime = 0;				// ������ʱ����
////	paramCB.nvm.param.common.record.trip.distance = 0;					// Trip����λ:0.1Km
////	paramCB.nvm.param.common.record.trip.calories = 0;					// ��·���λ:KCal
////	paramCB.nvm.param.common.record.trip.speedMax = 0;					// ����ٶ� 
////	paramCB.runtime.trip.speedAverage = 0;								// ƽ���ٶȣ���λ:0.1Km/h	

////	// ��0�������Ĳ�����ʼ��
////	paramCB.nvm.param.common.record.today.trip = 0; 					// ������̣���λ:0.1Km 	
////	paramCB.nvm.param.common.record.today.calories = 0; 				// ���տ�·���λ:KCal
////	paramCB.nvm.param.common.record.today.ridingTime = 0;				// ��������ʱ������
////	paramCB.nvm.param.common.record.today.month = 1;					// ���ղ�����Ӧ������
////	paramCB.nvm.param.common.record.today.day = 1;						// ���ղ�����Ӧ������
////	paramCB.nvm.param.common.record.today.year = 2019;					// ���ղ�����Ӧ������

////	paramCB.nvm.param.protocol.driver.steelNumOfStartAssist = 2;						// ������ʼ�ĴŸ���
////	paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor = 1;						// ���ٴ�����һȦ�Ÿ���
////	paramCB.nvm.param.protocol.driver.assistDirectionForward = FALSE;					// ��������
////	paramCB.nvm.param.protocol.driver.assistPercent = 64;								// ��������
////	paramCB.nvm.param.protocol.driver.slowStart = 2;									// ����������
////	paramCB.nvm.param.protocol.driver.turnBarSpeed6kmphLimit = FALSE;					// ת���Ƿ�����6km/h
////	paramCB.nvm.param.protocol.driver.turnBarLevel = FALSE; 							// ת���Ƿ�ֵ�
////	paramCB.nvm.param.protocol.driver.currentLimit = 15000; 							// �������ޣ���λ:mA
////	paramCB.nvm.param.protocol.driver.lowVoltageThreshold = 30000;						// Ƿѹ���ޣ���λ:mV

////	paramCB.nvm.param.protocol.driver.controlMode = 2;									// ����������ģʽ
////	paramCB.nvm.param.protocol.driver.zeroStartOrNot = FALSE;							// ���������������
////	paramCB.nvm.param.protocol.driver.switchCruiseMode = FALSE; 						// Ѳ���л�ģʽ
////	paramCB.nvm.param.protocol.driver.switchCruiseWay = FALSE;							// �л�Ѳ���ķ�ʽ
////	paramCB.nvm.param.protocol.driver.assistSensitivity = 3;							// ����������
////	paramCB.nvm.param.protocol.driver.assistStartIntensity = 3; 						// ��������ǿ��
////	paramCB.nvm.param.protocol.driver.assistSteelType = 12;								// �����Ÿ�����
////	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum = 0;						// ���ٵ����������Ÿ���(δʹ��)
////	paramCB.nvm.param.protocol.driver.turnBarSpeedLimit = 55;							// ת������
////	paramCB.nvm.param.protocol.driver.motorFuture = 100;								// ������Բ���
////	paramCB.nvm.param.protocol.driver.turnBarFunction = 0;								// ת�ѹ���
////	paramCB.nvm.param.protocol.driver.motorPulseCode = 0;								// �����λ����
////	paramCB.nvm.param.protocol.driver.assistSensorSignalNum = 6;						// �����������ź���
////	paramCB.nvm.param.protocol.driver.assistTrim = 1;									// ����΢��
////	paramCB.nvm.param.protocol.driver.absBrake = 2; 									// ABSɲ��
////	paramCB.nvm.param.protocol.driver.hmiProperty = 70; 								// �Ǳ�����
////	paramCB.nvm.param.protocol.driver.assistSwitch = TRUE;								// ��������
////	paramCB.nvm.param.protocol.driver.speedLimitSwitch = FALSE; 						// ���ٿ���
////	paramCB.nvm.param.common.pushSpeedSwitch = 1;										// Ĭ�����Ƴ�����
////	
////	paramCB.nvm.param.protocol.wheelSizeID = PARAM_WHEEL_SIZE_26_INCH;					// �־�
////	paramCB.nvm.param.protocol.pushSpeedLimit = 250; 									// �Ƴ���������ֵ����λ:0.1Km/h
////	paramCB.nvm.param.protocol.powerMax = 250;											// ���������,��λ:W

////	paramCB.nvm.param.protocol.newWheelSizeInch = 275;									// ���־�
////	paramCB.nvm.param.protocol.newPerimeter = 2220;										// �µ��־��ܳ�
////	paramCB.nvm.param.common.uartProtocol = 0;											// Э��ѡ�� 0��KM5S	1��﮵�2��		2���˷�	 3��JЭ��
////	paramCB.nvm.param.common.pushAssistSwitch=0;										// �Ƴ�����										
////	paramCB.nvm.param.protocol.eBikeName = 0;											// ����	0��PACE500 	1��PACE350	2��LEVEL	3��SINCH	4��AVENTURE  5��PACE
////	paramCB.nvm.param.protocol.powerMax = 250;											// ���������,��λ:W	
////	paramCB.nvm.param.protocol.batteryCap = 0;											// �������
////	paramCB.nvm.param.protocol.showWheelSizeInch = 0;									// ��ʾ�ܳ�
////	paramCB.nvm.param.protocol.tiresSize = 0;											// ���ֿ��
////	
////	paramCB.nvm.param.protocol.runProtocol =0;											//Э��
////	paramCB.nvm.param.common.defaultAssist = 1;											// Ĭ�ϵ�λ
////	paramCB.nvm.param.common.logoMenu = 0;												// LOGO����
////	paramCB.nvm.param.common.uartLevel = 1;												// ���ڵ�ƽ		0:--3.3V       1:--5V
////	paramCB.nvm.param.protocol.driver.cruiseSwitch = FALSE;								// ����Ѳ�����ܿ���
////	paramCB.nvm.param.protocol.beepSwitch = FALSE;										// ����������
////	paramCB.nvm.param.protocol.limitSpeedBeepOn = 0;									// ���ٷ���������
////	paramCB.nvm.param.protocol.resFactorySet = TRUE;									// �ָ�����������

////	paramCB.nvm.param.protocol.menuPassword	= PARAM_NVM_MENU_PASSWORD;					// �˵���������
////	paramCB.nvm.param.protocol.powerOnPassword = PARAM_NVM_POWERON_PASSWORD;			// ������������
////	paramCB.nvm.param.protocol.powerPasswordSwitch = 0;									// �رտ������� 0�������� 1��������
////	paramCB.nvm.param.protocol.menuPasswordSwitch = 0;									// �رղ˵����� 0�������� 1��������
////	paramCB.nvm.param.protocol.carModel[0] = 'B';										// ����
////	paramCB.nvm.param.protocol.carModel[1] = 'N';
////	
////	paramCB.nvm.param.protocol.driver.smartType = SMART_TYPE_S;							// S_���ܳ�����
////	paramCB.nvm.param.protocol.driver.assistModeEnable = FALSE;							// ����ģʽ����
////	
////	paramCB.nvm.param.protocol.driver.assistMode = ASSIST_MODE_TORQUE;					// ����ģʽ
////	
////	// �������λ���趨ÿ����λPWMֵ��������
////	NVM_SetPWMLimitValue();

////	// �������
////	NVM_Save(TRUE);

////}

//// �ж�NVM�Ƿ��Ѹ�ʽ��
//BOOL NVM_IsFormatOK(void)
//{
//	uint16 i;

//	// ����Ƚϣ�ֻҪ����һ����ͬ������Ϊδ��ʽ��
//	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
//	{
//		if(formatFlag[i] != paramCB.nvm.param.common.nvmFormat[i])
//		{
//			return FALSE;
//		}
//	}

//	return TRUE;
//}

//// �������ʶ
//void NVM_SetDirtyFlag(BOOL writeAtOnce)
//{
//	paramCB.nvmWriteRequest = TRUE;

//	if (writeAtOnce)
//	{
//		paramCB.nvmWriteEnable = TRUE;
//	}
//}

//// �µ�һ�쿪ʼ��!!
//// ��ǰ����������ʷ��¼�Ƚϣ����������������
//void PARAM_NewDayStart(void)
//{
//	// ��ǰ��������ʷ��¼�Ƚ�
//	// ������̣���ʷ��ѣ�����֮
//	if (paramCB.nvm.param.common.record.today.trip > paramCB.nvm.param.common.record.total.maxTripOf24hrs)
//	{
//		paramCB.nvm.param.common.record.total.maxTripOf24hrs = paramCB.nvm.param.common.record.today.trip;
//	}

//	// �����������
//	paramCB.nvm.param.common.record.today.trip = 0;

//	// ��������ʱ������
//	paramCB.nvm.param.common.record.today.ridingTime = 0;

//	// �������п�·������
//	paramCB.nvm.param.common.record.today.calories = 0;

//	// ����������
//	paramCB.nvm.param.common.record.today.day = paramCB.runtime.rtc.day;
//	paramCB.nvm.param.common.record.today.month = paramCB.runtime.rtc.month;
//	paramCB.nvm.param.common.record.today.year = paramCB.runtime.rtc.year;

//	// ������Trip����������ƽ���ٶȣ����㹫ʽ:Trip��������� �� Trip��������ʱ��
//	//if(0 != paramCB.nvm.param.common.record.trip.ridingTime)
//	//{
//	//	paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.distance * 3600 / paramCB.nvm.param.common.record.trip.ridingTime;
//	//}

//	// NVM���£�������д��
//	NVM_SetDirtyFlag(FALSE);	
//}

///*********************************************************************
//* �� �� ��: PARAM_CaculateCaloriesByParameters
//* �������: ASSIST_ID_E assistLevel 		ʾ��: ��λ
//			uint16 workAssistRatio		ʾ��: ������	
//			uint16 motorPower			ʾ��: �������,��λ:W
//			uint16 speed				ʾ��: �ٶȣ���λ:0.1km/h
//			uint16 torque				ʾ��: ����, ��λ:N.m
//			uint16 time					ʾ��: ʱ��, ��λ:s
//* ��������: ��
//* �� �� ֵ: uint16	ʾ��: ���ز���ʱ���ڵĿ�·��
//* ��������:   ���ݵ�λ��������ʡ������ȡ��ٶȵ���Ϣ�������·��
//***********
//* �޸���ʷ:
//*   1.�޸�����: ZJ
//*     �޸�����: 2017��06��21��
//*     �޸�����: �º��� 		   
//**********************************************************************/
//uint16 PARAM_CaculateCaloriesByParameters(ASSIST_ID_E assistLevel, uint16 workAssistRatio, uint16 motorPower, uint16 speed, uint16 torque, uint16 time)
//{
//	uint32 uwPowerByPeople = 0;			// ��λ:W
//	uint16 uwCalories = 0;				// ��λ:��
//	float fCalCoff = 1 / 4.184;			// 1 W = 1 kj/ms = 1 j/s; 1 �� = 4.184 j
//	
//	// �����˲����Ĺ���
//	if ( ASSIST_0 == assistLevel )
//	{
//		// �������������ٶȽ��м��㣬 P=F*V; F=G*f; G�����복��������f��Ħ��ϵ��
//		uwPowerByPeople = 750UL * speed / 3600;		// 750 * 0.01 * speed * 100.f / 3600
//	}
//	else
//	{
//		// �������������������������������м��㣬ʾ��:������Ϊ3/1����ʾ�����3�������˳�1����		
//		uwPowerByPeople = workAssistRatio ? (motorPower / workAssistRatio) : 0;
//	}

//	// ���ּ��㿨·��,��λ:cal
//	uwCalories = uwPowerByPeople * (time / 1000.0) * fCalCoff;

//	return uwCalories;
//}

///*********************************************************************
//* �� �� ��: PARAM_CaculateTripDistance
//* �������: PARAM_CB* pParamCB
//* ��������: ��
//* �� �� ֵ: void 
//* ��������: ����������̣������ٶȡ���̡���·��ȵ�
//***********
//* �޸���ʷ:
//*   1.�޸�����: ZJ
//*     �޸�����: 2017��08��06��
//*     �޸�����: �º��� 		   
//**********************************************************************/
//void PARAM_CaculateTripDistance(void)
//{
//	float fTempAvgSpeed = 0.0;
//	float fTempTrip = 0.0;
//	
//	static float tripBuff = 0.0;
//		
//	// ���㵥��С��̣���λ:0.1KM����ʽ: �ٶ�*����ʱ��
//	fTempTrip = (float)paramCB.runtime.speed * PARAM_GetParamCycleTime() / 3600000.f;

//	tripBuff += fTempTrip;		// Trip������

//	// ��Trip������������̡������������ˢ��
//	if (tripBuff > 1.0f)
//	{
//		tripBuff -= 1.0f;

//		// ��Trip�������������ˢ��
//		paramCB.nvm.param.common.record.trip.distance ++;

//		// �����������ˢ��
//		paramCB.nvm.param.common.record.today.trip ++;

//		// ���˵�������������ˢ��
//		if ( paramCB.nvm.param.common.record.total.maxTripOf24hrs < paramCB.nvm.param.common.record.today.trip )	// ˢ�¸������
//		{
//			paramCB.nvm.param.common.record.total.maxTripOf24hrs = paramCB.nvm.param.common.record.today.trip;
//		}

//		// �Գ������������ˢ��
//		paramCB.nvm.param.common.record.total.distance ++;
//		
//		// ������Trip����������ƽ���ٶȣ���ʽ:Trip�������� / Trip������������ʱ��
//		// ����ʱ��Ϊ0ʱ�Ĵ���
//		if(0 == paramCB.nvm.param.common.record.trip.ridingTime)
//		{
//			fTempAvgSpeed = 0;
//		}
//		// ����ʱ�䲻Ϊ0ʱ�Ĵ���
//		else
//		{
//			fTempAvgSpeed = (float)paramCB.nvm.param.common.record.trip.distance * 3600.0f  / paramCB.nvm.param.common.record.trip.ridingTime;
//		}

//		if (abs(fTempAvgSpeed - paramCB.runtime.trip.speedAverage) >= 1)
//		{
//			paramCB.runtime.trip.speedAverage = fTempAvgSpeed;

//			// ƽ���ٶȲ��ܴ�������ٶ�
//			if (paramCB.runtime.trip.speedAverage >= paramCB.nvm.param.common.record.trip.speedMax)
//			{
//				paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.speedMax * 0.9f;
//			}
//		}

//		// Ӳ���޸���E2PPROM����������޸�Ϊ1KMд��Flashһ��
//		if ((paramCB.nvm.param.common.record.total.distance - paramCB.nvm.param.common.record.total.preDistance) >= 10)
//		{
//			// ���±Ƚ�ֵ
//			paramCB.nvm.param.common.record.total.preDistance = paramCB.nvm.param.common.record.total.distance;
//			
//			// NVM����,����д��
//			NVM_SetDirtyFlag(TRUE);
//		}
//	}
//}

//// ��·�����
//void PARAM_CaculateCalories(void)
//{
//	static uint16 calorieBuff = 0;

//	// ��ȡ���������ϱ����ڵĿ�·���ӵ���������
//	calorieBuff += PARAM_CaculateCaloriesByParameters(paramCB.runtime.assist,
//												PARAM_GetWorkAssistRatio(), 
//												paramCB.runtime.power, 
//												paramCB.runtime.speed, 
//												paramCB.runtime.torque, 
//												paramCB.runtime.paramCycleTime);

//	// ����Ŀ�·�ﳬ��һ��ֵʱ�����浽NVM��
//	if (calorieBuff >= 1000)
//	{
//		calorieBuff -= 1000;

//		// �Գ��������Ŀ�·������
//		paramCB.nvm.param.common.record.total.calories ++;
//		
//		// ��Trip���������Ŀ�·������
//		paramCB.nvm.param.common.record.trip.calories ++;

//		// ��0�������Ŀ�·������
//		paramCB.nvm.param.common.record.today.calories ++;

//		// NVM���£�������д��
//		NVM_SetDirtyFlag(FALSE);
//	}
//}

//// �����ٶ���
//void PARAM_CaculateSpeed(void)
//{
//	// ������Trip������������ٶ�
//	if ( paramCB.nvm.param.common.record.trip.speedMax < paramCB.runtime.speed )
//	{
//		paramCB.nvm.param.common.record.trip.speedMax = paramCB.runtime.speed;

//		// NVM���£�������д��
//		NVM_SetDirtyFlag(FALSE);
//	}

//	// ���ó��ٸ澯
//	if ( paramCB.runtime.speed >= paramCB.nvm.param.common.speed.limitVal )
//	{		
//		paramCB.runtime.overSpeedAlarm = TRUE;
//	}
//	else
//	{
//		paramCB.runtime.overSpeedAlarm = FALSE;
//	}

//	// �����û�����ٶ�
//	if (paramCB.runtime.speed > paramCB.runtime.record.userMaxSpeed)
//	{
//		paramCB.runtime.record.userMaxSpeed = paramCB.runtime.speed;
//	}
//}

//// ��������ʱ��
//void PARAM_CALLBACK_CaculateRidingTime(uint32 param)
//{
//	// ��ǰʱ����ϵͳ����Ľ����������ڲ�һ�£���Ϊ���µ�һ�쿪ʼ��
//	if ((paramCB.runtime.rtc.year != paramCB.nvm.param.common.record.today.year)
//	|| (paramCB.runtime.rtc.day != paramCB.nvm.param.common.record.today.day)
//	|| (paramCB.runtime.rtc.month != paramCB.nvm.param.common.record.today.month))
//	{
//		PARAM_NewDayStart();
//	}

//	// �ٶ�Ϊ0������ʱ�䲻��ʱ
//	if (0 == paramCB.runtime.speed)
//	{
//		// �˶�ʱ�����㣬��ֹʱ�����
//		paramCB.runtime.record.moveKeepTime = 0;

//		if (paramCB.runtime.record.stopKeepTime < 0xFFFF)
//		{
//			paramCB.runtime.record.stopKeepTime ++;
//		}
//		
//		return ;
//	}
//	// �ٶȲ�Ϊ0��˵�����˶�
//	else
//	{
//		// �˶�ʱ���������ֹʱ������
//		if (paramCB.runtime.record.moveKeepTime < 0xFFFF)
//		{
//			paramCB.runtime.record.moveKeepTime ++;
//		}
//		paramCB.runtime.record.stopKeepTime = 0;
//	}

//	// �Ǽ�¼״̬�����м���
//	//if (RECORD_STATE_STANDBY == RECORD_GetState())
//	//{
//	//	return;
//	//}

//	// �Գ�������������ʱ������
//	paramCB.nvm.param.common.record.total.ridingTime ++;

//	// ��Trip��������������ʱ������
//	paramCB.nvm.param.common.record.trip.ridingTime ++;

//	// ��0������������ʱ������
//	paramCB.nvm.param.common.record.today.ridingTime ++;

//	// NVM���£�������д�룬Ĭ���ö�ʱ����ʱд�룬������ںܿ��Flashд���ķ���
//	NVM_SetDirtyFlag(FALSE);

//	// ���ݵ�ص�ѹ�������ص����ٷֱ�
//	// ����������ADCģ�����
//}

///*********************************************************************
//* �� �� ��: PARAM_Init
//* �������: void
//* ��������: ��
//* �� �� ֵ: uint32 
//* ��������: ��ʼ������
//***********
//* �޸���ʷ:
//*   1.�޸�����: ZJ
//*     �޸�����: 2017��08��06��
//*     �޸�����: �º��� 		   
//**********************************************************************/
//void PARAM_Init(void)
//{
//	// ��ȡNVM
//	NVM_Load();
//	
//	// δ��ʽ�������ʽ��
//	if (!NVM_IsFormatOK())
//	{
//		// ��֤��д��ʱ���е�
//		POWER_ON();
//		
//		NVM_Format();
//	}
//	
//	// ����Ĭ�ϵ���������
//	PARAM_SetDefaultRunningData();
//	
//	// ������Trip����������ƽ���ٶȣ����㹫ʽ:Trip��������� �� Trip��������ʱ��
//	if (0 != paramCB.nvm.param.common.record.trip.ridingTime)
//	{
//		paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.distance * 3600.0f / paramCB.nvm.param.common.record.trip.ridingTime;

//		// ƽ���ٶȲ��ܴ�������ٶ�
//		if (paramCB.runtime.trip.speedAverage >= paramCB.nvm.param.common.record.trip.speedMax)
//		{
//			paramCB.runtime.trip.speedAverage = paramCB.nvm.param.common.record.trip.speedMax * 0.9f;
//		}
//	}

//	// ע���������ʱ�䶨ʱ��
//	TIMER_AddTask(TIMER_ID_PARAM_TIME,
//					1000UL,
//					PARAM_CALLBACK_CaculateRidingTime,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);

//	// ע����д���ݵ�NVM��ʱ��
//	TIMER_AddTask(TIMTR_ID_NVM_WRITE_REQUEST,
//					600000UL,
//					NVM_CALLBACK_EnableWrite,
//					0,
//					TIMER_LOOP_FOREVER,
//					ACTION_MODE_ADD_TO_QUEUE);
//	
//	// ��д��ָ��Ǳ��ϵ�ʱ�����ȹص�״̬
//	POWER_OFF();
//}

///*********************************************************************
//* �� �� ��: PARAM_Process
//* �������: void
//* ��������: ��
//* �� �� ֵ: void 
//* ��������: ģ����̴�����
//***********
//* �޸���ʷ:
//*   1.�޸�����: ZJ
//*     �޸�����: 2017��08��06��
//*     �޸�����: �º��� 		   
//**********************************************************************/
//void PARAM_Process(void)
//{
//	// ������Ҫ���¼���ʱ������
//	if (paramCB.recaculateRequest)
//	{
//		// �����������
//		paramCB.recaculateRequest = FALSE;
//		
//		// �����ٶ�������
//		PARAM_CaculateSpeed();

//		// �������������
//		PARAM_CaculateTripDistance();

//		// ���㿨·��������
//		PARAM_CaculateCalories();
//		
//		// ��������ʱ��������
//		// ʱ���������ɶ�ʱ������
//	}

//	// ���㹦��������
//	paramCB.runtime.power = 1.0f * paramCB.runtime.battery.voltage * paramCB.runtime.battery.current / 1000000.0f;

//	// NVM��д�����󣬲���д��ʹ��ʱ���ſ���д��
//	if ((paramCB.nvmWriteRequest) && (paramCB.nvmWriteEnable))
//	{
//		paramCB.nvmWriteRequest = FALSE;
//		paramCB.nvmWriteEnable = FALSE;

//		// ���¸�λд���ݵ�NVM��ʱ��
//		TIMER_ResetTimer(TIMTR_ID_NVM_WRITE_REQUEST);
//		
//		// ���컯����
//		NVM_Save(FALSE);
//	}
//	// �ж��Ƿ���Ҫ�ָ���������
//	if (!PARAM_GetResFactorySet())
//	{
//		PARAM_SetResFactorySet(TRUE);
//		PARAM_FactoryReset();  
//	}
//}

//// Trip����
//void PARAM_ClearTrip(void)
//{
//	// Trip�������
//	paramCB.nvm.param.common.record.trip.distance = 0;

//	// ƽ���ٶȺ�����ٶ�����
//	paramCB.runtime.trip.speedAverage = 0;
//	paramCB.nvm.param.common.record.trip.speedMax = 0;

//	// Trip����ʱ������
//	paramCB.nvm.param.common.record.trip.ridingTime = 0;

//	// Trip���п�·������
//	paramCB.nvm.param.common.record.trip.calories = 0;

//	// NVM���£�����д��
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

//// ��ȡ��Ӧ�־�ID���ܳ�
//uint16 PARAM_GetPerimeter(uint8 wheelSizeID)
//{
//	CHECK_PARAM_OVER_SAME_RETURN_VALUE(wheelSizeID, PARAM_WHEEL_SIZE_MAX, NULL);
//	
//	return (uint16)paramCB.runtime.perimeter[wheelSizeID];
//}

//// ���ö�Ӧ�־�ID���ܳ�
//void PARAM_SetPerimeter(uint8 wheelSizeID, uint8 perimeterMs)
//{
//	CHECK_PARAM_OVER_SAME_RETURN(wheelSizeID, PARAM_WHEEL_SIZE_MAX);

//	paramCB.runtime.perimeter[wheelSizeID] = perimeterMs;
//}


//// ��ȡ�ٶȽӿ�
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

//// ���س�������
//void PARAM_FactoryReset(void)
//{
//	// ��ʽ������
//	//NVM_Format();

//	//uint8 i;

//	// д��ʽ����ʶ
//	//for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
//	//{
//	//	paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
//	//}

//	// ���� ��Ҫ������NVM�Ĳ��� ����
//	// ����ϵͳ����
//	//paramCB.nvm.param.common.unit = UNIT_METRIC;										// ��λ, ������Ӣ��
//	//paramCB.nvm.param.common.brightness = BACK_LIGHT_LEVEL_ON;							// ��������
//	//paramCB.nvm.param.common.powerOffTime = 10;											// �Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min									// ������������������
//	
//	// �Գ��������Ĳ�����ʼ��
//	//paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// ���˼�¼����λ:0.1Km
//	//paramCB.nvm.param.common.record.total.distance = 0;					// ����̣���λ:0.1Km
//	//paramCB.nvm.param.common.record.total.ridingTime = 0;				// ������ʱ�䣬��λ:s
//	//paramCB.nvm.param.common.record.total.calories = 0;					// �ܿ�·���λ:KCal

//	// ��Trip���������Ĳ�����ʼ��
//	paramCB.nvm.param.common.record.trip.ridingTime = 0;				// ������ʱ����
//	paramCB.nvm.param.common.record.trip.distance = 0;					// Trip����λ:0.1Km
//	paramCB.nvm.param.common.record.trip.calories = 0;					// ��·���λ:KCal
//	paramCB.nvm.param.common.record.trip.speedMax = 0;					// ����ٶ�	
//	paramCB.runtime.trip.speedAverage = 0;								// ƽ���ٶȣ���λ:0.1Km/h	

//	// ��0�������Ĳ�����ʼ��
//	paramCB.nvm.param.common.record.today.trip = 0;						// ������̣���λ:0.1Km		
//	paramCB.nvm.param.common.record.today.calories = 0;					// ���տ�·���λ:KCal
//	paramCB.nvm.param.common.record.today.ridingTime = 0;				// ��������ʱ������
//	//paramCB.nvm.param.common.record.today.month = 1;						// ���ղ�����Ӧ������
//	//paramCB.nvm.param.common.record.today.day = 1;						// ���ղ�����Ӧ������
//	//paramCB.nvm.param.common.record.today.year = 2019;					// ���ղ�����Ӧ������

//	// �������
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

////// ��ȡ��λPWM����ֵ,ע��������Ҫ�ж϶����Ƿ�Ƿ�
////uint8 PARAM_GetPwmTopLimit(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmTop[asist];
////}

////// ��ȡ��λPWM����ֵ,ע��������Ҫ�ж϶����Ƿ�Ƿ�
////uint8 PARAM_GetPwmBottomLimit(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmBottom[asist];
////}

////// ��ȡ��λPWM�̶�ֵ
////uint8 PARAM_GetPwmFixed(uint8 asist)
////{
////	if (asist >= ASSIST_ID_COUNT)
////	{
////		return 0;
////	}

////	return paramCB.nvm.param.protocol.pwmFixed[asist];
////}

////// ���õ�λPWM��ֵ
////void PARAM_SetPwmLimit(uint8 asist, uint8 pwm)
////{
////	CHECK_PARAM_OVER_SAME_RETURN(asist, ASSIST_ID_COUNT);

////	paramCB.nvm.param.protocol.pwmTop[asist] = pwm;
////}


