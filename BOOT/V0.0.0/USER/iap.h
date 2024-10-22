#ifndef __IAP_H__
#define __IAP_H__
 
#include "common.h"

//===========================================================================================================================
// �û���ظ����Լ���ѡоƬ�ͺ�����FLASH��С
#define IAP_N32_FLASH_SIZE 					(58 * 512UL - IAP_PARAM_FLASH_SIZE)			// ��ѡMCU��FLASH������С(��λ:�ֽ�)
#define IAP_N32_FLASH_BASE					0x08000000UL		// MCU��FLASH����ʼ��ַ
#define IAP_FLASH_ADRESS_OFFSET				0x00003400UL		// Ԥ��13KΪBOOT

#define IAP_FLASH_APP_ADDR					(IAP_N32_FLASH_BASE + IAP_FLASH_ADRESS_OFFSET)     // ��һ��Ӧ�ó�����ʼ��ַ(�����FLASH),����IAP_FLASH_ADRESS_OFFSET�Ŀռ�ΪIAPʹ��

// ���º��ɿ�����Ա�޸ģ��û��������
#define IAP_FLASH_PAGE_SIZE					512UL				// 512 bytes

// BOOT��Ϣ��ַ
#define IAP_MCU_BOOT_ADDEESS 				(IAP_N32_FLASH_BASE + (32-1)*IAP_FLASH_PAGE_SIZE + 96)

// APP��Ϣ��ַ
#define IAP_MCU_APP_ADDEESS 				(IAP_N32_FLASH_BASE + (32-1)*IAP_FLASH_PAGE_SIZE + 128)

// ���Ա�ʶ�汾��ַ
#define IAP_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE + (32-1)*IAP_FLASH_PAGE_SIZE + 320)

// ��־����
#define IAP_PARAM_FLASH_SIZE				2 * 512UL

#define IAP_FLASH_APP_READ_UPDATA_SIZE  512

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


//���ⲿ���ú���������
void IAP_Init(void);

// д��bin����
BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appLen);    

// ����APP����ҳ
BOOL IAP_EraseAPPArea(void);

// ��ת��APP
void IAP_JumpToAppFun(uint32 param);

//======================================================================================================
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead);
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite);
uint8 IAP_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);
void IAP_FLASH_WriteByteWithErase(uint32 writeAddr, uint8 *pBuffer, uint16 NumByteToWrite);
void IAP_FlashReadByteArray(uint32 readAddr, uint8 *pBuffer, uint16 numToRead);
int32_t app_flag_write(uint32_t data ,uint32_t start_add);
#endif

#ifndef  __IAP_H_
#define  __IAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "n32g003.h"

typedef  void (*iapfun)(void);										// ����һ���������͵Ĳ���.  

#define FLASH_PAGE_SIZE 				512	
#define FLASH_APP_BASE_ADDR 		0x08004000				//BOOTLOAD Ԥ��16K�ռ䣬APP�����0x08004000��ʼ 
#define FLASH_START_ADDR        0x08004000
#define app_update_flag_addr 		0x08007400	//��ַҪ����APP�����ݷ�Χ��demo��bin������0x080054F8��ֹ

void iap_load_app(u32 appxaddr);								
void IAP_UPDATE_APP(void);
int32_t app_flash_write(uint32_t *data ,uint32_t Flash_address);
uint32_t FLASH_ReadWord(uint32_t address);
int32_t app_flag_write(uint32_t data ,uint32_t start_add);
void send_ack(void);
void Usart_SendArray(UART_Module* pUSARTx, uint8_t* array, uint16_t num);

#ifdef __cplusplus
}
#endif

#endif
