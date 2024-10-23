#ifndef __STS_DRIVE_H__
#define __STS_DRIVE_H__

#include "common.h"

#define STS_UART_TX_MODE STS_UART_INTERRUPT_TX_MODE // ѡ���������ͻ����жϷ�������
#define STS_UART_TYPE_DEF USART1					// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define STS_UART_IRQn_DEF USART1_IRQn				// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define STS_UART_BAUD_RATE 115200 					// ͨѶ������

//#define STS_RST_RESET() gpio_bit_reset(GPIOA, GPIO_PIN_12)
//#define STS_RST_RELEASE() gpio_bit_set(GPIOA, GPIO_PIN_12)
//#define STS_EN_ENABLE() gpio_bit_reset(GPIOA, GPIO_PIN_11) // ��ʼ�㲥
//#define STS_EN_DISABLE() gpio_bit_set(GPIOA, GPIO_PIN_11)  // ��ȫ˯��״̬

//#define STS_BRTS_TX_REQUEST()
//#define STS_BRTS_TX_RELEASE() gpio_bit_set(GPIOC, GPIO_PIN_12)

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define STS_UART_BLOCKING_TX_MODE 0US
#define STS_UART_INTERRUPT_TX_MODE 1U
// �ߴ�
#define STS_UART_DRIVE_TX_QUEUE_SIZE 8			// �����㷢������֡�ߴ�
#define STS_UART_DRIVE_CMD_FRAME_LENGTH_MAX 150 // ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16 deviceID;
	volatile uint8 buff[STS_UART_DRIVE_CMD_FRAME_LENGTH_MAX]; // ����֡������
	volatile uint16 length;									  // ����֡��Ч���ݸ���
} STS_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	// ����֡���������ݽṹ
	struct
	{
		STS_UART_DRIVE_CMD_FRAME cmdQueue[STS_UART_DRIVE_TX_QUEUE_SIZE];
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
} STS_UART_CB;

extern STS_UART_CB STSUartCB; // UART���ݽṹ
//extern uint8 callringBuff[];
/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// UART��ʼ��
void STS_UART_Init(void);

// UARTģ�鴦�����
void STS_UART_Process(void);

// ע�������׳��ӿڷ���
void STS_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL STS_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ����ģ�鸴λ
void STS_MODULE_Reset(uint32 param);

// ��������һ���ֽ�����
void STS_UART_BC_SendData(uint8 data);

#endif
