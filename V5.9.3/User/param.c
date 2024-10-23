#include "common.h"
#include "param.h"
#include "spiflash.h"

#include "iap.h"
#include "dutInfo.h"

/*******************************************************************************
 *                                  变量定义                                   *
 ********************************************************************************/
PARAM_CB paramCB;

// 默认运行参数
void PARAM_SetDefaultRunningData(void)
{
	// ■■ 不需要保存至NVM的参数 ■■

	paramCB.runtime.jumpFlag = (BOOL)0;

	// 固件版本号

	// 验证BOOT版本

	// 读取BOOT版本字符

	// 读取APP版本字符

	// 读取二维码版本字符

	// 读取UI版本

	// 读取硬件版本

	// 读取SN号

	// 读取蓝牙MAC地址
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

void PARAM_Process(void)
{
	// NVM有写入请求，并且写入使能时，才可以写入
	//	if((paramCB.nvmWriteRequest) && (paramCB.nvmWriteEnable))
	//	{
	//		paramCB.nvmWriteRequest = FALSE;
	//		paramCB.nvmWriteEnable = FALSE;

	//		// 重新复位写数据到NVM定时器
	//		TIMER_ResetTimer(TIMTR_ID_NVM_WRITE_REQUEST);

	//		// 差异化保存
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
