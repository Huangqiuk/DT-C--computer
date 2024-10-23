#ifndef __BLE_PROTOCOL_H__
#define __BLE_PROTOCOL_H__

#include "common.h"

//=====================================================================================================
#define BLE_PROTOCOL_RX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ������ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��
#define BLE_PROTOCOL_TXRX_TIME_OUT_CHECK_ENABLE		0		// ͨѶ˫����ճ�ʱ��⹦�ܿ���:0��ֹ��1ʹ��

#define BLE_PROTOCOL_RX_QUEUE_SIZE					5		// ����������гߴ�
#define BLE_PROTOCOL_TX_QUEUE_SIZE					5		// ����������гߴ�

#define BLE_PROTOCOL_CMD_HEAD						0x55	// ����ͷ

#define BLE_PROTOCOL_UPDATA_LENGTH_MAX				128    // ��������󳤶�

#define BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 		150		// ���������֡����
#define BLE_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 		150		// ����������֡����
#define BLE_PROTOCOL_RX_FIFO_SIZE					200		// ����һ����������С
#define BLE_PROTOCOL_CMD_FRAME_LENGTH_MIN			4			// ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У���

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define BLE_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT	(uint32)((1000.0/BLE_UART_BAUD_RATE*10*BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)		// ��λ:ms
#define BLE_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT		(uint32)(((1000.0/BLE_UART_BAUD_RATE*10*BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX*2.0+0.9)+2*TIMER_TIME)*2)	// ��λ:ms

// UART����Э�鶨��	
typedef enum
{
	BLE_PROTOCOL_CMD_HEAD_INDEX = 0,					// ֡ͷ����
	BLE_PROTOCOL_CMD_CMD_INDEX, 						// ����������
	BLE_PROTOCOL_CMD_LENGTH_INDEX,						// ���ݳ�������

	BLE_PROTOCOL_CMD_DATA1_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA2_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA3_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA4_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA5_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA6_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA7_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA8_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA9_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA10_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA11_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA12_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA13_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA14_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA15_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA16_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA17_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA18_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA19_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA20_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA21_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA22_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA23_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA24_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA25_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA26_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA27_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA28_INDEX,						// ��������
	BLE_PROTOCOL_CMD_DATA29_INDEX, 						// ��������
	BLE_PROTOCOL_CMD_DATA30_INDEX, 						// ��������

	BLE_PROTOCOL_CMD_INDEX_MAX
}BLE_PROTOCOL_DATE_FRAME;

// ��������
typedef enum{
	// ϵͳ����
	BLE_CMD_HEARTBEAT_DOWN = 0x00,						// �����������
	BLE_CMD_HEARTBEAT_UP = 0x01,						// ��������Ӧ������
	BLE_CMD_AUTHENTICATE_DOWN = 0x02,					// ��Ȩ�������
	BLE_CMD_AUTHENTICATE_UP = 0x03,						// ��Ȩ����Ӧ������
	BLE_CMD_FIRMVERSION_DOWN = 0x04,					// ѯ�ʹ̼��汾�������
	BLE_CMD_FIRMVERSION_UP = 0x05,						// ѯ�ʹ̼��汾����Ӧ������
	BLE_CMD_SETTIMER_DOWN = 0x06,						// ������ʱ�������
	BLE_CMD_SETTIMER_UP = 0x07,							// ������ʱ����Ӧ������
	BLE_CMD_ERROR_UP = 0x08,							// ��������ϱ�������
	BLE_CMD_ERROR_DOWN = 0x09,							// �������Ӧ����գ�����
	BLE_CMD_PROTOCOL_VERSION_DOWN = 0x0A,				// Э��汾������
	BLE_CMD_PROTOCOL_VERSION_UP = 0x0B,					// Э��汾������
	
	// IAP����
	BLE_CMD_RESET_DOWN = 0x20,							// ��λ�������
	BLE_CMD_RESET_UP = 0x21,							// ��λ����Ӧ������
	BLE_CMD_FIRMUPDATE_DOWN = 0x22,						// �̼������������
	BLE_CMD_FIRMUPDATE_UP = 0x23,						// �̼���������Ӧ������
	BLE_CMD_DATAIN_DOWN = 0x24,							// ���ݰ�д���������
	BLE_CMD_DATAIN_UP = 0x25,							// ���ݰ�д������Ӧ������
	BLE_CMD_DATAIN_FINISH_DOWN = 0x26,					// ���ݰ�д������������
	BLE_CMD_DATAIN_FINISH_UP = 0x27,					// ���ݰ�д���������Ӧ������

	// ��������
	BLE_CMD_GETSTATUS_DOWN = 0x30,						// ��ȡ�Ǳ�״̬�������
	BLE_CMD_GETSTATUS_UP = 0x31,						// �Ǳ�״̬������Ӧ������
	BLE_CMD_CONTROL_DOWN = 0x32,						// �Ǳ�����������
	BLE_CMD_CONTROL_UP = 0x33,							// �Ǳ��������Ӧ������
	BLE_CMD_CHECK_SELF_DOWN = 0x34,						// �Ǳ�ʼ�Լ죬����
	BLE_CMD_CHECK_SELF_UP = 0x35,						// �Ǳ��Լ����ϱ�������
	BLE_CMD_FINISH_UP = 0x36,							// �Ǳ�����ϱ�������
	BLE_CMD_SELECT_GETSTATUS_DOWN = 0x37,				// ѡ���Ի�ȡ�����ϱ�������
	BLE_CMD_SELECT_GETSTATUS_UP = 0x38,					// ѡ���Ի�ȡ�����ϱ�������

	// ���ݲɼ�����
	BLE_CMD_GETDATA_RUNNING_DOWN = 0x60,				// ��ȡʵʱ�˶����ݣ�����
	BLE_CMD_GETDATA_RUNNING_UP = 0x61,					// ��ȡʵʱ�˶�����Ӧ������
	BLE_CMD_START_HISTORICAL_DATA_UP = 0x62,			// ����ͬ����ʷ�˶����ݣ�����
	BLE_CMD_START_HISTORICAL_DATA_DOWN = 0x63,			// ����ͬ����ʷ�˶�����Ӧ���������
	BLE_CMD_REPORT_HISTORICAL_DATA_UP = 0x64,			// �ϱ���ʷ�˶����ݣ�����
	BLE_CMD_REPORT_HISTORICAL_DATA_DOWN = 0x65,			// �ϱ���ʷ�˶������������
	
	//�����
	BLE_CMD_START_HISTORICAL_DATA_UP_NEW = 0x66,			// �ϱ���ʷ�˶������������
	BLE_CMD_START_HISTORICAL_DATA_DOWN_NEW = 0x67,			// �ϱ���ʷ�˶���������Ӧ������
	BLE_CMD_REPORT_HISTORICAL_DATA_UP_NEW = 0x68,			// �ϱ���ʷ�˶����ݣ�����
	BLE_CMD_REPORT_HISTORICAL_DATA_DOWN_NEW = 0x69,			// �ϱ���ʷ�˶�����ָ��Ӧ������
	
	// ����ҵ��
	BLE_CMD_GET_VOLCUR_VALUE = 0xF0,							// ��ȡ��ѹ����������ָ��
	BLE_CMD_GET_VOLCUR_VALUE_ACK = 0xF1,						// ��ȡ��������ָ��
	BLE_CMD_MAX,

}BLE_PROTOCOL_CMD;

// �������������
typedef enum{
	CONTROL_CMD_NULL = 0,
	LIGHT,												// ���ƴ��
	AUTO_SHUTDOWN_TIME,									// �����Զ��ػ�ʱ��
	SPEED_LIMIT,										// ����ֵ
	UNIT,												// ������ʾ��λ
	ASSIST,												// ����������λ
	LOCK,												// ��������״̬
	BACKLIGHT,											// ���ñ�������
	CONTROL_CMD_MAX,
}BLE_PROTOCOL_CONTROL_CMD;

// �豸����ID 
typedef enum{
	NULL_DEVICE = 0,			
	HMI_DEVICE,											// �Ǳ�
	BMS_DEVICE,											// ���
	COLTROLA_DEVICE,									// ������A
	COLTROLB_DEVICE,									// ������B
	TURN_DEVICE,										// ת���
	HETEL_DEVICE,										// ͷ��
	WLK_DEVICE,											// ���߰���
	LOCK_DEVICE,										// ������
	TRANSMISSION_DEVICE,								// ������
	
}BLE_PROTOCOL_DEVICE;

// UART_RX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX];		// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}BLE_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16  deviceID;
	uint8	buff[BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX];		// ����֡������
	uint16	length; 										// ����֡��Ч���ݸ���
}BLE_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8	buff[BLE_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16	head;
		volatile uint16	end;
		uint16 currentProcessIndex;											// ��ǰ��������ֽڵ�λ���±�
	}rxFIFO;
	
	// ����֡���������ݽṹ
	struct{
		BLE_PROTOCOL_RX_CMD_FRAME	cmdQueue[BLE_PROTOCOL_RX_QUEUE_SIZE];
		uint16			head;												// ����ͷ����
		uint16			end;												// ����β����
	}rx;

	// ����֡���������ݽṹ
	struct{
		BLE_PROTOCOL_TX_CMD_FRAME	cmdQueue[BLE_PROTOCOL_TX_QUEUE_SIZE];
		uint16	head;														// ����ͷ����
		uint16	end;														// ����β����
		uint16	index;														// ��ǰ����������������֡�е�������
		BOOL	txBusy;														// ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	}tx;

	// �������ݽӿ�
	BOOL (*sendDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	uint32 ageFlag;
	BOOL isTimeCheck;
	BLE_PROTOCOL_DEVICE DeviceStyle;
	BOOL bmsState;
	BOOL colAState;
	BOOL colBState;
}BLE_PROTOCOL_CB;

extern BLE_PROTOCOL_CB bleProtocolCB;		

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// Э���ʼ��
void BLE_PROTOCOL_Init(void);

// Э�����̴���
void BLE_PROTOCOL_Process(void);

// ��������֡�������������
void BLE_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void BLE_PROTOCOL_TxAddFrame(void);

// �Ǳ������Ϣ�ϱ�
void BLE_PROTOCOL_TxErrorInfo(uint32 param);

// �Ǳ�����ͬ����ʷ�����ϱ�
void BLE_PROTOCOL_TxStartHistoryData(uint32 param);

// �Ǳ��������ظ�
void BLE_PROTOCOL_SendCmdControlAck(uint8 controlCmd, uint8 paramH, uint8 paramL);

// ���ͻ�ȡMAC��ַ
void BLE_PROTOCOL_SendGetMacAddr(uint32 param);

// �Ǳ�λ��Ϣ�ϱ�
void BLE_PROTOCOL_SendAssistInfor(uint8 param);

// �Ǳ�״̬��Ϣ�ظ�
void BLE_PROTOCOL_SendCmdStatusAck(void);

// ���͹㲥���ݰ�
void BLE_PROTOCOL_SendAdv(uint32 param);
#endif

