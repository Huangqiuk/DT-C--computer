#include "common.h"
#include "system.h"
#include "timer.h"
#include "ArmUart.h"
// #include "Param.h"
#include "ArmProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/
// UART��ʼ��
void ARM_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void ARM_UART_DataStructureInit(ARM_UART_CB *pCB);

// ���ʹ���
void ARM_UART_TxProcess(ARM_UART_CB *pCB);

// �����ж��ֽڷ���
void ARM_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void ARM_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART0_IRQHandler(void);

// BLEģ�鸨����������
void ARM_CMD_GPIO_CtrlPin(void);

// ȫ�ֱ�������
ARM_UART_CB armUartCB;

// UART��ʼ��
void ARM_UART_HwInit(uint32 baud)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART1);
	usart_deinit(ARM_UART_TYPE_DEF); // ��λ����

	// ��������
	usart_baudrate_set(ARM_UART_TYPE_DEF, baud);						  // ������
	usart_word_length_set(ARM_UART_TYPE_DEF, USART_WL_8BIT);			  // 8λ����λ
	usart_stop_bit_set(ARM_UART_TYPE_DEF, USART_STB_1BIT);				  // һ��ֹͣλ
	usart_parity_config(ARM_UART_TYPE_DEF, USART_PM_NONE);				  // ����żУ��
	usart_hardware_flow_rts_config(ARM_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(ARM_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(ARM_UART_TYPE_DEF, USART_TRANSMIT_ENABLE); // ʹ�ܷ���
	usart_receive_config(ARM_UART_TYPE_DEF, USART_RECEIVE_ENABLE);	 // ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(ARM_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(ARM_UART_TYPE_DEF, USART_INT_RBNE); // �����ж�

	usart_enable(ARM_UART_TYPE_DEF); // ʹ�ܴ���
}

// ���ݽṹ��ʼ��
void ARM_UART_DataStructureInit(ARM_UART_CB *pCB)
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
	for (i = 0; i < ARM_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	armUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void ARM_UART_TxProcess(ARM_UART_CB *pCB)
{
// �жϷ�ʽ
#if (ARM_UART_TX_MODE == ARM_UART_INTERRUPT_TX_MODE)
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
		ARM_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

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
		ARM_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void ARM_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(ARM_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(ARM_UART_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void ARM_UART_BC_SendData(uint8 data)
{
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�)
	usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC);

	// �������
	usart_data_transmit(ARM_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while (usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_TC) != SET)
		;
}

// ����lock Ӧ��
void BLE_PROTOCOL_SendLockOk(uint32 param)
{
	uint8 bleMacStr[] = "TTM:LOCK-OK";

	// ʹ��BLE����
	ARM_BRTS_TX_REQUEST();

	// ��������������ӵ�����㻺����
	ARM_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
uint8 armRxBuff[200] = {0};
uint8 callringBuff[20] = {0};
uint8 bleIndex = 0;

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
void USART1_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// ��������
		rxdata = (uint8)usart_data_receive(ARM_UART_TYPE_DEF);

		if (NULL != armUartCB.receiveDataThrowService)
		{
			(*armUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}

		if (!armUartCB.rcvBleMacOK)
		{
			//			BLE_UART_AtChannelDataProcess(rxdata);
		}

		//=================================================================
		// ���������ж�
		if ((bleIndex + 1) < sizeof(armRxBuff) / sizeof(armRxBuff[0]))
		{
			armRxBuff[bleIndex++] = rxdata;
			armRxBuff[bleIndex] = '\0';
		}
	}

// �ж�ģʽ����
#if (ARM_UART_TX_MODE == ARM_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = armUartCB.tx.head;
		uint16 end;
		uint16 index = armUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < armUartCB.tx.cmdQueue[head].length)
		{
			txdata = armUartCB.tx.cmdQueue[head].buff[armUartCB.tx.index++];

			// �������
			usart_data_transmit(ARM_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			armUartCB.tx.cmdQueue[head].length = 0;
			armUartCB.tx.head++;
			armUartCB.tx.head %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
			armUartCB.tx.index = 0;

			head = armUartCB.tx.head;
			end = armUartCB.tx.end;

			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = armUartCB.tx.cmdQueue[head].buff[armUartCB.tx.index++];

				// �������
				usart_data_transmit(ARM_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				usart_interrupt_disable(ARM_UART_TYPE_DEF, USART_INT_TC);

				armUartCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(ARM_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж�
	{
		usart_flag_get(ARM_UART_TYPE_DEF, USART_FLAG_ORERR); 							//----------------------- �����������жϱ�־λ
		usart_data_receive(ARM_UART_TYPE_DEF);				 							//----------------------- ��ռĴ���
	}
}

// UART��ʼ��
void ARM_UART_Init(void)
{
	// ARM���ڳ�ʼ��
	ARM_UART_HwInit(ARM_UART_BAUD_RATE);

	// ARM���ݽṹ��ʼ��
	ARM_UART_DataStructureInit(&armUartCB);
}

// UARTģ�鴦�����
void ARM_UART_Process(void)
{
	// ���ʹ���
	ARM_UART_TxProcess(&armUartCB);
}

// ע�������׳��ӿڷ���
void ARM_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	armUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL ARM_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = armUartCB.tx.head;
	uint16 end = armUartCB.tx.end;

	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if ((end + 1) % ARM_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	armUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		armUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	armUartCB.tx.cmdQueue[end].length = length;

	// ���ͻ��ζ��и���λ��
	armUartCB.tx.end++;
	armUartCB.tx.end %= ARM_UART_DRIVE_TX_QUEUE_SIZE;
	armUartCB.tx.cmdQueue[armUartCB.tx.end].length = 0;

	return TRUE;
}

// ARM��λ
void ARM_MODULE_Reset(uint32 param)
{
	// ARM��λ
	ARM_RST_RESET();

	// �ͷŸ�λ
	ARM_RST_RELEASE();

	ARM_EN_ENABLE();
}
