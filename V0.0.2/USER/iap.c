#include "common.h"
#include "iap.h"
#include "timer.h"
#include "uartdrive.h"
#include "uartprotocol.h"
#include "param.h"
#include "BleUart.h"
#include "BleProtocol.h"
#include "uartDrive.h"
#include "uartProtocol.h"

/*********************************************************
* ����������
*********************************************************/
IAP_CB iapCB;

// FLASH��д����
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
	//����һ���������͵Ĳ���	
	typedef void (*IAP_Func)(void);
	IAP_Func jumpToApp;

	if(((*(vu32*)appAddr) & 0x2FFE0000) == 0x20000000)	// ���ջ����ַ�Ƿ�Ϸ�.
	{ 
		if (((*(vu32*)(appAddr+4)) & 0xFFF80000) != 0x08000000)   // APP��Ч
		{
			return;
		}

		// ��ת֮ǰ�����жϣ���λ����ΪĬ��ֵ
		DI();
		RCC_EnableAPB2PeriphReset(0xFFFFFFFF, ENABLE);
		RCC_EnableAPB2PeriphReset(0xFFFFFFFF, DISABLE);
		RCC_EnableAPB1PeriphReset(0xFFFFFFFF, ENABLE);
		RCC_EnableAPB1PeriphReset(0xFFFFFFFF, DISABLE);
		
		jumpToApp = (IAP_Func)*(vu32*)(appAddr+4);		// �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
	  	__set_MSP(*(vu32*)appAddr);						// ��ʼ���û�����Ķ�ջָ��
		jumpToApp();									// ��ת���û�����APP
	}
}

// ����ʱ�䵽��ת��APP����������
void IAP_JumpToAppFun(uint32 param)
{
	// ���ջ����ַ�Ƿ�Ϸ�.
	if(((*(volatile uint32*)IAP_FLASH_APP_ADDR) & 0x2FFE0000) != 0x20000000)	
	{
		return;
	}
	
	IAP_RunAPP(IAP_FLASH_APP_ADDR);
}

uint8 IAP_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
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

/*******************************************************************************
  * @��������	IAP_Init
  * @����˵��   
  * @�������   
  * @���ز���   ��
*******************************************************************************/
void IAP_Init(void)
{
	uint8 i;
	uint8 bootVersion[32];
	uint32 bBuff[1] = {0};
	
	iapCB.mcuUID.sn0 = *(uint32 *)(0x1FFFF7F0);
	iapCB.mcuUID.sn1 = *(uint32 *)(0x1FFFF7F4);
	iapCB.mcuUID.sn2 = *(uint32 *)(0x1FFFF7F8);

	// �����ж�������ʼλ��
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);

	// ע��:��дflash���뿪��HSIʱ��
	FLASH_ClockInit();

    // �̶���ַ��ȡBOOT�汾��
	IAP_FlashReadWordArray(IAP_MCU_BOOT_ADDEESS, (uint32 *)bootVersion, 8);
	
	// �����ȡ�İ汾�Ų�������ǿ�Ƹ���Ϊ��ǰAPP�汾
	if ((BOOT_VERSION_LENGTH != bootVersion[0]) || (!IAP_CmpareN((const uint8*)&bootVersion[1], (const uint8*)BOOT_VERSION, BOOT_VERSION_LENGTH))) 
	{
		bootVersion[0] = BOOT_VERSION_LENGTH;
		for (i = 0; i < BOOT_VERSION_LENGTH ; i++)
		{
			bootVersion[i + 1] = BOOT_VERSION[i];
		}
		IAP_FlashWriteWordArrayWithErase(IAP_MCU_BOOT_ADDEESS, (uint32*)bootVersion, (BOOT_VERSION_LENGTH + 4) / 4);
	}
	
	//	IAP_FlashWriteWordArrayWithErase(PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS,bBuff,1);
	
	
	// ������ʱ����ECOָ���λ��
	TIMER_AddTask(TIMER_ID_ECO_SEND,
					50,
					UART_PROTOCOL_SendEco,
					0,
					4,
					ACTION_MODE_DO_AT_ONCE);
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
	uint32 *iapBuf = NULL;
	BOOL writeStatus;
	
	// �����ж�
	if (NULL == appBuf)
	{
		return FALSE;
	}

	// ���������Ƿ�Ƿ�����ַ������IAP��������ַ�����Բ����Լ����Լ�����
	if ((appAddr < IAP_FLASH_APP_ADDR) || ((appAddr + appSize) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE)))
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
	uint32 i;
	uint16 j;
	uint16 readHalfWord;
	uint32 readAddr;
	
	for (i = (IAP_FLASH_APP_ADDR/IAP_FLASH_PAGE_SIZE); i < (IAP_N32_FLASH_SIZE/IAP_FLASH_PAGE_SIZE); i++)
	{
		// ҳ��ַ
		readAddr = i*IAP_FLASH_PAGE_SIZE ;

		for (j = 0; j < IAP_FLASH_PAGE_SIZE; j += 4)
		{
			readHalfWord = IAP_FlashReadWord(readAddr);	// ��
		  	readAddr += 4;

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
	FLASH_STS status = FLASH_COMPL;
	BOOL eraseStatus = TRUE;
	
	DI();				// �ر����ж�
    FLASH_Unlock();

    // ������й����־λ
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    // ����APP����ҳ
	for (i = (IAP_FLASH_ADRESS_OFFSET/IAP_FLASH_PAGE_SIZE); i < (IAP_N32_FLASH_SIZE/IAP_FLASH_PAGE_SIZE); i++)
	{
		status = FLASH_EraseOnePage(i*IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE);

		if (FLASH_COMPL != status)
		{
			eraseStatus = FALSE;
			break;
		}
	}

	FLASH_Lock();		// ����
	EI(); 				// �������ж�

	return eraseStatus;
}
 
// ����APP1
BOOL IAP_EraseAPP1Area(void)
{
	uint16 i;
	FLASH_STS status = FLASH_COMPL;
	BOOL eraseStatus = TRUE;
	
	DI();				// �ر����ж�
  FLASH_Unlock();

    // ������й����־λ
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

   // ����APP����ҳ
	for (i = 0 ; i < (IAP_FLASH_APP1_SIZE/IAP_FLASH_PAGE_SIZE); i++)
	{
		status = FLASH_EraseOnePage(i*IAP_FLASH_PAGE_SIZE + IAP_FLASH_APP1_ADDR);

		if (FLASH_COMPL != status)
		{
			eraseStatus = FALSE;

			break;
		}
	}

	FLASH_Lock();		// ����
	EI(); 				// �������ж�

	return eraseStatus;
}
 
/*******************************************************************************
  * @��������	IAP_FlashReadHalfWord
  * @����˵��   ��ȡָ����ַ�İ���(16λ����)
  * @�������   faddr:FLASH��ǰ��ַ
  * @���ز���   �ӵ�ǰFLASH��ַ��ʼ������ȡ16���ֽڣ����֣������ݣ�*(vu16*)faddr��ע���ַ����Ϊ4�ı���
*******************************************************************************/
uint32 IAP_FlashReadWord(uint32 flashAddr)
{
	return *(volatile uint32*)flashAddr; 
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
	if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numToWrite) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE+IAP_PARAM_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	for (i = 0; i < numToWrite; i++)
	{
		FLASH_ProgramWord(writeAddr, pBuffer[i]);			// д
		readWord = IAP_FlashReadWord(writeAddr);		// ��
	  	writeAddr += 4;

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
	if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numToWrite) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}
	
	DI();				// �ر����ж�
	FLASH_Unlock();		// ����

	// ������й����־λ
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	// д��ҳ
	if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, numToWrite))	
	{
		writeStatus = FALSE;
	}

	FLASH_Lock();		// ����
	EI(); 				// �������ж�
	
	return writeStatus;
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
	if ((readAddr < IAP_FLASH_APP_ADDR) || ((readAddr + numToRead) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE+IAP_PARAM_FLASH_SIZE)))
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
  * @��������	IAP_FlashReadHalfByteArray
  * @����˵��   ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @�������   readAddr:��ȡ��ַ pBuffer:�����������׵�ַ numToWrite:��Ҫ����ĳ��ȣ���λ�����֣�
  * @���ز���   ��
*******************************************************************************/
void IAP_FlashReadByteArray(uint32 readAddr, uint8 *pBuffer, uint16 numToRead) 
{
	
	uint16 i;
	uint8 offsetTmp;
	uint32 readBuf = 0;
	uint8 tmpBuf[4096] = {0};
	
	// �����ж�
	if (NULL == pBuffer)
	{
		return;
	}

	// ���������Ƿ�Ƿ�
	if ((readAddr < IAP_FLASH_APP_ADDR) || ((readAddr + numToRead * 4) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE+IAP_PARAM_FLASH_SIZE)))
	{
		return;	// �Ƿ���ַ
	}

	//tmpBuf = (uint8 *)malloc((numToRead/4 + 2)*4);

	// �ж��Ƿ�4�ֽڶ��룬�������4�ֽڶ��룬�����ƫ�ƴ���
	if (readAddr % 4 != 0)
	{
		offsetTmp = readAddr % 4;
		readAddr -= offsetTmp; 

		// ��ȡԭʼ����
		for (i = 0; i < (numToRead - 1)/4 + 2; i++)
		{
			readBuf = IAP_FlashReadWord(readAddr);

			tmpBuf[4 * i] = (uint8)readBuf;
			tmpBuf[4 * i + 1] = (uint8)(readBuf >> 8);
			tmpBuf[4 * i + 2] = (uint8)(readBuf >> 16);
			tmpBuf[4 * i + 3] = (uint8)(readBuf >> 24);

			readAddr += 4;
		}

		for (i = 0; i < numToRead ; i++)
		{
			pBuffer[i] = tmpBuf[i + offsetTmp];
		}

	}
	else
	{
		for (i = 0; i < (numToRead - 1)/4 + 1; i++)
		{
			readBuf = IAP_FlashReadWord(readAddr);		// ��ȡ4���ֽ�.
			
			tmpBuf[4 * i] = (uint8)readBuf;
			tmpBuf[4 * i + 1] = (uint8)(readBuf >> 8);
			tmpBuf[4 * i + 2] = (uint8)(readBuf >> 16);
			tmpBuf[4 * i + 3] = (uint8)(readBuf >> 24);
			
			readAddr += 4;								// ƫ��4���ֽ�.	
		}	

		for (i = 0; i < numToRead ; i++)
		{
			pBuffer[i] = tmpBuf[i];
		}
	}

	//free(tmpBuf);
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
	if ((writeAddr < IAP_FLASH_APP_ADDR) || ((writeAddr + numByteToWrite) > (IAP_N32_FLASH_BASE+IAP_N32_FLASH_SIZE + IAP_PARAM_FLASH_SIZE)))
	{
		return FALSE;	// �Ƿ���ַ
	}

	DI();				// �ر����ж�
	FLASH_Unlock();		// ����

	// ����1024Byte�ռ䱣��ҳ����
	pFlashBuff = (uint32 *)malloc(IAP_FLASH_PAGE_SIZE);
	if (NULL == pFlashBuff)
	{
		return FALSE;
	}

	offaddr = writeAddr - IAP_N32_FLASH_BASE;		// ʵ��ƫ�Ƶ�ַ
	pagePos = offaddr / IAP_FLASH_PAGE_SIZE;		// ҳ��ַ
	pageOffsent = offaddr % IAP_FLASH_PAGE_SIZE;	// ��ҳ��ƫ��
	pageRemain = IAP_FLASH_PAGE_SIZE - pageOffsent;	// ҳʣ��ռ��С

	if (numByteToWrite <= pageRemain)
	{
		pageRemain = numByteToWrite;				// �����ڸ�ҳ��Χ
	}
	
	while(1)
	{
		// ��������ҳ������
		IAP_FlashReadWordArray(pagePos*IAP_FLASH_PAGE_SIZE+IAP_N32_FLASH_BASE, pFlashBuff, IAP_FLASH_PAGE_SIZE/4);

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
			FLASH_EraseOnePage(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE);

			// ����
			for (i = 0; i < pageRemain/4; i++)
			{
				pFlashBuff[i + pageOffsent/4] = pBuffer[i]; 
			}

			// д����ҳ
			if (!IAP_FlashWriteWordArrayAndCheck(pagePos * IAP_FLASH_PAGE_SIZE + IAP_N32_FLASH_BASE, pFlashBuff, IAP_FLASH_PAGE_SIZE/4))	
			{
				// �ͷ����ݻ�����
				free(pFlashBuff);
	
				writeStatus = FALSE;

				break;
				//return writeStatus;
			}
		}
		else 
		{
			// д�Ѿ������˵�,ֱ��д������ʣ������.
			if (!IAP_FlashWriteWordArrayAndCheck(writeAddr, pBuffer, pageRemain/4))	
			{
				// �ͷ����ݻ�����
				free(pFlashBuff);
	
				writeStatus = FALSE;

				break;
				//return writeStatus;
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
		   	
			if (numByteToWrite > IAP_FLASH_PAGE_SIZE)
			{
				pageRemain = IAP_FLASH_PAGE_SIZE;	// ��һ����������д����
			}
			else
			{
				pageRemain = numByteToWrite;		// ��һ����������д����	
			}
		}	
		
	}

	// �ͷ����ݻ�����
	free(pFlashBuff);

	FLASH_Lock();		// ����
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
		dataTemp = *(volatile uint32*)(addrIndex + IAP_FLASH_APP_ADDR);
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



