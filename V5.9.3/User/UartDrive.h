#ifndef 	__UART_DRIVE_H__
#define 	__UART_DRIVE_H__

/*******************************************************************************
 *                                  ͷ�ļ�	                                   *
********************************************************************************/
#include "common.h"

/*******************************************************************************
 *                                  �궨��	                                   *
********************************************************************************/
#define UART_DRIVE_TX_MODE						UART_DRIVE_INTERRUPT_TX_MODE	// ѡ���������ͻ����жϷ�������
#define UART_DRIVE_TYPE_DEF						USART0							// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define UART_DRIVE_IRQn_DEF						USART0_IRQn 					// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define UART_DRIVE_BAUD_RATE					115200	// ͨѶ������

#define UART_DRIVE_TX_QUEUE_SIZE				8		// �����㷢������֡�ߴ�
#define UART_DRIVE_CMD_FRAME_LENGTH_MAX			150		// ����֡��󳤶�

// �������������� ���²��ֲ��������޸� ������������������������������
#define UART_DRIVE_BLOCKING_TX_MODE				0U
#define UART_DRIVE_INTERRUPT_TX_MODE			1U

// UART����֡����
typedef struct
{
	volatile uint16	deviceID;
	volatile uint8	buff[UART_DRIVE_CMD_FRAME_LENGTH_MAX];	// ����֡������ 
	volatile uint16	length;									// ����֡��Ч���ݸ���
}UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);
	
	// ����֡���������ݽṹ
	struct{
		UART_DRIVE_CMD_FRAME cmdQueue[UART_DRIVE_TX_QUEUE_SIZE];
		volatile uint16 head;						// ����ͷ����
		volatile uint16 end; 						// ����β����
		volatile uint16 index;						// ��ǰ����������������֡�е�������
		volatile BOOL txBusy;						// ��������
	}tx;
}UART_DRIVE_CB;

extern UART_DRIVE_CB uartDriveCB;

/******************************************************************************
* ���ⲿ�ӿ�������
******************************************************************************/
// UART��ʼ������main�Ĵ�ѭ��֮ǰ���ã����ģ���ʼ��
void UART_DRIVE_Init(void);

void UART_DRIVE_InitSelect(uint32 baud);

// UARTģ�鴦����ڣ���main�Ĵ�ѭ���е���
void UART_DRIVE_Process(void);

// ע�������׳��ӿڷ���
void UART_DRIVE_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL UART_DRIVE_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

#endif


