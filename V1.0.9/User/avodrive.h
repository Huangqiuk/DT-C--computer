#ifndef 	__AVO_DRIVE_H__
#define 	__AVO_DRIVE_H__

#include "common.h"

#define AVO_UART_TX_MODE AVO_UART_INTERRUPT_TX_MODE // ѡ���������ͻ����жϷ�������
#define AVO_UART_TYPE_DEF USART0					// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define AVO_UART_IRQn_DEF USART0_IRQn				// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define AVO_UART_BAUD_RATE 9600 					// ͨѶ������

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define AVO_UART_BLOCKING_TX_MODE 0US
#define AVO_UART_INTERRUPT_TX_MODE 1U
// �ߴ�
#define AVO_UART_DRIVE_TX_QUEUE_SIZE 8			// �����㷢������֡�ߴ�
#define AVO_UART_DRIVE_CMD_FRAME_LENGTH_MAX 150 // ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16 deviceID;
	volatile uint8 buff[AVO_UART_DRIVE_CMD_FRAME_LENGTH_MAX]; // ����֡������
	volatile uint16 length;									  // ����֡��Ч���ݸ���
} AVO_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	// ����֡���������ݽṹ
	struct
	{
		AVO_UART_DRIVE_CMD_FRAME cmdQueue[AVO_UART_DRIVE_TX_QUEUE_SIZE];
		volatile uint16 head;  // ����ͷ����
		volatile uint16 end;   // ����β����
		volatile uint16 index; // ��ǰ����������������֡�е�������
		volatile BOOL txBusy;  // ��������
	} tx;

	// ����
	struct
	{
		BOOL startFlag;

		uint8 fifoBuff[50];
		uint8 macBuff[20];
		uint8 index;
	} rx;

	BOOL rcvBleMacOK;
} AVO_UART_CB;

extern AVO_UART_CB AVOUartCB; // UART���ݽṹ
extern uint8 callringBuff[];
/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// UART��ʼ��
void AVO_UART_Init(void);

// UARTģ�鴦�����
void AVO_UART_Process(void);

// ע�������׳��ӿڷ���
void AVO_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL AVO_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ��������һ���ֽ�����
void AVO_UART_BC_SendData(uint8 data);

#endif
