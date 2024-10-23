#ifndef __RECORD_H__
#define __RECORD_H__

#include "common.h"
#include "ds1302.h"
#include "iap.h"

// ��Э��ÿ10���ӵ�������������С
#define TEN_MIN_DISTANCE                         48  

// ��Ԫ���Կ���
#define RECORD_UNIT_TEST_ON						0

// �Ƿ������ģ�飬��(1)�����������ݼ�¼���ܣ���(0)�򱻺���
#define RECORD_WITH_BLE_MODULE					1

// ��������������ֹͣʱ����ֵ
#define RECORD_MOVE_TIME_THRESHOLD				8		// ��λ:s
#define RECORD_STOP_TIME_THRESHOLD				10		// ��λ:s

// ����ʱ�洢�������� ���5min�洢һ����һ����Դ�288����¼
#define RECORD_RIDING_COUNT						288
// ����ʱ����´洢����ʱ���� 5min
#define RECORD_RIDING_TIME						300

// NVM��ʽ����ʶ
#define RECORD_NVM_FORMAT_FLAG					"2020000807134122"//"20171012111700"
#define RECORD_NVM_FORMAT_FLAG_SIZE				sizeof(RECORD_NVM_FORMAT_FLAG)

// ����E2�еĲ��������ݽṹ�������	

// �ܼ�¼�����ı����ַ
#define RECORD_ADDR_TOTAL						(IAP_MCU_RECORD_ADDR_BASE+RECORD_NVM_FORMAT_FLAG_SIZE)

// �ѷ��͵ļ�¼����
#define RECORD_ADDR_SEND_COUNT					(RECORD_ADDR_TOTAL+2)

// ������¼�ı���λ��
#define RECORD_ADDR_DATA_START					(RECORD_ADDR_SEND_COUNT+2)

// ĩ����¼�ı���λ��
#define RECORD_ADDR_DATA_END						(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM-3)*PARAM_MCU_PAGE_SIZE)	// Ԥ��5K��ϵͳ������¼ʹ��

// BLEͬ��ʱ���Flagλ��
#define RECORD_ADDR_BLE_CHECK_TIME_START		(RECORD_ADDR_DATA_START+sizeof(TRACK_TIME)+2+2+2+2+1)

// ���м�¼��������
#define RECORD_COUNT_MAX						((RECORD_ADDR_DATA_END-RECORD_ADDR_DATA_START)/sizeof(RECORD_PARAM_CB))

// ���м�¼���ݵ�ַ
#define IAP_MCU_RECORD_ADDR_BASE		(IAP_N32_FLASH_BASE+(PARAM_MCU_PAGE_NUM-5)*PARAM_MCU_PAGE_SIZE)

// ��������DS1302Ƭ��RAM�б����ַ
enum
{
	RECORD_PARAM_ADDR_RTC_VALID_FLAG = 0,
	RECORD_PARAM_ADDR_DIRTY_FLAG,
	RECORD_PARAM_ADDR_DATE_YEAR,
	RECORD_PARAM_ADDR_DATE_MONTH,
	RECORD_PARAM_ADDR_DATE_DAY,
	RECORD_PARAM_ADDR_DATE_HOUR,
	RECORD_PARAM_ADDR_DATE_MINUTE,
	RECORD_PARAM_ADDR_DATE_SECOND,
	RECODE_PARAM_ADDR_TRIPTIMER_H,
	RECODE_PARAM_ADDR_TRIPTIMER_L,
	RECORD_PARAM_ADDR_MAXSPEED_H,
	RECORD_PARAM_ADDR_MAXSPEED_L,
	RECORD_PARAM_ADDR_KCAL_H,
	RECORD_PARAM_ADDR_KCAL_L,
	RECORD_PARAM_ADDR_TRIP_H,
	RECORD_PARAM_ADDR_TRIP_L,
	RECODE_PARAM_ADDR_TIMESLOT,
	
	RECORD_PARAM_ADDR_IS_CHECK_BLE_TIME,
	RECORD_PARAM_ADDR_CHECK,
	RECORD_PARAM_ADDR_MAX
};

// ʱ������ṹ��
typedef struct
{
	uint8 year;
	uint8 month;
	uint8 day;
	uint8 hour;
	uint8 min;
	uint8 sec;
}TRACK_TIME;

// һ�ֽڶ���
#pragma pack(1)
// ����
typedef struct
{
	TRACK_TIME dateTime;
	
	// ��������
	uint16 tripTimer;			// �˶�ʱ������λ��
	uint16 speedMax;			// ����ٶȣ���λ:0.1Km/h
	uint16 calories;			// ��·���λ:KCal
	uint16 trip; 				// Trip����λ:0.1Km
	uint8 timeSlot;				// ����ʱ���
	
	uint8 isTimeCheck;			// ����ͬʱʱ���־λ, 
								// 1:ͬ��У׼��
								// 0:�ϵ�ʱ�ж�Ϊ0ʱ��ǿ��Ϊ0xFF�����ϵ����������ʱ��Ч��δУ׼��
								// FF:����������Ч
	uint8_t packCheck;			// ���ݰ�У��λ
}RECORD_PARAM_CB;
#pragma pack()

// ���м�¼���ݽṹ
typedef union
{
	RECORD_PARAM_CB param;

	uint8 array[sizeof(RECORD_PARAM_CB)];
}RECORD_ITEM_CB;

typedef struct 
{
	uint8 hour;					// �˶�����ʱ���
	uint8 calories;				// ��·��
	uint8 trip;					// ���о���
	uint16 speedMax;			// ����ٶ�
	uint16 min;					// �˶�����
	uint16 ridingTime;			// ����ʱ��
}RECORD_PARAM_TMP;

typedef union
{
	RECORD_PARAM_TMP recordparam;
	
	uint8 array[sizeof(RECORD_PARAM_TMP)];
}RECORD_ITEM_TMP;

// δ��ʱ��������ݽṹ
typedef struct 
{
	uint8 tripFlag;								// δ��ʱ���б�־

	uint16 tirpNum;								// δ��ʱ���м�¼����

	uint32 tirpTime;							// δ��ʱ����ʱ�䣬��λs
	
	RECORD_ITEM_TMP recordItemTmp[RECORD_RIDING_COUNT];			// δ��ʱ���м�¼�����粻�����

}RECORD_ITEM_TMP_CB;

extern RECORD_ITEM_TMP_CB recordTmpCB;

// ϵͳ״̬����
typedef enum
{
	RECORD_STATE_NULL = 0,							// ��״̬

	RECORD_STATE_STANDBY,							// ����״̬

	RECORD_STATE_RECORDING,							// ������¼״̬

	RECORD_STATE_MAX								// ״̬��

}RECORD_STATE_E;


// ������Ϣ�ܿؽṹ��
typedef struct
{
	uint8 nvmFormat[RECORD_NVM_FORMAT_FLAG_SIZE];	// NVM��ʽ����ʶ

	RECORD_STATE_E state;				// ��ǰϵͳ״̬
	RECORD_STATE_E preState;			// ��һ��״̬

	// ��ʼ����
	struct
	{
		uint32 ridingTime;
		uint32 distance;
		uint32 calories;
	}start;

	uint8 ds1302Addr[RECORD_PARAM_ADDR_MAX];
}RECORD_CB;


extern RECORD_CB recordCB;





//�����Э�黺��ṹ��

typedef struct
{
	uint16 year;
	uint8 month;
	uint8 day;
	uint8 hour;
	uint8 min;
	uint8 sec;
}NEW_RECORD_TIME_E;


typedef struct 
{
	NEW_RECORD_TIME_E powerontime;             // ��ʼʱ��
	
	NEW_RECORD_TIME_E powerofftime;					// ����ʱ��
	
	uint32 powerontime_count;	             // ��������				
	uint16 ridingTime;		               	// ����ʱ��
	uint16 speedMax;			             // ����ٶ�
	
	uint16 calories; 			          // ��·��
	uint16 trip; 				        // ���о���
	
	uint8 p_Arr;             // �����ָ�룬�������������
	uint8 Arr[TEN_MIN_DISTANCE];    // ʮ���ӱ���һ����
	
	uint8 isTimeCheck;          // ʱ��У׼�ɹ���־
	
}NEW_RECORD_PARAM_DATA;


typedef union
{
	NEW_RECORD_PARAM_DATA param;

	uint8 array[sizeof(NEW_RECORD_PARAM_DATA)];
}NEW_RECORD_ITEM_CB;

extern NEW_RECORD_ITEM_CB new_record_data;



// ������Ϣģ���ϵ��ʼ��׼��
void RECORD_Init(void);

// ������ݼ�¼
void RECORD_ClearRecords(void);

// д��һ�����м�¼�㣬�����ǰ�������������ݣ�����¿�ʼʱ�䣻����������������ݣ���ֻ���½���ʱ��
void RECORD_CALLBACK_DoBackup(uint32 param);

// ����ǰ����д��NVM
void RECORD_SaveToNVM(void);

// ��ȡһ����¼
BOOL RECORD_ReadFromNVM(RECORD_ITEM_CB* pTrack);

// ��ȡ���м�¼����
uint16 RECORD_GetRecordCount(void);

// ��ȡ�ѷ��ͼ�¼����
uint16 RECORD_GetRecordSendCount(void);

// ״̬Ǩ��
void RECORD_EnterState(uint32 state);

// ״̬������
void RECORD_Process(void);

// ��ȡ��¼״̬
RECORD_STATE_E RECORD_GetState(void);

// ȷ����ʷ���ݷ��ͳɹ�֮��ˢ���ѷ�����ʷ���ݣ����ѷ�����ʷ���ݼ�һ
BOOL RECORD_RefreashSendCount(void);

// ��ѯ�Ƿ���δͬ����ʷ����
BOOL RECORD_IsHistoricalData(void);

// ˢ���ѷ�����ʷ��������
void RECORD_RefreashSaveCount(void);

// ���м�¼У��
uint8_t RECORD_Check(uint8_t *ptr, uint8_t len);

#endif



