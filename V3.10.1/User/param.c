#include "common.h"
#include "param.h"
#include "spiflash.h"

#include "delay.h"
#include "bleuart.h"
#include "gsmuart.h"
#include "iapctrluart.h"
#include "iap.h"
#include "e2prom.h"


/*******************************************************************************
 *                                  ��������                                   *
********************************************************************************/
PARAM_CB paramCB;

// Ĭ�����в���
void PARAM_SetDefaultRunningData(void)
{
	uint8 length = 0;
	uint8 buf[50];
	uint8 i;
	uint8 * pBuff;
	
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
	paramCB.runtime.watchUid.sn0 = *(uint32*)(0x1FFFF7E8); 
	paramCB.runtime.watchUid.sn1 = *(uint32*)(0x1FFFF7EC); 
	paramCB.runtime.watchUid.sn2 = *(uint32*)(0x1FFFF7F0);

	sprintf((char*)paramCB.runtime.watchUid.snStr, "%08lX%08lX%08lX", paramCB.runtime.watchUid.sn2, paramCB.runtime.watchUid.sn1, paramCB.runtime.watchUid.sn0);

	// ��ȡ��־������
	SPI_FLASH_ReadArray(paramCB.runtime.flagArr, SPI_FLASH_TEST_FLAG_ADDEESS, 16);

	
	
	paramCB.runtime.bootVersion[0] = IAP_VERSION_LENGTH;
	for(i = 0; i < IAP_VERSION_LENGTH ; i++)
	{
		paramCB.runtime.bootVersion[i + 1] = IAP_VERSION[i];
	}

	// ��֤BOOT�汾
	SPI_FLASH_ReadArray(&length, SPI_FLASH_BOOT_ADDEESS, 1);
	if (IAP_VERSION_LENGTH != length)
	{
		length = IAP_VERSION_LENGTH;
		SPI_FLASH_WriteByte(SPI_FLASH_BOOT_ADDEESS, length);
	}
	
	// ��ȡBOOT�汾�ַ�
	SPI_FLASH_ReadArray(buf, SPI_FLASH_BOOT_ADDEESS + 1, IAP_VERSION_LENGTH);
	
	// �����ȡ�İ汾�Ų�������ǿ�Ƹ���Ϊ��ǰBOOT�汾
	if (!PARAM_CmpareN((const uint8*)buf, (const uint8*)IAP_VERSION, IAP_VERSION_LENGTH)) 
	{
		SPI_FLASH_WriteWithErase((uint8*)IAP_VERSION, SPI_FLASH_BOOT_ADDEESS + 1, IAP_VERSION_LENGTH);
	}

	// ��ȡAPP�汾�ַ�
	pBuff = &(paramCB.runtime.appVersion[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_APP_ADDEESS);
	if (31 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_APP_ADDEESS + 1, pBuff[0]);
	}

	// ��ȡ��ά��汾�ַ�
	pBuff = &(paramCB.runtime.qrCodeStr[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_QR_CODE_ADDEESS);
	if (63 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_QR_CODE_ADDEESS + 1, pBuff[0]);
	}

	// ��ȡUI�汾
	pBuff = &(paramCB.runtime.uiVersion[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_UI_VERSION_ADDEESS);
	if (31 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_UI_VERSION_ADDEESS + 1, pBuff[0]);
	}
	
	// ��ȡӲ���汾
	pBuff = &(paramCB.runtime.hardVersion[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_HW_VERSION_ADDEESS);
	if (31 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_HW_VERSION_ADDEESS + 1, pBuff[0]);
	}

	// ��ȡSN��
	pBuff = &(paramCB.runtime.snCode[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_SN_ADDEESS);
	if (31 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_SN_ADDEESS + 1, pBuff[0]);
	}

	// ��ȡ����MAC��ַ
	pBuff = &(paramCB.runtime.bluMac[0]);
	pBuff[0] = SPI_FLASH_ReadByte(SPI_FLASH_BLE_MAC_ADDEESS);
	if (31 < pBuff[0])
	{
		pBuff[0] = 3;
		pBuff[1] = 'E';
		pBuff[2] = 'R';
		pBuff[3] = 'R';
		pBuff[4] = 0;
	}
	else
	{
		SPI_FLASH_ReadArray(&(pBuff[1]), SPI_FLASH_BLE_MAC_ADDEESS + 1, pBuff[0]);
	}	
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


void Param_Process(void)
{
	uint8 i;
	uint8 TempStr[140];
	uint8 hmiTypeStr[10];
	
	if ((paramCB.runtime.qrRegEnable) && (bleUartCB.rcvBleMacOK) && (gsmUartCB.rcvIccidOK || gsmUartCB.rcvIccidTimeOut))
	{
		// ��ȡHMI����
		/*E2PROM_ReadByteArray(PARAM_NVM_HMI_TYPE_START_ADDRESS, hmiTypeStr, PARAM_NVM_HMI_TYPE_SIZE);
		hmiTypeStr[PARAM_NVM_HMI_TYPE_SIZE] = '\0';

		// �ж�Ŀǰ�Ǳ��Ǻ�������
		if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC18SU))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC18SU))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC28EU))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC28SU))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC28NU))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC18EC))
		{}
		else if (0 == strcmp((const char*)hmiTypeStr, (const char*)HMI_TYPE_BC28EC))
		{}
		// ����֧�ֵ��Ǳ���������
		else
		{
			// �����ַ���
			strcpy((char*)hmiTypeStr, (const char*)"BC18Uxx");
		}
		*/

		// �����ַ���
		strcpy((char*)hmiTypeStr, (const char*)"BC18Uxx");
	
		// ����MAC��ַ��SPIflash����	
		paramCB.runtime.bluMac[0] = 17;		// ����
		for (i = 0; i < 17; i++)
		{
			paramCB.runtime.bluMac[i+1] = bleUartCB.rx.macBuff[i];
		}
		
		SPI_FLASH_WriteWithErase(&(paramCB.runtime.bluMac[0]), SPI_FLASH_BLE_MAC_ADDEESS, 17 + 1);
		

		// ������ЧGSM		
		for (i = 0; i < 20; i++)
		{
			gsmUartCB.rx.iccidBuff[i] = '0';
		}
		gsmUartCB.rx.iccidBuff[i] = '\0';

		
		// ����GSMģ��
		if ((gsmUartCB.rcvIccidOK) && (!gsmUartCB.rcvIccidTimeOut))
		{
			sprintf((char*)TempStr, "%s%s%s%s%s", paramCB.runtime.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_YES, hmiTypeStr);
		}
		// û��GSMģ�飬��ʱ���µ�
		else
		{	
			sprintf((char*)TempStr, "%s%s%s%s%s", paramCB.runtime.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_NO, hmiTypeStr);
		}

		// ע���ά�����׼����ϣ������ϱ�
		IAP_CTRL_UART_SendCmdQrRegParam(TempStr);

		paramCB.runtime.qrRegEnable = FALSE;
		bleUartCB.rcvBleMacOK = FALSE;
		gsmUartCB.rcvIccidOK = FALSE;
		gsmUartCB.rcvIccidTimeOut = FALSE;
	}
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

