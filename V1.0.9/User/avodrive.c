#include "AvoDrive.h"
#include "AvoProtocol.h"
#include "common.h"
#include "system.h"
#include "timer.h"
#include "DutInfo.h"


/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/
// UART��ʼ��
void AVO_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void AVO_UART_DataStructureInit(AVO_UART_CB *pCB);

// ���ʹ���
void AVO_UART_TxProcess(AVO_UART_CB *pCB);

// �����ж��ֽڷ���
void AVO_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void AVO_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART1_IRQHandler(void);

// ȫ�ֱ�������
AVO_UART_CB AVOUartCB;

// UART��ʼ��
void AVO_UART_HwInit(uint32 baud)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); // TX
  
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_10);  //RX

	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART0);
	usart_deinit(AVO_UART_TYPE_DEF); // ��λ����

	// ��������
	usart_baudrate_set(AVO_UART_TYPE_DEF, baud);						  // ������
	usart_word_length_set(AVO_UART_TYPE_DEF, USART_WL_8BIT);			  // 8λ����λ
	usart_stop_bit_set(AVO_UART_TYPE_DEF, USART_STB_1BIT);				  // һ��ֹͣλ
	usart_parity_config(AVO_UART_TYPE_DEF, USART_PM_NONE);				  // ����żУ��
	usart_hardware_flow_rts_config(AVO_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(AVO_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(AVO_UART_TYPE_DEF, USART_TRANSMIT_ENABLE); // ʹ�ܷ���
	usart_receive_config(AVO_UART_TYPE_DEF, USART_RECEIVE_ENABLE);	 // ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(AVO_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(AVO_UART_TYPE_DEF, USART_INT_RBNE); // �����ж�

	usart_enable(AVO_UART_TYPE_DEF); // ʹ�ܴ���
}

// ���ݽṹ��ʼ��
void AVO_UART_DataStructureInit(AVO_UART_CB *pCB)
{
	uint16 i;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	pCB->tx.txBusy = FALSE;
	pCB->tx.index = 0;
	pCB->tx.head = 0;
	pCB->tx.end = 0;
	for (i = 0; i < AVO_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	AVOUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void AVO_UART_TxProcess(AVO_UART_CB *pCB)
{
// �жϷ�ʽ
#if (AVO_UART_TX_MODE == AVO_UART_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;						   // ��ǰ�������ݵ�������
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length; // ��ǰ���͵�����֡�ĳ���
	uint16 head = pCB->tx.head;							   // ��������֡����ͷ������
	uint16 end = pCB->tx.end;							   // ��������֡����β������

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	// ����Ϊ�գ�������
	if (head == end)
	{
		return;
	}

	// ����æ���˳�
	if (pCB->tx.txBusy)
	{
		return;
	}

	// ����ִ�е����˵�����зǿա���

	// ��ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
	if (index < length)
	{
		AVO_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}

// ������ʽʱ��������
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

	// �����Ϸ��Լ���
	if (NULL == pCB)
	{
		return;
	}

	// ����Ϊ�գ�������
	if (head == end)
	{
		return;
	}

	// ��ǰ����֡δ������ʱ����������
	while (index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		AVO_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= AVO_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void AVO_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
//	usart_flag_get(AVO_UART_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(AVO_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(AVO_UART_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void AVO_UART_BC_SendData(uint8 data)
{
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�)
//	usart_flag_get(AVO_UART_TYPE_DEF, USART_FLAG_TC);

	// �������
	usart_data_transmit(AVO_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while (usart_flag_get(AVO_UART_TYPE_DEF, USART_FLAG_TC) != SET);
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
uint8 AVORxBuff[200] = {0};
uint8 callringBuff[20] = {0};
uint8 bleIndex = 0;

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART0_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (usart_interrupt_flag_get(AVO_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// ��������
		rxdata = (uint8)usart_data_receive(AVO_UART_TYPE_DEF);

		if (NULL != AVOUartCB.receiveDataThrowService)
		{
			(*AVOUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
	}

// �ж�ģʽ����
#if (AVO_UART_TX_MODE == AVO_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (usart_interrupt_flag_get(AVO_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = AVOUartCB.tx.head;
		uint16 end;
		uint16 index = AVOUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < AVOUartCB.tx.cmdQueue[head].length)
		{
			txdata = AVOUartCB.tx.cmdQueue[head].buff[AVOUartCB.tx.index++];

			// �������
			usart_data_transmit(AVO_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			AVOUartCB.tx.cmdQueue[head].length = 0;
			AVOUartCB.tx.head++;
			AVOUartCB.tx.head %= AVO_UART_DRIVE_TX_QUEUE_SIZE;
			AVOUartCB.tx.index = 0;

			head = AVOUartCB.tx.head;
			end = AVOUartCB.tx.end;

			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = AVOUartCB.tx.cmdQueue[head].buff[AVOUartCB.tx.index++];

				// �������
				usart_data_transmit(AVO_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				usart_interrupt_disable(AVO_UART_TYPE_DEF, USART_INT_TC);

				AVOUartCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(AVO_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж�
	{
		usart_flag_get(AVO_UART_TYPE_DEF, USART_FLAG_ORERR); 							//----------------------- �����������жϱ�־λ
		usart_data_receive(AVO_UART_TYPE_DEF);				 							//----------------------- ��ռĴ���
	}
}

// UART��ʼ��
void AVO_UART_Init(void)
{
	// AVO���ڳ�ʼ��
	AVO_UART_HwInit(AVO_UART_BAUD_RATE);

	// AVO���ݽṹ��ʼ��
	AVO_UART_DataStructureInit(&AVOUartCB);
}

// UARTģ�鴦�����
void AVO_UART_Process(void)
{
	// ���ʹ���
	AVO_UART_TxProcess(&AVOUartCB);
}

// ע�������׳��ӿڷ���
void AVO_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	AVOUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL AVO_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = AVOUartCB.tx.head;
	uint16 end = AVOUartCB.tx.end;

	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if ((end + 1) % AVO_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	AVOUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		AVOUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	AVOUartCB.tx.cmdQueue[end].length = length;

	// ���ͻ��ζ��и���λ��
	AVOUartCB.tx.end++;
	AVOUartCB.tx.end %= AVO_UART_DRIVE_TX_QUEUE_SIZE;
	AVOUartCB.tx.cmdQueue[AVOUartCB.tx.end].length = 0;

	return TRUE;
}
