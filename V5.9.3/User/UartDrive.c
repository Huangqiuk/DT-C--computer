#include "uartDrive.h"
// #include "param.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/

// ���ݽṹ��ʼ��
void UART_DRIVE_DataStructInit(UART_DRIVE_CB *pCB);

// UART��ʼ��
void UART_DRIVE_HwInit(uint32 baud);

// ���ʹ���
void UART_DRIVE_TxProcess(UART_DRIVE_CB *pCB);

// �����ж��ֽڷ���
void UART_DRIVE_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void UART_DRIVE_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART1_IRQHandler(void);

// ȫ�ֱ�������
UART_DRIVE_CB uartDriveCB;

// ��������������������������������������������������������������������������������������
// UART��ʼ��
void UART_DRIVE_Init(void)
{
	// Ӳ��UART����
	UART_DRIVE_HwInit(UART_DRIVE_BAUD_RATE);

	// UART���ݽṹ��ʼ��
	UART_DRIVE_DataStructInit(&uartDriveCB);
}

void UART_DRIVE_InitSelect(uint32 baud)
{
	// Ӳ��UART����
	UART_DRIVE_HwInit(baud);

	// UART���ݽṹ��ʼ��
	UART_DRIVE_DataStructInit(&uartDriveCB);
}

// UARTģ�鴦�����
void UART_DRIVE_Process(void)
{
	// �ڲ�ͨѶ�ӿڵķ��ʹ���
	UART_DRIVE_TxProcess(&uartDriveCB);
}

// ��Э���ע�����ݽ��սӿ�
void UART_DRIVE_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	uartDriveCB.receiveDataThrowService = service;
}

// ���ݽṹ��ʼ��
void UART_DRIVE_DataStructInit(UART_DRIVE_CB *pCB)
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
	for (i = 0; i < UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}
}

// UART��ʼ��
void UART_DRIVE_HwInit(uint32 baud)
{
	// GD32
	// ʱ������
	rcu_periph_clock_enable(RCU_GPIOA);

	// GPIO����
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9); // ��������

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_10); // ��������

	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART0); // ʱ��USARTʱ��
	usart_deinit(UART_DRIVE_TYPE_DEF);	 // ��λ����

	// ʱ������
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14);

	// ���ڵ�ƽ�л� 3.3/5V
	//	if (PARAM_GetUartLevel())
	//	{
	//		gpio_bit_reset(GPIOB, GPIO_PIN_14);
	//	}
	//	else
	//	{
	//		gpio_bit_set(GPIOB, GPIO_PIN_14);
	//	}

	// ��������
	usart_baudrate_set(UART_DRIVE_TYPE_DEF, baud);							// ������
	usart_word_length_set(UART_DRIVE_TYPE_DEF, USART_WL_8BIT);				// 8λ����λ
	usart_stop_bit_set(UART_DRIVE_TYPE_DEF, USART_STB_1BIT);				// һ��ֹͣλ
	usart_parity_config(UART_DRIVE_TYPE_DEF, USART_PM_NONE);				// ����żУ��
	usart_hardware_flow_rts_config(UART_DRIVE_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(UART_DRIVE_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(UART_DRIVE_TYPE_DEF, USART_TRANSMIT_ENABLE); // ʹ�ܷ���
	usart_receive_config(UART_DRIVE_TYPE_DEF, USART_RECEIVE_ENABLE);   // ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(UART_DRIVE_IRQn_DEF, 2, 1);

	usart_interrupt_enable(UART_DRIVE_TYPE_DEF, USART_INT_RBNE); // �����ж�

	usart_enable(UART_DRIVE_TYPE_DEF); // ʹ�ܴ���
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ���ڽ��պ��������ڵ��жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART0_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// ��������
		rxdata = (uint8)usart_data_receive(UART_DRIVE_TYPE_DEF);

		if (NULL != uartDriveCB.receiveDataThrowService)
		{
			(*uartDriveCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
	}

// �ж�ģʽ����
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = uartDriveCB.tx.head;
		uint16 end;
		uint16 index = uartDriveCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < uartDriveCB.tx.cmdQueue[head].length)
		{
			txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

			// �������
			usart_data_transmit(UART_DRIVE_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			uartDriveCB.tx.cmdQueue[head].length = 0;
			uartDriveCB.tx.head++;
			uartDriveCB.tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
			uartDriveCB.tx.index = 0;

			head = uartDriveCB.tx.head;
			end = uartDriveCB.tx.end;

			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = uartDriveCB.tx.cmdQueue[head].buff[uartDriveCB.tx.index++];

				// �������
				usart_data_transmit(UART_DRIVE_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ��Ϳ��ж�
				usart_interrupt_disable(UART_DRIVE_TYPE_DEF, USART_INT_TC);

				uartDriveCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(UART_DRIVE_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж�
	{
		usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_ORERR); //----------------------- �����������жϱ�־λ
		usart_data_receive(UART_DRIVE_TYPE_DEF);			   //----------------------- ��ռĴ���
	}
}

// �����ж��ֽڷ���
void UART_DRIVE_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(UART_DRIVE_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(UART_DRIVE_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void UART_DRIVE_BC_SendData(uint8 data)
{
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�)
	usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC);

	// �������
	usart_data_transmit(UART_DRIVE_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while (usart_flag_get(UART_DRIVE_TYPE_DEF, USART_FLAG_TC) != SET)
		;
}

// ������������
void UART_DRIVE_BC_StartTx(UART_DRIVE_CB *pCB)
{
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
		UART_DRIVE_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
}

// ���ʹ���,���ַ��ͻ������ǿ�ʱ,�����жϷ���
void UART_DRIVE_TxProcess(UART_DRIVE_CB *pCB)
{
// �жϷ�ʽ
#if (UART_DRIVE_TX_MODE == UART_DRIVE_INTERRUPT_TX_MODE)
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
		UART_DRIVE_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}

// ������ʽʱ��������
#else
	UART_DRIVE_BC_StartTx(pCB);
#endif
}

// ���ͻ����������һ������������
BOOL UART_DRIVE_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = uartDriveCB.tx.head;
	uint16 end = uartDriveCB.tx.end;

	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if ((end + 1) % UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	uartDriveCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		uartDriveCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	uartDriveCB.tx.cmdQueue[end].length = length;

	// ���ͻ��ζ��и���λ��
	uartDriveCB.tx.end++;
	uartDriveCB.tx.end %= UART_DRIVE_TX_QUEUE_SIZE;
	uartDriveCB.tx.cmdQueue[uartDriveCB.tx.end].length = 0;

	return TRUE;
}
