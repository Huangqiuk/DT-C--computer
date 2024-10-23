#ifndef __BLE_HOST_UART_H__
#define __BLE_HOST_UART_H__

#include "common.h"

#define BLE_HOST_UART_TX_MODE							BLE_HOST_UART_INTERRUPT_TX_MODE	// ѡ���������ͻ����жϷ�������
#define BLE_HOST_UART_TYPE_DEF							UART2							// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define BLE_HOST_UART_IRQn_DEF							UART2_IRQn 						// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define BLE_HOST_UART_BAUD_RATE							115200							// ͨѶ������

#define BLE_BRTS_TX_REQUEST() 		
#define BLE_BRTS_TX_RELEASE()	

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define BLE_HOST_UART_BLOCKING_TX_MODE							0U
#define BLE_HOST_UART_INTERRUPT_TX_MODE							1U
// �ߴ�
#define BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE						8			// �����㷢������֡�ߴ�
#define BLE_HOST_UART_DRIVE_CMD_FRAME_LENGTH_MAX				150			// ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16	deviceID;
	volatile uint8	buff[BLE_HOST_UART_DRIVE_CMD_FRAME_LENGTH_MAX];	// ����֡������ 
	volatile uint16	length;										// ����֡��Ч���ݸ���
}BLE_HOST_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);
	
	// ����֡���������ݽṹ
	struct{
		BLE_HOST_UART_DRIVE_CMD_FRAME cmdQueue[BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE];
		volatile uint16 head;						// ����ͷ����
		volatile uint16 end; 						// ����β����
		volatile uint16 index;						// ��ǰ����������������֡�е�������
		volatile BOOL txBusy;						// ��������
	}tx;

	// ����
	struct
	{
		BOOL startFlag;
		
		uint8 fifoBuff[50];
		uint8 macBuff[20];
		uint8 index;
	}rx;

	BOOL rcvBleMacOK;
}BLE_HOST_UART_CB;

extern BLE_HOST_UART_CB bleHostUartCB;						// UART���ݽṹ
extern uint8 callringBuff[];
/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// UART��ʼ��
void BLE_HOST_UART_Init(void);

// UARTģ�鴦�����
void BLE_HOST_UART_Process(void);

// ע�������׳��ӿڷ���
void BLE_HOST_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL BLE_HOST_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ��������һ���ֽ�����
void BLE_HOST_UART_BC_SendData(uint8 data);

#endif

