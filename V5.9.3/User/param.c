#include "common.h"
#include "param.h"
#include "spiflash.h"

#include "iap.h"
#include "dutInfo.h"

/*******************************************************************************
 *                                  ��������                                   *
 ********************************************************************************/
PARAM_CB paramCB;

// Ĭ�����в���
void PARAM_SetDefaultRunningData(void)
{
	// ���� ����Ҫ������NVM�Ĳ��� ����

	paramCB.runtime.jumpFlag = (BOOL)0;

	// �̼��汾��

	// ��֤BOOT�汾

	// ��ȡBOOT�汾�ַ�

	// ��ȡAPP�汾�ַ�

	// ��ȡ��ά��汾�ַ�

	// ��ȡUI�汾

	// ��ȡӲ���汾

	// ��ȡSN��

	// ��ȡ����MAC��ַ
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
	uint8 appVer[3] = {0};
	uint8 bootVer[3] = {0};

	// д��汾��Ϣ
	bootVer[0] = BOOT_VERSION_1;
	bootVer[1] = BOOT_VERSION_2;
	bootVer[2] = BOOT_VERSION_3;
	appVer[0] = APP_VERSION_1;
	appVer[1] = APP_VERSION_2;
	appVer[2] = APP_VERSION_3;
	SPI_FLASH_EraseSector(SPI_FLASH_BOOT_VERSION_ADDEESS);
	SPI_FLASH_EraseSector(SPI_FLASH_APP_VERSION_ADDEESS);
	SPI_FLASH_WriteArray(bootVer, SPI_FLASH_BOOT_VERSION_ADDEESS, 3);
	SPI_FLASH_WriteArray(appVer, SPI_FLASH_APP_VERSION_ADDEESS, 3);

	// dut������Ϣ��ʼ��
	DutInfoUpdata();
}

void PARAM_Process(void)
{
	// NVM��д�����󣬲���д��ʹ��ʱ���ſ���д��
	//	if((paramCB.nvmWriteRequest) && (paramCB.nvmWriteEnable))
	//	{
	//		paramCB.nvmWriteRequest = FALSE;
	//		paramCB.nvmWriteEnable = FALSE;

	//		// ���¸�λд���ݵ�NVM��ʱ��
	//		TIMER_ResetTimer(TIMTR_ID_NVM_WRITE_REQUEST);

	//		// ���컯����
	//		NVM_Save(FALSE);
	//	}
}

uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
{
	uint16 i = 0;

	while (i < len)
	{
		if (*(str1 + i) != *(str2 + i))
		{
			return 0;
		}
		i++;
	}
	return 1;
}

void FACTORY_NVM_SetDirtyFlag(BOOL req)
{
	paramCB.factoryNvmWriteRequest = TRUE;
}
