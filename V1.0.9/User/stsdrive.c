#include "common.h"
#include "system.h"
#include "timer.h"
#include "StsDrive.h"
#include "DutInfo.h"
#include "StsProtocol.h"

/******************************************************************************
 * ���ڲ��ӿ�������
 ******************************************************************************/
// UART��ʼ��
void STS_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void STS_UART_DataStructureInit(STS_UART_CB *pCB);

// ���ʹ���
void STS_UART_TxProcess(STS_UART_CB *pCB);

// �����ж��ֽڷ���
void STS_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
void STS_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART1_IRQHandler(void);

// BLEģ�鸨����������
//void STS_CMD_GPIO_CtrlPin(void);

// ȫ�ֱ�������
STS_UART_CB STSUartCB;

// UART��ʼ��
void STS_UART_HwInit(uint32 baud)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2); // TX
  
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_3);  //RX

	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART1);
	usart_deinit(STS_UART_TYPE_DEF); // ��λ����

	// ��������
	usart_baudrate_set(STS_UART_TYPE_DEF, baud);						  // ������
	usart_word_length_set(STS_UART_TYPE_DEF, USART_WL_8BIT);			  // 8λ����λ
	usart_stop_bit_set(STS_UART_TYPE_DEF, USART_STB_1BIT);				  // һ��ֹͣλ
	usart_parity_config(STS_UART_TYPE_DEF, USART_PM_NONE);				  // ����żУ��
	usart_hardware_flow_rts_config(STS_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(STS_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(STS_UART_TYPE_DEF, USART_TRANSMIT_ENABLE); // ʹ�ܷ���
	usart_receive_config(STS_UART_TYPE_DEF, USART_RECEIVE_ENABLE);	 // ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(STS_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(STS_UART_TYPE_DEF, USART_INT_RBNE); // �����ж�

	usart_enable(STS_UART_TYPE_DEF); // ʹ�ܴ���
}

// ���ݽṹ��ʼ��
void STS_UART_DataStructureInit(STS_UART_CB *pCB)
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
	for (i = 0; i < STS_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}

	STSUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void STS_UART_TxProcess(STS_UART_CB *pCB)
{
// �жϷ�ʽ
#if (STS_UART_TX_MODE == STS_UART_INTERRUPT_TX_MODE)
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
		STS_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

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
		STS_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);

		index = pCB->tx.index;
	}

	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head++;
	pCB->tx.head %= STS_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void STS_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	usart_flag_get(STS_UART_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(STS_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(STS_UART_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void STS_UART_BC_SendData(uint8 data)
{
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�)
	usart_flag_get(STS_UART_TYPE_DEF, USART_FLAG_TC);

	// �������
	usart_data_transmit(STS_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while (usart_flag_get(STS_UART_TYPE_DEF, USART_FLAG_TC) != SET);
}

// ����lock Ӧ��
//void BLE_PROTOCOL_SendLockOk(uint32 param)
//{
//	uint8 bleMacStr[] = "TTM:LOCK-OK";
//
//	// ʹ��BLE����
//	STS_BRTS_TX_REQUEST();
//
//	// ��������������ӵ�����㻺����
//	STS_UART_AddTxArray(0xFFFF, bleMacStr, sizeof(bleMacStr));
//}

/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None
 * @Notes
 *************************************************************/
uint8 STSRxBuff[200] = {0};
//uint8 callringBuff[20] = {0};
//uint8 bleIndex = 0;

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
	if (usart_interrupt_flag_get(STS_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET)
	{
		uint8 rxdata = 0x00;

		// ��������
		rxdata = (uint8)usart_data_receive(STS_UART_TYPE_DEF);

		if (NULL != STSUartCB.receiveDataThrowService)
		{
////             dut_info.passThroughStsToDut = TRUE;
			(*STSUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}

		if (!STSUartCB.rcvBleMacOK)
		{
			//			BLE_UART_AtChannelDataProcess(rxdata);
		}

		//=================================================================
		// ���������ж�
//		if ((bleIndex + 1) < sizeof(STSRxBuff) / sizeof(STSRxBuff[0]))
//		{
//			STSRxBuff[bleIndex++] = rxdata;
//			STSRxBuff[bleIndex] = '\0';
//		}
	}

// �ж�ģʽ����
#if (STS_UART_TX_MODE == STS_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (usart_interrupt_flag_get(STS_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = STSUartCB.tx.head;
		uint16 end;
		uint16 index = STSUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < STSUartCB.tx.cmdQueue[head].length)
		{
			txdata = STSUartCB.tx.cmdQueue[head].buff[STSUartCB.tx.index++];

			// �������
			usart_data_transmit(STS_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			STSUartCB.tx.cmdQueue[head].length = 0;
			STSUartCB.tx.head++;
			STSUartCB.tx.head %= STS_UART_DRIVE_TX_QUEUE_SIZE;
			STSUartCB.tx.index = 0;

			head = STSUartCB.tx.head;
			end = STSUartCB.tx.end;

			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = STSUartCB.tx.cmdQueue[head].buff[STSUartCB.tx.index++];

				// �������
				usart_data_transmit(STS_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				usart_interrupt_disable(STS_UART_TYPE_DEF, USART_INT_TC);

				STSUartCB.tx.txBusy = FALSE;
			}
		}
	}
#endif

	// Other USARTx interrupts handler can go here ...
	if (usart_interrupt_flag_get(STS_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET) //----------------------- ��������ж�
	{
		usart_flag_get(STS_UART_TYPE_DEF, USART_FLAG_ORERR); 							//----------------------- �����������жϱ�־λ
		usart_data_receive(STS_UART_TYPE_DEF);				 							//----------------------- ��ռĴ���
	}
}

// UART��ʼ��
void STS_UART_Init(void)
{
	// STS���ڳ�ʼ��
	STS_UART_HwInit(STS_UART_BAUD_RATE);

	// STS���ݽṹ��ʼ��
	STS_UART_DataStructureInit(&STSUartCB);
}

// UARTģ�鴦�����
void STS_UART_Process(void)
{
	// ���ʹ���
	STS_UART_TxProcess(&STSUartCB);
}

// ע�������׳��ӿڷ���
void STS_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	STSUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL STS_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = STSUartCB.tx.head;
	uint16 end = STSUartCB.tx.end;

	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if ((end + 1) % STS_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	STSUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		STSUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	STSUartCB.tx.cmdQueue[end].length = length;

	// ���ͻ��ζ��и���λ��
	STSUartCB.tx.end++;
	STSUartCB.tx.end %= STS_UART_DRIVE_TX_QUEUE_SIZE;
	STSUartCB.tx.cmdQueue[STSUartCB.tx.end].length = 0;

	return TRUE;
}

// STS��λ
//void STS_MODULE_Reset(uint32 param)
//{
//	// STS��λ
//	STS_RST_RESET();
//
//	// �ͷŸ�λ
//	STS_RST_RELEASE();
//
//	STS_EN_ENABLE();
//}
