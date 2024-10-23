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
#include "uartprotocol.h"
/*******************************************************************************
 *                                  �궨��	                                   *
********************************************************************************/
#define PARAM_MCU_PAGE_NUM					64
#define PARAM_MCU_PAGE_SIZE					2048UL

// DT SN��ַ
#define PARAM_MCU_DT_SN_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 0)

// �ͻ�SN��ַ
#define PARAM_MCU_CUSTOMER_SN_ADDEESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 32)

// Ӳ���汾��ַ
#define PARAM_MCU_HW_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 64)

// BOOT�汾��ַ
#define PARAM_MCU_BOOT_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 96)

// APP�汾��ַ
#define PARAM_MCU_APP_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 128)

// APP������־��ַ
#define PARAM_MCU_BLE_UPDATA_FLAG_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 160)

// APP�����ɹ���־��ַ
#define PARAM_UPDATA_SUCCESS_FLAG_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 168)

// APP����CRC��ַ
#define PARAM_MCU_BLE_UPDATA_CRC_ADDEESS	(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 176)

// ��ά���ַ
#define PARAM_MCU_QR_ADDEESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 184)

// ���Ա�ʶ�汾��ַ
#define PARAM_MCU_TEST_FLAG_ADDEESS			(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 328)

#define NVM_FORMAT_FLAG_STRING				"202012101277"
#define NVM_FORMAT_FLAG_SIZE				sizeof(NVM_FORMAT_FLAG_STRING)

// NVM������FLASH�е��׵�ַ
#define PARAM_NVM_START_ADDRESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 2)*PARAM_MCU_PAGE_SIZE)

// NVM������FLASH�еı��õ�ַ
#define PARAM_NVM_BACKUP_START_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 3)*PARAM_MCU_PAGE_SIZE)

// �ָ��������õı�������
#define PARAM_NVM_FACTORY_RESET_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 512)

// ���м�¼�������
//#define RECORD_COUNT_MAX					10


// ��ʾ��λ����ö�ٶ���
typedef enum
{
	UNIT_METRIC = 0,			// ����
	UNIT_INCH,					// Ӣ��

	UINT_COUNT	
}UNIT_E;

// �綯���г�����������ʾ��
#define PARAM_E_BIKE_CODE						0x0E0B
// �綯���峵����������ʾ��
#define PARAM_E_HUA_CODE						0x0E05
// ������ʾ�ַ�UP
#define PARAM_E_UPDATA_CODE						0x1112

#define METRIC_TO_INCH_COFF		(0.621371f)	// 1 km = 0.621371192237334 inch

// ��ص�������Դ
typedef enum
{
	//BATTERY_DATA_SRC_ADC = 0,				// ��ѹ���Ǳ����
	
	BATTERY_DATA_SRC_CONTROLLER,			// ��ѹ�ɿ������ϱ�,�Ǳ�������
	BATTERY_DATA_SRC_BMS,					// ����ֱ���ɿ������ϱ�
	BATTERY_DATA_SRC_ADC,					// ��ѹ�������Ǳ����

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

// �����־�ID
typedef enum{
	//PARAM_WHEEL_SIZE_4_5_INCH = 0,
	//PARAM_WHEEL_SIZE_6_INCH,
	//PARAM_WHEEL_SIZE_6_4_INCH,
	//PARAM_WHEEL_SIZE_8_INCH,
	//PARAM_WHEEL_SIZE_10_INCH,
	PARAM_WHEEL_SIZE_12_INCH,
	PARAM_WHEEL_SIZE_14_INCH,
	PARAM_WHEEL_SIZE_16_INCH,
	PARAM_WHEEL_SIZE_18_INCH,
	PARAM_WHEEL_SIZE_20_INCH,
	PARAM_WHEEL_SIZE_22_INCH,
	PARAM_WHEEL_SIZE_24_INCH,
	PARAM_WHEEL_SIZE_26_INCH,
	PARAM_WHEEL_SIZE_27_INCH,
	PARAM_WHEEL_SIZE_27_5_INCH,
	PARAM_WHEEL_SIZE_28_INCH,
	PARAM_WHEEL_SIZE_29_INCH,
	PARAM_WHEEL_SIZE_700C,
	
	PARAM_WHEEL_SIZE_MAX
}PARAM_WHEEL_SIZE_ID;

// ����������Ͷ���
typedef enum
{
	ERROR_TYPE_NO_ERROR = 0x00,							// �޴���
	ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x20,			// ͨѶ���ճ�ʱ
	ERROR_TYPE_BAT_ERROR = 0x10,						// ��ع���
	ERROR_TYPE_DRIVER_ERROR = 0x08,						// ����������
	ERROR_TYPE_HALL_ERROR = 0x04,						// ��������
	ERROR_TYPE_BREAK_ERROR = 0x02,						// ɲ������
	ERROR_TYPE_TURN_ERROR = 0x01,						// ת�ѹ���	
	
	//ERROR_TYPE_BATTERY_UNDER_VOLTAGE_ERROR = 6,		// ���Ƿѹ
	//ERROR_TYPE_MOTOR_ERROR = 0x08,					// �������
	//ERROR_TYPE_TURN_ERROR = 8,						// ת�ѹ���
	//ERROR_TYPE_THROTTLE_ERROR = 0x40,					// ���Ź���
	
}ERROR_TYPE_E;

// ������λö�ٶ���
typedef enum
{
	ASSIST_0 = 0,
	ASSIST_1,
	ASSIST_2,
	ASSIST_3,
	ASSIST_4,
	ASSIST_5,
	ASSIST_6,
	ASSIST_7,
	ASSIST_8,
	ASSIST_9,
	ASSIST_CRUISE,
	ASSIST_P,

	ASSIST_ID_COUNT
}ASSIST_ID_E;

// ��Ʒ���Ͷ���
typedef enum
{
	HMI_TYPE_BC18EU = 0,				// ��̫���BC18��TFT����UARTͨѶ
	HMI_TYPE_BC18SU,					// ��̫���BC18��TFT����BLE��UARTͨѶ
	HMI_TYPE_BC28EU,					// ��̫���BC28��TFT����UARTͨѶ
	HMI_TYPE_BC28SU,					// ��̫���BC28��TFT����BLE��UARTͨѶ
	HMI_TYPE_BC28NU,					// ��̫���BC28��TFT����BLE��GSM��GPS��UARTͨѶ
	HMI_TYPE_BC18EC,					// �������BC18��TFT����CANͨѶ
	HMI_TYPE_BC28EC,					// �������BC28��TFT����CANͨѶ

	
	HMI_TYPE_BN400,						// ������Ǳ�BN400, �������ʾ, UARTͨѶ

	HMI_TYPE_MAX,
}HMI_TYPE_E;


// ����ϵͳע������籣����NVM�С���
typedef struct
{
	// ϵͳ����
	struct
	{
		PARAM_WHEEL_SIZE_ID wheelSizeID;	// �־�ID
		BOOL resFactorySet;					// �ָ�����������

		uint8 rgbSwitch;				// RGB�ƿ���	1-������Χ�ƣ�0-�رշ�Χ��
		uint8 ledRgb;						// ������LED�Ƶ�RGB��ɫ	0-255 ����ͬ��ɫ�����ڵ�ɫ����ʱ����������ɫ��
		uint8 rgbMode;					// RGB��ģʽ����	0���ر�	1����ɫ����	2��ȫ�ʺ���	3����ɫ����	4������ģʽ	5������ģʽ
		uint8 headlight;				// ���
		uint8 dayLight;					// ���е�
		uint8 leftLight;				// ��ת��
		uint8 rightLight;				// ��ת��
		uint16 powerMax;				// ���������,��λ:W
		
		// ���������ò���
		struct
		{
			uint8 steelNumOfSpeedSensor;		// ���Դ�����һȦ�Ÿ���(�� ���ٴŸ�)
			uint16 currentLimit; 				// �������ޣ���λ:mA
			uint16 lowVoltageThreshold;			// Ƿѹ���ޣ���λ:mV

			uint8 controlMode;					// ����������ģʽ
			BOOL zeroStartOrNot;				// ���������������
			BOOL switchCruiseWay;				// �л�Ѳ���ķ�ʽ
			BOOL switchCruiseMode;				// Ѳ���л�ģʽ
			uint8 assistSensitivity;			// ����������
			uint8 assistStartIntensity;			// ��������ǿ��
			uint8 assistSteelType;				// �����Ÿ�����
			uint8 reversalHolzerSteelNum;		// ���ٵ����������Ÿ���
			BOOL speedLimitSwitch;				// ����λ����
			BOOL CruiseEnabled;					// Ѳ��ʹ��

			BOOL breakType;						// ɲ������
		}driver;
	}protocol;

	// ��������
	struct
	{
		//HMI_TYPE_E hmiType;					// �������
		UNIT_E unit;						// ��λ, ������Ӣ�� 	
		ASSIST_ID_E assistMax;				// ���������λ
		ASSIST_ID_E assist; 				// ������λ
		
		uint32 busAliveTime;				// ͨѶ���ϳ�ʱʱ�䣬��λ:ms
		uint8  brightness;					// ��������
		uint8  powerOffTime; 				// �Զ��ػ�ʱ�䣬��λ:����
		BOOL existBle;						// �Ƿ��������

		uint8  workAssistRatio;				// ������������������

		uint8 percentageMethod;				// �������㷽ʽ

		BOOL lockFlag; 						// ����״̬
		
		// �ٶ���
		struct
		{
			uint16 limitVal;				// �������ޣ���λ:0.1Km/h
			uint8  filterLevel;				// �ٶ�ƽ���� 111
		}speed;

		// �����
		struct
		{
			uint8 voltage;					// ��ع����ѹ����λ:V
			uint8 filterLevel;				// �����仯ʱ�䣬��λ:��
		}battery;
		
		struct
		{
			// �Գ���������ͳ������
			struct
			{
				uint32 distance;			// ������������̣���λ:0.1Km				
				uint32 preDistance;			// ������������̣���λ:0.1Km
				uint16 maxTripOf24hrs;		// ���˼�¼����λ:0.1Km
				uint32 ridingTime;			// ����ʱ�䣬��λ:s
				uint32 calories;			// ���������ܿ�·���λ:KCal
			}total;

			// ��Trip����������ͳ������
			struct
			{
				uint32 distance;			// Trip����λ:0.1Km
				uint32 calories;			// ��·���λ:KCal
				uint16 speedMax;			// ����ٶȣ���λ:0.1Km/h

				uint32 ridingTime;			// ������ʱ����
			}trip;

			// ��0��������ͳ������
			struct
			{
				uint16 trip;				// ������̣���λ:0.1Km 	
				uint16 calories;			// ���տ�·���λ:KCal

				uint32 ridingTime;			// ���������ۼ�ʱ�䣬��λ:s

				uint16 year;				// ���ղ�����Ӧ������
				uint8 month; 				// ���ղ�����Ӧ������
				uint8 day;					// ���ղ�����Ӧ������
			}today;

		}record;

		// �ŵ���󣬿��Ա�ֻ֤��ǰ���ֵ��д��֮��ſ�ʼд��ʽ����ʶ�ַ���
		uint8 nvmFormat[NVM_FORMAT_FLAG_SIZE];			// NVM��ʽ����ʶ
	}common;
}NVM_CB;

// NVM���ߴ�
#define PARAM_NVM_DATA_SIZE						sizeof(NVM_CB)

// �������й��������ݣ����粻�������
typedef struct
{
	// DT SN��
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

	// ���Ա�־
	uint32 testflag[16];

	uint32 ageTestFlag;
	uint8 pucTestFlag;

	uint32 segContrl;

	BOOL jumpFlag;
	
	uint8 devVersion[3];					// �Ǳ��ͺ�
	uint8 fwBinNum;							// �̼��汾���

	struct
	{
		uint8 devVersionRemain;				// �豸�����汾��(����汾��)
		uint8 devReleaseVersion;			// ����������ķ����汾��(��Ʒ����汾��)
		uint8 fwUpdateVersion;				// ���Է����汾��(�����汾��)
		uint8 fwDebugReleaseVersion;		// ������ʱ�汾��
	}fwBinVersion;
	
	uint16 perimeter[PARAM_WHEEL_SIZE_MAX];	// �����ܳ�����λ:mm
	
	BOOL bleConnectState;
	
	uint8 bleRadio; 

	BOOL RgbLedCtrlFlag; //��Χ������״̬
	uint8 blueNumber;
	
	uint8 carNum[14]; //�������к�
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

		uint32 array[(PARAM_NVM_DATA_SIZE + 3) /4];
	}nvm;
	uint32 preValue[(PARAM_NVM_DATA_SIZE + 3) /4];

	BOOL nvmWriteRequest;			// NVM��������
	BOOL nvmWriteEnable;			// NVM����ʹ��

	// ���������־
	BOOL recaculateRequest;		// �������������־��TRUE:������㣻FALSE:����Ҫ����
}PARAM_CB;

#define PARAM_SIZE	(sizeof(PARAM_CB))

extern PARAM_CB paramCB;		
extern const uint32 PARAM_MCU_VERSION_ADDRESS[];

/******************************************************************************
* ���ⲿ�ӿں꡿
******************************************************************************/
// ���ò��������־
#define PARAM_SetRecaculateRequest(a)			paramCB.recaculateRequest=a

// ����Ѳ�����أ�0:��ʾ�رգ�1:��ʾ��
#define PARAM_SetCruiseSwitch(enable)			paramCB.runtime.cruiseEnabe=enable
// ��ȡѲ�����أ�0:��ʾ�رգ�1:��ʾ��
#define PARAM_GetCruiseSwitch()					(paramCB.runtime.cruiseEnabe)

// ���ô�ƿ��� 
#define PARAM_SetLightSwitch(enable)		do{\
												paramCB.runtime.lightSwitch=enable;\
											}while(0)
// ��ȡ��ƿ���
#define PARAM_GetLightSwitch()				(paramCB.runtime.lightSwitch)

// ����β�ƿ��� 
#define PARAM_SetTailLightSwitch(a)				paramCB.runtime.tailLight=a

// ��ȡβ�ƿ��� 
#define PARAM_GetTailLightSwitch()				(paramCB.runtime.tailLight)



// �򿪻�ر��Ƴ�����
#define PARAM_EnablePushAssist(enable)		do{\
												paramCB.runtime.pushAssistOn=enable;\
											}while(0)
// ��ȡ�Ƴ���������״̬
#define PARAM_IsPushAssistOn()				(paramCB.runtime.pushAssistOn)

// ����ܿ���ֵ
#define PARAM_SetSegContrl(a)					paramCB.runtime.segContrl=a
// ����ܿ���ֵ
#define PARAM_GetSegContrl()					(paramCB.runtime.segContrl)

// ���������Ƿ�����״̬
#define	PARAM_SetBleConnectState(a)			paramCB.runtime.bleConnectState=a
// ��ȡ�����Ƿ�����״̬
#define	PARAM_GetBleConnectState()			(paramCB.runtime.bleConnectState)

//=======================================================================
#define PARAM_GetTestMode()						(paramCB.runtime.ageTest.ageTestEnable)

#define PARAM_SetTxCount(a)						paramCB.runtime.ageTest.txCount = a
#define PARAM_GetTxCount()						(paramCB.runtime.ageTest.txCount)

#define PARAM_SetRxCount(a)						paramCB.runtime.ageTest.rxCount = a
#define PARAM_GetRxCount()						(paramCB.runtime.ageTest.rxCount)

#define PARAM_SetAgeTest(a)						paramCB.runtime.ageTest.ageTestResoult = a
#define PARAM_GetAgeTest(a)						(paramCB.runtime.ageTest.ageTestResoult)


#define PARAM_SetStopModeTimeInterval(a)		paramCB.runtime.stopMode.timeInterval=a+100
#define PARAM_GetStopModeTimeInterval()			(paramCB.runtime.stopMode.timeInterval)

#define PARAM_SetStopModeOnOff(a)				paramCB.runtime.stopMode.onOff=a
#define PARAM_GetStopModeOnOff()				(paramCB.runtime.stopMode.onOff)

#define PARAM_SetAgeTestFlag(a)					paramCB.runtime.ageTestFlag=a
#define PARAM_GetAgeTestFlag()					(paramCB.runtime.ageTestFlag)

#define PARAM_SetPucTestFlag(a)					paramCB.runtime.pucTestFlag=a
#define PARAM_GetPucTestFlag()					(paramCB.runtime.pucTestFlag)
//==========================================================================================

// ��������ʱ��
#define PARAM_SetRTC(YY,MM,DD,hh,mm,ss)		do{\
													paramCB.runtime.rtc.year=YY;\
													paramCB.runtime.rtc.month=MM;\
													paramCB.runtime.rtc.day=DD;\
													paramCB.runtime.rtc.hour=hh;\
													paramCB.runtime.rtc.minute=mm;\
													paramCB.runtime.rtc.second=ss;\
												}while(0)
// ����ϵͳʱ����
#define PARAM_SetRTCSecond(a)				paramCB.runtime.rtc.second=a
// ��ȡϵͳʱ����
#define PARAM_GetRTCSecond()				(paramCB.runtime.rtc.second)
	
// ����ϵͳʱ���
#define PARAM_SetRTCMin(a)					paramCB.runtime.rtc.minute=a
// ��ȡϵͳʱ���
#define PARAM_GetRTCMin()					(paramCB.runtime.rtc.minute)
	
// ����ϵͳʱ��ʱ
#define PARAM_SetRTCHour(a)					paramCB.runtime.rtc.hour=a
// ��ȡϵͳʱ��ʱ
#define PARAM_GetRTCHour()					(paramCB.runtime.rtc.hour)
	
// ����ϵͳʱ����
#define PARAM_SetRTCDay(a)					paramCB.runtime.rtc.day=a
// ��ȡϵͳʱ����
#define PARAM_GetRTCDay()					(paramCB.runtime.rtc.day)
	
// ����ϵͳʱ����
#define PARAM_SetRTCMonth(a)				paramCB.runtime.rtc.month=a
// ��ȡϵͳʱ����
#define PARAM_GetRTCMonth()					(paramCB.runtime.rtc.month)
	
// ����ϵͳʱ����
#define PARAM_SetRTCYear(a)					paramCB.runtime.rtc.year=a
// ��ȡϵͳʱ����
#define PARAM_GetRTCYear()					(paramCB.runtime.rtc.year)


// ���õ�ذٷֱ�
#define PARAM_SetBatteryPercent(a)				paramCB.runtime.battery.percent=a
// ��ȡ��ذٷֱ�
#define PARAM_GetBatteryPercent()				(paramCB.runtime.battery.percent)	

#define PARAM_SetErrorCode(a)				paramCB.runtime.errorType=a
// ��ȡ�������
#define PARAM_GetErrorCode()				(paramCB.runtime.errorType)

// ���õ�ص�������Դ
#define PARAM_SetBatteryDataSrc(a)			paramCB.runtime.battery.batteryDataSrc=a
// ��ȡ��ص�������Դ
#define PARAM_GetBatteryDataSrc()			(paramCB.runtime.battery.batteryDataSrc)


// ����Ѳ��״̬��0:��ֹ��1:����Ѳ�� 222
#define PARAM_SetCruiseState(enable)		paramCB.runtime.cruiseState=enable
// ��ȡѲ��״̬��0:��ֹ��1:����Ѳ��
#define PARAM_GetCruiseState()				(paramCB.runtime.cruiseState)

// ����6km�Ƴ�����״̬ 222
#define PARAM_SetPushAssistState(a)			paramCB.runtime.PushAssistState=a
// ��ȡ6km�Ƴ�����״̬
#define PARAM_GetPushAssistState()			(paramCB.runtime.PushAssistState)

// ���õ�ص�������λ:mA 222
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// ��ȡ��������λ:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)


// �����ٶȣ���λ:0.1Km/h 222
#define PARAM_SetSpeed(a)					paramCB.runtime.speed=a
// ��ȡ�ٶȣ���λ:0.1Km/h
//#define PARAM_GetSpeed()					(paramCB.runtime.speed)


// ����������λ
//#define PARAM_SetAssistLevel(a)				do{\
												paramCB.runtime.assist=a;\
											}while(0)
// ��ȡ������λ
//#define PARAM_GetAssistLevel()				(paramCB.runtime.assist)

// ����RGB�ƿ���
#define PARAM_SetRgbSwitch(a)				paramCB.nvm.param.protocol.rgbSwitch=a
// ��ȡRGB�ƿ���
#define PARAM_GetRgbSwitch()				(paramCB.nvm.param.protocol.rgbSwitch)

// ����RGB��ɫ
#define PARAM_SetRgbColour(a)				paramCB.nvm.param.protocol.ledRgb=a
// ��ȡRGB��ɫ
#define PARAM_GetRgbColour()				(paramCB.nvm.param.protocol.ledRgb)

// ����RGB��ģʽ
#define PARAM_SetRgbMode(a)					paramCB.nvm.param.protocol.rgbMode=a
// ��ȡRGB��ģʽ
#define PARAM_GetRgbMode()					(paramCB.nvm.param.protocol.rgbMode)

// ���ô�ƿ���
#define PARAM_Setheadlight(a)				paramCB.nvm.param.protocol.headlight=a
// ��ȡ��ƿ���
#define PARAM_Getheadlight()				(paramCB.nvm.param.protocol.headlight) 

// �������еƿ���
#define PARAM_SetdayLight(a)				paramCB.nvm.param.protocol.dayLight=a
// ��ȡ���п���
#define PARAM_GetdayLight()					(paramCB.nvm.param.protocol.dayLight) 

// ������ת�ƿ���
#define PARAM_SetleftLight(a)				paramCB.nvm.param.protocol.leftLight=a
// ��ȡ��ת�ƿ���
#define PARAM_GetleftLight()					(paramCB.nvm.param.protocol.leftLight) 

// ������ת�ƿ���
#define PARAM_SetrightLight(a)				paramCB.nvm.param.protocol.rightLight=a
// ��ȡ��ת�ƿ���
#define PARAM_GetrightLight()					(paramCB.nvm.param.protocol.rightLight) 


// ����������λ
#define PARAM_SetAssistLevel(a)				do{\
												paramCB.nvm.param.common.assist=a;\
											}while(0)
// ��ȡ������λ
#define PARAM_GetAssistLevel()				(paramCB.nvm.param.common.assist)

// ���õ������,��λ:W
#define PARAM_SetPower(a)					paramCB.runtime.power=a
// ��ȡ�������,��λ:W
#define PARAM_GetPower()					(paramCB.runtime.power)	

// ��ȡ����������������ʱ�䣬��λ:s
#define PARAM_GetTotalRidingTime()			(paramCB.nvm.param.common.record.total.ridingTime)
// ��ȡ�����������ܿ�·���λ:KCal
#define PARAM_GetTotalCalories()			(paramCB.nvm.param.common.record.total.calories)

// �û�����ٶ�����
#define PARAM_ClearUserMaxSpeed()			paramCB.runtime.record.userMaxSpeed=0
// ��ȡ�û�����ٶ�
#define PARAM_GetUserMaxSpeed()				(paramCB.runtime.record.userMaxSpeed)


// ��������״̬
#define PARAM_SetLockFlag(a)				paramCB.nvm.param.common.lockFlag=a;
// ��ȡ����״̬
#define PARAM_GetLockFlag()					(paramCB.nvm.param.common.lockFlag)

// �����Ƿ�����
#define PARAM_SetIsLocked(locked)			paramCB.runtime.isLocked=locked
// ��ȡ����״̬
#define PARAM_GetIsLocked()					(paramCB.runtime.isLocked)

// �������������,��λ:W
#define PARAM_SetPowerMax(a)				paramCB.nvm.param.protocol.powerMax=a
// ��ȡ���������,��λ:W
#define PARAM_GetPowerMax()					(paramCB.nvm.param.protocol.powerMax)


// ���õ�ص�ѹ�ȼ�,��λ:V 111
#define PARAM_SetBatteryVoltageLevel(a)		paramCB.nvm.param.common.battery.voltage=a 
// ��ȡ��ص�ѹ�ȼ�,��λ:V
#define PARAM_GetBatteryVoltageLevel()		(paramCB.nvm.param.common.battery.voltage)

// ���õ�ص�ѹ����λ:mV
#define PARAM_SetBatteryVoltage(mV)			paramCB.runtime.battery.voltage=mV
// ��ȡ��ص�ѹ����λ:mV
#define PARAM_GetBatteryVoltage()			(paramCB.runtime.battery.voltage)


// ���õ�ص�������λ:mA
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// ��ȡ��������λ:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)


// ���ñ������� 111
#define PARAM_SetBrightness(a)				paramCB.nvm.param.common.brightness=a
// ��ȡ��������
#define PARAM_GetBrightness()				(paramCB.nvm.param.common.brightness)

// �����Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min 111
#define PARAM_SetPowerOffTime(a)			paramCB.nvm.param.common.powerOffTime=a
// ��ȡ�Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min
#define PARAM_GetPowerOffTime()				(paramCB.nvm.param.common.powerOffTime)

// ��������������ģʽ 111
#define PARAM_SetDriverControlMode(a)		paramCB.nvm.param.protocol.driver.controlMode=a
// ��ȡ����������ģʽ
#define PARAM_GetDriverControlMode()		(paramCB.nvm.param.protocol.driver.controlMode)

// �������λ 111
#define PARAM_SetMaxAssist(a)				do{\
												paramCB.nvm.param.common.assistMax=a;\
											}while(0)
// ��ȡ���λ
#define PARAM_GetMaxAssist()				(paramCB.nvm.param.common.assistMax)

// ������������������� 111
#define PARAM_SetZeroStartOrNot(a)			paramCB.nvm.param.protocol.driver.zeroStartOrNot=a
// ��ȡ���������������
#define PARAM_GetZeroStartOrNot()			(paramCB.nvm.param.protocol.driver.zeroStartOrNot)

// �����л�Ѳ���ķ�ʽ 111
#define PARAM_SetSwitchCruiseWay(a)			paramCB.nvm.param.protocol.driver.switchCruiseWay=a
// ��ȡ�л�Ѳ���ķ�ʽ
#define PARAM_GetSwitchCruiseWay()			(paramCB.nvm.param.protocol.driver.switchCruiseWay)

// ����Ѳ���л�ģʽ 111
#define PARAM_SetSwitchCruiseMode(a)		paramCB.nvm.param.protocol.driver.switchCruiseMode=a
// ��ȡѲ���л�ģʽ
#define PARAM_GetSwitchCruiseMode()			(paramCB.nvm.param.protocol.driver.switchCruiseMode)

// ����Ѳ��ʹ��
#define PARAM_SetCruiseEnabled(a)			paramCB.nvm.param.protocol.driver.CruiseEnabled=a
// ��ȡѲ��ʹ��
#define PARAM_GetCruiseEnabled()			(paramCB.nvm.param.protocol.driver.CruiseEnabled)


// ����һȦ�Ÿ����������ٴŸ� 111
#define PARAM_SetCycleOfSteelNum(a)			paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor=a
// ��ȡһȦ�Ÿ����������ٴŸ�
#define PARAM_GetCycleOfSteelNum()			(paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor)

// �����־�ID 111
#define PARAM_SetWheelSizeID(a)				paramCB.nvm.param.protocol.wheelSizeID=a
// ��ȡ�־�ID
#define PARAM_GetWheelSizeID()				(paramCB.nvm.param.protocol.wheelSizeID)

// �������������� 111
#define PARAM_SetAssistSensitivity(a)		paramCB.nvm.param.protocol.driver.assistSensitivity=a
// ��ȡ����������
#define PARAM_GetAssistSensitivity()		(paramCB.nvm.param.protocol.driver.assistSensitivity)

// ������������ǿ�� 111
#define PARAM_SetAssistStartIntensity(a)	paramCB.nvm.param.protocol.driver.assistStartIntensity=a
// ��ȡ��������ǿ��
#define PARAM_GetAssistStartIntensity()		(paramCB.nvm.param.protocol.driver.assistStartIntensity)

// �������ٵ����������Ÿ��� 11
#define PARAM_SetReversalHolzerSteelNum(a)	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum=a
// ��ȡ���ٵ����������Ÿ���
#define PARAM_GetReversalHolzerSteelNum()	(paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum)

// �����������ޣ���λ:0.1Km/h	 111
#define PARAM_SetSpeedLimit(a)				paramCB.nvm.param.common.speed.limitVal=a
// ��ȡ�������ޣ���λ:0.1Km/h	
//#define PARAM_GetSpeedLimit()				(paramCB.nvm.param.common.speed.limitVal)

// ���õ������ޣ���λ:mA 111
#define PARAM_SetCurrentLimit(a)			paramCB.nvm.param.protocol.driver.currentLimit=a
// ��ȡ�������ޣ���λ:mA
#define PARAM_GetCurrentLimit()				(paramCB.nvm.param.protocol.driver.currentLimit)

// ����Ƿѹ���ޣ���λ:mV 111
#define PARAM_SetLowVoltageThreshold(a)		paramCB.nvm.param.protocol.driver.lowVoltageThreshold=a
// ��ȡǷѹ���ޣ���λ:mV
#define PARAM_GetLowVoltageThreshold()		(paramCB.nvm.param.protocol.driver.lowVoltageThreshold)

// ���������Ÿ�����
#define PARAM_SetSteelType(a)				paramCB.nvm.param.protocol.driver.assistSteelType=a
// ��ȡ�����Ÿ�����
#define PARAM_GetSteelType()				(paramCB.nvm.param.protocol.driver.assistSteelType)

// �������ٿ��� 111
#define PARAM_SetSpeedLimitSwitch(a)		paramCB.nvm.param.protocol.driver.speedLimitSwitch=a
// ��ȡ���ٿ���
#define PARAM_GetSpeedLimitSwitch()			(paramCB.nvm.param.protocol.driver.speedLimitSwitch)

// ���õ�ص����仯ʱ�䣬��λ:s 111
#define PARAM_SetBatteryCapVaryTime(a)		paramCB.nvm.param.common.battery.filterLevel=a
// ��ȡ��ص����仯ʱ�䣬��λ:s
#define PARAM_GetBatteryCapVaryTime()		(paramCB.nvm.param.common.battery.filterLevel)

// ��������ͨѶ���ϳ�ʱʱ�䣬��λ:ms 111
#define PARAM_SetBusAliveTime(a)			paramCB.nvm.param.common.busAliveTime=a
// ��ȡ����ͨѶ���ϳ�ʱʱ�䣬��λ:ms
#define PARAM_GetBusAliveTime()				(paramCB.nvm.param.common.busAliveTime)

// �����ٶȱ仯ƽ����
#define PARAM_SetSpeedFilterLevel(a)		paramCB.nvm.param.common.speed.filterLevel=a
// ��ȡ�ٶȱ仯ƽ����
#define PARAM_GetSpeedFilterLevel()			(paramCB.nvm.param.common.speed.filterLevel)

// ���ûָ�����������
#define PARAM_SetResFactorySet(a)				paramCB.nvm.param.protocol.resFactorySet=a
// ��ȡ�ָ�����������
#define PARAM_GetResFactorySet()				(paramCB.nvm.param.protocol.resFactorySet)


// �����˶�״̬����ʱ�䣬��λ:s
#define PARAM_SetMoveTimeKeepTime(a)		paramCB.runtime.record.moveKeepTime=a
// ��ȡ�˶�״̬����ʱ�䣬��λ:s
#define PARAM_GetMoveTimeKeepTime()			(paramCB.runtime.record.moveKeepTime)

// ���þ�ֹ״̬����ʱ�䣬��λ:s
#define PARAM_SetStopTimeKeepTime(a)		paramCB.runtime.record.stopKeepTime=a
// ��ȡ��ֹ״̬����ʱ�䣬��λ:s
#define PARAM_GetStopTimeKeepTime()			(paramCB.runtime.record.stopKeepTime)


// ���ö�ʱ��ȡ������ʱ��
#define PARAM_SetParamCycleTime(ms)			paramCB.runtime.paramCycleTime=ms
// ��ȡ��ʱ��ȡ������ʱ��
#define PARAM_GetParamCycleTime()			(paramCB.runtime.paramCycleTime)

// �������м�¼����
#define PARAM_SetRecordCount(count)			paramCB.runtime.recordCount=count
// ��ȡ���м�¼����
#define PARAM_GetRecordCount()				(paramCB.runtime.recordCount)

// ���õ�ص�������λ:mA
#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
// ��ȡ��������λ:mA
#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)

// ���õ�������͸澯
#define PARAM_SetBatteryLowAlarm(a)			paramCB.runtime.battery.lowVoltageAlarm=a
// ��ȡ��������͸澯
#define PARAM_GetBatteryLowAlarm()			(paramCB.runtime.battery.lowVoltageAlarm)

// ���õ�λ, ������Ӣ�� 
#define PARAM_SetUnit(a)					paramCB.nvm.param.common.unit=a
// ��ȡ��λ, ������Ӣ�� 
#define PARAM_GetUnit()						(paramCB.nvm.param.common.unit)

// �����Ƿ�������
#define PARAM_SetExistBle(a)				paramCB.nvm.param.common.existBle=a
// ��ȡ�Ƿ�������
#define PARAM_GetExistBle()					(paramCB.nvm.param.common.existBle)

// ������ʾģʽ
#define PARAM_SetDisMode(a)					paramCB.runtime.disMode=a
// ��ȡ��ʾģʽ
#define PARAM_GetDisMode()					(paramCB.runtime.disMode)

// ���������
#define PARAM_SetTotalDistance(a)			paramCB.runtime.totalDistance=a
// ��ȡ�����
//#define PARAM_GetTotalDistance()			(paramCB.runtime.totalDistance)

// ���ñ������
#define PARAM_SetTripDistance(a)			paramCB.runtime.tripMileage=a
// ��ȡ�������
//#define PARAM_GetTripDistance()				(paramCB.runtime.tripMileage)

// ���ùػ�״̬
#define PARAM_SetShutDownState(a)			paramCB.runtime.shutDownState=a
// ��ȡ�ػ�״̬
#define PARAM_GetShutDownState()			(paramCB.runtime.shutDownState)

// ��������ADC
#define PARAM_SetThrottleAdc(a)				paramCB.runtime.throttleAdc=a
// ��ȡ����ADC
#define PARAM_GetThrottleAdc()				(paramCB.runtime.throttleAdc)

// ���õ���ɲ��ADC
#define PARAM_SetBreakAdc(a)				paramCB.runtime.breakAdc=a
// ��ȡ����ɲ��ADC
#define PARAM_GetBreakAdc()					(paramCB.runtime.breakAdc)

// ���÷�Χ��
#define PARAM_SetAtmosphereLight(a)			paramCB.runtime.atmosphereLight=a
// ��ȡ��Χ��
#define PARAM_GetAtmosphereLight()			(paramCB.runtime.atmosphereLight)

// ����ɲ������
#define PARAM_SetBreakType(a)				paramCB.nvm.param.protocol.driver.breakType=(a)
// ��ȡɲ������
#define PARAM_GetBreakType()				(paramCB.nvm.param.protocol.driver.breakType)


// ����ϵͳ����������
#define PARAM_SetWorkAssistRatio(a)			paramCB.nvm.param.common.workAssistRatio=a
// ��ȡϵͳ����������
#define PARAM_GetWorkAssistRatio()			(paramCB.nvm.param.common.workAssistRatio)

// ���õ������״̬
#define PARAM_SetMotorState(a)				paramCB.runtime.motorState=a
// ��ȡ�������״̬
#define PARAM_GetMotorState()				(paramCB.runtime.motorState)

// �������ݱ���״̬
#define PARAM_SetSaveDateState(a)			paramCB.runtime.saveDateState=a
// ��ȡ��������״̬
#define PARAM_GetSaveDateState()			(paramCB.runtime.saveDateState)

// �����¶�����
#define PARAM_SetTempData(a)				paramCB.runtime.tempData=a
// ��ȡ�¶�����
#define PARAM_GetTempData()					(paramCB.runtime.tempData)

// ���ÿ��������±�־
#define PARAM_SetControllerOvertemp(a)			paramCB.runtime.controllerOvertemp=a
// ��ȡ���������±�־
#define PARAM_GetControllerOvertemp()			(paramCB.runtime.controllerOvertemp)

// ���õ�����±�־
#define PARAM_SetmotorOvertemp(a)			paramCB.runtime.motorOvertemp=a
// ��ȡ������±�־
#define PARAM_GetmotorOvertemp()			(paramCB.runtime.motorOvertemp)

// ���õ�ع��±�־
#define PARAM_SetbatteryOvertemp(a)			paramCB.runtime.batteryOvertemp=a
// ��ȡ��ع��±�־
#define PARAM_GetbatteryOvertemp()			(paramCB.runtime.batteryOvertemp)

// ���÷�����
#define PARAM_SetBuzzer(a)			paramCB.runtime.buzzer=a
// ��ȡ������
#define PARAM_GetBuzzer()			(paramCB.runtime.buzzer)

// �����������յȼ�
#define PARAM_SetEnergyRecoveryLevel(a)			paramCB.runtime.energyRecoveryLevel=a
// ��ȡ�������յȼ�
#define PARAM_GetEnergyRecoveryLevel()			(paramCB.runtime.energyRecoveryLevel)

// ��ȡ�Ǳ���
#define PARAM_GetDevVersion()				(paramCB.runtime.devVersion)

// ��ȡ�̼��汾���
#define PARAM_GetFwBinNum()					(paramCB.runtime.fwBinNum)

// ��ȡ�̼��汾�ű���λ
#define PARAM_GetDevVersionRemain()			(paramCB.runtime.fwBinVersion.devVersionRemain)


// ��ȡ�̼������汾��
#define PARAM_Get_DevReleaseVersion()		(paramCB.runtime.fwBinVersion.devReleaseVersion)


// ��ȡ�̼����������汾��
#define PARAM_Get_FwUpdateVersion()			(paramCB.runtime.fwBinVersion.fwUpdateVersion)


// ��ȡ�̼��޸�bug�����汾��
#define PARAM_Get_FwDebugReleaseVersion()	(paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)


/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// ģ���ʼ�������ģ���ʼ��
void PARAM_Init(void);

// ģ����̴���
void PARAM_Process(void);

// NVM���£��������ָ���Ƿ�����д��
void NVM_SetDirtyFlag(BOOL writeAtOnce);

uint16 PARAM_GetPerimeter(uint8 wheelSizeID);

void NVM_Save(BOOL saveAll);

uint16 PARAM_GetSpeed(void);

// ���س�������
void PARAM_FactoryReset(void);

// ��ȡ����̽ӿ�
uint16 PARAM_GetTotalDistance(void);

// ��ȡ�ٶȽӿ�
uint16 PARAM_GetSpeed(void);
#endif









