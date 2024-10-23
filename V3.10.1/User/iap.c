/**********************************************************************************************************************************
	Description:
	1.������Bootloader���������򣩵�options for target�����ú�FLASH����ʼλ��,��0x08000000~0x08010000,size = 10000;
	2.�����������main�������������ж�������ʼ��ַ��NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
	3.�����û�appӦ�ó������ʼ��ַ�ͷ�Χ����0x08010000~0x08090000,size = 80000;
	4.�����û�app������ж�������ʼ��ַ���NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);

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

// IAPͨ��ͷ�ļ�
#include "common.h"
#include "iap.h"
#include "iapctrluart.h"
#include "timer.h"
#include "e2prom.h"
#include "Spiflash.h"
#include "lcd.h"
#include "BleProtocol.h"
#include "Watchdog.h" 

/*********************************************************
* ����������
*********************************************************/
IAP_CB iapCB;

// FLASH��д����
// ���ֲ���
BOOL IAP_FlashWriteHalfWordArray(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void IAP_FlashReadHalfWordArray(uint32 readAddr, uint16 *pBuffer, uint16 numToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
BOOL IAP_FlashWriteHalfWordArrayAndCheck(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite);
uint16 IAP_FlashReadHalfWord(uint32 flashAddr);
// �ֲ���
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite);
uint32 IAP_FlashReadWord(uint32 flashAddr);

// APP������
void IAP_RunAPP(uint32 appAddr);

/*******************************************************************************
  * @��������	IAP_RunAPP
  * @����˵��   ��ת��Ӧ�ó����
  * @�������   appAddr:Ӧ�ó������ʼ��ַ
  * @���ز���   ��
*******************************************************************************/
void IAP_RunAPP(uint32 appAddr)
{
    // appAddr��ŵ����û�����Flash���׵�ַ��(*(vu32*)appAddr����˼��ȡ�û������׵�ַ��������ݣ����������û�����Ķ�ջ��ַ
    // ��ջ��ַָ��RAM,��RAM����ʼ��ַ��0x20000000,���������ж����ִ��:�ж��û�����Ķ�ջ��ַ�Ƿ�����:0x20000000~0x2001ffff�����У�
    // �������Ĵ�С�϶�����<128K RAM�����䣬�����Ҫ����������ж�

	//����һ���������͵Ĳ���	
	typedef void (*IAP_Func)(void);
	IAP_Func jumpToApp;

	if(((*(uint32*)appAddr) & 0x2FFE0000) == 0x20000000)	// ���ջ����ַ�Ƿ�Ϸ�.
	{ 
		if (((*(uint32*)(appAddr+4)) & 0xFFF80000) != 0x08000000)   // APP��Ч
		{
			return;
		}

		// ��ת֮ǰ�����жϣ���λ����ΪĬ��ֵ
		DI();
		RCU_APB2RST |= 0xFFFFFFFF;
		RCU_APB2RST &= ~0xFFFFFFFF;
		RCU_APB1RST |= 0xFFFFFFFF;
		RCU_APB1RST &= ~0xFFFFFFFF;
		
		jumpToApp = (IAP_Func)*(uint32*)(appAddr+4);		// �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
	  	__set_MSP(*(uint32*)appAddr);						// ��ʼ���û�����Ķ�ջָ��
		jumpToApp();										// ��ת���û�����APP
	}
}

// ����ʱ�䵽��ת��APP����������
void IAP_JumpToAppFun(uint32 param)
{
	// ���ջ����ַ�Ƿ�Ϸ�.
	if(((*(uint32*)param) & 0x2FFE0000) != 0x20000000)	
	{
		return;
	}

	// APP��Ч
	if (((*(uint32*)(param+4)) & 0xFFF80000) != 0x08000000)
	{
		return;
	}
	
	// ִ��APP����
	IAP_RunAPP(param);
}

// �ж�ָ����ַAPP�Ƿ�Ϸ�
uint8 IAP_CheckAppRightful(uint32 addr)
{
	// ���ջ����ַ�Ƿ�Ϸ�.
	if(((*(uint32*)addr) & 0x2FFE0000) != 0x20000000)	
	{
		return 0;
	}

	// APP��Ч
	if (((*(uint32*)(addr+4)) & 0xFFF80000) != 0x08000000)
	{
		return 0;
	}

	return 1;
}


/*******************************************************************************
  * @��������	IAP_Init
  * @����˵��   
  * @�������   
  * @���ز���   ��
*******************************************************************************/
void IAP_Init(void)
{		
	iapCB.mcuUID.sn0 = *(uint32*)(0x1FFFF7E8); 
	iapCB.mcuUID.sn1 = *(uint32*)(0x1FFFF7EC); 
	iapCB.mcuUID.sn2 = *(uint32*)(0x1FFFF7F0);
	
	// �����ж�������ʼλ��
	nvic_vector_table_set(NVIC_VECTTAB_FLASH,0x0000);
	
	TIMER_AddTask(TIMER_ID_IAP_ECO_REQUEST,
					50,
					IAP_CTRL_UART_SendCmdProjectApply,
					IAP_CTRL_UART_CMD_UP_PROJECT_APPLY,
					3,
					ACTION_MODE_ADD_TO_QUEUE);
		
}

/*******************************************************************************
  * @��������	IAP_WriteAppBin
  * @����˵��   �����յ���bin�ļ�����д��ָ����ַ��FLASH����
  * @�������   appAddr:Ӧ�ó������ʼ��ַ appBuf��Ӧ�ó��򻺳����� appSize:Ӧ�ó������ݴ�С����λ���ֽڣ�
  * @���ز���   ��
*******************************************************************************/
BOOL IAP_WriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize)
{
	uint16 i = 0;
	uint16 j = 0;
	uint32 curAppAddr = appAddr; 	// ��ǰд��ĵ�ַ
	uint16 iapBuf[SPI_FLASH_APP_READ_UPDATA_SIZE];			// ��16λ������Ҫ�ʹ���ͨѶЭ��һ֡��ֽ��й�126*2=252���ֽ�
	BOOL writeStatus;
	
	// �����ж�
	if (NULL == appBuf)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�����ַ������IAP��������ַ�����Բ����Լ����Լ�����
	if ((appAddr < IAP_FLASH_APP1_ADDR) || (appAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	// ѭ��д��
	for (j = 0; j < appSize; j += 2)
	{
		// �ֽ�����
		iapBuf[i++] = (uint16)(*appBuf) + ((uint16)(*(appBuf+1)) << 8);
		// ƫ��2���ֽ�
		appBuf += 2;
	}

	// ֱ��д��FLASH
	writeStatus = IAP_FlashWriteHalfWordArray(curAppAddr, iapBuf, appSize/2);	

	return writeStatus;
}


/*******************************************************************************
  * @��������	IAP_BleWriteAppBin
  * @����˵��   �����յ���bin�ļ�����д��ָ����ַ��FLASH����
  * @�������   appAddr:Ӧ�ó������ʼ��ַ appBuf��Ӧ�ó��򻺳����� appSize:Ӧ�ó������ݴ�С����λ���ֽڣ�
  * @���ز���   ��
*******************************************************************************/
BOOL IAP_BleWriteAppBin(uint32 appAddr, uint8 *appBuf, uint32 appSize)
{
	uint16 i = 0;
	uint16 j = 0;
	uint32 curAppAddr = appAddr; 	// ��ǰд��ĵ�ַ
	uint32 *iapBuf = NULL;
	BOOL writeStatus;
	
	// �����ж�
	if (NULL == appBuf)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�����ַ������IAP��������ַ�����Բ����Լ����Լ�����
	if ((appAddr < IAP_FLASH_APP1_ADDR) || ((appAddr + appSize) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;		// �Ƿ���ַ
	}

	// ������ʱ�ڴ�
	iapBuf = (uint32 *)malloc(1024);
	if (NULL == iapBuf)
	{
		return FALSE;
	}
	
	// ѭ��д��
	for (i = 0, j = 0; j < appSize; j += 4)
	{
		// �ֽ�����
		iapBuf[i++] = (uint32)(*appBuf) + ((uint32)(*(appBuf+1)) << 8) + ((uint32)(*(appBuf+2)) << 16) + ((uint32)(*(appBuf+3)) << 24);
		// ƫ��4���ֽ�
		appBuf += 4;
	}      
	
	// ֱ��д��FLASH
	writeStatus = IAP_FlashWriteWordArray(curAppAddr, iapBuf, appSize/4);	
	
	// �ͷ��ڴ�
	free(iapBuf);
	
	return writeStatus;
}


// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckFlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < (IAP_GD32_FLASH_SIZE/IAP_STM32_PAGE_SIZE); i++)
	{
		// ҳ��ַ
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// ��
		  	readAddr += 2;

			// ȫΪ0xFFFF˵��Ϊ��
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}  
		
	}

	return TRUE;
}

// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckApp1FlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP1_OFFSET + IAP_GD32_APP1_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		// ҳ��ַ
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// ��
		  	readAddr += 2;

			// ȫΪ0xFFFF˵��Ϊ��
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}  
		
	}

	return TRUE;
}

// ��ѯFlash�Ƿ�Ϊ��
BOOL IAP_CheckApp2FlashIsBlank(void)
{
	uint16 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_ADRESS_APP2_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP2_OFFSET + IAP_GD32_APP2_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		// ҳ��ַ
		readAddr = i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE;

		for (j = 0; j < IAP_STM32_PAGE_SIZE; j += 2)
		{
			readHalfWord = IAP_FlashReadHalfWord(readAddr);	// ��
		  	readAddr += 2;

			// ȫΪ0xFFFF˵��Ϊ��
			if (readHalfWord == 0xFFFF)
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}
		
	}

	return TRUE;
}


// ����APP����ҳ
BOOL IAP_EraseAPPArea(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// ����APP����ҳ
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < (IAP_GD32_FLASH_SIZE/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}

		// �忴�Ź�
		WDT_Clear();
	}

	fmc_lock();		// ����
	EI(); 				// �������ж�

	return eraseStatus;
}

// ����APP1����ҳ
BOOL IAP_EraseAPP1Area(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// ����APP����ҳ
	for (i = (IAP_FLASH_ADRESS_APP1_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP1_OFFSET + IAP_GD32_APP1_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}

		// �忴�Ź�
		WDT_Clear();
	}

	fmc_lock();			// ����
	EI(); 				// �������ж�

	return eraseStatus;
}

// ����APP2����ҳ
BOOL IAP_EraseAPP2Area(void)
{
	uint16 i;
	fmc_state_enum status = FMC_READY;
	BOOL eraseStatus = TRUE;

	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// ����APP����ҳ
	for (i = (IAP_FLASH_ADRESS_APP2_OFFSET/IAP_STM32_PAGE_SIZE); i < ((IAP_FLASH_ADRESS_APP2_OFFSET + IAP_GD32_APP2_SIZE)/IAP_STM32_PAGE_SIZE); i++)
	{
		status = fmc_page_erase(i*IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

		if (FMC_READY != status)
		{
			eraseStatus = FALSE;

			break;
		}
		
		// �忴�Ź�
		WDT_Clear();
	}

	fmc_lock();			// ����
	EI(); 				// �������ж�

	return eraseStatus;
}

/*******************************************************************************
  * @��������	IAP_FlashReadHalfWord
  * @����˵��   ��ȡָ����ַ�İ���(16λ����)
  * @�������   faddr:FLASH��ǰ��ַ
  * @���ز���   �ӵ�ǰFLASH��ַ��ʼ������ȡ16���ֽڣ����֣������ݣ�*(vu16*)faddr��ע���ַ����Ϊ2�ı���
*******************************************************************************/
uint16 IAP_FlashReadHalfWord(uint32 flashAddr)
{
	return *(volatile uint16*)flashAddr; 
}

/*******************************************************************************
  * @��������	IAP_FlashWriteHalfWordArrayAndCheck
  * @����˵��   ��FLASH��д��ָ�����ȵ����ݣ�����ʹ�ÿ⺯��FLASH_ProgramHalfWord��ÿ��д����֣����Ҽ�����д������У��
  * @�������   writeAddr:Ӧ�ó������ʼ��ַ pBuffer���������������� numToWrite:д����ֵ���Ŀ
  * @���ز���   BOOL,���ΪTRUE,��ò���д���У��ɹ�������ʧ��
----ע��:�˺������ܵ�������ʹ�ã���Ϊû�н�������-------------------------------
*******************************************************************************/
BOOL IAP_FlashWriteHalfWordArrayAndCheck(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite)   
{ 			 		 
	uint16 i;
	uint16 readHalfWord;

	// �����ж�
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	for (i = 0; i < numToWrite; i++)
	{
		fmc_halfword_program(writeAddr, pBuffer[i]);		// д
		readHalfWord = IAP_FlashReadHalfWord(writeAddr);	// ��
	  	writeAddr += 2;

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
		
	  	// д��FLASH�еĻ������������ȡ�������Ƿ�ƥ��
		if (readHalfWord == pBuffer[i])
		{
			continue;
		}
		else
		{
			return FALSE;
		}
	}  
	return TRUE;
} 

/*******************************************************************************
  * @��������	IAP_FlashWriteHalfWordArray
  * @����˵��   ��ָ����С�Ļ��������ݰ�ҳд��FLASH����
  * @�������   writeAddr:д���ַ pBuffer:�����������׵�ַ numToWrite:��Ҫд��ĳ��ȣ���λ�����֣�
  * @���ز���   BOOL�͵ı���writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteHalfWordArray(uint32 writeAddr, uint16 *pBuffer, uint16 numToWrite)	
{
#if 0

	uint32 secpos;	    // ������ַ
	uint16 secoff;	    // ������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16 secremain;  	// ������ʣ���ַ(16λ�ּ���)	   
 	uint16 i;    
	uint32 offaddr;    	// ȥ��0X08000000��ĵ�ַ
	BOOL writeStatus = FALSE;
	FLASH_Status status = FLASH_COMPLETE;
	static uint16 iapFlashBuf[IAP_STM32_PAGE_SIZE/2];	// �����2K�ֽ�

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	DI();			// �ر����ж�
	FLASH_Unlock();	// ����

	// ������й����־λ
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	offaddr = writeAddr-IAP_GD32_FLASH_BASE;			// ʵ��ƫ�Ƶ�ַ.
	secpos = offaddr/IAP_STM32_PAGE_SIZE;				// ������ַ  0~127 for STM32F103RBT6
	secoff = (offaddr%IAP_STM32_PAGE_SIZE)/2;			// �������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain = IAP_STM32_PAGE_SIZE/2 - secoff;			// ����ʣ��ռ��С 

	// �����ڸ�������Χ
	if (numToWrite <= secremain) 
	{
		secremain = numToWrite;
	}
	
	while (1)
	{
		// ������������������
		IAP_FlashReadHalfWordArray((secpos*IAP_STM32_PAGE_SIZE+IAP_GD32_FLASH_BASE),
									iapFlashBuf,
									IAP_STM32_PAGE_SIZE/2);

		// У������
		for (i = 0; i < secremain; i++)
		{
			if (iapFlashBuf[secoff + i] != 0xFFFF)
			{
				// ��������ݲ�ȫ��0xFFFF��˵����Ҫ����
				break;
			}
		}

		// ��Ҫ����
		if (i < secremain)
		{
			// �����������
			status = FLASH_ErasePage(secpos*IAP_STM32_PAGE_SIZE+IAP_STM32_FLASH_BASE);

			if (FLASH_COMPLETE != status)
			{
				return FALSE;
			}
			
			for (i = 0; i < secremain; i++) // ����
			{
				iapFlashBuf[i + secoff] = pBuffer[i];	  
			}

			// д����������
			writeStatus = IAP_FlashWriteHalfWordArrayAndCheck((secpos*IAP_STM32_PAGE_SIZE+IAP_GD32_FLASH_BASE), 
																iapFlashBuf, 
																IAP_STM32_PAGE_SIZE/2);
		}
		// ����Ҫ����
		else
		{
			writeStatus = IAP_FlashWriteHalfWordArrayAndCheck(writeAddr,pBuffer,secremain); //д�Ѿ������˵�,ֱ��д������ʣ������. 
		}
		
		if (!writeStatus)
		{
			break;
		}
		
		// д�������
		if (numToWrite == secremain)
		{
			break;
		}
		// д��δ����
		else 
		{
			secpos++;							// ������ַ��1
			secoff = 0;							// ƫ��λ��Ϊ0 	 
		 	pBuffer += secremain;  	     		// ָ��ƫ��
			writeAddr += secremain;				// д��ַƫ��	   
		  	numToWrite -= secremain;			// �ֽ�(16λ)���ݼ�

		  	// ��һ����������д����
			if (numToWrite > (IAP_STM32_PAGE_SIZE/2))
			{
				secremain = IAP_STM32_PAGE_SIZE/2;	
			}
			// ��һ����������д����
			else 
			{
				secremain = numToWrite;
			}
		}	 
	}
	
	FLASH_Lock();		// ����
	EI(); 				// �������ж�
	
	return writeStatus;

#else
	BOOL writeStatus = TRUE;
	
	// �����ж�
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || (writeAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	// д��ҳ
	if (!IAP_FlashWriteHalfWordArrayAndCheck(writeAddr, pBuffer, numToWrite))	
	{
		writeStatus = FALSE;
	}

	fmc_lock();		// ����
	EI(); 			// �������ж�
	
	return writeStatus;

#endif
}

/*******************************************************************************
  * @��������	IAP_FlashReadHalfWordArray
  * @����˵��   ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @�������   readAddr:��ȡ��ַ pBuffer:�����������׵�ַ numToWrite:��Ҫ����ĳ��ȣ���λ�����֣�
  * @���ز���   ��
*******************************************************************************/
void IAP_FlashReadHalfWordArray(uint32 readAddr, uint16 *pBuffer, uint16 numToRead)   	
{
	uint16 i;

	// �����ж�
	if (NULL == pBuffer)
	{
		return;
	}

	// ���������Ƿ�Ƿ�
	if ((readAddr < IAP_GD32_FLASH_BASE) || (readAddr >= (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return;	// �Ƿ���ַ
	}
	
	for (i = 0; i < numToRead; i++)
	{
		pBuffer[i] = IAP_FlashReadHalfWord(readAddr);	// ��ȡ2���ֽ�.
		readAddr += 2; 									// ƫ��2���ֽ�.	
	}
}


/*******************************************************************************
  * @��������	IAP_FlashWriteWordArray
  * @����˵��   ��ָ����С�Ļ��������ݰ�ҳд��FLASH����
  * @�������   writeAddr:д���ַ pBuffer:�����������׵�ַ numToWrite:��Ҫд��ĳ��ȣ���λ���֣�
  * @���ز���   BOOL�͵ı���writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArray(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)	
{
	BOOL writeStatus = TRUE;
	
	// �����ж�
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);


	// д��ҳ
	if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, numToWrite))	
	{
		writeStatus = FALSE;
	}

	fmc_lock();			// ����
	EI(); 				// �������ж�
	
	return writeStatus;
}

/*******************************************************************************
  * @��������	IAP_FlashWriteHalfWordArrayAndCheck
  * @����˵��   ��FLASH��д��ָ�����ȵ����ݣ�����ʹ�ÿ⺯��IAP_WordProgram��ÿ��д���֣����Ҽ�����д������У��
  * @�������   writeAddr:Ӧ�ó������ʼ��ַ pBuffer���������������� numToWrite:����λ���֣�
  * @���ز���   BOOL,���ΪTRUE,��ò���д���У��ɹ�������ʧ��
----ע��:�˺������ܵ�������ʹ�ã���Ϊû�н�������-------------------------------
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayAndCheck(uint32 writeAddr, uint32 *pBuffer, uint16 numToWrite)   
{ 			 		 
	uint16 i;
	uint32 readWord;

	// �����ж�
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE+1024UL)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	for (i = 0; i < numToWrite; i++)
	{
		fmc_word_program(writeAddr, pBuffer[i]);		// д
		readWord = IAP_FlashReadWord(writeAddr);		// ��
	  	writeAddr += 4;

		fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 

	  	// д��FLASH�еĻ������������ȡ�������Ƿ�ƥ��
		if (readWord == pBuffer[i])
		{
			continue;
		}
		else
		{
			return FALSE;
		}
	}  
	return TRUE;
} 

/*******************************************************************************
  * @��������	IAP_FlashReadHalfWordArray
  * @����˵��   ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @�������   readAddr:��ȡ��ַ pBuffer:�����������׵�ַ numToWrite:��Ҫ����ĳ��ȣ���λ�����֣�
  * @���ز���   ��
*******************************************************************************/
void IAP_FlashReadWordArray(uint32 readAddr, uint32 *pBuffer, uint16 numToRead)   	
{
	uint16 i;

	// �����ж�
	if (NULL == pBuffer)
	{
		return;
	}

	// ���������Ƿ�Ƿ�
	if ((readAddr < IAP_GD32_FLASH_BASE) || ((readAddr + numToRead) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE+1024UL)))
	{
		return;	// �Ƿ���ַ
	}
	
	for (i = 0; i < numToRead; i++)
	{
		pBuffer[i] = IAP_FlashReadWord(readAddr);	// ��ȡ4���ֽ�.
		readAddr += 4;								// ƫ��4���ֽ�.	
	}
}

/*******************************************************************************
  * @��������	IAP_FlashReadHalfWord
  * @����˵��   ��ȡָ����ַ����(32λ����)
  * @�������   faddr:FLASH��ǰ��ַ
  * @���ز���   �ӵ�ǰFLASH��ַ��ʼ������ȡ32��λ���֣������ݣ�*(uint32*)faddr��ע���ַ����Ϊ4�ı���
*******************************************************************************/
uint32 IAP_FlashReadWord(uint32 flashAddr)
{
	return *(volatile uint32*)flashAddr; 
}

/*******************************************************************************
  * @��������	IAP_FlashWriteWordArrayWithErase
  * @����˵��   ��ָ����С�Ļ��������ݰ�ҳд��FLASH����
  * @�������   writeAddr:д���ַ pBuffer:�����������׵�ַ numToWrite:��Ҫд��ĳ��ȣ���λ���֣�
  * @���ز���   BOOL�͵ı���writeSucceed
*******************************************************************************/
BOOL IAP_FlashWriteWordArrayWithErase(uint32 writeAddr, uint32 *pBuffer, uint16 NumWordToWrite)	
{
	BOOL writeStatus = TRUE;
	uint32 pagePos = 0;						// ������ַ
	uint16 pageOffsent = 0;					// �������ڵ�ƫ��
	uint16 pageRemain = 0;  				// ����ʣ��ռ��С 
	uint32 offaddr;    						// ȥ��0X08000000��ĵ�ַ
 	uint16 i = 0;
 	uint32 * pFlashBuff = NULL;				// �����ȡ����������
 	uint16 numByteToWrite = NumWordToWrite * 4;
	
	// �����ж�
	if (NULL == pBuffer|| (NULL == numByteToWrite))
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�
	if ((writeAddr < IAP_GD32_FLASH_BASE) || ((writeAddr + numByteToWrite) > (IAP_GD32_FLASH_BASE+IAP_GD32_FLASH_SIZE + 1024UL)))
	{
		return FALSE;	// �Ƿ���ַ
	}

	// ����1024Byte�ռ䱣��ҳ����
	pFlashBuff = (uint32 *)malloc(IAP_STM32_PAGE_SIZE);
	if (NULL == pFlashBuff)
	{
		return FALSE;
	}

	offaddr = writeAddr - IAP_GD32_FLASH_BASE;		// ʵ��ƫ�Ƶ�ַ
	pagePos = offaddr / IAP_STM32_PAGE_SIZE;		// ҳ��ַ
	pageOffsent = offaddr % IAP_STM32_PAGE_SIZE;	// ��ҳ��ƫ��
	pageRemain = IAP_STM32_PAGE_SIZE - pageOffsent;	// ҳʣ��ռ��С

	if (numByteToWrite <= pageRemain)
	{
		pageRemain = numByteToWrite;				// �����ڸ�ҳ��Χ
	}
	
	DI();				// �ر����ж�
	fmc_unlock();		// ����

	// ������й����־λ
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	
	while(1)
	{
		// ��������ҳ������
		IAP_FlashReadWordArray(pagePos*IAP_STM32_PAGE_SIZE+IAP_STM32_PAGE_SIZE, pFlashBuff, IAP_STM32_PAGE_SIZE/4);

		for (i = 0; i < pageRemain/4; i++)	// У������
		{
			if (pFlashBuff[pageOffsent/4 + i] != 0xFFFFFFFF)
			{
				// ��������ݲ�ȫ��0xFFFFFFFF��˵����Ҫ����
				break;
			}
		}

		if (i < pageRemain/4)					// ��Ҫ����
		{
			// �������ҳ
			//IAP_PageErase(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);
			fmc_page_erase(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE);

			// ����
			for (i = 0; i < pageRemain/4; i++)
			{
				pFlashBuff[i + pageOffsent/4] = pBuffer[i]; 
			}

			// д����ҳ
			if (!IAP_FlashWriteWordArrayAndCheck(pagePos * IAP_STM32_PAGE_SIZE + IAP_GD32_FLASH_BASE, pFlashBuff, IAP_STM32_PAGE_SIZE/4))	
			{
				// �ͷ����ݻ�����
				free(pFlashBuff);

				fmc_lock();			// ����
				EI();				// �������ж�
				writeStatus = FALSE;
				return writeStatus;
			}
		}
		else 
		{
			// д�Ѿ������˵�,ֱ��д������ʣ������.
			if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, pageRemain/4))	
			{
				// �ͷ����ݻ�����
				free(pFlashBuff);

				fmc_lock();			// ����
				EI(); 				// �������ж�
				writeStatus = FALSE;
				return writeStatus;
			}
		}
		
		if (numByteToWrite == pageRemain)
		{
			break;									// д�������
		}
		else										// д��δ����
		{
			pagePos++;								// ������ַ��1
			pageOffsent = 0;						// ƫ��λ��Ϊ0 	 

		   	pBuffer += (pageRemain/4);  			// ָ��ƫ��
			writeAddr += pageRemain;				// д��ַƫ��	   
		   	numByteToWrite -= pageRemain;			// �ֽ����ݼ�
		   	
			if (numByteToWrite > IAP_STM32_PAGE_SIZE)
			{
				pageRemain = IAP_STM32_PAGE_SIZE;	// ��һ����������д����
			}
			else
			{
				pageRemain = numByteToWrite;		// ��һ����������д����	
			}
		}	
		
	}

	// �ͷ����ݻ�����
	free(pFlashBuff);	
	fmc_lock();			// ����
	EI(); 				// �������ж�
	return writeStatus;
}

/*******************************************************************************
  * @��������	IAP_FlashReadForCrc8
  * @����˵��   ����д��flash�����ݽ���crc8У��
  * @�������   ���ݳ��ȣ�ֻ����4�ı���
  * @���ز���   ����crc8��У����
*******************************************************************************/
uint8 IAP_FlashReadForCrc8(uint32 datalen)
{
	uint8 crc = 0;
	uint8 i,j;
	uint8 buf[4] = {0};
	uint32 addrIndex = 0;
	uint32 dataTemp;
	uint32 len = datalen/4;

	while (len--)
	{
		dataTemp = *(volatile uint32*)(addrIndex + IAP_FLASH_APP2_ADDR);
		addrIndex += 4;
		// С��ģʽ
		buf[3] = (uint8)(dataTemp >> 24);
		buf[2] = (uint8)(dataTemp >> 16);
		buf[1] = (uint8)(dataTemp >> 8);
		buf[0] = (uint8)dataTemp;
		
		for(j = 0; j < 4; j++)
		{
			crc ^= buf[j];
			
			for (i = 0; i < 8; i++)
			{
				if (crc & 0x01)
				{
					crc = (crc >> 1) ^ 0x8C;
				}
				else
				{
					crc >>= 1;
				}
			}
		}
	}
	return crc;
}


