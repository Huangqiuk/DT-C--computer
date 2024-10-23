#ifndef __POWER_DRIVE_H__
#define __POWER_DRIVE_H__

#include "common.h"

#define POWER_UART_TX_MODE POWER_UART_INTERRUPT_TX_MODE // ѡ���������ͻ����жϷ�������
#define POWER_UART_TYPE_DEF USART2				// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define POWER_UART_IRQn_DEF USART2_IRQn				// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define POWER_UART_BAUD_RATE 115200 					// ͨѶ������

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define POWER_UART_BLOCKING_TX_MODE 0US
#define POWER_UART_INTERRUPT_TX_MODE 1U
// �ߴ�
#define POWER_UART_DRIVE_TX_QUEUE_SIZE 8			// �����㷢������֡�ߴ�
#define POWER_UART_DRIVE_CMD_FRAME_LENGTH_MAX 150 // ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16 deviceID;
	volatile uint8 buff[POWER_UART_DRIVE_CMD_FRAME_LENGTH_MAX]; // ����֡������
	volatile uint16 length;									  // ����֡��Ч���ݸ���
} POWER_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	// ����֡���������ݽṹ
	struct
	{
		POWER_UART_DRIVE_CMD_FRAME cmdQueue[POWER_UART_DRIVE_TX_QUEUE_SIZE];
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
} POWER_UART_CB;

extern POWER_UART_CB POWERUartCB; // UART���ݽṹ
//extern uint8 callringBuff[];
/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// UART��ʼ��
void POWER_UART_Init(void);

// UARTģ�鴦�����
void POWER_UART_Process(void);

// ע�������׳��ӿڷ���
void POWER_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL POWER_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ����ģ�鸴λ
void POWER_MODULE_Reset(uint32 param);

// ��������һ���ֽ�����
void POWER_UART_BC_SendData(uint8 data);

#endif
