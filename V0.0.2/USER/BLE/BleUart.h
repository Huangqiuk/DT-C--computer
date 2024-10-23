#ifndef __BLE_UART_H__
#define __BLE_UART_H__

#include "common.h"

#define BLE_UART_TX_MODE					BLE_UART_INTERRUPT_TX_MODE			// ѡ���������ͻ����жϷ�������
#define BLE_UART_TYPE_DEF					USART2								// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define BLE_UART_IRQn_DEF					USART2_IRQn 						// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define BLE_UART_BAUD_RATE					9600								// ͨѶ������

#define BLE_RST_RESET() 			
#define BLE_RST_RELEASE()			
#define BLE_EN_ENABLE() 			
#define BLE_EN_DISABLE()		

#define BLE_BRTS_TX_REQUEST() 		
#define BLE_BRTS_TX_RELEASE()		

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define BLE_UART_BLOCKING_TX_MODE							0U
#define BLE_UART_INTERRUPT_TX_MODE							1U
// �ߴ�
#define BLE_UART_DRIVE_TX_QUEUE_SIZE						3			// �����㷢������֡�ߴ�
#define BLE_UART_DRIVE_CMD_FRAME_LENGTH_MAX					150			// ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16	deviceID;
	volatile uint8	buff[BLE_UART_DRIVE_CMD_FRAME_LENGTH_MAX];	// ����֡������ 
	volatile uint16	length;										// ����֡��Ч���ݸ���
}BLE_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);
	
	// ����֡���������ݽṹ
	struct{
		BLE_UART_DRIVE_CMD_FRAME cmdQueue[BLE_UART_DRIVE_TX_QUEUE_SIZE];
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
}BLE_UART_CB;

extern BLE_UART_CB bleUartCB;						// UART���ݽṹ

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/

// UART��ʼ��
void BLE_UART_Init(void);

// UARTģ�鴦�����
void BLE_UART_Process(void);

// ע�������׳��ӿڷ���
void BLE_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL BLE_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ����ģ�鸴λ
void BLE_MODULE_Reset(uint32 param);

// ��������һ���ֽ�����
void BLE_UART_BC_SendData(uint8 data);

#endif

