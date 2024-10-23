#ifndef __ARM_PROTOCOL_H__
#define __ARM_PROTOCOL_H__

#include "common.h"

//=====================================================================================================

#define ARM_PROTOCOL_RX_QUEUE_SIZE 5 // ����������гߴ�
#define ARM_PROTOCOL_TX_QUEUE_SIZE 5 // ����������гߴ�

#define ARM_PROTOCOL_CMD_HEAD 0x55 // ����ͷ

#define ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX 150 // ���������֡����
#define ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX 150 // ����������֡����
#define ARM_PROTOCOL_RX_FIFO_SIZE 300			 // ����һ����������С
#define ARM_PROTOCOL_CMD_FRAME_LENGTH_MIN 4		 // ����֡��С���ȣ�����:����ͷ�������֡����ݳ��ȡ�У���

// ͨѶ��ʱʱ�䣬���ݲ����ʺ������ȷ����ͬʱ����������Ϊ2��
#define ARM_PROTOCOL_BUS_UNIDIRECTIONAL_TIME_OUT (uint32)((1000.0 / ARM_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME)	   // ��λ:ms
#define ARM_PROTOCOL_BUS_BIDIRECTIONAL_TIME_OUT (uint32)(((1000.0 / ARM_UART_BAUD_RATE * 10 * BLE_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX * 2.0 + 0.9) + 2 * TIMER_TIME) * 2) // ��λ:ms

// UART����Э�鶨��
typedef enum
{
	ARM_PROTOCOL_CMD_HEAD_INDEX = 0, // ֡ͷ����
	ARM_PROTOCOL_CMD_CMD_INDEX,		 // ����������
	ARM_PROTOCOL_CMD_LENGTH_INDEX,	 // ���ݳ�������

	ARM_PROTOCOL_CMD_DATA1_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA2_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA3_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA4_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA5_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA6_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA7_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA8_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA9_INDEX,  // ��������
	ARM_PROTOCOL_CMD_DATA10_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA11_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA12_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA13_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA14_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA15_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA16_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA17_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA18_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA19_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA20_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA21_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA22_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA23_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA24_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA25_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA26_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA27_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA28_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA29_INDEX, // ��������
	ARM_PROTOCOL_CMD_DATA30_INDEX, // ��������

	ARM_PROTOCOL_CMD_INDEX_MAX
} ARM_PROTOCOL_DATE_FRAME;

// ��������
typedef enum
{

	UART_ARM_CMD_GET_TOOL_INFO = 0x10,	   // ��ȡ��¼��������Ϣ
	UART_ARM_CMD_TOOL_UP_BEGIN = 0x11,	   // ��¼������������ʼ
	UART_ARM_CMD_TOOL_WRITE_DATA = 0x12,   // ��¼����������д��
	UART_ARM_CMD_TOOL_UP_END = 0x13,	   // ��¼��������������
	UART_ARM_CMD_TOOL_DUT_UP = 0x14,	   // ����DUT��������
	UART_ARM_CMD_TOOL_DUT_PROCESS = 0x15,  // DUT���������ϱ�
	UART_ARM_CMD_TOOL_CLEAR_BUFF = 0x16,   // �������������
	UART_ARM_CMD_TOOL_SET_DUT_INFO = 0x17, // ��¼��д��DUT������Ϣ
	UART_ARM_CMD_TOOL_GET_DUT_INFO = 0x18, // ��ȡ��¼����¼��DUT������Ϣ
	UART_ARM_CMD_SET_TOOL_RANK = 0x19,	   // д����¼�����������Ϣ
	UART_ARM_CMD_GET_TOOL_RANK = 0x1A,	   // ��ȡ��¼�����������Ϣ
	UART_ARM_CMD_PROMPT_THE_USER = 0x1B,   // �û���ʾ��Ϣ

	UART_DUT_CMD_UP_CONFIG = 0x30, // д��DUT������������=0x30
	UART_DUT_CMD_UP_UI = 0x31,	   // UI����д�루���У�������=0x31��
	UART_DUT_CMD_UP_BOOT = 0x32,   // BOOT����д�루���У�������=0x32��
	UART_DUT_CMD_UP_APP = 0x33,	   // APP����д�루������=0x33��
	UART_DUT_CMD_UP_END = 0x34,	   // �����������������=0x34��
	UART_DUT_CMD_UP_UI_VER = 0x35, // UI�汾д�루������=0x35��

	ARM_CMD_MAX,

} ARM_PROTOCOL_CMD;

// UART_RX����֡����
typedef struct
{
	uint16 deviceID;
	uint8 buff[ARM_PROTOCOL_RX_CMD_FRAME_LENGTH_MAX]; // ����֡������
	uint16 length;									  // ����֡��Ч���ݸ���
} ARM_PROTOCOL_RX_CMD_FRAME;

// UART_TX����֡����
typedef struct
{
	uint16 deviceID;
	uint8 buff[ARM_PROTOCOL_TX_CMD_FRAME_LENGTH_MAX]; // ����֡������
	uint16 length;									  // ����֡��Ч���ݸ���
} ARM_PROTOCOL_TX_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// һ�����ջ�����
	struct
	{
		volatile uint8 buff[ARM_PROTOCOL_RX_FIFO_SIZE];
		volatile uint16 head;
		volatile uint16 end;
		uint16 currentProcessIndex; // ��ǰ��������ֽڵ�λ���±�
	} rxFIFO;

	// ����֡���������ݽṹ
	struct
	{
		ARM_PROTOCOL_RX_CMD_FRAME cmdQueue[ARM_PROTOCOL_RX_QUEUE_SIZE];
		uint16 head; // ����ͷ����
		uint16 end;	 // ����β����
	} rx;

	// ����֡���������ݽṹ
	struct
	{
		ARM_PROTOCOL_TX_CMD_FRAME cmdQueue[ARM_PROTOCOL_TX_QUEUE_SIZE];
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
} ARM_PROTOCOL_CB;

extern ARM_PROTOCOL_CB armProtocolCB;

/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// Э���ʼ��
void ARM_PROTOCOL_Init(void);

// Э�����̴���
void ARM_PROTOCOL_Process(void);

// ��������֡�������������
void ARM_PROTOCOL_TxAddData(uint8 data);

// �������ͣ��������Զ�У����������֡�����ݳ��ȣ�����������У����
void ARM_PROTOCOL_TxAddFrame(void);

#endif
