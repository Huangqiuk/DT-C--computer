/********************************************************************************
  * �� �� ��: Param.c
  * �� �� ��: ����
  * �޸�����: Fly & ZJ & Workman
  * �޸�����: 2017��08��06��
  * ���ܽ���: ���ļ��������в����Ļ�ȡ����㣬�ṩ�ɿ������ݸ��ϲ���ʾ����            
  ******************************************************************************
  * ע������:
  *
  * 							��Ȩ���̫�Ƽ�����.
  *s
*********************************************************************************/

/*******************************************************************************
 *                                  ͷ�ļ�	                                   *
********************************************************************************/
#include "Param.h"
#include "iap.h"
#include "timer.h"
#include "Uartprotocol.h"
#include "powerCtl.h"

// ��������
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
 *                                  ��������                                   *
********************************************************************************/
PARAM_CB paramCB;

const uint32 PARAM_MCU_VERSION_ADDRESS[] = {PARAM_MCU_DT_SN_ADDEESS, PARAM_MCU_CUSTOMER_SN_ADDEESS, PARAM_MCU_HW_ADDEESS, \
											PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS, \
											PARAM_MCU_BLE_UPDATA_CRC_ADDEESS, PARAM_MCU_QR_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS	};
// NVM��ʽ����ʶ
const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

uint8 flashBuf[PARAM_MCU_PAGE_SIZE];
uint8 userSetFlashBuf[PARAM_MCU_PAGE_SIZE];
uint32 flashBackupFlag;

// ����ϵͳ����
void NVM_Load(void)
{
//==========================================================================================
	// ��ȡ�������ı�ʶ�����Ƿ���Ч��־
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS, 
							(uint32*)&flashBackupFlag, 
							1);

	// ����������Ч
	if (0x12345678 == flashBackupFlag)
	{
		// �������������ݵ�������
		IAP_FlashReadWordArray(PARAM_NVM_BACKUP_START_ADDRESS, 
								(uint32*)flashBuf, 
								PARAM_MCU_PAGE_SIZE / 4);
		
		// ������������д������������
		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
								(uint32*)flashBuf, 
								PARAM_MCU_PAGE_SIZE / 4);

		// �屸�����ݱ�־
		flashBackupFlag = 0x00000000;
		IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + 1020, 
								(uint32*)&flashBackupFlag, 
								1);
	}

	// ��ȡNVM�е�����
	IAP_FlashReadWordArray(PARAM_NVM_START_ADDRESS, 
							(uint32 *)paramCB.nvm.array, 
							(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// ����ϵͳ����
void NVM_Save(BOOL saveAll)
{
//==============================================================================	
	// �ȴ����һ��������ȡ���ݿ����������ڶ����������ݣ�Ȼ��д��һ����־�����������Ÿ������һ�������������������ı�־
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
						
	// ֻ����仯������
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
									(uint32*)paramCB.nvm.array, 
									(PARAM_NVM_DATA_SIZE + 3) / 4);

	// ���־
	flashBackupFlag = 0x00000000;
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_BACKUP_START_ADDRESS + 1020, 
							(uint32*)&flashBackupFlag, 
							1);
}

// ��ʼ�����ð����
void NVM_Load_FactoryReset(void)
{
	// �ӱ�����ַ����ȡ�������ݿ�����ϵͳ��������
	IAP_FlashReadWordArray(PARAM_NVM_FACTORY_RESET_ADDRESS, 
							(uint32*)paramCB.nvm.array, 
							(PARAM_MCU_PAGE_SIZE + 3) / 4);
							
	// ֻ����仯������
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_START_ADDRESS, 
									(uint32*)paramCB.nvm.array, 
									(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// �����ʼ�����ò���
void NVM_Save_FactoryReset(BOOL saveAll)
{
	// ֻ����仯������
	IAP_FlashWriteWordArrayWithErase(PARAM_NVM_FACTORY_RESET_ADDRESS, 
						(uint32*)paramCB.nvm.array, 
						(PARAM_NVM_DATA_SIZE + 3) / 4);
}

// nvmдģʽ
void NVM_CALLBACK_EnableWrite(uint32 param)
{
	paramCB.nvmWriteEnable = TRUE;
}

// Ĭ�����в���
void PARAM_SetDefaultRunningData(void)
{
	
	uint8 i;
	uint8 * pBuff = NULL;
	// ���� ����Ҫ������NVM�Ĳ��� ����

	paramCB.runtime.jumpFlag = (BOOL)0;
	
	// �̼��汾��
	for (i = 0;i < 3;i++)
	{
		paramCB.runtime.devVersion[i] = DEV_VERSION[i];
	}
	
	paramCB.runtime.fwBinNum = FW_BINNUM;
	paramCB.runtime.fwBinVersion.devVersionRemain = DEV_VERSION_REMAIN;
	paramCB.runtime.fwBinVersion.devReleaseVersion = DEV_RELEASE_VERSION;
	paramCB.runtime.fwBinVersion.fwUpdateVersion = FW_UPDATE_VERSION;
	paramCB.runtime.fwBinVersion.fwDebugReleaseVersion = FW_DEBUG_RELEASE_VERSION;
	
	// �Ǳ�Ψһ���к�
	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0);
	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F4); 
	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F8);

	pBuff = (uint8 *)&(paramCB.runtime.appVersion);

	paramCB.runtime.bleRadio = 0;	
	
	/*
	IAP_FlashReadWordArray(PARAM_MCU_BOOT_ADDEESS,(uint32 *)pBuff, 8);
	
	// �����ȡ�İ汾�Ų�������ǿ�Ƹ���Ϊ��ǰAPP�汾
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

// ��ʽ������
void NVM_Format(void)
{
	uint8 i;
	
	// д��ʽ����ʶ
	for (i = 0; i < NVM_FORMAT_FLAG_SIZE; i++)
	{
		paramCB.nvm.param.common.nvmFormat[i] = formatFlag[i];
	}

	// ���� ��Ҫ������NVM�Ĳ��� ����
	// ����ϵͳ����
	paramCB.nvm.param.common.battery.voltage = 36;
	paramCB.nvm.param.common.battery.filterLevel = 1;								
	paramCB.nvm.param.common.speed.limitVal = 250; 										// ����ֵ����λ:0.1Km/h
	paramCB.nvm.param.common.unit = UNIT_METRIC;										// ��λ, ������Ӣ��
	paramCB.nvm.param.common.powerOffTime = 10;											// �Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min
	paramCB.nvm.param.common.assistMax = ASSIST_3;										// ֧�ֵ�λ
	paramCB.nvm.param.common.assist = ASSIST_1;											// ��λ
	paramCB.nvm.param.common.busAliveTime = 10000;										// ���߹��ϳ�ʱʱ��
	paramCB.nvm.param.common.existBle = FALSE;											// �Ƿ�������
	paramCB.nvm.param.common.workAssistRatio = 2;										// ������������������
	
	// �Գ��������Ĳ�����ʼ��
	paramCB.nvm.param.common.record.total.maxTripOf24hrs = 0;			// ���˼�¼����λ:0.1Km
	paramCB.nvm.param.common.record.total.distance = 0; 				// ����̣���λ:0.1Km
	paramCB.nvm.param.common.record.total.ridingTime = 0;				// ������ʱ�䣬��λ:s
	paramCB.nvm.param.common.record.total.calories = 0; 				// �ܿ�·���λ:KCal


	// ��0�������Ĳ�����ʼ��
	paramCB.nvm.param.common.record.today.trip = 0; 					// ������̣���λ:0.1Km 	
	paramCB.nvm.param.common.record.today.calories = 0; 				// ���տ�·���λ:KCal
	paramCB.nvm.param.common.record.today.ridingTime = 0;				// ��������ʱ������
	paramCB.nvm.param.common.record.today.month = 1;					// ���ղ�����Ӧ������
	paramCB.nvm.param.common.record.today.day = 1;						// ���ղ�����Ӧ������
	paramCB.nvm.param.common.record.today.year = 2019;					// ���ղ�����Ӧ������

	paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor = 110; 						// ���ٴ�����һȦ�Ÿ���

	paramCB.nvm.param.protocol.driver.currentLimit = 12000;								// �������ޣ���λ:mA
	paramCB.nvm.param.protocol.driver.lowVoltageThreshold = 30000;						// Ƿѹ���ޣ���λ:mV

	paramCB.nvm.param.common.percentageMethod = BATTERY_DATA_SRC_ADC;					// ������ȡ��ʽ(0:�������ϱ���ѹ) (1:�������ϱ�����) (2:�Ǳ��Լ�������ѹ����)

	paramCB.nvm.param.protocol.driver.controlMode = 2;									// ����������ģʽ
	paramCB.nvm.param.protocol.driver.zeroStartOrNot = TRUE;							// ���������������
	paramCB.nvm.param.protocol.driver.switchCruiseMode = FALSE;							// Ѳ���л�ģʽ
	paramCB.nvm.param.protocol.driver.switchCruiseWay = FALSE;							// �л�Ѳ���ķ�ʽ
	paramCB.nvm.param.protocol.driver.assistSensitivity = 1;							// ����������
	paramCB.nvm.param.protocol.driver.assistStartIntensity = 3;							// ��������ǿ��
	paramCB.nvm.param.protocol.driver.assistSteelType = 5;								// �����Ÿ�����
	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum = 0;						// ���ٵ����������Ÿ���(δʹ��)
	paramCB.nvm.param.protocol.driver.speedLimitSwitch = FALSE;							// ���ٿ���
	paramCB.nvm.param.protocol.driver.CruiseEnabled = TRUE;							// Ѳ��ʹ�ܱ�־(Ĭ��ʹ��)
	
	paramCB.nvm.param.protocol.wheelSizeID = PARAM_WHEEL_SIZE_16_INCH;					// �־�

	paramCB.nvm.param.protocol.driver.breakType = FALSE;								// ɲ������0������ɲ��(AD)��1���ϵ�ɲ��(��ƽ)
	paramCB.nvm.param.common.lockFlag = FALSE;											// 0-����״̬��1-����״̬��
	paramCB.nvm.param.protocol.rgbSwitch = 1;
	paramCB.nvm.param.protocol.ledRgb = 4;
	paramCB.nvm.param.protocol.rgbMode = 4;
	
	// �������
	NVM_Save(TRUE);
}

// �ж�NVM�Ƿ��Ѹ�ʽ��
BOOL NVM_IsFormatOK(void)
{
	uint16 i;

	// ����Ƚϣ�ֻҪ����һ����ͬ������Ϊδ��ʽ��
	for (i=0; i<NVM_FORMAT_FLAG_SIZE; i++)
	{
		if(formatFlag[i] != paramCB.nvm.param.common.nvmFormat[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}

// �������ʶ
void NVM_SetDirtyFlag(BOOL writeAtOnce)
{
	paramCB.nvmWriteRequest = TRUE;

	if (writeAtOnce)
	{
		paramCB.nvmWriteEnable = TRUE;
	}
}

// ��������ʱ��
void PARAM_CALLBACK_CaculateRidingTime(uint32 param)
{

}

/*********************************************************************
* �� �� ��: PARAM_Init
* �������: void
* ��������: ��
* �� �� ֵ: uint32 
* ��������: ��ʼ������
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��08��06��
*     �޸�����: �º��� 		   
**********************************************************************/
void PARAM_Init(void)
{
	// ����Ĭ�ϵ���������
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


