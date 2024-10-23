/********************************************************************************************************
	�˷�V5.2ͨѶЭ��
********************************************************************************************************/
#ifndef 	__UART_PROTOCOL2_H__
#define 	__UART_PROTOCOL2_H__

/*******************************************************************************
 *                                  ͷ�ļ�	                                   *
********************************************************************************/
#include "common.h"

//==========��Э��ǿ��أ����ݲ�ͬЭ������޸�======================================
// UART����ͨѶ����ʱ��
#define UART_PROTOCOL2_COMMUNICATION_TIME_OUT		10000UL	// ��λ:ms

// P��λ
#define PROTOCOL_ASIST_P							ASSIST_P

#define UART_PROTOCOL2_CMD_SEND_TIME				120		// �����ʱ�䣬�˷�Э����ʱ��������Ҫ100ms����

//=====================================================================================================
#define UART_PROTOCOL2_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define UART_PROTOCOL2_RX_QUEUE_SIZE				3		// ����������гߴ�
#define UART_PROTOCOL2_TX_QUEUE_SIZE				5		// ����������гߴ�

#define UART_PROTOCOL2_CMD_WRITE					0x16	// д����
#define UART_PROTOCOL2_CMD_READ						0x11	// ������

#define UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MIN 		2		// ��С��������֡����
#define UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX 		5		// ���������֡����
#define UART_PROTOCOL2_RX_CMD_FRAME_LENGTH_MAX 		28		// ����������֡����
#define UART_PROTOCOL2_RX_FIFO_SIZE					200		// ����һ����������С

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define UART_PROTOCOL2_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define UART_PROTOCOL2_BUS_BIDIRECTIONAL_TIME_OUT	(uint32)(((1000.0/UART_DRIVE_BAUD_RATE*10*UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	UART_PROTOCOL2_CMD_DATA1_INDEX = 0,					// ��������
	UART_PROTOCOL2_CMD_DATA2_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA3_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA4_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA5_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA6_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA7_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA8_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA9_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA10_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA11_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA12_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA13_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA14_INDEX,						// ��������
	UART_PROTOCOL2_CMD_DATA15_INDEX,						// ��������

	UART_PROTOCOL2_CMD_INDEX_MAX
}UART_PROTOCOL2_DATE_FRAME;

// ����
typedef enum{
	UART_PROTOCOL2_CMD_NULL = 0,							// ������

	UART_PROTOCOL2_CMD_READ_PROTOCOL_VERSION = 0x1190,

	UART_PROTOCOL2_CMD_READ_DRIVER_STATUS = 0x1108,
	
	UART_PROTOCOL2_CMD_READ_WORK_STATUS = 0x1131,

	UART_PROTOCOL2_CMD_READ_INSTANTANEOUS_CURRENT = 0x110A,

	UART_PROTOCOL2_CMD_READ_BATTERY_CAPACITY = 0x1111,

	UART_PROTOCOL2_CMD_READ_RPM = 0x1120,

	UART_PROTOCOL2_CMD_READ_LIGHT_SWITCH = 0x111B,

	UART_PROTOCOL2_CMD_READ_BATTERY_INFO1 = 0x1160,

	UART_PROTOCOL2_CMD_READ_BATTERY_INFO2 = 0x1161,

	UART_PROTOCOL2_CMD_READ_REMAINDER_MILEAGE = 0x1122,

	UART_PROTOCOL2_CMD_READ_DRIVER_ERROR_CODE = 0x1123,

	UART_PROTOCOL2_CMD_READ_CADENCE = 0x1121,

	UART_PROTOCOL2_CMD_READ_TQKG = 0x1124,		// ʵ������Ϊ0x1122���������ظ����ˣ�ֱ�Ӹ�һ�������ڵļ���

	//============================================
	UART_PROTOCOL2_CMD_WRITE_LIMIT_SPEED = 0x161F,

	UART_PROTOCOL2_CMD_WRITE_ASSIST_LEVEL = 0x160B,

	UART_PROTOCOL2_CMD_WRITE_LIGHT_SWITCH = 0x161A,

	UART_PROTOCOL2_CMD_WRITE_CLEAR_DRIVER_ERROR_CODE = 0x1623,

	UART_PROTOCOL2_CMD_MAX								// ��������
}UART_PROTOCOL2_CMD;

// UART����Э�鶨��	
typedef enum
{
	UART_PROTOCOL2_TX_CMD_TYPE_ASSIST = 0,
	UART_PROTOCOL2_TX_CMD_TYPE_LIGHT,	
}UART_PROTOCOL2_TX_AT_ONCE_CMD_TYPE;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL2_RX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL2_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[UART_PROTOCOL2_TX_CMD_FRAME_LENGTH_MAX];	// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}UART_PROTOCOL2_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[UART_PROTOCOL2_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		BOOL			rxFIFOBusy;
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL2_RX_CMD_FRAME	cmdQueue[UART_PROTOCOL2_RX_QUEUE_SIZE];
		uint16			head;						// ����ͷ����
		uint16			end;						// ����β����
		uint8			expectLength;
	}rx;

	// ����֡���������ݽṹ
	struct{
		UART_PROTOCOL2_TX_CMD_FRAME	cmdQueue[UART_PROTOCOL2_TX_QUEUE_SIZE];
		uint16	head;						// ����ͷ����
		uint16	end;						// ����β����
		uint16	index;						// ��ǰ����������������֡�е�������
		BOOL	txBusy;						// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	BOOL txPeriodRequest;		// ��������Է�����������
	BOOL txAtOnceRequest;		// ������Ϸ�����������
	UART_PROTOCOL2_TX_AT_ONCE_CMD_TYPE txAtOnceCmdType;

	UART_PROTOCOL2_CMD cmdType;
	uint8 step;
	
}UART_PROTOCOL2_CB;

extern UART_PROTOCOL2_CB uartProtocolCB2;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void UART_PROTOCOL2_Init(void);

// Э�����̴���
void UART_PROTOCOL2_Process(void);

// ��������֡�������������
void UART_PROTOCOL2_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void UART_PROTOCOL2_TxAddFrame(void);

//==================================================================================
// �����·������־���������ò���Ҳ�ǵ��ô˽ӿ�
void UART_PROTOCOL2_SetTxAtOnceRequest(uint32 param);

void UART_PROTOCOL2_SetTxAtOnceCmdType(UART_PROTOCOL2_TX_AT_ONCE_CMD_TYPE type);

void UART_PROTOCOL2_CALLBACK_SetTxPeriodRequest(uint32 param);

/*
// ����������Ͷ���
typedef enum
{
	ERROR_TYPE_NO_ERROR = 0x01,								// �޴���	
	ERROR_TYPE_BREAKED = 0x03,								// ��ɲ��
	ERROR_TYPE_TURN_BAR_NOT_BACK = 0x04,					// ת��û�й�λ
	ERROR_TYPE_TURN_BAR_ERROR = 0x05,						// ת�ѹ���
	ERROR_TYPE_LOW_VOLTAGE_PROTECT = 0x06,					// �͵�ѹ����
	ERROR_TYPE_OVER_VOLTAGE_PROTECT = 0x07,					// ����ѹ����
	ERROR_TYPE_MOTOR_HALL_ERROR = 0x08,						// ��������ź��߹���
	ERROR_TYPE_MOTOR_PHASE_ERROR = 0x09,					// ������߹���
	ERROR_TYPE_CONTROLLER_TEMP_REACH_PROTECT_POINT = 0x10,	// �������¶��Ѵﵽ������
	ERROR_TYPE_MOTOR_TEMP_REACH_PROTECT_POINT = 0x11,		// ����¶ȸ��Ѵﵽ������
	ERROR_TYPE_CURRENT_SENSOR_ERROR = 0x12,					// ��������������
	ERROR_TYPE_BATTERY_INSIDE_TEMP_ERROR = 0x13, 			// ������¶ȹ���
	ERROR_TYPE_MOTOR_INSIDE_TEMP_ERROR = 0x14,				// ������¶ȴ���������
	ERROR_TYPE_SPEED_SENSOR_ERROR = 0x21,					// �ٶȴ���������
	ERROR_TYPE_BMS_COMMUNICATION_ERROR = 0x22,				// BMSͨѶ����
	ERROR_TYPE_LIGHT_ERROR = 0x23,							// ��ƹ���
	ERROR_TYPE_LIGHT_SENSOR_ERROR = 0x24,					// ��ƴ���������
	ERROR_TYPE_TORQUE_SENSOR_TORQUE_ERROR = 0x25,			// ���ش����������źŹ���
	ERROR_TYPE_TORQUE_SENSOR_SPEED_ERROR = 0x26,			// ���ش������ٶȹ���
	ERROR_TYPE_COMMUNICATION_TIME_OUT = 0x30,				// ͨѶ��ʱ
	
}ERROR_TYPE_E;
*/

#endif


