#include "common.h"
#include "timer.h"
#include "BleHostUart.h"
//#include "stringOperation.h"
#include "Param.h"
#include "BleHostProtocol.h"

/******************************************************************************
* ���ڲ��ӿ�������
******************************************************************************/
// UART��ʼ��
void BLE_HOST_UART_HwInit(uint32 baud);

// ���ݽṹ��ʼ��
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB);

// ���ʹ���
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB);

// �����ж��ֽڷ���
void BLE_HOST_UART_IR_StartSendData(uint8 data);

//  ��������״̬�жϺ���
void BLE_HOST_UART_BleConnectState(uint32 param);

// ��������һ���ֽ�����
//void BLE_HOST_UART_BC_SendData(uint8 data);

// UART�����뷢���жϴ��������ڲ�ͨѶ
void UART2_Handler(void);

// ȫ�ֱ�������
BLE_HOST_UART_CB bleHostUartCB;

// UART��ʼ��
void BLE_HOST_UART_HwInit(uint32 baud)
{
		GPIO_InitType GPIO_InitStructure;
		USART_InitType USART_InitStructure;
		NVIC_InitType NVIC_InitStructure;
	
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_AFIO, ENABLE);    
		RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART3, ENABLE);                       //                                      
	

    // Initialize GPIO_InitStructure
    GPIO_InitStruct(&GPIO_InitStructure);
    // USART Tx 
    GPIO_InitStructure.Pin            = GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_USART3;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // USART Rx 
    GPIO_InitStructure.Pin            = GPIO_PIN_11;
    GPIO_InitStructure.GPIO_Current   = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_USART3;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // USART configuration
    USART_InitStructure.BaudRate            = baud;
    USART_InitStructure.WordLength          = USART_WL_8B;
    USART_InitStructure.StopBits            = USART_STPB_1;
    USART_InitStructure.Parity              = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;
    USART_Init(BLE_HOST_UART_TYPE_DEF, &USART_InitStructure);

    // Enable the USART Interrupt
    NVIC_InitStructure.NVIC_IRQChannel            = BLE_HOST_UART_IRQn_DEF;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable USART Receive and Transmit interrupts   
    USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_RXDNE, ENABLE);    
		
    // USART_ConfigInt(UART_DRIVE_TYPE_DEF, USART_INT_TXC, ENABLE);    
    // Enable the USART
    USART_Enable(BLE_HOST_UART_TYPE_DEF, ENABLE);
		
}	

// ���ݽṹ��ʼ��
void BLE_HOST_UART_DataStructureInit(BLE_HOST_UART_CB* pCB)
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
	for(i = 0; i < BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE; i++)
	{
		pCB->tx.cmdQueue[i].length = 0;
	}	

	bleHostUartCB.rcvBleMacOK = FALSE;
}

// ���ʹ���
void BLE_HOST_UART_TxProcess(BLE_HOST_UART_CB* pCB)
{
	// �жϷ�ʽ
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
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
		BLE_HOST_UART_IR_StartSendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
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
		BLE_HOST_UART_BC_SendData(pCB->tx.cmdQueue[head].buff[pCB->tx.index++]);
		
		index = pCB->tx.index;
	}
	
	// ��ǰ����֡������ʱ��ɾ��֮
	pCB->tx.cmdQueue[head].length = 0;
	pCB->tx.head ++;
	pCB->tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	pCB->tx.index = 0;
#endif
}

// �����ж��ֽڷ���
void BLE_HOST_UART_IR_StartSendData(uint8 data)
{
	// ����һ���ֽ�
	USART_SendData(BLE_HOST_UART_TYPE_DEF, data);

	// �򿪷�������ж�
	USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC, ENABLE);
}


// ��������һ���ֽ�����
void BLE_HOST_UART_BC_SendData(uint8 data)
{	
	// �������
	USART_SendData(BLE_HOST_UART_TYPE_DEF, data);

	// δ�����꣬�����ȴ�
	while(USART_GetFlagStatus(BLE_HOST_UART_TYPE_DEF, USART_FLAG_TXC) == RESET);	
}

/**************************************************************
 * @brief  void BLE_HOST_UART_BleConnectState(void);
 * @input  ��������״̬�жϺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
uint8 bleHostRxBuff[200] = {0};
uint8 bleHostIndex = 0;
void BLE_HOST_UART_BleConnectState(uint32 param)
{	
	// ����TTM����
	BLE_HOST_UART_TTM_AnalysisCmd(bleHostRxBuff, bleHostIndex);

	bleHostIndex = 0;
}

/**************************************************************
 * @brief  void UART2_Handler(void);
 * @input  ����x���պ���������x���жϷ��ͺ���
 * @output None
 * @return None	
 * @Notes					
*************************************************************/
void USART3_IRQHandler(void)
{
	// �ж�DR�Ƿ������ݣ��жϽ���
	if (USART_GetIntStatus(BLE_HOST_UART_TYPE_DEF, USART_INT_RXDNE) != RESET)
	{
		uint8 rxdata = 0x00;
		uint32_t chr;
		
		// ��������
		rxdata = (uint8)USART_ReceiveData(BLE_HOST_UART_TYPE_DEF);

		if (NULL != bleHostUartCB.receiveDataThrowService)
		{			
			(*bleHostUartCB.receiveDataThrowService)(0xFFF, &rxdata, 1);
		}
		
//=================================================================
		// ���������ж�
		if ((bleHostIndex + 1)< sizeof(bleHostRxBuff)/sizeof(bleHostRxBuff[0]))
		{
			bleHostRxBuff[bleHostIndex++] = rxdata;
			bleHostRxBuff[bleHostIndex] = '\0';
		}

		TIMER_AddTask(TIMER_ID_BLE_HOST_CONNECT_STATE,
					10,
					BLE_HOST_UART_BleConnectState,
					0,
					1,
					ACTION_MODE_ADD_TO_QUEUE);
	}


// �ж�ģʽ����
#if (BLE_HOST_UART_TX_MODE == BLE_HOST_UART_INTERRUPT_TX_MODE)
	// �ж�DR�Ƿ������ݣ��жϷ���
	if (USART_GetIntStatus(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC) != RESET)
	{
		uint16 head = bleHostUartCB.tx.head;
		uint16 end;
		uint16 index = bleHostUartCB.tx.index;
		uint8 txdata = 0x00;

		// ��������жϱ�־
		USART_ClrIntPendingBit(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC);

		// ִ�е����˵����һ�������Ѿ�������ϣ���ǰ����֡δ������ʱ��ȡ��һ���ֽڷŵ����ͼĴ�����
		if (index < bleHostUartCB.tx.cmdQueue[head].length)
		{
			txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];
			
			// �������
			USART_SendData(BLE_HOST_UART_TYPE_DEF, txdata);
		}
		// ��ǰ����֡������ʱ��ɾ��֮
		else
		{
			bleHostUartCB.tx.cmdQueue[head].length = 0;
			bleHostUartCB.tx.head ++;
			bleHostUartCB.tx.head %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
			bleHostUartCB.tx.index = 0;

			head = bleHostUartCB.tx.head;
			end = bleHostUartCB.tx.end;
			
			// ����֡���зǿգ�����������һ������֡
			if (head != end)
			{
				txdata = bleHostUartCB.tx.cmdQueue[head].buff[bleHostUartCB.tx.index++];

				// �������
				USART_SendData(BLE_HOST_UART_TYPE_DEF, txdata);
			}
			// ����֡����Ϊ��ֹͣ���ͣ����ÿ���
			else
			{
				// �رշ�������ж�
				USART_ConfigInt(BLE_HOST_UART_TYPE_DEF, USART_INT_TXC, DISABLE);
				
				bleHostUartCB.tx.txBusy = FALSE;				
			}
		}		
	}
#endif
	// Other USARTx interrupts handler can go here ...				 
	if(USART_GetFlagStatus(BLE_HOST_UART_TYPE_DEF, USART_FLAG_OREF) != RESET) //----------------------- ��������ж� 	
	{
		USART_ClrFlag(BLE_HOST_UART_TYPE_DEF,USART_FLAG_OREF); 		  //----------------------- �����������жϱ�־λ 
		USART_ReceiveData(BLE_HOST_UART_TYPE_DEF);						  //----------------------- ��ռĴ���
	}
}


// UART��ʼ��
void BLE_HOST_UART_Init(void)
{
	// ���ڳ�ʼ��
	BLE_HOST_UART_HwInit(BLE_HOST_UART_BAUD_RATE);

	// UART���ݽṹ��ʼ��
	BLE_HOST_UART_DataStructureInit(&bleHostUartCB);
//	
//				TIMER_AddTask(TIMER_ID_BLE_HELMET_PAIR,
//							BLE_OUTFIT_CONNECT_TIME,
//							BLE_HOST_UART_CALLBALL_OutfitConnect,
//							TRUE,
//							BLE_OUTFIT_PAIR_COUNT,
//							ACTION_MODE_DO_AT_ONCE);
}

// UARTģ�鴦�����
void BLE_HOST_UART_Process(void)
{
	// ���ʹ���
	BLE_HOST_UART_TxProcess(&bleHostUartCB);
}

// ע�������׳��ӿڷ���
void BLE_HOST_UART_RegisterDataSendService(void (*service)(uint16 id, uint8 *pData, uint16 length))
{
	bleHostUartCB.receiveDataThrowService = service;
}

// ���ͻ����������һ������������
BOOL BLE_HOST_UART_AddTxArray(uint16 id, uint8 *pArray, uint16 length)
{
	uint16 i;
	uint16 head = bleHostUartCB.tx.head;
	uint16 end = bleHostUartCB.tx.end;
	
	// ��������
	if ((NULL == pArray) || (0 == length))
	{
		return FALSE;
	}

	// ���ͻ������������������
	if((end + 1) % BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE == head)
	{
		return FALSE;
	}

	bleHostUartCB.tx.cmdQueue[end].deviceID = id;
	for (i = 0; i < length; i++)
	{
		bleHostUartCB.tx.cmdQueue[end].buff[i] = *pArray++;
	}
	bleHostUartCB.tx.cmdQueue[end].length = length;
	
	// ���ͻ��ζ��и���λ��
	bleHostUartCB.tx.end ++;
	bleHostUartCB.tx.end %= BLE_HOST_UART_DRIVE_TX_QUEUE_SIZE;
	bleHostUartCB.tx.cmdQueue[bleHostUartCB.tx.end].length = 0;

	return TRUE;
}
