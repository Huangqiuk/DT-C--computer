/** ******************************************************************************
  * �� �� ��: Param.h
  * �� �� ��: ����
  * �޸�����: ZJ & Workman & Fly
  * �޸�����: 2017��08��06��
  * ���ܽ���: ���ļ��������в����Ļ�ȡ����㣬�ṩ�ɿ������ݸ��ϲ���ʾ����
  ******************************************************************************
  * ע������:
  *
  * 							��Ȩ���̫�Ƽ�����.
  *
*********************************************************************************/

/****************************** ��Ȩ���̫�Ƽ����� *********** �ļ���ʼ *********/
#ifndef 	__PARAM_H__
#define 	__PARAM_H__

/*******************************************************************************
 *                                  ͷ�ļ�	                                   *
********************************************************************************/
#include "common.h"
#include "uartProtocol.h"
#include "pwmLed.h"
#include "iap.h"
/*******************************************************************************
 *                                  �궨��	                                   *
********************************************************************************/
#define PARAM_MCU_PAGE_NUM					64
#define PARAM_MCU_PAGE_SIZE					2048UL

// SN��ַ
#define PARAM_MCU_SN_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 0)

// LIME SN��ַ
#define PARAM_MCU_LIME_SN_ADDEESS			(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 32)

// Ӳ���汾��ַ
#define PARAM_MCU_HW_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 64)

// BOOT�汾��ַ
#define PARAM_MCU_BOOT_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 96)

// APP�汾��ַ
#define PARAM_MCU_APP_ADDEESS				(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 128)

// ���Ա�ʶ�汾��ַ
#define PARAM_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE + (PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 320)

static uint32 PARAM_MCU_VERSION_ADDRESS[6] = {PARAM_MCU_BOOT_ADDEESS, PARAM_MCU_APP_ADDEESS, PARAM_MCU_TEST_FLAG_ADDEESS, \
											PARAM_MCU_SN_ADDEESS, PARAM_MCU_LIME_SN_ADDEESS, PARAM_MCU_HW_ADDEESS}; 

// ��ʾ��λ����ö�ٶ���
typedef enum
{
	UNIT_METRIC = 0,			// ����
	UNIT_INCH,					// Ӣ��

	UINT_COUNT	
}UNIT_E;


// ͨѶ������
#define PARAM_COMMUNICATION_ERROR_CODE					10

// ��ѹ����������
#define PARAM_UNDERVOLTAGE_PROTECTTION_ERROR_CODE		2

// �޴��������
#define PARAM_NO_ERROR_ERROR_CODE		0

#define METRIC_TO_INCH_COFF		(0.621371f)	// 1 km = 0.621371192237334 inch

// ��ص�������Դ
typedef enum
{
	BATTERY_DATA_SRC_ADC = 0,				// HMI���ݵ�ѹ���㷽ʽ
	BATTERY_DATA_SRC_BMS,					// BMS�ϱ�

	BATTERY_DATA_SRC_COUNT
}BATTERY_DATA_SRC_E;

// ϵͳ״̬
typedef enum
{
	SYSTEM_STATE_STOP = 0,

	SYSTEM_STATE_RUNNING,

	SYSTEM_STATE_MAX
}SYSTEM_STATE;

// LED����ģʽ
typedef enum
{
	PARAM_LED_MODE_ALL_OFF = 0,					// ����
	PARAM_LED_MODE_ALL_ON = 1,					// ����
	PARAM_LED_MODE_BLINK = 2,					// ��˸
	PARAM_LED_MODE_BREATH = 3,					// ����
	PARAM_LED_MODE_BREATH_50 = 4,				// ���� ����50%����
	PARAM_LED_MODE_BREATH_20 = 5,				// ���� ����20%����

	PARAM_LED_MODE_ALL_ON_50 = 6,				// ���� 50%����
	PARAM_LED_MODE_ALL_ON_20 = 7,				// ���� 20%����

	PARAM_LED_MODE_ALL_OFF_50 = 0x16,			// ���� 50%����(Ԥ��)
	PARAM_LED_MODE_ALL_OFF_20 = 0x17,			// ���� 20%����(Ԥ��)

	PARAM_LED_MODE_INVALID = 0xFF,				// ��Ч��ģʽ
}PARAM_LED_MODE_E;


// ����ϵͳע������籣����NVM�С���
typedef struct
{
	uint8 temp;
	BOOL uartLevel;
}NVM_CB;

// NVM���ߴ�
#define PARAM_NVM_DATA_SIZE						sizeof(NVM_CB)

// �������й��������ݣ����粻�������
typedef struct
{
	// SN��
	struct
	{
		uint8 snLenth;
		uint8 snStr[30+1];
	}SnCode;

	// LIME SN��
	struct
	{
		uint8 LimeSnLenth;
		uint8 LimeSnStr[30+1];
	}LimeSnCode;

	// Ӳ���汾��
	struct
	{
		uint8 hwVersionLenth;
		uint8 hwVersion[30+1];
	}hwVersion;

	// BOOT�汾��
	struct
	{
		uint8 bootVersionLenth;
		uint8 bootVersion[30+1];
	}bootVersion;

	// APP�汾��
	struct
	{
		uint8 appVersionLenth;
		uint8 appVersion[30+1];
	}appVersion;
	
	// �Ǳ�Ψһ���к�
	struct
	{
		uint32 sn0;
		uint32 sn1;
		uint32 sn2;
		uint8 snStr[24+1];
	}watchUid;

	struct
	{
		uint8 tipPic;

		uint8 ridingWarningPic;

		uint8 updataPercent;

		uint8 errorCode;
		
		uint8 phase;
		uint8 ridingError;
		
	}limeUi;
	uint8 interfaceCode;	

	// ���Ա�־
	uint32 testflag[16];

	uint32 ageTestFlag;
	uint8 pucTestFlag;

	// LED����ģʽ
	PARAM_LED_MODE_E ledMode[LED_NAME_MAX];

	uint32 segContrl;
	uint8 nfcFlag;
	uint8 matchingResults;
	// ��ز���
	struct
	{
		uint16 voltage;							// ��ǰ��ص�ѹ����λ:mV
		uint16 current; 						// ��ǰ��ص�������λ:mA
		uint8 lowVoltageAlarm;					// ��������͸澯
		uint8 percent;							// ��ذٷֱ�
		uint8 number;							// ��ؿ���
		BATTERY_DATA_SRC_E batteryDataSrc;		// �����������Դ
		uint8 state;							// ���״̬
		uint8 preState;
		uint8 charingValue;
		uint8 batLeve;
		uint8 virPerLattice;
	}battery;

	uint16 ridingSpeedLimit;
	uint16 ridingSpeed;
	uint8 ridingSpeedUnit;
	uint8 upDatingValue;

	
	// ��ʾʱ����У����־
	BOOL displayTimeCorrectedFlag;
	// ��ʾʱ����У����־
	uint8 displayTimeMode;
	
	// ��紫��������
	struct
	{	
		uint16 sampleValue;

		BOOL varyDirection;

		BOOL enable;
	}photoSensor;

	struct
	{
		BOOL  ageTestEnable;			// 
		uint32 txCount;
		uint32 rxCount;
		uint8 ageTestResoult;
	}ageTest;

	struct
	{
		uint32 timeInterval;
		BOOL onOff;
	}stopMode;	
}RUNTIME_DATA_CB;

/*******************************************************************************
 *                                  �ṹ��	                                   *
********************************************************************************/
// ���� ������в��� : ��Ϊ������:����NVM�벻����NVM ����
typedef struct
{
	RUNTIME_DATA_CB runtime;

	union
	{
  		NVM_CB param;

		uint8 array[PARAM_NVM_DATA_SIZE];
	}nvm;
	uint8 preValue[PARAM_NVM_DATA_SIZE];

	BOOL nvmWriteRequest;			// NVM��������
	BOOL nvmWriteEnable;			// NVM����ʹ��

	// ���������־
	BOOL recaculateRequest;		// �������������־��TRUE:������㣻FALSE:����Ҫ����
}PARAM_CB;

#define PARAM_SIZE	(sizeof(PARAM_CB))

extern PARAM_CB paramCB;		

/******************************************************************************
* ���ⲿ�ӿں꡿
******************************************************************************/
// ���ò��������־
#define PARAM_SetRecaculateRequest(a)		paramCB.recaculateRequest=a

// ����ܿ���ֵ
#define PARAM_SetSegContrl(a)				paramCB.runtime.segContrl=a
// ����ܿ���ֵ
#define PARAM_GetSegContrl()				(paramCB.runtime.segContrl)


//=======================================================================
#define PARAM_GetTestMode()						(paramCB.runtime.ageTest.ageTestEnable)

#define PARAM_SetTxCount(a)						paramCB.runtime.ageTest.txCount = a
#define PARAM_GetTxCount()						(paramCB.runtime.ageTest.txCount)

#define PARAM_SetRxCount(a)						paramCB.runtime.ageTest.rxCount = a
#define PARAM_GetRxCount()						(paramCB.runtime.ageTest.rxCount)

#define PARAM_SetAgeTest(a)						paramCB.runtime.ageTest.ageTestResoult = a
#define PARAM_GetAgeTest(a)						(paramCB.runtime.ageTest.ageTestResoult)


#define PARAM_SetStopModeTimeInterval(a)		paramCB.runtime.stopMode.timeInterval=a+100
#define PARAM_GetStopModeTimeInterval()		(paramCB.runtime.stopMode.timeInterval)

#define PARAM_SetStopModeOnOff(a)				paramCB.runtime.stopMode.onOff=a
#define PARAM_GetStopModeOnOff()				(paramCB.runtime.stopMode.onOff)

#define PARAM_SetLimeUiTipPic(a)				paramCB.runtime.limeUi.tipPic=a
#define PARAM_GetLimeUiTipPic()					(paramCB.runtime.limeUi.tipPic)

#define PARAM_SetLimeUiRidingWarningPic(a)		paramCB.runtime.limeUi.ridingWarningPic=a
#define PARAM_GetLimeUiRidingWarningPic()		(paramCB.runtime.limeUi.ridingWarningPic)

#define PARAM_SetLimeUiUpdataPercent(a)			paramCB.runtime.limeUi.updataPercent=a
#define PARAM_GetLimeUiUpdataPercent()			(paramCB.runtime.limeUi.updataPercent)

#define PARAM_SetLimeUiErrorCode(a)				paramCB.runtime.limeUi.errorCode=a
#define PARAM_GetLimeUiErrorCode()				(paramCB.runtime.limeUi.errorCode)


#define PARAM_SetRidingUiPhase(a)				paramCB.runtime.limeUi.phase=a
#define PARAM_GetRidingUiPhase()				(paramCB.runtime.limeUi.phase)

#define PARAM_SetRidingError(a)					paramCB.runtime.limeUi.ridingError=a
#define PARAM_GetRidingError()					(paramCB.runtime.limeUi.ridingError)

#define PARAM_SetInterfaceCode(a)				paramCB.runtime.interfaceCode=a
#define PARAM_GetInterfaceCode()				(paramCB.runtime.interfaceCode)

#define PARAM_SetAgeTestFlag(a)					paramCB.runtime.ageTestFlag=a
#define PARAM_GetAgeTestFlag()					(paramCB.runtime.ageTestFlag)

#define PARAM_SetPucTestFlag(a)					paramCB.runtime.pucTestFlag=a
#define PARAM_GetPucTestFlag()					(paramCB.runtime.pucTestFlag)


#define PARAM_SetLedMode(i,a)					paramCB.runtime.ledMode[i]=a
#define PARAM_GetLedMode(i)						(paramCB.runtime.ledMode[i])

// ���õ�ذٷֱ�
#define PARAM_SetBatteryPercent(a)			paramCB.runtime.battery.percent=a
// ��ȡ��ذٷֱ�
#define PARAM_GetBatteryPercent()			(paramCB.runtime.battery.percent)	

// ���õ�ؿ���
#define PARAM_SetBatteryNumber(a)			paramCB.runtime.battery.number=a
// ��ȡ��ؿ���
#define PARAM_GetBatteryNumber()			(paramCB.runtime.battery.number)


#define PARAM_SetVirBatteryLattice(a)		paramCB.runtime.battery.virPerLattice=a
#define PARAM_GetVirBatteryLattice()		(paramCB.runtime.battery.virPerLattice)

#define PARAM_SetRidingSpeedLimit(a)			paramCB.runtime.ridingSpeedLimit=a
#define PARAM_GetRidingSpeedLimit()				(paramCB.runtime.ridingSpeedLimit)

#define PARAM_SetRidingSpeed(a)					paramCB.runtime.ridingSpeed=a
#define PARAM_GetRidingSpeed()					(paramCB.runtime.ridingSpeed)

#define PARAM_SetRidingSpeedUnit(a)				paramCB.runtime.ridingSpeedUnit=a
#define PARAM_GetRidingSpeedUnit()				(paramCB.runtime.ridingSpeedUnit)
											

#define PARAM_SetUartLevel(a)							paramCB.nvm.param.uartLevel=a
#define PARAM_GetUartLevel()							(paramCB.nvm.param.uartLevel)

#define PARAM_SetMatchResults(a)			paramCB.runtime.matchingResults=a
#define PARAM_GetMatchResults()				(paramCB.runtime.matchingResults)

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// ģ���ʼ�������ģ���ʼ��
void PARAM_Init(void);

// ģ����̴���
void PARAM_Process(void);

// LED���ܲ���
void PARAM_LedFunctionTest(uint32 param);

#endif

