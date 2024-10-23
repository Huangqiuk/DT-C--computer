#ifndef __ARM_UART_H__
#define __ARM_UART_H__

#include "common.h"

#define ARM_UART_TX_MODE ARM_UART_INTERRUPT_TX_MODE // ѡ���������ͻ����жϷ�������
#define ARM_UART_TYPE_DEF USART1					// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)
#define ARM_UART_IRQn_DEF USART1_IRQn				// ѡ�񴮿�(�����жϺ�ʱ�Ӷ���Ҫ�޸�)

#define ARM_UART_BAUD_RATE 115200 					// ͨѶ������

#define ARM_RST_RESET() gpio_bit_reset(GPIOA, GPIO_PIN_12)
#define ARM_RST_RELEASE() gpio_bit_set(GPIOA, GPIO_PIN_12)
#define ARM_EN_ENABLE() gpio_bit_reset(GPIOA, GPIO_PIN_11) // ��ʼ�㲥
#define ARM_EN_DISABLE() gpio_bit_set(GPIOA, GPIO_PIN_11)  // ��ȫ˯��״̬

#define ARM_BRTS_TX_REQUEST()
#define ARM_BRTS_TX_RELEASE() gpio_bit_set(GPIOC, GPIO_PIN_12)

// �������������� ���²��ֲ��������޸� ������������������������������
// �жϷ�ʽ
#define ARM_UART_BLOCKING_TX_MODE 0U
#define ARM_UART_INTERRUPT_TX_MODE 1U
// �ߴ�
#define ARM_UART_DRIVE_TX_QUEUE_SIZE 8			// �����㷢������֡�ߴ�
#define ARM_UART_DRIVE_CMD_FRAME_LENGTH_MAX 150 // ����֡��󳤶�

// UART����֡����
typedef struct
{
	volatile uint16 deviceID;
	volatile uint8 buff[ARM_UART_DRIVE_CMD_FRAME_LENGTH_MAX]; // ����֡������
	volatile uint16 length;									  // ����֡��Ч���ݸ���
} ARM_UART_DRIVE_CMD_FRAME;

// UART���ƽṹ�嶨��
typedef struct
{
	// ���������׳��ӿ�
	void (*receiveDataThrowService)(uint16 id, uint8 *pData, uint16 length);

	// ����֡���������ݽṹ
	struct
	{
		ARM_UART_DRIVE_CMD_FRAME cmdQueue[ARM_UART_DRIVE_TX_QUEUE_SIZE];
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
} ARM_UART_CB;

extern ARM_UART_CB armUartCB; // UART���ݽṹ
extern uint8 callringBuff[];
/******************************************************************************
 * ���ⲿ�ӿ�������
 ******************************************************************************/

// UART��ʼ��
void ARM_UART_Init(void);

// UARTģ�鴦�����
void ARM_UART_Process(void);

// ע�������׳��ӿڷ���
void ARM_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length));

// ���ͻ����������һ������������
BOOL ARM_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length);

// ����ģ�鸴λ
void ARM_MODULE_Reset(uint32 param);

// ��������һ���ֽ�����
void ARM_UART_BC_SendData(uint8 data);

#endif
