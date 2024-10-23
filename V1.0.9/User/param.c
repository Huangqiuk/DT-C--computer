#include "param.h"
#include "Spiflash.h"
#include "string.h"
#include "common.h"
#include "dutInfo.h"
#include "iap.h"

//// ����ȫ�ֱ���
// DUT_INFO dut_info;

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
