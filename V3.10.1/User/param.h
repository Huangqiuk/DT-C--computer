#ifndef 	__PARAM_H__
#define 	__PARAM_H__

#include "common.h"

#define SN_ADDR1  0x1FFFF7E8
#define SN_ADDR2  0x1FFFF7EC
#define SN_ADDR3  0x1FFFF7F0

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
	}watchUid;

	BOOL qrRegEnable;
	BOOL jumpFlag;
	
	uint8 devVersion[3];				// �Ǳ��ͺ�
	uint16 fwBinNum;					// �̼��汾���
	struct
	{
		uint8 devVersionRemain;				// �豸�����汾��(����汾��)
		uint8 devReleaseVersion;			// ����������ķ����汾��(��Ʒ����汾��)
		uint8 fwUpdateVersion;				// ���Է����汾��(�����汾��)
		uint8 fwDebugReleaseVersion;		// ������ʱ�汾��
	}fwBinVersion;
}RUNTIME_DATA_CB;


// ��ȡ�Ǳ���
#define PARAM_GetDevVersion()				(paramCB.runtime.devVersion)


// ��ȡ�̼��汾���
#define PARAM_GetFwBinNum()					(paramCB.runtime.fwBinNum)


// ��ȡ�̼��汾�ű���λ
#define PARAM_GetDevVersionRemain()			(paramCB.runtime.fwBinVersion.devVersionRemain)


// ��ȡ�̼������汾��
#define PARAM_Get_DevReleaseVersion()		(paramCB.runtime.fwBinVersion.devReleaseVersion)


// ��ȡ�̼����������汾��
#define PARAM_Get_FwUpdateVersion()			(paramCB.runtime.fwBinVersion.fwUpdateVersion)


// ��ȡ�̼��޸�bug�����汾��
#define PARAM_Get_FwDebugReleaseVersion()	(paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)

/*******************************************************************************
 *                                  �ṹ��	                                   *
********************************************************************************/
// ���� ������в��� : ��Ϊ������:����NVM�벻����NVM ����
typedef struct
{
	RUNTIME_DATA_CB runtime;
}PARAM_CB;

extern PARAM_CB paramCB;


typedef enum{
	FLASH_AGING_TEST_NULL,
	FLASH_AGING_TEST_OK,
	FLASH_AGING_TEST_NG,
}FLASH_AGING_TEST_RES_E;


// ��������
void PARAM_Init(void);

uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);

void Param_Process(void);


#endif

