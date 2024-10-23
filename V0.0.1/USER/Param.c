/********************************************************************************
  * 文 件 名: Param.c
  * 版 本 号: 初版
  * 修改作者: Fly & ZJ & Workman
  * 修改日期: 2017年08月06日
  * 功能介绍: 该文件用于骑行参数的获取与计算，提供可靠的数据给上层显示或传输            
  ******************************************************************************
  * 注意事项:
  *
  * 							版权归迪太科技所有.
  *
*********************************************************************************/

/*******************************************************************************
 *                                  头文件	                                   *
********************************************************************************/
#include "Param.h"
#include "iap.h"
#include "timer.h"
#include "uartProtocol.h"


// 函数声明
void NVM_Load(void);
void NVM_Save(BOOL saveAll);
void NVM_Format(void);
BOOL NVM_IsFormatOK(void);
void PARAM_SetDefaultRunningData(void);
void PARAM_NewDayStart(void);
void PARAM_ClearTrip(void);

void PARAM_CaculateTripDistance(void);
void PARAM_CaculateCalories(void);
void PARAM_CaculateSpeed(void);
void PARAM_CALLBACK_CaculateRidingTime(uint32 param);

uint8 PARAM_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);


/*******************************************************************************
 *                                  变量定义                                   *
********************************************************************************/
PARAM_CB paramCB;

// NVM格式化标识
// const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

// 默认运行参数
void PARAM_SetDefaultRunningData(void)
{
	uint8 i;
	uint8 * pBuff = NULL;
	// ■■ 不需要保存至NVM的参数 ■■

	// 仪表唯一序列号  //0x1FFFF7F0
	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0); 
	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F0 + 0x04U); 
	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F0 + 0x08U);

	pBuff = (uint8 *)&(paramCB.runtime.appVersion);
	
	IAP_FlashReadWordArray(PARAM_MCU_APP_ADDEESS,(uint32 *)pBuff, 8);
	
	// 如果读取的版本号不符，则强制更新为当前APP版本
	if ((APP_VERSION_LENGTH != pBuff[0]) || (!PARAM_CmpareN((const uint8*)&pBuff[1], (const uint8*)APP_VERSION, APP_VERSION_LENGTH))) 
	{
		pBuff[0] = APP_VERSION_LENGTH;
		for(i = 0; i < APP_VERSION_LENGTH ; i++)
		{
			pBuff[i + 1] = APP_VERSION[i];
		}
		IAP_FlashWriteWordArrayWithErase(PARAM_MCU_APP_ADDEESS, (uint32*)pBuff, (APP_VERSION_LENGTH + 4) / 4);
	}

	pBuff = (uint8 *)&(paramCB.runtime.bootVersion);
	IAP_FlashReadWordArray(PARAM_MCU_BOOT_ADDEESS, (uint32 *)pBuff, 8);

	pBuff = (uint8 *)&(paramCB.runtime.SnCode);
	IAP_FlashReadWordArray(PARAM_MCU_SN_ADDEESS, (uint32 *)pBuff, 8);

	pBuff = (uint8 *)&(paramCB.runtime.LimeSnCode);
	IAP_FlashReadWordArray(PARAM_MCU_LIME_SN_ADDEESS, (uint32 *)pBuff, 8);

	pBuff = (uint8 *)&(paramCB.runtime.hwVersion);
	IAP_FlashReadWordArray(PARAM_MCU_HW_ADDEESS, (uint32 *)pBuff, 8);

	pBuff = (uint8 *)&(paramCB.runtime.testflag);
	IAP_FlashReadWordArray(PARAM_MCU_TEST_FLAG_ADDEESS, (uint32 *)pBuff, 16);	

	paramCB.runtime.matchingResults = bFALSE;
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

/*********************************************************************
* 函 数 名: PARAM_Process
* 函数入参: void
* 函数出参: 无
* 返 回 值: void 
* 功能描述: 模块过程处理函数
***********
* 修改历史:
*   1.修改作者: ZJ
*     修改日期: 2017年08月06日
*     修改描述: 新函数 		   
**********************************************************************/
void PARAM_Process(void)
{
	
}


/*********************************************************************
* 函   数  名: PARAM_LedFunctionTest
* 函数入参: uint32 param 
* 函数出参: uint32 ledFlag
* 返   回  值: void
* 功能描述:  调用使LED依次亮灭(功能测试使用，定时器调用函数)
***********
* 修改历史:
*   1.修改作者: MLei
*     修改日期: 2018/12/25
*     修改描述: 新函数
**********************************************************************/
uint32 ledFlag = 0; 
void PARAM_LedFunctionTest(uint32 param)
{
	ledFlag = (ledFlag + 1) % 4;

	switch (ledFlag)
	{
		case 0:
			// 红灯设置为常亮
			LED_SET(LED_NAME_RED, 100, 100, 100, 0, 0, 0);
			// 黄灯设置为常灭
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// 绿灯设置为常灭
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);

			break;

		case 1:
			// 红灯设置为常亮
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// 黄灯设置为常灭
			LED_SET(LED_NAME_YELLOW, 100, 100, 100, 0, 0, 0);
			// 绿灯设置为常灭
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);
			
			break;

		case 2:
			// 红灯设置为常亮
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// 黄灯设置为常灭
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// 绿灯设置为常灭
			LED_SET(LED_NAME_GREEN, 100, 100, 100, 0, 0, 0);
			
			break;

		case 3:
			// 红灯设置为常亮
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// 黄灯设置为常灭
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// 绿灯设置为常灭
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);

			break;

		default:
			// 红灯设置为常亮
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// 黄灯设置为常灭
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// 绿灯设置为常灭
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);
			
			break;
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

