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
/*******************************************************************************
 *                                  �궨��	                                   *
********************************************************************************/
#define PARAM_MCU_PAGE_NUM					58
#define PARAM_MCU_PAGE_SIZE					512UL

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

#define NVM_FORMAT_FLAG_STRING				"202410141210"
#define NVM_FORMAT_FLAG_SIZE				sizeof(NVM_FORMAT_FLAG_STRING)

// NVM������FLASH�е��׵�ַ
#define PARAM_NVM_START_ADDRESS				(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 2)*PARAM_MCU_PAGE_SIZE)

// NVM������FLASH�еı��õ�ַ
#define PARAM_NVM_BACKUP_START_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 3)*PARAM_MCU_PAGE_SIZE)

// �ָ��������õı�������
#define PARAM_NVM_FACTORY_RESET_ADDRESS		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM - 1)*PARAM_MCU_PAGE_SIZE + 512)

// �˵�����
#define PARAM_NVM_MENU_PASSWORD 							2020		

// ��������
#define PARAM_NVM_POWERON_PASSWORD 							2020

// ����Ա���룬��������
#define PARAM_NVM_ADMIN_PASSWORD 							1314


//// ��ʾ��λ����ö�ٶ���
//typedef enum
//{
//	UNIT_METRIC = 0,			// ����
//	UNIT_INCH,					// Ӣ��

//	UINT_COUNT	
//}UNIT_E;

//// �綯���г�����������ʾ��
//#define PARAM_E_BIKE_CODE						0x0E0B
//// �綯���峵����������ʾ��
//#define PARAM_E_HUA_CODE						0x0E05
//// ������ʾ�ַ�UP
//#define PARAM_E_UPDATA_CODE						0x1112


//#define METRIC_TO_INCH_COFF		(0.621371f)	// 1 km = 0.621371192237334 inch

//// ��ص�������Դ
//typedef enum
//{
//	//BATTERY_DATA_SRC_ADC = 0,				// ��ѹ���Ǳ����
//	
//	BATTERY_DATA_SRC_CONTROLLER,			// ��ѹ�ɿ������ϱ�,�Ǳ�������
//	BATTERY_DATA_SRC_BMS,					// ����ֱ���ɿ������ϱ�
//	BATTERY_DATA_SRC_ADC,					// �Ǳ����ѹ���Ǳ�������

//	BATTERY_DATA_SRC_COUNT
//}BATTERY_DATA_SRC_E;


//// ϵͳ״̬
//typedef enum
//{
//	SYSTEM_STATE_STOP = 0,

//	SYSTEM_STATE_RUNNING,

//	SYSTEM_STATE_MAX
//}SYSTEM_STATE;

//// ����ģʽ����
//typedef enum
//{
//	SMART_TYPE_S = 0,

//	SMART_TYPE_I,

//	SMART_TYPE_MAX
//}SMART_TYPE_E;

//// ����ģʽ
//typedef enum
//{
//	ASSIST_MODE_TORQUE = 0,

//	ASSIST_MODE_CADENCE,

//	ASSIST_MODE_MAX
//}ASSIST_MODE_E;

//// LED����ģʽ
//typedef enum
//{
//	PARAM_LED_MODE_ALL_OFF = 0,					// ����
//	PARAM_LED_MODE_ALL_ON = 1,					// ����
//	PARAM_LED_MODE_BLINK = 2,					// ��˸
//	PARAM_LED_MODE_BREATH = 3,					// ����
//	PARAM_LED_MODE_BREATH_50 = 4,				// ���� ����50%����
//	PARAM_LED_MODE_BREATH_20 = 5,				// ���� ����20%����

//	PARAM_LED_MODE_ALL_ON_50 = 6,				// ���� 50%����
//	PARAM_LED_MODE_ALL_ON_20 = 7,				// ���� 20%����

//	PARAM_LED_MODE_ALL_OFF_50 = 0x16,			// ���� 50%����(Ԥ��)
//	PARAM_LED_MODE_ALL_OFF_20 = 0x17,			// ���� 20%����(Ԥ��)

//	PARAM_LED_MODE_INVALID = 0xFF,				// ��Ч��ģʽ
//}PARAM_LED_MODE_E;

//// �����־�ID
//typedef enum{
//	//PARAM_WHEEL_SIZE_4_5_INCH = 0,
//	//PARAM_WHEEL_SIZE_6_INCH,
//	//PARAM_WHEEL_SIZE_6_4_INCH,
//	//PARAM_WHEEL_SIZE_8_INCH,
//	//PARAM_WHEEL_SIZE_10_INCH,
//	PARAM_WHEEL_SIZE_12_INCH,
//	PARAM_WHEEL_SIZE_14_INCH,
//	PARAM_WHEEL_SIZE_16_INCH,
//	PARAM_WHEEL_SIZE_18_INCH,
//	PARAM_WHEEL_SIZE_20_INCH,
//	PARAM_WHEEL_SIZE_22_INCH,
//	PARAM_WHEEL_SIZE_24_INCH,
//	PARAM_WHEEL_SIZE_26_INCH,
//	PARAM_WHEEL_SIZE_27_INCH,
//	PARAM_WHEEL_SIZE_27_5_INCH,
//	PARAM_WHEEL_SIZE_28_INCH,
//	PARAM_WHEEL_SIZE_29_INCH,
//	PARAM_WHEEL_SIZE_700C,
//	
//	PARAM_WHEEL_SIZE_MAX
//}PARAM_WHEEL_SIZE_ID;


//// ������λö�ٶ���
//typedef enum
//{
//	ASSIST_0 = 0,
//	ASSIST_1,
//	ASSIST_2,
//	ASSIST_3,
//	ASSIST_4,
//	ASSIST_5,
//	ASSIST_6,
//	ASSIST_7,
//	ASSIST_8,
//	ASSIST_9,
//	ASSIST_P,

//	ASSIST_S,	// ��������

//	ASSIST_ID_COUNT
//}ASSIST_ID_E;


//// ���������λ����
//typedef enum
//{
//	ASSISTMAX_0 = 0,
//	ASSISTMAX_1,
//	ASSISTMAX_2,
//	ASSISTMAX_3,
//	ASSISTMAX_4,
//	ASSISTMAX_5,
//	ASSISTMAX_6,
//	ASSISTMAX_7,
//	ASSISTMAX_8,
//	ASSISTMAX_9,
//	
//	ASSISTMAX,	

//}ASSIST_ID_E_MAX;

//// ��Ʒ���Ͷ���
//typedef enum
//{
//	HMI_TYPE_BC18EU = 0,				// ��̫���BC18��TFT����UARTͨѶ
//	HMI_TYPE_BC18SU,					// ��̫���BC18��TFT����BLE��UARTͨѶ
//	HMI_TYPE_BC28EU,					// ��̫���BC28��TFT����UARTͨѶ
//	HMI_TYPE_BC28SU,					// ��̫���BC28��TFT����BLE��UARTͨѶ
//	HMI_TYPE_BC28NU,					// ��̫���BC28��TFT����BLE��GSM��GPS��UARTͨѶ
//	HMI_TYPE_BC18EC,					// �������BC18��TFT����CANͨѶ
//	HMI_TYPE_BC28EC,					// �������BC28��TFT����CANͨѶ

//	
//	HMI_TYPE_BN400,						// ������Ǳ�BN400, �������ʾ, UARTͨѶ

//	HMI_TYPE_MAX,
//}HMI_TYPE_E;

//// ����ͨѶ��ƽ����
//typedef enum
//{
//	UART_LEVEL_3_3 = 0,				// ����ͨѶ��ƽ3.3V
//	UART_LEVEL_5_0,					// ����ͨѶ��ƽ5V

//	UART_LEVEL_MAX,
//}UART_LEVEL_TYPE_E;

//// ����ϵͳע������籣����NVM�С���
//typedef struct
//{
//	// ϵͳ����
//	struct
//	{
//		PARAM_WHEEL_SIZE_ID wheelSizeID;	// �־�ID
//		uint16 newWheelSizeInch;
//		uint16 showWheelSizeInch;			// ������Ϣ��ʾ�־�����ʵ��Э���·��޹�
//		uint16 newPerimeter;
//		
//		uint8 pwmTop[ASSIST_ID_COUNT]; 		// ��λPWMֵ����
//		uint8 pwmBottom[ASSIST_ID_COUNT];	// ��λPWMֵ����
//		uint16 powerMax;					// ���������,��λ:W
//		uint16 pushSpeedLimit;				// �Ƴ������������ޣ���λ:0.1Km/h

//		uint8 pwmFixed[ASSIST_ID_COUNT];	// PWM��λ�̶�ֵ

//		uint8 eBikeName;					// ����
//		uint16 motorSys;					// �������
//		uint16 batteryCap;					// ��ع���
//		uint8 tiresSize;					// �ֿ�
//		uint8 carModel[2];					// ����
//		uint16 menuPassword;				// ����˵���������
//		uint16 powerOnPassword;				// ������������
//		uint8 powerPasswordSwitch;			// �������뿪��
//		uint8 menuPasswordSwitch;			// �˵����뿪��	

//		BOOL beepSwitch;					// ����������
//		uint8 limitSpeedBeepOn;				// ���ٷ���������

//		BOOL resFactorySet;					// �ָ�����������

//    uint8 runProtocol;

//		// ���������ò���
//		struct
//		{
//			uint8 steelNumOfStartAssist;		// ������ʼ�ĴŸ���
//			uint8 steelNumOfSpeedSensor;		// ���Դ�����һȦ�Ÿ���(�� ���ٴŸ�)
//			BOOL assistDirectionForward;		// ��������
//			uint8 assistPercent;				// ��������
//			uint8 slowStart; 					// ����������
//			BOOL turnBarSpeed6kmphLimit;		// ת������6km/h
//			BOOL turnBarLevel;					// ת�ѷֵ�
//			uint32 currentLimit; 				// �������ޣ���λ:mA
//			uint16 lowVoltageThreshold;			// Ƿѹ���ޣ���λ:mV

//			uint8 controlMode;					// ����������ģʽ
//			BOOL zeroStartOrNot;				// ���������������
//			BOOL switchCruiseWay;				// �л�Ѳ���ķ�ʽ
//			BOOL switchCruiseMode;				// Ѳ���л�ģʽ
//			uint8 assistSensitivity;			// ����������
//			uint8 assistStartIntensity;			// ��������ǿ��
//			uint8 assistSteelType;				// �����Ÿ�����
//			uint8 reversalHolzerSteelNum;		// ���ٵ����������Ÿ���
//			uint8 turnBarSpeedLimit;			// ת������ֵ,��λ:Km/h
//			uint8 motorFuture;					// ������Բ���			
//			uint8 turnBarFunction;				// ת�ѹ���
//			uint8 motorPulseCode;				// �����λ����
//			uint8 assistSensorSignalNum;		// �����������ź���
//			uint8 assistTrim;					// ����΢��
//			uint8 absBrake;						// ABSɲ��ǿ��
//			uint8 hmiProperty;					// �Ǳ�����
//			BOOL assistSwitch;					// ��������
//			BOOL speedLimitSwitch;				// ����λ����
//			BOOL cruiseSwitch;					// ����Ѳ�����ܿ���
//			
//			uint8 assistRate[5];				// ��λ������
//			
//			ASSIST_MODE_E assistMode;			// ����ģʽ
//			BOOL assistModeEnable;				// ����ģʽ����

//			SMART_TYPE_E smartType;				// ���������������
//		}driver;
//	}protocol;

//	// ��������
//	struct
//	{
//		HMI_TYPE_E hmiType;					// �������
//			UNIT_E unit;					// ��λ, ������Ӣ�� 	
//		ASSIST_ID_E assistMax;				// ���������λ 
//		uint32 busAliveTime;				// ͨѶ���ϳ�ʱʱ�䣬��λ:ms
//		uint8  brightness;					// ��������
//		uint8  powerOffTime; 				// �Զ��ػ�ʱ�䣬��λ:����
//		uint8  workAssistRatio;				// ������������������

//		BOOL existBle;						// �Ƿ��������
//		uint8 uartLevel;					// ����ͨ�ŵ�ƽ
//		BATTERY_DATA_SRC_E percentageMethod;// �������㷽ʽ
//		uint8 uartProtocol;					// Э��ѡ�� 0��KM5S	1��﮵�2��		2���˷�	 3��JЭ��
//		uint8 pushSpeedSwitch;				// �Ƴ���������  0���Ƴ��������ܣ�1Ϊ���Ƴ���������
//		uint8 defaultAssist;				// Ĭ�ϵ�λ	ȡֵ��Χ0~9
//		uint8 logoMenu;						// LOGO��	0Ϊ��̫���棬1Ϊ���Խ��棬2Ϊ�ͻ�����

//		BOOL pushAssistSwitch; 				// �Ƴ�����
//		uint32 adRatioRef;
//		// �ٶ���
//		struct
//		{
//			uint16 limitVal;				// �������ޣ���λ:0.1Km/h
//			uint8  filterLevel;				// �ٶ�ƽ����
//		}speed;

//		// �����
//		struct
//		{
//			uint8 voltage;					// ��ع����ѹ����λ:V
//			uint8 filterLevel;				// �����仯ʱ�䣬��λ:��
//		}battery;
//		
//		struct
//		{
//			// �Գ���������ͳ������
//			struct
//			{
//				uint32 distance;			// ������������̣���λ:0.1Km				
//				uint32 preDistance;			// ������������̣���λ:0.1Km
//				uint16 maxTripOf24hrs;		// ���˼�¼����λ:0.1Km
//				uint32 ridingTime;			// ����ʱ�䣬��λ:s
//				uint32 calories;			// ���������ܿ�·���λ:KCal
//			}total;

//			// ��Trip����������ͳ������
//			struct
//			{
//				uint32 distance;			// Trip����λ:0.1Km
//				uint32 calories;			// ��·���λ:KCal
//				uint16 speedMax;			// ����ٶȣ���λ:0.1Km/h

//				uint32 ridingTime;			// ������ʱ����
//			}trip;

//			// ��0��������ͳ������
//			struct
//			{
//				uint16 trip;				// ������̣���λ:0.1Km 	
//				uint16 calories;			// ���տ�·���λ:KCal

//				uint32 ridingTime;			// ���������ۼ�ʱ�䣬��λ:s

//				uint16 year;				// ���ղ�����Ӧ������
//				uint8 month; 				// ���ղ�����Ӧ������
//				uint8 day;					// ���ղ�����Ӧ������
//			}today;
//		}record;

//		// �ŵ���󣬿��Ա�ֻ֤��ǰ���ֵ��д��֮��ſ�ʼд��ʽ����ʶ�ַ���
//		uint8 nvmFormat[NVM_FORMAT_FLAG_SIZE];			// NVM��ʽ����ʶ
//	}common;	
//}NVM_CB;

//// NVM���ߴ�
//#define PARAM_NVM_DATA_SIZE						sizeof(NVM_CB)

//// �������й��������ݣ����粻�������
//typedef struct
//{
//	uint32 flagArr[64];
//	BOOL leftTurnSwitch;
//	BOOL rightTurnSwitch;
//	
//	uint8 blueUpgradePer;
//	
//	// DT SN��
//	struct
//	{
//		uint8 snLenth;
//		uint8 snStr[30+1];
//	}SnCode;

//	// LIME SN��
//	struct
//	{
//		uint8 LimeSnLenth;
//		uint8 LimeSnStr[30+1];
//	}LimeSnCode;

//	// Ӳ���汾��
//	struct
//	{
//		uint8 hwVersionLenth;
//		uint8 hwVersion[30+1];
//	}hwVersion;

//	// BOOT�汾��
//	struct
//	{
//		uint8 bootVersionLenth;
//		uint8 bootVersion[30+1];
//	}bootVersion;

//	// APP�汾��
//	struct
//	{
//		uint8 appVersionLenth;
//		uint8 appVersion[30+1];
//	}appVersion;
//	
//	// �Ǳ�Ψһ���к�
//	volatile struct
//	{
//		uint32 sn0;
//		uint32 sn1;
//		uint32 sn2;
//		uint8 snStr[24+1];
//	}watchUid;	

//	// ���Ա�־
//	uint32 testflag[16];

//	uint32 ageTestFlag;
//	uint8  pucTestFlag;
//	uint32 segContrl;

//	// ��ز���
//	struct
//	{
//		uint16 voltage;							// ��ǰ��ص�ѹ����λ:mV
//		uint16 current; 						// ��ǰ��ص�������λ:mA
//		uint8 lowVoltageAlarm;					// ��������͸澯
//		uint8 percent;							// ��ذٷֱ�
//		BATTERY_DATA_SRC_E batteryDataSrc;		// �����������Դ
//		uint8 state;							// ���״̬
//		uint8 preState;
//		uint8 powerOffstate;
//	}battery;

//	struct
//	{
//		BOOL  ageTestEnable;			 
//		uint32 txCount;
//		uint32 rxCount;
//		uint8 ageTestResoult;
//	}ageTest;
//	
//	struct
//	{
//		uint8 keyNowValue;
//	}keyValue;

//	// �������в��������������ʾ�빫ʽ����
//	// ʵʱʱ��
//	TIME_E rtc;
//	BOOL cruiseEnabe;				// Ѳ�����أ�0:��ʾ�ر�;1:��ʾ�� 
//	BOOL cruiseState;				// ��ǰѲ��״̬��0:��ֹ;1:��ʾ����Ѳ��״̬ 
//	BOOL pushAssistOn; 				// 6km/h���й��ܣ�0:��ʾ�ر�;1:��ʾ��
//	BOOL PushAssistState;			// ��ǰ6km/h�Ƴ�����״̬
//	BOOL isHavePushAssist;			// �Ƿ������й���
//	BOOL lightSwitch; 				// ��ƿ��أ�0:��ʾ�ر�;1:��ʾ�� 

//	uint16 speed;					// �ٶȣ���λ:0.1Km/h
//	uint8 cadence;					// ̤Ƶ����λ:rpm
//	uint32 pulseNum; 				// �������
//	uint8 pwm;						// ���͸���������pwmֵ

//	uint8 errorType; 		// ��ǰ�������
//	SYSTEM_STATE ridingState; 		// ��ǰϵͳ״̬
//	BOOL overSpeedAlarm;			// ���ٸ澯��TRUE:��ʾ���� 	
//	BOOL underVoltageAlarm;			// Ƿѹ�澯��0:��ʾ��Ƿѹ��1:��ʾǷѹ

//	uint8 torque;					// ����,��λ:N.m
//	uint16 paramCycleTime;			// ��ʱ��ȡ������ʱ�䣬�����ٶ�

//	ASSIST_ID_E assist;				// ������λ 
//	uint16 recordCount;				// ����ͳ����������
//	uint8 ringStatus;				// ��������
//	uint8 warningStatus;			// ���/����

//	uint16 power;					// �������,��λ:W
//	uint16 perimeter[PARAM_WHEEL_SIZE_MAX];	// �����ܳ�����λ:mm

//	uint8 motorTemperature;			// ��������¶�
//	BOOL bleConnectState;			// �����Ƿ����ӵ�״̬

//	struct
//	{
//		uint16 speedAverage;		// ��Trip����������ƽ���ٶȣ���λ:0.1Km/h
//	}trip;

//	struct
//	{
//		uint16 moveKeepTime;		// �˶�����ʱ��
//		uint16 stopKeepTime;		// ��ֹ����ʱ��
//		uint16 userMaxSpeed;		// ���û���������������ٶ�
//	}record;
//	
//	uint8 devVersion[3];				// �Ǳ��ͺ�
//	uint8 fwBinNum;					// �̼��汾���
//	struct
//	{
//		uint8 devVersionRemain;				// �豸�����汾��(����汾��)
//		uint8 devReleaseVersion;			// ����������ķ����汾��(��Ʒ����汾��)
//		uint8 fwUpdateVersion;				// ���Է����汾��(�����汾��)
//		uint8 fwDebugReleaseVersion;		// ������ʱ�汾��
//	}fwBinVersion;
//	
//	struct
//	{
//		uint8 devVersion[3];				// �ͺ�
//		uint16 Version;						// �̼��汾��
//		uint16 fwBinNum;					// �̼��汾���
//	}ColAInfo;

//	struct
//	{
//		uint8 devVersion[3];				// �ͺ�
//		uint16 Version;						// �̼��汾��
//		uint16 fwBinNum;					// �̼��汾���
//	}ColBInfo;
//	// 
//	struct
//	{
//		uint8 devVersion[3];				// �ͺ�
//		uint16 Version;						// �̼��汾��
//		uint16 fwBinNum;					// �̼��汾���
//	}BMSInfo;

//	uint32 blueNumber;				// ��������
//	
//	uint32 uiVersion;				// UI�汾
//	uint32 btCapacity;				// �������
//	uint8 bleRadio;
//	uint32 fwVer;				// ����汾
//	uint32 hwVer;				// Ӳ���汾
//}RUNTIME_DATA_CB;

///*******************************************************************************
// *                                  �ṹ��	                                   *
//********************************************************************************/
//// ���� ������в��� : ��Ϊ������:����NVM�벻����NVM ����
//typedef struct
//{
//	RUNTIME_DATA_CB runtime;

//	union
//	{
//  		NVM_CB param;

//		uint8 array[PARAM_NVM_DATA_SIZE];
//	}nvm;
//	uint8 preValue[PARAM_NVM_DATA_SIZE];

//	BOOL nvmWriteRequest;			// NVM��������
//	BOOL nvmWriteEnable;			// NVM����ʹ��

//	// ���������־
//	BOOL recaculateRequest;		// �������������־��TRUE:������㣻FALSE:����Ҫ����
//}PARAM_CB;

//#define PARAM_SIZE	(sizeof(PARAM_CB))

//extern PARAM_CB paramCB;		
//extern const uint32 PARAM_MCU_VERSION_ADDRESS[];

///******************************************************************************
//* ���ⲿ�ӿں꡿
//******************************************************************************/
//// ���ò��������־
//#define PARAM_SetRecaculateRequest(a)		paramCB.recaculateRequest=a
//	
//	
//// ����Ѳ�����أ�0:��ʾ�رգ�1:��ʾ��
//#define PARAM_SetCruiseSwitch(enable)		paramCB.runtime.cruiseEnabe=enable
//// ��ȡѲ�����أ�0:��ʾ�رգ�1:��ʾ��
//#define PARAM_GetCruiseSwitch()				(paramCB.runtime.cruiseEnabe)
//	
//	
//// ����Ѳ��״̬��0:��ֹ��1:����Ѳ��
//#define PARAM_SetCruiseState(enable)		paramCB.runtime.cruiseState=enable
//// ��ȡѲ��״̬��0:��ֹ��1:����Ѳ��
//#define PARAM_GetCruiseState()				(paramCB.runtime.cruiseState)
//	
//	
//// �򿪻�ر��Ƴ�����
//#define PARAM_EnablePushAssist(enable)		do{\
//													paramCB.runtime.pushAssistOn=enable;\
//												}while(0)
//// ��ȡ�Ƴ���������״̬
//#define PARAM_IsPushAssistOn()				(paramCB.runtime.pushAssistOn) 
//	
//	
//// ����6km�Ƴ�����״̬
//#define PARAM_SetPushAssistState(a)			paramCB.runtime.PushAssistState=a
//// ��ȡ6km�Ƴ�����״̬
//#define PARAM_GetPushAssistState()			(paramCB.runtime.PushAssistState)
//	
//	
//	
//// �����Ƿ����Ƴ���������
//#define PARAM_SetIsHavePushAssist(a)		paramCB.runtime.isHavePushAssist=a
//// ��ȡ�Ƿ����Ƴ���������
//#define PARAM_GetIsHavePushAssist()			(paramCB.runtime.isHavePushAssist)
//	
//	
//// ���ô�ƿ��� 
//#define PARAM_SetLightSwitch(enable)		do{\
//													paramCB.runtime.lightSwitch=enable;\
//												}while(0)
//// ��ȡ��ƿ���
//#define PARAM_GetLightSwitch()				(paramCB.runtime.lightSwitch)
//	
//#define PARAM_SetTxCount(a)						paramCB.runtime.ageTest.txCount = a
//#define PARAM_GetTxCount()						(paramCB.runtime.ageTest.txCount)

//#define PARAM_SetRxCount(a)						paramCB.runtime.ageTest.rxCount = a
//#define PARAM_GetRxCount()						(paramCB.runtime.ageTest.rxCount)


//// ��ȡ��ת��ƿ���
//#define PARAM_GetLeftTurntSwitch()				(paramCB.runtime.leftTurnSwitch)

//// ��ȡ��ת��ƿ���
//#define PARAM_GetRightTurnSwitch()				(paramCB.runtime.rightTurnSwitch)

//											// ������ת��ƿ���
//#define PARAM_SetLeftTurntSwitch(enable)		paramCB.runtime.leftTurnSwitch=enable
//																								
//// ������ת��ƿ���
//#define PARAM_SetRightTurnSwitch(enable)	paramCB.runtime.rightTurnSwitch=enable		
//																
//								


//// �Ƴ�����												
//#define PARAM_SetPushAssistSwitch(a)			paramCB.nvm.param.common.pushAssistSwitch=a;
//#define PARAM_GetPushAssistSwitch()				(paramCB.nvm.param.common.pushAssistSwitch);



//// ���õ�ص�ѹ����λ:mV
//#define PARAM_SetBatteryVoltage(mV)			paramCB.runtime.battery.voltage=mV
//// ��ȡ��ص�ѹ����λ:mV
//#define PARAM_GetBatteryVoltage()			(paramCB.runtime.battery.voltage)
//	
//// ���õ�ص�������λ:mA
//#define PARAM_SetBatteryCurrent(mA)			paramCB.runtime.battery.current=mA
//// ��ȡ��������λ:mA
//#define PARAM_GetBatteryCurrent()			(paramCB.runtime.battery.current)
//	
//// �����ٶȣ���λ:0.1Km/h
//#define PARAM_SetSpeed(a)					paramCB.runtime.speed=a
//// ��ȡ�ٶȣ���λ:0.1Km/h
////#define PARAM_GetSpeed()					(paramCB.runtime.speed)
//	
//// ����̤Ƶ
//#define PARAM_SetCadence(a)					paramCB.runtime.cadence=a
//// ��ȡ̤Ƶ
//#define PARAM_GetCadence()					(paramCB.runtime.cadence)
//	
//// �����������
//#define PARAM_SetPulseNum(a)				paramCB.runtime.pulseNum=a
//// ��ȡ�������
//#define PARAM_GetPulseNum()					(paramCB.runtime.pulseNum)
//	
//// ���ô������
//#define PARAM_SetErrorCode(a)				paramCB.runtime.errorType=a
//// ��ȡ�������
//#define PARAM_GetErrorCode()				(paramCB.runtime.errorType)
//	
//// ��������״̬��0:��ʾ��ֹ��1:��ʾ����״̬
//#define PARAM_SetRidingState(a)				paramCB.runtime.ridingState=a
//// ��ȡ����״̬��0:��ʾ��ֹ��1:��ʾ����״̬
//#define PARAM_GetRidingState()				(paramCB.runtime.ridingState)
//	
//// ���ó��ٸ澯��1:��ʾ����
//#define PARAM_SetOverSpeedAlarm(a)			paramCB.runtime.overSpeedAlarm=a
//// ��ȡ���ٸ澯��1:��ʾ����
//#define PARAM_GetOverSpeedAlarm()			(paramCB.runtime.overSpeedAlarm)
//	
//// ��������
//#define PARAM_SetTorque(a)					paramCB.runtime.torque=a
//// ��ȡ����
//#define PARAM_GetTorque()					(paramCB.runtime.torque)
//	
//// ����Ƿѹ�澯��0:��ʾ��Ƿѹ��1:��ʾǷѹ
//#define PARAM_SetUnderVoltageAlarm(a)		paramCB.runtime.underVoltageAlarm=a
//// ��ȡǷѹ�澯��0:��ʾ��Ƿѹ��1:��ʾǷѹ
//#define PARAM_GetUnderVoltageAlarm()		(paramCB.runtime.underVoltageAlarm)
//	
//// ���õ�������͸澯
//#define PARAM_SetBatteryLowAlarm(a)			paramCB.runtime.battery.lowVoltageAlarm=a
//// ��ȡ��������͸澯
//#define PARAM_GetBatteryLowAlarm()			(paramCB.runtime.battery.lowVoltageAlarm)
//	
//// ���ö�ʱ��ȡ������ʱ��
//#define PARAM_SetParamCycleTime(ms)			paramCB.runtime.paramCycleTime=ms
//// ��ȡ��ʱ��ȡ������ʱ��
//#define PARAM_GetParamCycleTime()			(paramCB.runtime.paramCycleTime)
//	
//// ��������ʱ��
//#define PARAM_SetRTC(YY,MM,DD,hh,mm,ss)		do{\
//													paramCB.runtime.rtc.year=YY;\
//													paramCB.runtime.rtc.month=MM;\
//													paramCB.runtime.rtc.day=DD;\
//													paramCB.runtime.rtc.hour=hh;\
//													paramCB.runtime.rtc.minute=mm;\
//													paramCB.runtime.rtc.second=ss;\
//												}while(0)
//// ����ϵͳʱ����
//#define PARAM_SetRTCSecond(a)				paramCB.runtime.rtc.second=a
//// ��ȡϵͳʱ����
//#define PARAM_GetRTCSecond()				(paramCB.runtime.rtc.second)
//	
//// ����ϵͳʱ���
//#define PARAM_SetRTCMin(a)					paramCB.runtime.rtc.minute=a
//// ��ȡϵͳʱ���
//#define PARAM_GetRTCMin()					(paramCB.runtime.rtc.minute)
//	
//// ����ϵͳʱ��ʱ
//#define PARAM_SetRTCHour(a)					paramCB.runtime.rtc.hour=a
//// ��ȡϵͳʱ��ʱ
//#define PARAM_GetRTCHour()					(paramCB.runtime.rtc.hour)
//	
//// ����ϵͳʱ����
//#define PARAM_SetRTCDay(a)					paramCB.runtime.rtc.day=a
//// ��ȡϵͳʱ����
//#define PARAM_GetRTCDay()					(paramCB.runtime.rtc.day)
//	
//// ����ϵͳʱ����
//#define PARAM_SetRTCMonth(a)				paramCB.runtime.rtc.month=a
//// ��ȡϵͳʱ����
//#define PARAM_GetRTCMonth()					(paramCB.runtime.rtc.month)
//	
//// ����ϵͳʱ����
//#define PARAM_SetRTCYear(a)					paramCB.runtime.rtc.year=a
//// ��ȡϵͳʱ����
//#define PARAM_GetRTCYear()					(paramCB.runtime.rtc.year)
//	
//// ���õ�ذٷֱ�
//#define PARAM_SetBatteryPercent(a)			paramCB.runtime.battery.percent=a
//// ��ȡ��ذٷֱ�
//#define PARAM_GetBatteryPercent()			(paramCB.runtime.battery.percent)	
//	
//// ���õ�ص�������Դ
//#define PARAM_SetBatteryDataSrc(a)			paramCB.runtime.battery.batteryDataSrc=a
//// ��ȡ��ص�������Դ
//#define PARAM_GetBatteryDataSrc()			(paramCB.runtime.battery.batteryDataSrc)	
//	
//#define PARAM_SetPercentageMethod(a)		paramCB.nvm.param.common.percentageMethod=a
//#define PARAM_GetPercentageMethod()			(paramCB.nvm.param.common.percentageMethod)

//// ����������λ
//#define PARAM_SetAssistLevel(a)				do{\
//													paramCB.runtime.assist=a;\
//												}while(0)
//// ��ȡ������λ
////#define PARAM_GetAssistLevel()			(paramCB.runtime.assist)
//	
//// �������м�¼����
//#define PARAM_SetRecordCount(count)			paramCB.runtime.recordCount=count
//// ��ȡ���м�¼����
//#define PARAM_GetRecordCount()				(paramCB.runtime.recordCount)
//	
//// ������������
//#define PARAM_SetRingStatus(a)				paramCB.runtime.ringStatus=a
//// ��ȡ��������
//#define PARAM_GetRingStatus()				(paramCB.runtime.ringStatus)
//	
//// �������/����
//#define PARAM_SetWarningStatus(a)			paramCB.runtime.warningStatus=a
//// ��ȡ���/����
//#define PARAM_GetWarningStatus()			(paramCB.runtime.warningStatus)	
//	
//// ���õ������,��λ:W
//#define PARAM_SetPower(a)					paramCB.runtime.power=a
//// ��ȡ�������,��λ:W
//#define PARAM_GetPower()					(paramCB.runtime.power)	

//// ���ö���Ѳ�����ܿ���
//#define PARAM_SetCruiseEnableSwitch(a)		paramCB.nvm.param.protocol.driver.cruiseSwitch=a
//// ��ȡ����Ѳ�����ܿ���
//#define PARAM_GetCruiseEnableSwitch()		paramCB.nvm.param.protocol.driver.cruiseSwitch

//// �������������,��λ:W
//#define PARAM_SetPowerMax(a)				paramCB.nvm.param.protocol.powerMax=a
//// ��ȡ���������,��λ:W
//#define PARAM_GetPowerMax()					(paramCB.nvm.param.protocol.powerMax)
//	
//// ���õ�ص�ѹ�ȼ�,��λ:V
//#define PARAM_SetBatteryVoltageLevel(a)		paramCB.nvm.param.common.battery.voltage=a
//// ��ȡ��ص�ѹ�ȼ�,��λ:V
//#define PARAM_GetBatteryVoltageLevel()		(paramCB.nvm.param.common.battery.voltage)
//	
//// ����������ʼ�ĴŸ���
//#define PARAM_SetAssitStartOfSteelNum(a)	paramCB.nvm.param.protocol.driver.steelNumOfStartAssist=a
//// ��ȡ������ʼ�ĴŸ���
//#define PARAM_GetAssitStartOfSteelNum()		(paramCB.nvm.param.protocol.driver.steelNumOfStartAssist)
//	
//// ����һȦ�Ÿ����������ٴŸ�
//#define PARAM_SetCycleOfSteelNum(a)			paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor=a
//// ��ȡһȦ�Ÿ����������ٴŸ�
//#define PARAM_GetCycleOfSteelNum()			(paramCB.nvm.param.protocol.driver.steelNumOfSpeedSensor)
//	
//// ������������
//#define PARAM_SetAssistDirection(a)			paramCB.nvm.param.protocol.driver.assistDirectionForward=a
//// ��ȡ��������
//#define PARAM_GetAssistDirection()			(paramCB.nvm.param.protocol.driver.assistDirectionForward)
//	
//// ���õ����������
//#define PARAM_SetAssistPercent(a)			paramCB.nvm.param.protocol.driver.assistPercent=a
//// ��ȡ�����������
//#define PARAM_GetAssistPercent()			(paramCB.nvm.param.protocol.driver.assistPercent)
//	
//// ���û���������
//#define PARAM_SetSlowStart(a)				paramCB.nvm.param.protocol.driver.slowStart=a
//// ��ȡ����������
//#define PARAM_GetSlowStart()				(paramCB.nvm.param.protocol.driver.slowStart)
//	
//// ����ת������6km/h
//#define PARAM_SetTurnBarSpeed6kmphLimit(a)	paramCB.nvm.param.protocol.driver.turnBarSpeed6kmphLimit=a
//// ��ȡת������6km/h
//#define PARAM_GetTurnBarSpeed6kmphLimit()	(paramCB.nvm.param.protocol.driver.turnBarSpeed6kmphLimit)
//	
//// ����ת�ѷֵ�
//#define PARAM_SetTurnbarLevel(a)			paramCB.nvm.param.protocol.driver.turnBarLevel=a
//// ��ȡת�ѷֵ�
//#define PARAM_GetTurnbarLevel()				(paramCB.nvm.param.protocol.driver.turnBarLevel)
//	
//// ����ת������
//#define PARAM_SetTurnBarSpeedLimit(a)		paramCB.nvm.param.protocol.driver.turnBarSpeedLimit=a
//// ��ȡת������
//#define PARAM_GetTurnBarSpeedLimit()		(paramCB.nvm.param.protocol.driver.turnBarSpeedLimit)
//	
//// ���õ������ޣ���λ:mA
//#define PARAM_SetCurrentLimit(a)			paramCB.nvm.param.protocol.driver.currentLimit=a
//// ��ȡ�������ޣ���λ:mA
//#define PARAM_GetCurrentLimit()				(paramCB.nvm.param.protocol.driver.currentLimit)
//	
//// ����Ƿѹ���ޣ���λ:mV
//#define PARAM_SetLowVoltageThreshold(a)		paramCB.nvm.param.protocol.driver.lowVoltageThreshold=a
//// ��ȡǷѹ���ޣ���λ:mV
//#define PARAM_GetLowVoltageThreshold()		(paramCB.nvm.param.protocol.driver.lowVoltageThreshold)
//	
//// ��������������ģʽ
//#define PARAM_SetDriverControlMode(a)		paramCB.nvm.param.protocol.driver.controlMode=a
//// ��ȡ����������ģʽ
//#define PARAM_GetDriverControlMode()		(paramCB.nvm.param.protocol.driver.controlMode)

//// ���õ�����Բ���
//#define PARAM_SetMotorFuture(a)				paramCB.nvm.param.protocol.driver.motorFuture=a
//// ��ȡ������Բ���
//#define PARAM_GetMotorFuture()				(paramCB.nvm.param.protocol.driver.motorFuture)
//	
//// �������������������
//#define PARAM_SetZeroStartOrNot(a)			paramCB.nvm.param.protocol.driver.zeroStartOrNot=a
//// ��ȡ���������������
//#define PARAM_GetZeroStartOrNot()			(paramCB.nvm.param.protocol.driver.zeroStartOrNot)
//	
//// �����л�Ѳ���ķ�ʽ
//#define PARAM_SetSwitchCruiseWay(a)			paramCB.nvm.param.protocol.driver.switchCruiseWay=a
//// ��ȡ�л�Ѳ���ķ�ʽ
//#define PARAM_GetSwitchCruiseWay()			(paramCB.nvm.param.protocol.driver.switchCruiseWay)
//	
//// ����Ѳ���л�ģʽ
//#define PARAM_SetSwitchCruiseMode(a)		paramCB.nvm.param.protocol.driver.switchCruiseMode=a
//// ��ȡѲ���л�ģʽ
//#define PARAM_GetSwitchCruiseMode()			(paramCB.nvm.param.protocol.driver.switchCruiseMode)
//	
//// ��������������
//#define PARAM_SetAssistSensitivity(a)		paramCB.nvm.param.protocol.driver.assistSensitivity=a
//// ��ȡ����������
//#define PARAM_GetAssistSensitivity()		(paramCB.nvm.param.protocol.driver.assistSensitivity)
//	
//// ������������ǿ��
//#define PARAM_SetAssistStartIntensity(a)	paramCB.nvm.param.protocol.driver.assistStartIntensity=a
//// ��ȡ��������ǿ��
//#define PARAM_GetAssistStartIntensity()		(paramCB.nvm.param.protocol.driver.assistStartIntensity)
//	
//// ���������Ÿ�����
//#define PARAM_SetSteelType(a)				paramCB.nvm.param.protocol.driver.assistSteelType=a
//// ��ȡ�����Ÿ�����
//#define PARAM_GetSteelType()				(paramCB.nvm.param.protocol.driver.assistSteelType)
//	
//// �������ٵ����������Ÿ���
//#define PARAM_SetReversalHolzerSteelNum(a)	paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum=a
//// ��ȡ���ٵ����������Ÿ���
//#define PARAM_GetReversalHolzerSteelNum()	(paramCB.nvm.param.protocol.driver.reversalHolzerSteelNum)
//	
//// ����ת�ѹ���
//#define PARAM_SetTurnBarFunction(a)			paramCB.nvm.param.protocol.driver.turnBarFunction=a
//// ��ȡת�ѹ���
//#define PARAM_GetTurnBarFunction()			(paramCB.nvm.param.protocol.driver.turnBarFunction)
//	
//// ���õ����λ����
//#define PARAM_SetMotorPulseCode(a)			paramCB.nvm.param.protocol.driver.motorPulseCode=a
//// ��ȡ�����λ����
//#define PARAM_GetMotorPulseCode()			(paramCB.nvm.param.protocol.driver.motorPulseCode)
//	
//// ���������������ź���
//#define PARAM_SetAssistSensorSignalNum(a)	paramCB.nvm.param.protocol.driver.assistSensorSignalNum=a
//// ��ȡ�����������ź���
//#define PARAM_GetAssistSensorSignalNum()	(paramCB.nvm.param.protocol.driver.assistSensorSignalNum)
//	
//// ��������΢��
//#define PARAM_SetAssistTrim(a)				paramCB.nvm.param.protocol.driver.assistTrim=a
//// ��ȡ����΢��
//#define PARAM_GetAssistTrim()				(paramCB.nvm.param.protocol.driver.assistTrim)
//	
//// ����absɲ��ǿ��
//#define PARAM_SetAbsBrake(a)				paramCB.nvm.param.protocol.driver.absBrake=a
//// ��ȡabsɲ��ǿ��
//#define PARAM_GetAbsBrake()					(paramCB.nvm.param.protocol.driver.absBrake)
//	
//// �����Ǳ�����
//#define PARAM_SetHmiProperty(a)				paramCB.nvm.param.protocol.driver.hmiProperty=a
//// ��ȡ�Ǳ�����
//#define PARAM_GetHmiProperty()				(paramCB.nvm.param.protocol.driver.hmiProperty)
//	
//// ������������
//#define PARAM_SetAssistSwitch(a)			paramCB.nvm.param.protocol.driver.assistSwitch=a
//// ��ȡ��������
//#define PARAM_GetAssistSwitch()				(paramCB.nvm.param.protocol.driver.assistSwitch)
//	
//// �������ٿ���
//#define PARAM_SetSpeedLimitSwitch(a)		paramCB.nvm.param.protocol.driver.speedLimitSwitch=a
//// ��ȡ���ٿ���
//#define PARAM_GetSpeedLimitSwitch()			(paramCB.nvm.param.protocol.driver.speedLimitSwitch)
//	
//// �����������ޣ���λ:0.1Km/h	
//#define PARAM_SetSpeedLimit(a)				paramCB.nvm.param.common.speed.limitVal=a
//// ��ȡ�������ޣ���λ:0.1Km/h	
////#define PARAM_GetSpeedLimit() 			(paramCB.nvm.param.common.speed.limitVal)
//	
//// �����Ƴ������������ޣ���λ:0.1Km/h	
//#define PARAM_SetPushSpeedLimit(a)			paramCB.nvm.param.protocol.pushSpeedLimit=a
//// ��ȡ�Ƴ������������ޣ���λ:0.1Km/h	
////#define PARAM_GetPushSpeedLimit() 		(paramCB.nvm.param.protocol.pushSpeedLimit)
//	
//// �����־�ID
//#define PARAM_SetWheelSizeID(a)				paramCB.nvm.param.protocol.wheelSizeID=a
//// ��ȡ�־�ID
//#define PARAM_GetWheelSizeID()				(paramCB.nvm.param.protocol.wheelSizeID)

//// ���÷���������
//#define PARAM_SetBeepSwitch(a)				paramCB.nvm.param.protocol.beepSwitch=a
//// ��ȡ����������
//#define PARAM_GetBeepSwitch()				(paramCB.nvm.param.protocol.beepSwitch)

//// ���ø��ٷ���������
//#define PARAM_SetLimitSpeedBeep(a)				paramCB.nvm.param.protocol.limitSpeedBeepOn=a
//// ��ȡ���ٷ���������
//#define PARAM_GetLimitSpeedBeep()				(paramCB.nvm.param.protocol.limitSpeedBeepOn)

//// ���ûָ�����������
//#define PARAM_SetResFactorySet(a)				paramCB.nvm.param.protocol.resFactorySet=a
//// ��ȡ�ָ�����������
//#define PARAM_GetResFactorySet()				(paramCB.nvm.param.protocol.resFactorySet)

//#define PARAM_SetShowWheelSizeInch(a)			paramCB.nvm.param.protocol.showWheelSizeInch=a
//#define PARAM_GetShowWheelSizeInch()			(paramCB.nvm.param.protocol.showWheelSizeInch)
//#define PARAM_SetNewWheelSizeInch(a)			paramCB.nvm.param.protocol.newWheelSizeInch=a
//#define PARAM_GetNewWheelSizeInch()				(paramCB.nvm.param.protocol.newWheelSizeInch)

//#define PARAM_SetNewperimeter(a)				paramCB.nvm.param.protocol.newPerimeter=a
//#define PARAM_GetNewperimeter()					(paramCB.nvm.param.protocol.newPerimeter)

//#define PARAM_SetRunProtocol(a)					paramCB.nvm.param.protocol.runProtocol=a
//#define PARAM_GetRunProtocol()					(paramCB.nvm.param.protocol.runProtocol)

//	
//// ����������ʱ����
//#define PARAM_SetTripRidingTime(a)			paramCB.nvm.param.common.record.trip.ridingTime=a
//// ��ȡ������ʱ����
//#define PARAM_GetTripRidingTime()			(paramCB.nvm.param.common.record.trip.ridingTime)
//	
//// ����Trip����λ:0.1Km
//#define PARAM_SetTrip(a)					paramCB.nvm.param.common.record.trip.distance=a
//// ��ȡTrip����λ:0.1Km
////#define PARAM_GetTrip()					(paramCB.nvm.param.common.record.trip.distance)
//	
//// ���ø��˼�¼����λ:0.1Km
//#define PARAM_SetPersonalBestTrip(a)		paramCB.nvm.param.common.record.total.maxTripOf24hrs=a
//// ��ȡ���˼�¼����λ:0.1Km
////#define PARAM_GetPersonalBestTrip()		(paramCB.nvm.param.common.record.total.maxTripOf24hrs)
//		
//// ���ÿ�·���λ:KCal
//#define PARAM_SetTripCalories(a)			paramCB.nvm.param.common.record.trip.calories=a
//// ��ȡ��·���λ:KCal
//#define PARAM_GetTripCalories()				(paramCB.nvm.param.common.record.trip.calories)

//// ��������̣���λ:0.1Km
//#define PARAM_SetTotalDistance(a)			paramCB.nvm.param.common.record.total.distance=a
//// ��ȡ����̣���λ:0.1Km
////#define PARAM_GetTotalDistance()			(paramCB.nvm.param.common.record.total.distance)
//	
//// ��ȡ����������������ʱ�䣬��λ:s
//#define PARAM_GetTotalRidingTime()			(paramCB.nvm.param.common.record.total.ridingTime)
//// ��ȡ�����������ܿ�·���λ:KCal
//#define PARAM_GetTotalCalories()			(paramCB.nvm.param.common.record.total.calories)
//	
//// ����ƽ���ٶȣ���λ:0.1Km/h
//#define PARAM_SetTripAverageSpeed(a)		paramCB.runtime.trip.speedAverage=a
//// ��ȡƽ���ٶȣ���λ:0.1Km/h
////#define PARAM_GetTripAverageSpeed()		(paramCB.runtime.trip.speedAverage)
//		
//// ��������ٶȣ���λ:0.1Km/h
//#define PARAM_SetTripMaxSpeed(a)			paramCB.nvm.param.common.record.trip.speedMax=a
//// ��ȡ����ٶȣ���λ:0.1Km/h
////#define PARAM_GetTripMaxSpeed()			(paramCB.nvm.param.common.record.trip.speedMax)
//	
//// ���ý�����̣���λ:0.1Km
//#define PARAM_SetToadyTrip(a)				paramCB.nvm.param.common.record.today.trip=a
//// ��ȡ������̣���λ:0.1Km
////#define PARAM_GetTodayTrip()				(paramCB.nvm.param.common.record.today.trip)
//	
//// ���ý��տ�·���λ:KCal
//#define PARAM_SetTodayCalories(a)			paramCB.nvm.param.common.record.today.calories=a
//// ��ȡ���տ�·���λ:KCal
//#define PARAM_GetTodayCalories()			(paramCB.nvm.param.common.record.today.calories)
//	
//// ���õ�λ, ������Ӣ�� 
//#define PARAM_SetUnit(a)					paramCB.nvm.param.common.unit=a
//// ��ȡ��λ, ������Ӣ�� 
//#define PARAM_GetUnit()						(paramCB.nvm.param.common.unit)
//	
//// ���ñ�������
//#define PARAM_SetBrightness(a)				paramCB.nvm.param.common.brightness=a
//// ��ȡ��������
//#define PARAM_GetBrightness()				(paramCB.nvm.param.common.brightness)
//		
//// �����ٶȱ仯ƽ����
//#define PARAM_SetSpeedFilterLevel(a)		paramCB.nvm.param.common.speed.filterLevel=a
//// ��ȡ�ٶȱ仯ƽ����
//#define PARAM_GetSpeedFilterLevel()			(paramCB.nvm.param.common.speed.filterLevel)
//	
//// �����Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min
//#define PARAM_SetPowerOffTime(a)			paramCB.nvm.param.common.powerOffTime=a
//// ��ȡ�Զ��ػ�ʱ�䣬��λ:���ӣ�>=0min
//#define PARAM_GetPowerOffTime()				(paramCB.nvm.param.common.powerOffTime)
//	
//// ����ϵͳ����������
//#define PARAM_SetWorkAssistRatio(a)			paramCB.nvm.param.common.workAssistRatio=a
//// ��ȡϵͳ����������
//#define PARAM_GetWorkAssistRatio()			(paramCB.nvm.param.common.workAssistRatio)
//	
//// ���ý�������ʱ����
//#define PARAM_SetTodayRidingTime(a)			paramCB.nvm.param.common.record.today.ridingTime=a
//// ��ȡ��������ʱ����
//#define PARAM_GetTodayRidingTime()			(paramCB.nvm.param.common.record.today.ridingTime)
//	
//// �������λ
//#define PARAM_SetMaxAssist(a)				do{\
//													paramCB.nvm.param.common.assistMax=a;\
//													NVM_SetPWMLimitValue();\
//												}while(0)
//// ��ȡ���λ
//#define PARAM_GetMaxAssist()				(paramCB.nvm.param.common.assistMax)

//// ����Э��
//#define PARAM_SetUartProtocol(a)			paramCB.nvm.param.common.uartProtocol=a
//// ��ȡЭ��
//#define PARAM_GetUartProtocol()				(paramCB.nvm.param.common.uartProtocol)
//	
//// �����Ƴ���������
//#define PARAM_SetPushSpeedSwitch(a)			paramCB.nvm.param.common.pushSpeedSwitch=a
//// ��ȡ�Ƴ���������
//#define PARAM_GetPushSpeedSwitch()			(paramCB.nvm.param.common.pushSpeedSwitch)
//	
//// ����Ĭ�ϵ�λ
//#define PARAM_SetDefaultAssist(a)			paramCB.nvm.param.common.defaultAssist=a
//// ��ȡĬ�ϵ�λ
//#define PARAM_GetDefaultAssist()			(paramCB.nvm.param.common.defaultAssist)
//	
//// ����LOGO����
//#define PARAM_SetlogoMenu(a)				paramCB.nvm.param.common.logoMenu=a
//// ��ȡLOGO����
//#define PARAM_GetlogoMenu()					(paramCB.nvm.param.common.logoMenu)
//	
//// ���ô���ͨѶ��ƽ
//#define PARAM_SetUartLevel(a)				paramCB.nvm.param.common.uartLevel=a
//// ��ȡ����ͨѶ��ƽ
//#define PARAM_GetUartLevel()				(paramCB.nvm.param.common.uartLevel)
//	
//// ��������ͨѶ���ϳ�ʱʱ�䣬��λ:ms
//#define PARAM_SetBusAliveTime(a)			paramCB.nvm.param.common.busAliveTime=a
//// ��ȡ����ͨѶ���ϳ�ʱʱ�䣬��λ:ms
//#define PARAM_GetBusAliveTime()				(paramCB.nvm.param.common.busAliveTime)
//	
//// ���õ�ص����仯ʱ�䣬��λ:s
//#define PARAM_SetBatteryCapVaryTime(a)		paramCB.nvm.param.common.battery.filterLevel=a
//// ��ȡ��ص����仯ʱ�䣬��λ:s
//#define PARAM_GetBatteryCapVaryTime()		(paramCB.nvm.param.common.battery.filterLevel)
//	
//// �����˶�״̬����ʱ�䣬��λ:s
//#define PARAM_SetMoveTimeKeepTime(a)		paramCB.runtime.record.moveKeepTime=a
//// ��ȡ�˶�״̬����ʱ�䣬��λ:s
//#define PARAM_GetMoveTimeKeepTime()			(paramCB.runtime.record.moveKeepTime)
//	
//// ���þ�ֹ״̬����ʱ�䣬��λ:s
//#define PARAM_SetStopTimeKeepTime(a)		paramCB.runtime.record.stopKeepTime=a
//// ��ȡ��ֹ״̬����ʱ�䣬��λ:s
//#define PARAM_GetStopTimeKeepTime()			(paramCB.runtime.record.stopKeepTime)
//	
//// �û�����ٶ�����
//#define PARAM_ClearUserMaxSpeed()			paramCB.runtime.record.userMaxSpeed=0
//// ��ȡ�û�����ٶ�
//#define PARAM_GetUserMaxSpeed()				(paramCB.runtime.record.userMaxSpeed)
//	
//// ���õ���¶�
//#define PARAM_SetMotorTemperature(a)		paramCB.runtime.motorTemperature=a
//// ��ȡ����¶�
//#define PARAM_GetMotorTemperature()			(paramCB.runtime.motorTemperature)
//	
//// ���õ��״̬
//#define PARAM_SetBatteryState(s)			paramCB.runtime.battery.state=s
//// ��ȡ���״̬
//#define PARAM_GetBatteryState()				(paramCB.runtime.battery.state)
//	
//// ��ȡFW�汾��
//#define PARAM_GetFWVersion()				(paramCB.runtime.fwVersion)
//	
//// ��ȡӲ���汾��
//#define PARAM_GetHWVersion()				(paramCB.runtime.hwVersion)
//	
//// ��ȡUI�汾��
//#define PARAM_GetUIVersion()				(paramCB.runtime.uiVersion)
//	
//// ��ȡ�������
//#define PARAM_GetBCVersion()				(paramCB.runtime.btCapacity)

//// ���ò��԰���ֵ
//#define PARAM_SetKeyValue(a)				paramCB.runtime.keyValue.keyNowValue=a

//// ��ȡ���԰���ֵ
//#define PARAM_GetKeyValue()					(paramCB.runtime.keyValue.keyNowValue)

//// �����Ƿ�������
//#define PARAM_SetExistBle(a)				paramCB.nvm.param.common.existBle=a
//// ��ȡ�Ƿ�������
//#define PARAM_GetExistBle()					(paramCB.nvm.param.common.existBle)

//// �˵���������
//#define	PARAM_SetMenuPassword(a)			paramCB.nvm.param.protocol.menuPassword=a
//#define	PARAM_GetMenuPassword()				(paramCB.nvm.param.protocol.menuPassword)

//// ������������
//#define	PARAM_SetPowerOnPassword(a)			paramCB.nvm.param.protocol.powerOnPassword=a
//#define	PARAM_GetPowerOnPassword()			(paramCB.nvm.param.protocol.powerOnPassword)

//// �ж��Ƿ��п�������
//#define	PARAM_SetPowerPasswordSwitch(a)		paramCB.nvm.param.protocol.powerPasswordSwitch=a
//#define	PARAM_GetPowerPasswordSwitch()		(paramCB.nvm.param.protocol.powerPasswordSwitch)

//// �ж��Ƿ��в˵�����
//#define	PARAM_SetMenuPasswordSwitch(a)		paramCB.nvm.param.protocol.menuPasswordSwitch=a
//#define	PARAM_GetMenuPasswordSwitch()		(paramCB.nvm.param.protocol.menuPasswordSwitch)

//#define PARAM_SeteBikeName(a)					paramCB.nvm.param.protocol.eBikeName=a
//#define PARAM_GeteBikeName()					(paramCB.nvm.param.protocol.eBikeName)

//#define PARAM_SetbatteryCap(a)					paramCB.nvm.param.protocol.batteryCap=a
//#define PARAM_GetbatteryCap()					(paramCB.nvm.param.protocol.batteryCap)

//#define PARAM_SettiresSize(a)					paramCB.nvm.param.protocol.tiresSize=a
//#define PARAM_GettiresSize()					(paramCB.nvm.param.protocol.tiresSize)
//// ���������Ƿ�����״̬
//#define	PARAM_SetBleConnectState(a)			paramCB.runtime.bleConnectState=a
//// ��ȡ�����Ƿ�����״̬
//#define	PARAM_GetBleConnectState()			(paramCB.runtime.bleConnectState)

//// ��ȡ�̼��汾���
//#define PARAM_GetFwBinNum()					(paramCB.runtime.fwBinNum)

//// ��ȡ�̼��汾�ű���λ
//#define PARAM_GetDevVersionRemain()			(paramCB.runtime.fwBinVersion.devVersionRemain)


//// ��ȡ�̼������汾��
//#define PARAM_Get_DevReleaseVersion()		(paramCB.runtime.fwBinVersion.devReleaseVersion)


//// ��ȡ�̼����������汾��
//#define PARAM_Get_FwUpdateVersion()			(paramCB.runtime.fwBinVersion.fwUpdateVersion)


//// ��ȡ�̼��޸�bug�����汾��
//#define PARAM_Get_FwDebugReleaseVersion()	(paramCB.runtime.fwBinVersion.fwDebugReleaseVersion)

//// ��������ģʽ
//#define PARAM_SetAssistMode(mode)			(paramCB.nvm.param.protocol.driver.assistMode=mode)
//// ��ȡ����ģʽ
//#define PARAM_GetAssistMode()				(paramCB.nvm.param.protocol.driver.assistMode)


//// ��������ģʽ����
//#define PARAM_SetAssistModeEnable(a)		(paramCB.nvm.param.protocol.driver.assistModeEnable=a)
//// ��ȡ����ģʽ����
//#define PARAM_GetAssistModeEnable()			(paramCB.nvm.param.protocol.driver.assistModeEnable)


//// �������ܳ�����
//#define PARAM_SetSmartType(a)				(paramCB.nvm.param.protocol.driver.smartType=a)
//// ��ȡ���ܳ�����
//#define PARAM_GetSmartType()				(paramCB.nvm.param.protocol.driver.smartType)


///******************************************************************************
//* ���ⲿ�ӿ�������
//******************************************************************************/
//// ģ���ʼ�������ģ���ʼ��
//void PARAM_Init(void);

//// ģ����̴���
//void PARAM_Process(void);

//// NVM���£��������ָ���Ƿ�����д��
//void NVM_SetDirtyFlag(BOOL writeAtOnce);
//uint16 PARAM_GetPerimeter(uint8 wheelSizeID);
//void NVM_Save(BOOL saveAll);

//uint16 PARAM_GetSpeed(void);
//uint8 PARAM_GetPwmTopLimit(uint8 asist);
//uint32 PARAM_GetTrip(void);
//uint32 PARAM_GetTotalDistance(void);
//uint16 PARAM_GetTripAverageSpeed(void);
//uint16 PARAM_GetTripMaxSpeed(void);
//ASSIST_ID_E  PARAM_GetAssistLevel(void);

//uint8 PARAM_GetPwmBottomLimit(uint8 asist);
//uint8 PARAM_GetPwmFixed(uint8 asist);
//uint16 PARAM_GetSpeedLimit(void);
//uint16 PARAM_GetPushSpeedLimit(void);
//void NVM_SetPWMLimitValue(void);

//// ���س�������
//void PARAM_FactoryReset(void);

//// �����ʼ�����ò���
//void NVM_Save_FactoryReset(BOOL saveAll);
//	
//// Trip����
//void PARAM_ClearTrip(void);

//// Э���ʼ��ѡ��
//void PARAM_ProtocolInitSelect(void);

//// ����Э��ѡ��
//void PARAM_ProtocolProcessSelect(void);

#endif


