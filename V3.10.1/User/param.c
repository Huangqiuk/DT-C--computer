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
 *                                  变量定义                                   *
********************************************************************************/
PARAM_CB paramCB;

// 默认运行参数
void PARAM_SetDefaultRunningData(void)
{
	uint8 length = 0;
	uint8 buf[50];
	uint8 i;
	uint8 * pBuff;
	
	// ■■ 不需要保存至NVM的参数 ■■
	
	paramCB.runtime.jumpFlag = (BOOL)0;
	
	// 固件版本号
	for (i = 0;i < 3;i++)
	{
		paramCB.runtime.devVersion[i] = DEV_VERSION[i];
		
	}
	paramCB.runtime.fwBinNum = FW_BINNUM;
	paramCB.runtime.fwBinVersion.devVersionRemain = DEV_VERSION_REMAIN;
	paramCB.runtime.fwBinVersion.devReleaseVersion = DEV_RELEASE_VERSION;
	paramCB.runtime.fwBinVersion.fwUpdateVersion = FW_UPDATE_VERSION;
	paramCB.runtime.fwBinVersion.fwDebugReleaseVersion = FW_DEBUG_RELEASE_VERSION;
	
	// 仪表唯一序列号
	paramCB.runtime.watchUid.sn0 = *(uint32*)(0x1FFFF7E8); 
	paramCB.runtime.watchUid.sn1 = *(uint32*)(0x1FFFF7EC); 
	paramCB.runtime.watchUid.sn2 = *(uint32*)(0x1FFFF7F0);

	sprintf((char*)paramCB.runtime.watchUid.snStr, "%08lX%08lX%08lX", paramCB.runtime.watchUid.sn2, paramCB.runtime.watchUid.sn1, paramCB.runtime.watchUid.sn0);

	// 读取标志区数据
	SPI_FLASH_ReadArray(paramCB.runtime.flagArr, SPI_FLASH_TEST_FLAG_ADDEESS, 16);

	
	
	paramCB.runtime.bootVersion[0] = IAP_VERSION_LENGTH;
	for(i = 0; i < IAP_VERSION_LENGTH ; i++)
	{
		paramCB.runtime.bootVersion[i + 1] = IAP_VERSION[i];
	}

	// 验证BOOT版本
	SPI_FLASH_ReadArray(&length, SPI_FLASH_BOOT_ADDEESS, 1);
	if (IAP_VERSION_LENGTH != length)
	{
		length = IAP_VERSION_LENGTH;
		SPI_FLASH_WriteByte(SPI_FLASH_BOOT_ADDEESS, length);
	}
	
	// 读取BOOT版本字符
	SPI_FLASH_ReadArray(buf, SPI_FLASH_BOOT_ADDEESS + 1, IAP_VERSION_LENGTH);
	
	// 如果读取的版本号不符，则强制更新为当前BOOT版本
	if (!PARAM_CmpareN((const uint8*)buf, (const uint8*)IAP_VERSION, IAP_VERSION_LENGTH)) 
	{
		SPI_FLASH_WriteWithErase((uint8*)IAP_VERSION, SPI_FLASH_BOOT_ADDEESS + 1, IAP_VERSION_LENGTH);
	}

	// 读取APP版本字符
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

	// 读取二维码版本字符
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

	// 读取UI版本
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
	
	// 读取硬件版本
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

	// 读取SN号
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

	// 读取蓝牙MAC地址
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
* 函 数 名: PARAM_Init
* 函数入参: void
* 函数出参: 无
* 返 回 值: uint32 
* 功能描述: 初始化参数
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年08月06日
*     修改描述: 新函数 		   
**********************************************************************/
void PARAM_Init(void)
{
	// 设置默认的运行数据
	PARAM_SetDefaultRunningData();
}


void Param_Process(void)
{
	uint8 i;
	uint8 TempStr[140];
	uint8 hmiTypeStr[10];
	
	if ((paramCB.runtime.qrRegEnable) && (bleUartCB.rcvBleMacOK) && (gsmUartCB.rcvIccidOK || gsmUartCB.rcvIccidTimeOut))
	{
		// 读取HMI类型
		/*E2PROM_ReadByteArray(PARAM_NVM_HMI_TYPE_START_ADDRESS, hmiTypeStr, PARAM_NVM_HMI_TYPE_SIZE);
		hmiTypeStr[PARAM_NVM_HMI_TYPE_SIZE] = '\0';

		// 判断目前仪表是何种类型
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
		// 不在支持的仪表类型里面
		else
		{
			// 拷贝字符串
			strcpy((char*)hmiTypeStr, (const char*)"BC18Uxx");
		}
		*/

		// 拷贝字符串
		strcpy((char*)hmiTypeStr, (const char*)"BC18Uxx");
	
		// 复制MAC地址到SPIflash备份	
		paramCB.runtime.bluMac[0] = 17;		// 长度
		for (i = 0; i < 17; i++)
		{
			paramCB.runtime.bluMac[i+1] = bleUartCB.rx.macBuff[i];
		}
		
		SPI_FLASH_WriteWithErase(&(paramCB.runtime.bluMac[0]), SPI_FLASH_BLE_MAC_ADDEESS, 17 + 1);
		

		// 设置无效GSM		
		for (i = 0; i < 20; i++)
		{
			gsmUartCB.rx.iccidBuff[i] = '0';
		}
		gsmUartCB.rx.iccidBuff[i] = '\0';

		
		// 存在GSM模块
		if ((gsmUartCB.rcvIccidOK) && (!gsmUartCB.rcvIccidTimeOut))
		{
			sprintf((char*)TempStr, "%s%s%s%s%s", paramCB.runtime.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_YES, hmiTypeStr);
		}
		// 没有GSM模块，超时导致的
		else
		{	
			sprintf((char*)TempStr, "%s%s%s%s%s", paramCB.runtime.watchUid.snStr, bleUartCB.rx.macBuff, gsmUartCB.rx.iccidBuff, GPS_TYPE_NO, hmiTypeStr);
		}

		// 注册二维码参数准备完毕，立刻上报
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

