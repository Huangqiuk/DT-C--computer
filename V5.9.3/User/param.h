#ifndef __PARAM_H__
#define __PARAM_H__

#include "common.h"

#define SN_ADDR1 0x1FFFF7E8
#define SN_ADDR2 0x1FFFF7EC
#define SN_ADDR3 0x1FFFF7F0

// �������й��������ݣ����粻�������
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
	// �Ǳ�Ψһ���к�
	struct
	{
		uint32 sn0;
		uint32 sn1;
		uint32 sn2;
		uint8 snStr[25];
	} watchUid;

	BOOL qrRegEnable;
	BOOL jumpFlag;

	uint8 devVersion[3]; // �Ǳ��ͺ�
	uint16 fwBinNum;	 // �̼��汾���
	struct
	{
		uint8 devVersionRemain;		 // �豸�����汾��(����汾��)
		uint8 devReleaseVersion;	 // ����������ķ����汾��(��Ʒ����汾��)
		uint8 fwUpdateVersion;		 // ���Է����汾��(�����汾��)
		uint8 fwDebugReleaseVersion; // ������ʱ�汾��
	} fwBinVersion;

} RUNTIME_DATA_CB;

typedef struct
{
	// ϵͳ��������
	struct
	{
		uint16 newWheelSize;
		uint16 newPerimeter;
		uint8 unit;
		uint8 speedLimitVal;		 // ��λ:1KM/H
		uint8 steelNumOfSpeedSensor; // ���Դ�����һȦ�Ÿ���(�� ���ٴŸ�)
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

// ����NVM���ߴ�
#define PARAM_FACTORY_NVM_DATA_SIZE sizeof(FACTORY_NVM_CB)

// ��ȡ�Ǳ���
#define PARAM_GetDevVersion() (paramCB.runtime.devVersion)

// ��ȡ�̼��汾���
#define PARAM_GetFwBinNum() (paramCB.runtime.fwBinNum)

// ��ȡ�̼��汾�ű���λ
#define PARAM_GetDevVersionRemain() (paramCB.runtime.fwBinVersion.devVersionRemain)

// ��ȡ�̼������汾��
#define PARAM_Get_DevReleaseVersion() (paramCB.runtime.fwBinVersion.devReleaseVersion)

// ��ȡ�̼����������汾��
#define PARAM_Get_FwUpdateVersion() (paramCB.runtime.fwBinVersion.fwUpdateVersion)

// ��ȡ�̼��޸�bug�����汾��
#define PARAM_Get_FwDebugReleaseVersion() (paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)

/*******************************************************************************
 *                                  �ṹ��	                                   *
 ********************************************************************************/
// ���� ������в��� : ��Ϊ������:����NVM�벻����NVM ����
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

// ��������
void PARAM_Init(void);

uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);

void PARAM_Process(void);

void FACTORY_NVM_SetDirtyFlag(BOOL req);

#endif
