#include "param.h"
#include "Spiflash.h"
#include "string.h"
#include "common.h"
#include "dutInfo.h"
#include "iap.h"

//// 定义全局变量
// DUT_INFO dut_info;

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
	uint8 appVer[3] = {0};
	uint8 bootVer[3] = {0};

	// 写入版本信息
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

	// dut机型信息初始化
	DutInfoUpdata();
}
