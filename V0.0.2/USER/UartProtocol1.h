/********************************************************************************************************
	﮵�2��ͨѶЭ��(������˼άЭ��)
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL1_H__
#define 	__UART_PROTOCOL1_H__

#include "common.h"

#define SMOOTH_BASE_TIME1							100		// ƽ����׼ʱ�䣬��λ:ms

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL1_COMMUNICATION_TIME_OUT		10000UL	// ��λ:ms

// P��λ
#define PROTOCOL_ASSIST_P							ASSIST_P

#define UART_PROTOCOL1_CMD_SEND_TIME				100		// �����ʱ��

//=====================================================================================================
#define UART_PROTOCOL1_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL1_RX_QUEUE_SIZE				3		// ����������гߴ�
#define UART_PROTOCOL1_TX_QUEUE_SIZE				3		// ����������гߴ�

#define UART_PROTOCOL1_CMD_RX_DEVICE_ADDR			0x02	// �豸��ַͷ
#define UART_PROTOCOL1_CMD_TX_DEVICE_ADDR			0x01	// �豸��ַͷ

#define UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX 		150		// ���������֡����
#define UART_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX 		150		// ����������֡����
#define UART_PROTOCOL1_RX_FIFO_SIZE					200		// ����һ����������С
#define UART_PROTOCOL1_CMD_FRAME_LENGTH_MIN			4		// ����֡��С���ȣ�����:�豸��ַ�����ݳ��ȡ������֡�У���


// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL1_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define UART_PROTOCOL1_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	UART_PROTOCOL1_CMD_DEVICE_ADDR_INDEX = 0,			// �豸��ַ
	UART_PROTOCOL1_CMD_LENGTH_INDEX,					// ���ݳ���
	UART_PROTOCOL1_CMD_CMD_INDEX, 						// ����������

	UART_PROTOCOL1_CMD_DATA1_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA2_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA3_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA4_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA5_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA6_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA7_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA8_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA9_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA10_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA11_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA12_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA13_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA14_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA15_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA16_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA17_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA18_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA19_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA20_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA21_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA22_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA23_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA24_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA25_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA26_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA27_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA28_INDEX,						// ��������
	UART_PROTOCOL1_CMD_DATA29_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA30_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA31_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA32_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA33_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA34_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA35_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA36_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA37_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA38_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA39_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA40_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA41_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA42_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA43_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA44_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA45_INDEX, 					// ��������
	UART_PROTOCOL1_CMD_DATA46_INDEX, 					// ��������

	UART_PROTOCOL1_CMD_INDEX_MAX
}UART_PROTOCOL1_DATE_FRAME;

// ��������
typedef enum{
	UART_PROTOCOL1_CMD_NULL = 0,								// ������

	UART_PROTOCOL1_CMD_HMI_CTRL_RUN = 0x01,					// ��������״̬

	UART_PROTOCOL1_CMD_WRITE_CONTROL_PARAM = 0xC0,			// д����������
	UART_PROTOCOL1_CMD_WRITE_CONTROL_PARAM_RESULT = 0xC1,	// д�������������Ӧ��
	UART_PROTOCOL1_CMD_READ_CONTROL_PARAM = 0xC2,			// ��ȡ����������
	UART_PROTOCOL1_CMD_READ_CONTROL_PARAM_REPORT = 0xC3,	// �����������ϱ�

	UART_PROTOCOL1_CMD_FLAG_ARRAY_READ = 0xA6,				// ����־������
	UART_PROTOCOL1_CMD_FLAG_ARRAY_WRITE = 0xA7,				// д��־������

	UART_PROTOCOL1_CMD_VERSION_TYPE_WRITE = 0xA8,			// д��汾��Ϣ
	UART_PROTOCOL1_CMD_VERSION_TYPE_READ = 0xA9,			// ��ȡ�汾��Ϣ

	UART_PROTOCOL1_CMD_TEST_LCD = 0xAA,						// LCD��ɫ��ʾ����
	UART_PROTOCOL1_CMD_TEST_KEY = 0xAB,						// ��������ָ��

	UART_PROTOCOL1_CMD_MAX									// ��������
}UART_PROTOCOL1_CMD;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL1_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL1_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL1_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL1_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[UART_PROTOCOL1_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;					// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL1_RX_CMD_FRAME	cmdQueue[UART_PROTOCOL1_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL1_TX_CMD_FRAME	cmdQueue[UART_PROTOCOL1_TX_QUEUE_SIZE];
		uint16	head;						// ����ͷ����
		uint16	end;						// ����β����
		uint16	index;						// ��ǰ����������������֡�е�������
		BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	BOOL txPeriodRequest;		// ��������Է�����������
	BOOL txAtOnceRequest;		// ������Ϸ�����������
	
	// ƽ���㷨���ݽṹ
	struct{
		uint16 realSpeed;
		uint16 proSpeed;
		uint16 difSpeed;
	}speedFilter;
}UART_PROTOCOL1_CB;

extern UART_PROTOCOL1_CB uartProtocolCB1;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void UART_PROTOCOL1_Init(void);

// Э�����̴���
void UART_PROTOCOL1_Process(void);

// ��������֡�������������
void UART_PROTOCOL1_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void UART_PROTOCOL1_TxAddFrame(void);

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void UART_PROTOCOL1_SetTxAtOnceRequest(uint32 param);

// �����ϻ��ۼӴ���
void UART_PROTOCOL1_SendCmdAging(uint32 param);

void UART_PROTOCOL1_CALLBACK_SetTxPeriodRequest(uint32 param);


/*
// ����������Ͷ���
typedef enum
{
	ERROR_TYPE_NO_ERROR = 0,						// �޴���
	ERROR_TYPE_BREAK_ERROR = 0x02,					// ɲ������
	ERROR_TYPE_BATTERY_UNDER_VOLTAGE_ERROR = 0x06,	// ���Ƿѹ
	ERROR_TYPE_MOTOR_ERROR = 0x07,					// �������
	ERROR_TYPE_TURN_ERROR = 0x08,					// ת�ѹ���
	ERROR_TYPE_DRIVER_ERROR = 0x09,					// ����������	
	ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x30,		// ͨѶ���ճ�ʱ
	ERROR_TYPE_HALL_ERROR = 0x14,					// ��������
	
}ERROR_TYPE_E;*/

// ���� ����·����������Ŀ����������趨1 ����
// bit0 bit1 ...bit7
typedef struct
{
	uint8 switchCruiseMode: 1; 			// Ѳ���л�ģʽ
	uint8 isPushModeOn: 1;				// 6km/h���й��ܣ�0:��ʾ�رգ�1:��ʾ��
	uint8 speedLimitState: 1;			// ����״̬
	uint8 switchCruiseWay: 1; 			// �л�Ѳ���ķ�ʽ
	uint8 comunicationError: 1;			// ͨѶ����
	uint8 isLightOn : 1;				// �ƹ����
	uint8 zeroStartOrNot : 1;			// ����������������
	uint8 driverState : 1;				// ����������״̬

}UART_PROTOCOL1_DRIVER_SET1_CB;
#define UART_PROTOCOL1_DRIVER_SET1_SIZE	(sizeof(UART_PROTOCOL1_DRIVER_SET1_CB))

// ���� ����·����������Ŀ����������趨2 ����
// bit0 bit1 ...bit7
typedef struct
{
	uint8 assistSteelType : 4;			// �����Ÿ�������
	uint8 reserveBit4 : 1;
	uint8 reserveBit5 : 1;
	uint8 autoCruise : 1;				// �Զ�Ѳ����־
	uint8 astern : 1;					// ������־

}UART_PROTOCOL1_DRIVER_SET2_CB;
#define UART_PROTOCOL1_DRIVER_SET2_SIZE	(sizeof(UART_PROTOCOL1_DRIVER_SET2_CB))

// ���� �������ϱ������Ĳ��� ����
typedef struct
{
	// ������״̬1
	uint8 motorPhaseError : 1;			// ���ȱ�࣬1: ���ȱ����� 0: ���������
	uint8 breakError : 1;				// ɲ�ѹ��ϣ�1: ɲ���й��� 0��ɲ���޹���
	uint8 cruiseState : 1;				// Ѳ��״̬��1: ����Ѳ�� 0: ����Ѳ��
	uint8 underVoltageProtectionError : 1;// Ƿѹ����״̬��1: ����Ƿѹ���� 0: ����Ƿѹ����
	uint8 driverError : 1;				// ����������״̬��1: ���������� 0: ������������
	uint8 turnBarError : 1;				// ת�ѹ���״̬��1: ת�ѹ��� 0: ת�Ѳ�����
	uint8 holzerError : 1;				// ��������״̬��1: �������� 0: ����������
	uint8 cruise6kmState : 1;			// 6kmѲ��״̬��1: ����6Km Ѳ�� 0: ����6Km Ѳ��

	// ������״̬2
	uint8 reserveBit10 : 2;				// �������Ƶ�λ(Ԥ��)
	uint8 DriverSpeedLimitState : 1;	// ����������״̬��0�����٣�1������
	uint8 chargeState : 1;				// ���״̬��0����磬1�����
	uint8 communicationError : 1;		// ͨѶ���ϣ�0������1����RX
	uint8 breakState : 1;				// �ϵ�ɲ��(״̬),0û��������1����
	uint8 assistSensorState : 1;		// ����������״̬��0������1����
	uint8 bicycleHorizontalState : 1;	// ���Ƿ���ˮƽ״̬��0ˮƽ��1��ˮƽ

	// �����ֽڵĵ�6 λΪ1 ʱ����ʾ��λΪ0.1A�� 
	// ������ֽڵĵ�6 λΪ0�������ĵ�λ��Ȼ��1A
	uint8 ucCurBatteryCurrentH;			// ��ǰ��ص��������ֽ�	
	uint8 ucCurBatteryCurrentL;			// ��ǰ��ص��������ֽ�

	uint8 currentRatio;					// ��������ֵ

	uint8 oneCycleTimeH;				// ��ǰһȦʱ�䣬���ֽ�
	uint8 oneCycleTimeL;				// ��ǰһȦʱ�䣬���ֽ�	
	
	uint8 batteryCapacity;				// �������

	uint8 remainderMileageH;			// ʣ����̣����ֽ�
	uint8 remainderMileageL;			// ʣ����̣����ֽ�
	
}UART_PROTOCOL1_RX_PARAM_CB;
#define UART_PROTOCOL1_RX_PARAM_SIZE			(sizeof(UART_PROTOCOL1_RX_PARAM_CB))


// �������������ݽ����ṹ�嶨��
typedef struct
{
	// ��������������
	union
	{
		uint8 val;
		UART_PROTOCOL1_DRIVER_SET1_CB param;
	}DriverSet1;

	// ��������������
	union
	{
		uint8 val;
		UART_PROTOCOL1_DRIVER_SET2_CB param;
	}DriverSet2;

	// �������ϱ������Ĳ���
	union
	{
		uint8 buff[UART_PROTOCOL1_RX_PARAM_SIZE];
		UART_PROTOCOL1_RX_PARAM_CB param;
	}rx;	
}UART_PROTOCOL1_PARAM_CB;

extern UART_PROTOCOL1_PARAM_CB hmiDriveCB1;


#endif


