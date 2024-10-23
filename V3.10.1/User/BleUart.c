#include "common.h"
#include "delay.h"
#include "system.h"
#include "timer.h"
#include "BleUart.h"
#include "stringOperation.h"
#include "Param.h"
#include "BleProtocol.h"
#include "Spiflash.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/
// UART��ʼ��
void BLE_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB);

// ���ʹ���
void BLE_UART_TxProcess(BLE_UART_CB* pCB);

// �����ж��ֽڷ���
void BLE_UART_IR_StartSendData(uint8 data);

// ��������һ���ֽ�����
//void BLE_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void USART1_IRQHandler(void);

// BLEģ�鸨����������
void BLE_CMD_GPIO_CtrlPin(void);

// ȫ�ֱ�������
BLE_UART_CB bleUartCB;

// UART��ʼ��
void BLE_UART_HwInit(uint32 baud)
{
	// ��ʱ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, GPIO_PIN_3);

	// UARTʱ������
	rcu_periph_clock_enable(RCU_USART0);
	usart_deinit(BLE_UART_TYPE_DEF);									// ��λ����
		
	// ��������
	usart_baudrate_set(BLE_UART_TYPE_DEF, baud);						// ������
	usart_word_length_set(BLE_UART_TYPE_DEF, USART_WL_8BIT);			// 8λ����λ
	usart_stop_bit_set(BLE_UART_TYPE_DEF, USART_STB_1BIT); 				// һ��ֹͣλ
	usart_parity_config(BLE_UART_TYPE_DEF, USART_PM_NONE); 				// ����żУ��
	usart_hardware_flow_rts_config(BLE_UART_TYPE_DEF, USART_RTS_DISABLE); // ��Ӳ������������
	usart_hardware_flow_cts_config(BLE_UART_TYPE_DEF, USART_CTS_DISABLE);
	usart_transmit_config(BLE_UART_TYPE_DEF, USART_TRANSMIT_ENABLE);	// ʹ�ܷ���
	usart_receive_config(BLE_UART_TYPE_DEF, USART_RECEIVE_ENABLE); 		// ʹ�ܽ���

	// �ж�����
	nvic_irq_enable(BLE_UART_IRQn_DEF, 2, 1);

	usart_interrupt_enable(BLE_UART_TYPE_DEF, USART_INT_RBNE); 			// �����ж�

	usart_enable(BLE_UART_TYPE_DEF);									// ʹ�ܴ��� 
}

// ���ݽṹ��ʼ��
void BLE_UART_DataStructureInit(BLE_UART_CB* pCB)
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
	for(i = 0; i < BLE_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}	

	bleUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void BLE_UART_TxProcess(BLE_UART_CB* pCB)
{
// �жϷ�ʽ
#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
	uint16 index = pCB->tx.index;							// ��ǰ�������ݵ�������
	uint16 length = pCB->tx.cmdQueue[pCB->tx.head].length;	// ��ǰ���͵�����֡�ĳ���
	uint16 head = pCB->tx.head;								// ��������֡����ͷ������
	uint16 end = pCB->tx.end;								// ��������֡����β������

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
		BLE_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		// ���÷���æ״̬
		pCB->tx.txBusy = TRUE;
	}


// ������ʽʱ��������
#else
	uint16 index = pCB->tx.index;
	uint16 head = pCB->tx.head;
	uint16 end = pCB->tx.end;

	// �����Ϸ��Լ���
	if(NULL == pCB)
	{
		return;
	}

	// ����Ϊ�գ�������
	if(head == end)
	{
		return;
	}
	
	// ��ǰ����֡δ������ʱ����������
	while(index < pCB->tx.cmdQueue[head].length)
	{
		// һֱ��䷢��
		BLE_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void BLE_UART_IR_StartSendData(uint8 data)
{
	// �ȶ�SR�������DR���TC��־���
	usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC);

	// ����һ���ֽ�
	usart_data_transmit(BLE_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	usart_interrupt_enable(BLE_UART_TYPE_DEF, USART_INT_TC);
}

// ��������һ���ֽ�����
void BLE_UART_BC_SendData(uint8 data)
{	
	// ��ֹ��ʧ��һ���ֽ�(����ձ�־λ,��ؼ�) 
	usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC);		

	// �������
	usart_data_transmit(BLE_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_TC) != SET);		
}


/**************************************************************
 * @brief  void USARTx_IRQHandler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleRxBuff[20] = {0};
uint8 bleIndex = 0;
void BLE_UART_BleConnectState(uint32 param)
{	
	bleIndex = 0;

	if (-1 != STRING_Find(bleRxBuff, "TTM:CONNECTED"))
	{
//		PARAM_SetBleConnectState(TRUE);
//		
//		// ������Ȩ��ʱ����10s�������û�м�Ȩ�ɹ����˳���������
//		TIMER_AddTask(TIMER_ID_BLE_CONNECT_VERIFICATE,
//					10000,
//					BLE_MODULE_Reset,
//					0,
//					1,
//					ACTION_MODE_ADD_TO_QUEUE);

	}

//	if (-1 != STRING_Find(bleRxBuff, "TTM:DISCONNECT"))
//	{
//		PARAM_SetBleConnectState(FALSE);
//	}
}

// ��������mac��ַ
void BLE_UART_AtChannelDataProcess(uint8 data)
{
	uint8 i;
	
	if (!bleUartCB.rx.startFlag)
	{
		if (data == 'T')				// ��ʼ��־
		{
			bleUartCB.rx.startFlag = TRUE;

			bleUartCB.rx.index = 0;

			bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;		
			
			return;
		}
	}
	else
	{
		// ������������
		bleUartCB.rx.fifoBuff[bleUartCB.rx.index++] = data;

		// Խ���ж�
		if (bleUartCB.rx.index >= sizeof(bleUartCB.rx.fifoBuff))
		{
			bleUartCB.rx.startFlag = TRUE;
			bleUartCB.rx.index = 0;

			return;
		}
		
		if (data == '\n')			// �յ�������־
		{
			bleUartCB.rx.startFlag = FALSE;
			bleUartCB.rx.index = 0;

			bleUartCB.rx.macBuff[2] = ':';
			bleUartCB.rx.macBuff[5] = ':';
			bleUartCB.rx.macBuff[8] = ':';
			bleUartCB.rx.macBuff[11] = ':';
			bleUartCB.rx.macBuff[14] = ':';
			bleUartCB.rx.macBuff[17] = '\0';

			// ���˻���ָ��
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-?") != NULL))
			{
				return ;
			}

			// ������������mac��ַ��ȡ����
			if ((strstr((const char*)bleUartCB.rx.fifoBuff, (const char*)"TTM:MAC-") != NULL) 
			&& (bleUartCB.rx.fifoBuff[0] == 'T') && (bleUartCB.rx.fifoBuff[1] == 'T') && (bleUartCB.rx.fifoBuff[2] == 'M'))
			{
				bleUartCB.rx.macBuff[0] = bleUartCB.rx.fifoBuff[8];
				bleUartCB.rx.macBuff[1] = bleUartCB.rx.fifoBuff[9];

				bleUartCB.rx.macBuff[3] = bleUartCB.rx.fifoBuff[10];
				bleUartCB.rx.macBuff[4] = bleUartCB.rx.fifoBuff[11];

				bleUartCB.rx.macBuff[6] = bleUartCB.rx.fifoBuff[12];
				bleUartCB.rx.macBuff[7] = bleUartCB.rx.fifoBuff[13];

				bleUartCB.rx.macBuff[9] = bleUartCB.rx.fifoBuff[14];
				bleUartCB.rx.macBuff[10] = bleUartCB.rx.fifoBuff[15];

				bleUartCB.rx.macBuff[12] = bleUartCB.rx.fifoBuff[16];
				bleUartCB.rx.macBuff[13] = bleUartCB.rx.fifoBuff[17];

				bleUartCB.rx.macBuff[15] = bleUartCB.rx.fifoBuff[18];
				bleUartCB.rx.macBuff[16] = bleUartCB.rx.fifoBuff[19];

				//for (i = 0; i < 12; i++)
				//{
				//	bleUartCB.rx.macBuff[i] = bleUartCB.rx.fifoBuff[i+10];
				//}
				//bleUartCB.rx.macBuff[i] = '\0';
				
				for (i = 0; i < sizeof(bleUartCB.rx.fifoBuff); i++)
				{
					bleUartCB.rx.fifoBuff[i] = 0;
				}				

				// ע���ö�ʱ��
				TIMER_KillTask(TIMER_ID_BLE_CONTROL);
				
				// ������ַ�������
				bleUartCB.rcvBleMacOK = TRUE;
			}	
		}
	}
}


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
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_RBNE) != RESET) 
	{
		uint8 rxdata = 0x00;
		
		// ��������
		rxdata = (uint8)usart_data_receive(BLE_UART_TYPE_DEF);
		
		if (NULL != bleUartCB.receiveDataThrowService)
		{			
			(*bleUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}

		if (!bleUartCB.rcvBleMacOK)
		{
			BLE_UART_AtChannelDataProcess(rxdata);
		}

//=================================================================
		// ���������ж�
		if ((bleIndex + 1)< sizeof(bleRxBuff)/sizeof(bleRxBuff[0]))
		{
			bleRxBuff[bleIndex++] = rxdata;
			bleRxBuff[bleIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_CONNECT_STATE,
					100,
					BLE_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}


// �ж�ģʽ����
#if (BLE_UART_TX_MODE == BLE_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_TC) != RESET)
	{
		uint16 head = bleUartCB.tx.head;
		uint16 end;
		uint16 index = bleUartCB.tx.index;
		uint8 txdata = 0x00;

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < bleUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];
			
			// �������
			usart_data_transmit(BLE_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			bleUartCB.tx.cmdQueue[head].length = 0;
			bleUartCB.tx.head ++;
			bleUartCB.tx.head %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
			bleUartCB.tx.index = 0;

			head = bleUartCB.tx.head;
			end = bleUartCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = bleUartCB.tx.cmdQueue[head].buff[bleUartCB.tx.index++];

				// �������
				usart_data_transmit(BLE_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				usart_interrupt_disable(BLE_UART_TYPE_DEF, USART_INT_TC);
				
				bleUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif

	// Other USARTx interrupts handler can go here ...				 
	if (usart_interrupt_flag_get(BLE_UART_TYPE_DEF, USART_INT_FLAG_ERR_ORERR) != RESET)	//----------------------- ��������ж� 
	{
		usart_flag_get(BLE_UART_TYPE_DEF, USART_FLAG_ORERR); 			//----------------------- �����������жϱ�־λ 
		usart_data_receive(BLE_UART_TYPE_DEF);									//----------------------- ��ռĴ���
	}
}

// BLEģ�鸨����������
void BLE_CMD_GPIO_CtrlPin(void)
{
//	// ��ʱ��
//	rcu_periph_clock_enable(RCU_GPIOA);
//	rcu_periph_clock_enable(RCU_GPIOC);
//	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
//	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);

//	// �ͷŸ�λ
//	BLE_RST_RELEASE();
//	// ����ʼ�㲥
//	BLE_EN_ENABLE();

//	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_12);

//	// �ϵ��һֱ���ڷ���׼��״̬
//	gpio_bit_reset(GPIOC, GPIO_PIN_12);
}

// UART��ʼ��
void BLE_UART_Init(void)
{
	// BLEģ�鸨���������ų�ʼ��
//	BLE_CMD_GPIO_CtrlPin();
	
	// �ɰ�׿�˵����������ʱ��Ӧ�ò���תboot��Ҫ���ε�����ģ���ʼ������ֹ�����Ͽ�����
	if(0xAA == SPI_FLASH_ReadByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS + 1))
	{
		// �����������ʱ����תboot���γ�ʼ����־λ������֮����Ҫ���³�ʼ����������ֹ����ʧ��
		SPI_FLASH_WriteByte(SPI_FLASH_BLE_UPDATA_FLAG_ADDEESS + 1, 0xFF);
	}
	else
	{
		// BLEģ�鸨���������ų�ʼ��
		BLE_CMD_GPIO_CtrlPin();
		
		// ���ڳ�ʼ��
		BLE_UART_HwInit(BLE_UART_BAUD_RATE);
	}

	// UART���ݽṹ��ʼ��
	BLE_UART_DataStructureInit(&bleUartCB);
}

// UARTģ�鴦�����
void BLE_UART_Process(void)
{
	// ���ʹ���
	BLE_UART_TxProcess(&bleUartCB);
}

// ע�������׳��ӿڷ���
void BLE_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL BLE_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleUartCB.tx.head;
	uint16 end = bleUartCB.tx.end;
	
	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if((end + 1) % BLE_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleUartCB.tx.cmdQueue[end].length = length;
	
	// ���ͻ��ζ��и���λ��
	bleUartCB.tx.end ++;
	bleUartCB.tx.end %= BLE_UART_DRIVE_TX_QUEUE_SIZE;
	bleUartCB.tx.cmdQueue[bleUartCB.tx.end].length = 0;

	return TRUE;
}

// ���ͻ�ȡMAC��ַ
void BLE_UART_SendCmdGetBleMacAddr(uint32 param)
{
	uint8 i;
	uint8 bleMacStr[] = "TTM:MAC-?";
	
	// ʹ��BLE����
	BLE_BRTS_TX_REQUEST();

	// ��ѯ����macָ��,��������ʽ����
	for (i = 0; bleMacStr[i] != '\0'; i++)
	{
		// ������ݣ���������
		BLE_UART_BC_SendData(bleMacStr[i]);
	}	
}

// ��ʱ���ص����ͻ�ȡ������ַ����
void BLE_UART_CALLBALL_GetBleMacAddrRequest(uint32 param)
{
	// ���ͻ�ȡMAC��ַ
	BLE_UART_SendCmdGetBleMacAddr(TRUE);
}

// BLEģ�鸴λ
void BLE_MODULE_Reset(uint32 param)
{
	// ������λ
	BLE_RST_RESET();
	Delayms(10);
	
	// �ͷŸ�λ
	BLE_RST_RELEASE();

	// ����ʼ�㲥
	BLE_EN_ENABLE();
}


