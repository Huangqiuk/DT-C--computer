#include "common.h"
#include "sysinfo.h"
#include "delay.h"
#include "bleuart.h"
#include "gsmuart.h"
#include "iapctrluart.h"
#include "iap.h"
#include "e2prom.h"

SYS_INFO_CB sysInfoCB;

void SYS_INFO_CreateWatchUID(void)
{ 
	sysInfoCB.watchUid.sn0 = *(uint32*)(0x1FFFF7E8); 
	sysInfoCB.watchUid.sn1 = *(uint32*)(0x1FFFF7EC); 
	sysInfoCB.watchUid.sn2 = *(uint32*)(0x1FFFF7F0);

	sprintf((char*)sysInfoCB.watchUid.snStr, "%08lX%08lX%08lX", sysInfoCB.watchUid.sn2, sysInfoCB.watchUid.sn1, sysInfoCB.watchUid.sn0);
}

void SYS_INFO_GetSysInfo(void)
{
	SYS_INFO_CreateWatchUID();
}

void SYS_INFO_ParamProcess(void)
{
	uint8 TempStr[140];
	uint8 hmiTypeStr[10];
	
	if ((sysInfoCB.qrRegEnable) && (bleUartCB.rcvBleMacOK) && (gsmUartCB.rcvIccidOK || gsmUartCB.rcvIccidTimeOut))
	{
		// ��ȡHMI����
		//E2PROM_ReadByteArray(PARAM_NVM_HMI_TYPE_START_ADDRESS, hmiTypeStr, PARAM_NVM_HMI_TYPE_SIZE);
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
			strcpy((char*)hmiTypeStr, (const char*)HMI_TYPE_BCxxxx);
		}
		
		// ����GSMģ��
		if ((gsmUartCB.rcvIccidOK) && (!gsmUartCB.rcvIccidTimeOut))
		{
			sprintf((char*)TempStr, "%s%s%s%s%s", sysInfoCB.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_YES, hmiTypeStr);
		}
		// û��GSMģ�飬��ʱ���µ�
		else
		{
			sprintf((char*)TempStr, "%s%s%s%s%s", sysInfoCB.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_NO, hmiTypeStr);
		}

		// ע���ά�����׼����ϣ������ϱ�
		IAP_CTRL_UART_SendCmdQrRegParam(TempStr);

		sysInfoCB.qrRegEnable = FALSE;
		bleUartCB.rcvBleMacOK = FALSE;
		gsmUartCB.rcvIccidOK = FALSE;
		gsmUartCB.rcvIccidTimeOut = FALSE;
	}
}


