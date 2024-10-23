#ifndef __IAP_H__
#define __IAP_H__
 
#include "common.h"

//===========================================================================================================================
// �û���ظ����Լ���ѡоƬ�ͺ�����FLASH��С
#define IAP_N32_FLASH_SIZE 					(64*2048UL - IAP_PARAM_FLASH_SIZE)			// ��ѡMCU��FLASH������С(��λ:�ֽ�)
#define IAP_N32_FLASH_BASE					0x08000000UL		// MCU��FLASH����ʼ��ַ
#define IAP_FLASH_ADRESS_OFFSET				0x00003000UL		// Ԥ��12KΪBOOT

#define IAP_FLASH_APP_ADDR					(IAP_N32_FLASH_BASE+IAP_FLASH_ADRESS_OFFSET)     // ��һ��Ӧ�ó�����ʼ��ַ(�����FLASH),����IAP_FLASH_ADRESS_OFFSET�Ŀռ�ΪIAPʹ��

// ���º��ɿ�����Ա�޸ģ��û��������
#define IAP_FLASH_PAGE_SIZE					2048UL				// 2K bytes

// BOOT��Ϣ��ַ
#define IAP_MCU_BOOT_ADDEESS 				(IAP_N32_FLASH_BASE + (64-1)*IAP_FLASH_PAGE_SIZE + 96)
// APP��Ϣ��ַ
#define IAP_MCU_APP_ADDEESS 				(IAP_N32_FLASH_BASE + (64-1)*IAP_FLASH_PAGE_SIZE + 128)

// ���Ա�ʶ�汾��ַ
#define IAP_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE + (64-1)*IAP_FLASH_PAGE_SIZE + 320)

// ��־����
#define IAP_PARAM_FLASH_SIZE				1*2048UL

// IAP���ƽṹ�嶨��
typedef struct
{
	BOOL appValid;			// APP�����Ƿ����

	volatile struct{
		uint32 sn0;				// MCUΨһ���к�
		uint32 sn1;
		uint32 sn2;
	}mcuUID;
	
}IAP_CB;

extern IAP_CB iapCB;



// ��������ò���
typedef struct
{
	uint8_t  Reserver[100];
	uint16_t updataState;
	uint32_t BootVersion;
    uint32_t AppVersion;
	uint16_t AppVersionNum;
}FLASH_PARAM_CB;

// ���һ�����ڶ�ȡ���ݵĹ�����
typedef struct
{
	union
	{
		FLASH_PARAM_CB prame;
		uint8_t buff[sizeof(FLASH_PARAM_CB)];
	}data;
}LOCAL_PARAM_CB;










//���ⲿ���ú���������
void IAP_Init(void);

// д��bin����
BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appLen);    

BOOL IAP_WriteAppBin2(uint32 appAddr, uint8 *appBuf, uint32 appLen);   
// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckFlashIsBlank(void);

// ����APP����ҳ
BOOL IAP_EraseAPPArea(void);

// ��ת��APP
void IAP_JumpToAppFun(uint32 param);

uint32 LoadPrameToLocal(void);

//======================================================================================================
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead);
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite);
uint8 IAP_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);
void IAP_FlashReadWordArray2(uint32 readAddr, uint32 *pBuffer, uint16 numToRead); 
#endif

