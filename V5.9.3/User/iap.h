/**********************************************************************************************************************************
	Description:
	1.������Bootloader���������򣩵�options for target�����ú�FLASH����ʼλ��,��0x08000000~0x08010000,size = 10000;
	2.�����������main�������������ж�������ʼ��ַ��NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
	3.�����û�appӦ�ó������ʼ��ַ�ͷ�Χ����0x08010000~0x08090000,size = 80000;
	4.�����û�app������ж�������ʼ��ַ��NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);

***************************************IAP��װ�ӿں���ʹ�ò���****************************************************
	1.���Ȱ������Ϸ���������������û����������Ӧ���ã�
	2.ͨ��USART/I2C/CAN����/SPI��ͨ�ŷ�ʽ��Ӧ�ó������ɵ�bin�ļ����͵�����Bootloader���������Ŀ����ϣ�������ķ�ʽ�Ի��������ݽ��д洢��
	3.���úý��ճɹ���־λflag,���жϽ��ճɹ�֮�󣬼���ʹ�ñ��ӿں����Գ������������
	4.�ӿں���1��BOOL IAP_WriteAppBin(u32 appAddr,u8 *appBuf,u32 appLen)����������Ӧ�ó������ʼ��ַ�������׵�ַ��Ӧ�ó���ĳ�����Ϊ���룬
	  ������д��FLASE����,��д���У��ɹ�����TRUE,���򣬷���FALSE,��ʾд��ʧ��;��appLen��0���ɶԹ̼����в�����
	5.�ӿں���2��void IAP_UpdataReset(void)�����������жϺ���1����ΪTRUE֮�󣬼��ɶԳ�����������λ��
	6.�ӿں���3��void IAP_RunAPP(u32 appAddr)��������������ת��ִ��Ӧ�ó���
	ע��Ϊ�˷�ֹ�����ܷɣ��ڽ��г�����תʱҪ�Գ����жϣ�if(((*(vu32*)(IAP_FLASH_APP1_ADDR+4))&0xFF000000)==IAP_STM32_FLASH_BASE),
	    ���������������������ת��Ӧ�ó���
	Others: 		// 
	Function List:	//
	1. ....
	History:		//
					// 
	1. Date:
	   Author:
	   Modification:
	2. ...  
**************************************************************************************************************************************/
#ifndef __IAP_H__
#define __IAP_H__
 
#include "common.h"

// ��������0��ʼ��UI_SECTOR_NUMBERλUIר��
#define UI_SECTOR_NUMBER								1152				// ����UI����4.5M=4.5*1024*1024/SPI_FLASH_SECTOR_SIZE=1152������

// ��ά���ַ�������ʼ�����ַ
//#define PARAM_NVM_QR_START_ADDR							0x0000
//#define PARAM_NVM_QR_SIZE								32					// û�ж�ά���ַ���������Ϊ0�������ά���ַ����̶�32���ַ�

// ���������E2PROM�еĵ�ַ
#define PARAM_NVM_HMI_TYPE_START_ADDRESS				(PARAM_NVM_QR_START_ADDR+PARAM_NVM_QR_SIZE)
#define PARAM_NVM_HMI_TYPE_SIZE							6	

// GPS������E2PROM�еĵ�ַ
#define PARAM_NVM_GPS_TYPE_START_ADDRESS				(PARAM_NVM_HMI_TYPE_START_ADDRESS+PARAM_NVM_HMI_TYPE_SIZE)
#define PARAM_NVM_GPS_TYPE_SIZE							1

//===========================================================================================================================
// �û���ظ����Լ���ѡоƬ�ͺ�����FLASH��С
#define IAP_GD32_FLASH_SIZE 			256*1024UL			// ��ѡSTM32��FLASH������С(��λ:�ֽ�)



// FLASH��ַ
#define IAP_GD32_FLASH_BASE  			0x08000000UL 	    	// STM32 FLASH����ʼ��ַ
#define IAP_FLASH_ADRESS_APP1_OFFSET	0x8000 					// ��ַƫ��,ΪIAP����32K�ռ䣬�˵�ַƫ�Ʊ���ΪҳIAP_STM32_PAGE_SIZE��������
#define IAP_GD32_APP1_SIZE				(IAP_GD32_FLASH_SIZE-IAP_FLASH_ADRESS_APP1_OFFSET)				// APP1��С
#define IAP_FLASH_APP1_ADDR				(IAP_GD32_FLASH_BASE+IAP_FLASH_ADRESS_APP1_OFFSET)     // APP1��ʼ��ַ

#define IAP_FLASH_ADRESS_APP2_OFFSET	0x10000 				// ��ַƫ��,ΪIAP+APP1����64K�ռ䣬�˵�ַƫ�Ʊ���ΪҳIAP_STM32_PAGE_SIZE��������
#define IAP_GD32_APP2_SIZE				(IAP_GD32_FLASH_SIZE-IAP_FLASH_ADRESS_APP2_OFFSET)		// APP2��С
#define IAP_FLASH_APP2_ADDR				(IAP_GD32_FLASH_BASE+IAP_FLASH_ADRESS_APP2_OFFSET)     // APP2��ʼ��ַ


// ���º��ɿ�����Ա�޸ģ��û��������
// �ж���ʹ�õ�FLASH��С����ͬ�ͺ�оƬ���⺯��ÿҳ��ռ�Ŀռ��СҲ����ͬ
#if IAP_GD32_FLASH_SIZE <= 256*1024UL
	#define IAP_STM32_PAGE_SIZE 		2048UL // �ֽ�
#else 
	#define IAP_STM32_PAGE_SIZE			4096UL
#endif	

// IAP���ƽṹ�嶨��
typedef struct
{
	BOOL appValid;			// APP�����Ƿ����
}IAP_CB;

extern IAP_CB iapCB;


//���ⲿ���ú���������
void IAP_Init(void);

BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize);    

BOOL IAP_BleWriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize);

// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckFlashIsBlank(void);

// ����APP����ҳ
BOOL IAP_EraseAPPArea(void);

void IAP_JumpToAppFun(uint32 param);

// �ж�ָ����ַAPP�Ƿ�Ϸ�
uint8 IAP_CheckAppRightful(uint32 addr);

// ����APP1����ҳ
BOOL IAP_EraseAPP1Area(void);

// ����APP2����ҳ
BOOL IAP_EraseAPP2Area(void);

// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckApp1FlashIsBlank(void);

// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckApp2FlashIsBlank(void);

// ���������ܵ�д����
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite)	;

// ����flash����У��
uint8 IAP_FlashReadForCrc8(uint32 datalen);

#endif

