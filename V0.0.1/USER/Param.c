/********************************************************************************
  * �� �� ��: Param.c
  * �� �� ��: ����
  * �޸�����: Fly & ZJ & Workman
  * �޸�����: 2017��08��06��
  * ���ܽ���: ���ļ��������в����Ļ�ȡ����㣬�ṩ�ɿ������ݸ��ϲ���ʾ����            
  ******************************************************************************
  * ע������:
  *
  * 							��Ȩ���̫�Ƽ�����.
  *
*********************************************************************************/

/*******************************************************************************
 *                                  ͷ�ļ�	                                   *
********************************************************************************/
#include "Param.h"
#include "iap.h"
#include "timer.h"
#include "uartProtocol.h"


// ��������
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
 *                                  ��������                                   *
********************************************************************************/
PARAM_CB paramCB;

// NVM��ʽ����ʶ
// const uint8 formatFlag[] = NVM_FORMAT_FLAG_STRING;

// Ĭ�����в���
void PARAM_SetDefaultRunningData(void)
{
	uint8 i;
	uint8 * pBuff = NULL;
	// ���� ����Ҫ������NVM�Ĳ��� ����

	// �Ǳ�Ψһ���к�  //0x1FFFF7F0
	paramCB.runtime.watchUid.sn0 = *(uint32 *)(0x1FFFF7F0); 
	paramCB.runtime.watchUid.sn1 = *(uint32 *)(0x1FFFF7F0 + 0x04U); 
	paramCB.runtime.watchUid.sn2 = *(uint32 *)(0x1FFFF7F0 + 0x08U);

	pBuff = (uint8 *)&(paramCB.runtime.appVersion);
	
	IAP_FlashReadWordArray(PARAM_MCU_APP_ADDEESS,(uint32 *)pBuff, 8);
	
	// �����ȡ�İ汾�Ų�������ǿ�Ƹ���Ϊ��ǰAPP�汾
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

/*********************************************************************
* �� �� ��: PARAM_Process
* �������: void
* ��������: ��
* �� �� ֵ: void 
* ��������: ģ����̴�����
***********
* �޸���ʷ:
*   1.�޸�����: ZJ
*     �޸�����: 2017��08��06��
*     �޸�����: �º��� 		   
**********************************************************************/
void PARAM_Process(void)
{
	
}


/*********************************************************************
* ��   ��  ��: PARAM_LedFunctionTest
* �������: uint32 param 
* ��������: uint32 ledFlag
* ��   ��  ֵ: void
* ��������:  ����ʹLED��������(���ܲ���ʹ�ã���ʱ�����ú���)
***********
* �޸���ʷ:
*   1.�޸�����: MLei
*     �޸�����: 2018/12/25
*     �޸�����: �º���
**********************************************************************/
uint32 ledFlag = 0; 
void PARAM_LedFunctionTest(uint32 param)
{
	ledFlag = (ledFlag + 1) % 4;

	switch (ledFlag)
	{
		case 0:
			// �������Ϊ����
			LED_SET(LED_NAME_RED, 100, 100, 100, 0, 0, 0);
			// �Ƶ�����Ϊ����
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// �̵�����Ϊ����
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);

			break;

		case 1:
			// �������Ϊ����
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// �Ƶ�����Ϊ����
			LED_SET(LED_NAME_YELLOW, 100, 100, 100, 0, 0, 0);
			// �̵�����Ϊ����
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);
			
			break;

		case 2:
			// �������Ϊ����
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// �Ƶ�����Ϊ����
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// �̵�����Ϊ����
			LED_SET(LED_NAME_GREEN, 100, 100, 100, 0, 0, 0);
			
			break;

		case 3:
			// �������Ϊ����
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// �Ƶ�����Ϊ����
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// �̵�����Ϊ����
			LED_SET(LED_NAME_GREEN, 100, 0, 100, 0, 0, 0);

			break;

		default:
			// �������Ϊ����
			LED_SET(LED_NAME_RED, 100, 0, 100, 0, 0, 0);
			// �Ƶ�����Ϊ����
			LED_SET(LED_NAME_YELLOW, 100, 0, 100, 0, 0, 0);
			// �̵�����Ϊ����
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

