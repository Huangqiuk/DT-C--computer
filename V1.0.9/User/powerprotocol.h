#ifndef __POWER_PROTOCOL_H__
#define __POWER_PROTOCOL_H__

#include "common.h"

//=====================================================================================================

#define POWER_PROTOCOL_RX_QUEUE_SIZE 3 // ����������гߴ�
#define POWER_PROTOCOL_TX_QUEUE_SIZE 3 // ����������гߴ�

#define POWER_PROTOCOL_CMD_HEAD 0x3A // ����ͷ
#define POWER_PROTOCOL_CMD_DEVICE_ADDR 0x1A // �豸��ַ

#define POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // ���������֡����
#define POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // ����������֡����
#define POWER_PROTOCOL_RX_FIFO_SIZE 200			 // ����һ����������С
#define POWER_PROTOCOL_CMD_FRAME_LENGTH_MIN 8		 // ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У���L��У���H��������ʶ0xD��������ʶOxA

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define POWER_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / POWER_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)	   // ��λ:ms
#define POWER_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / POWER_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // ��λ:ms
// UART����Э�鶨��
typedef enum
{
	POWER_PROTOCOL_CMD_HEAD_INDEX = 0, // ֡ͷ����
	POWER_PROTOCOL_CMD_DEVICE_ADDR_INDEX,  // �豸��ַ����
	POWER_PROTOCOL_CMD_CMD_INDEX,		 // ����������
	POWER_PROTOCOL_CMD_LENGTH_INDEX,	 // ���ݳ�������

	POWER_PROTOCOL_CMD_DATA1_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA2_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA3_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA4_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA5_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA6_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA7_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA8_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA9_INDEX,  // ��������
	POWER_PROTOCOL_CMD_DATA10_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA11_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA12_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA13_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA14_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA15_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA16_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA17_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA18_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA19_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA20_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA21_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA22_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA23_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA24_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA25_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA26_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA27_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA28_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA29_INDEX, // ��������
	POWER_PROTOCOL_CMD_DATA30_INDEX, // ��������

	POWER_PROTOCOL_CMD_INDEX_MAX
} POWER_PROTOCOL_DATE_FRAME;

// ��������
typedef enum
{

	POWER_PROTOCOL_CMD_NULL = 0,						      // ������
	POWER_PROTOCOL_CMD_CHECK_EXTERNAL_POWER = 0x01,           // ����Ƿ�����ӵ�Դ����
	POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_VOLTAGE = 0x02,       // �Ǳ����ѹ��ȡ
	POWER_PROTOCOL_CMD_GET_POWER_SUPPLY_CURRENT = 0x03,       // �Ǳ��������ȡ
	POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SUPPLY_VOLTAGE = 0x04,// ����DUT�����ѹ
	POWER_PROTOCOL_CMD_ADJUST_DUT_POWER_SWITCH = 0x05,        // DUT��Դ����
	POWER_PROTOCOL_CMD_BLUETOOTH_CONNECT = 0x06,              // ���� MAC ��ַ��ȡ
	
    // IAPָ��
    POWER_PROTOCOL_CMD_GET_SOFTWARE_INFO = 0x0A,             // ��ȡPOWER�����Ϣ   
	POWER_PROTOCOL_CMD_ECO_APP2_ERASE = 0x0B, 				 // APP2���ݲ���
	POWER_PROTOCOL_CMD_ECO_APP2_WRITE = 0x0C, 				 // APP2����д��
	POWER_PROTOCOL_CMD_ECO_APP_WRITE_FINISH = 0x0D, 		 // APP2�������    
	POWER_PROTOCOL_CMD_RST = 0x0E,						     // ��������
    POWER_PROTOCOL_CMD_UP_FINISH = 0x0F,					 // �����ɹ��ϱ�

	POWER_PROTOCOL_CMD_MAX									  // ��������

} POWER_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
	uint16 deviceID;
	uint8 buff[POWER_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // ����֡������
	uint16 length;									  // ����֡��Ч���ݸ���
} POWER_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16 deviceID;
	uint8 buff[POWER_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
	uint16 length;									  // ����֡��Ч���ݸ���
} POWER_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8 buff[POWER_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16 head;
		volatile uint16 end;
		uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
	} rxFIFO;

	// ����֡���������ݽṹ
	struct
	{
		POWER_PROTOCOL_RX_CMD_FRAME cmdQueue[POWER_PROTOCOL_RX_QUEUE_SIZE];
		uint16 head; // ����ͷ����
		uint16 end;	 // ����β����
	} rx;

	// ����֡���������ݽṹ
	struct
	{
		POWER_PROTOCOL_TX_CMD_FRAME cmdQueue[POWER_PROTOCOL_TX_QUEUE_SIZE];
		uint16 head;  // ����ͷ����
		uint16 end;	  // ����β����
		uint16 index; // ��ǰ����������������֡�е�������
		BOOL txBusy;  // ��������ΪTRUEʱ���Ӵ����Ͷ�����ȡ��һ��BYTE���뷢�ͼĴ���
	} tx;

	// �������ݽӿ�
	BOOL(*sendDataThrowService)
	(uint16 id, uint8 *pData, uint16 length);

	uint32 ageFlag;
	BOOL isTimeCheck;
} POWER_PROTOCOL_CB;

extern POWER_PROTOCOL_CB POWERProtocolCB;

/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// Э���ʼ��
void POWER_PROTOCOL_Init(void);

// Э�����̴���
void POWER_PROTOCOL_Process(void);

// ��������֡�������������
void POWER_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void POWER_PROTOCOL_TxAddFrame(void);

// ��������ظ�
void POWER_PROTOCOL_SendCmdAck(uint8 ackCmd);

// ���ʹ�Ӧ��Ļظ�
void POWER_PROTOCOL_SendCmdParamAck(uint8 ackCmd, uint8 ackParam);

/******************************************************************************
 * ���ⲿ���ݻ�ȡ��
 ******************************************************************************/
extern uint8 switch_cnt;

typedef union
{
    uint8_t value[4];
    uint32_t CURRENT;
} VOUT_CURRENT;

#endif
